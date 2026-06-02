#include "nav_pid_ctrl.h"

// ============ 外部符号声明 ============
// 原则：nav_pid_ctrl.cpp 是独立编译单元，不能包含 ugv_config.h / movtion_module.h，
// 否则会导致全局变量在多个编译单元中被重复定义（multiple definition）。
// 改为只声明本文件实际使用的外部符号。

// --- 来自 movtion_module.h ---
extern double speedGetA;              // 左轮线速度 (m/s)
extern double speedGetB;              // 右轮线速度 (m/s)
extern bool   usePIDCompute;          // 电机速度PID开关
extern bool   heartbeatStopFlag;      // 心跳停车标志
void leftCtrl(float pwmInputA);       // 左轮PWM直接控制
void rightCtrl(float pwmInputB);      // 右轮PWM直接控制
void setGoalSpeed(float L, float R);  // 设置目标速度

// --- 来自 ugv_config.h ---
extern double TRACK_WIDTH;            // 轮距 (m)
extern byte   InfoPrint;              // 调试打印级别
extern unsigned long lastCmdRecvTime; // 心跳计时器

// --- 来自 IMU_ctrl.h (定义在 ugv_config.h) ---
extern double icm_yaw;                // IMU 偏航角（度）

// ==================== 全局变量定义 ====================

// ----- 当前位置 -----
float nav_current_x = 0.0;
float nav_current_y = 0.0;
float nav_current_heading = 0.0;

// ----- 目标位置 -----
float nav_target_x = 0.0;
float nav_target_y = 0.0;

// ----- 状态标志 -----
bool nav_target_active = false;
bool nav_goal_reached = false;
bool nav_use_auto_odometry = true;  // 默认开启自动里程计

// ----- 里程计内部状态 -----
float nav_odom_x = 0.0;
float nav_odom_y = 0.0;
float nav_odom_heading = 0.0;
unsigned long nav_odom_last_update = 0;

// ----- 距离PID参数 -----
float nav_kp_dist = 150.0;
float nav_ki_dist = 30.0;
float nav_kd_dist = 0.0;

// ----- 航向PID参数 -----
float nav_kp_head = 80.0;
float nav_ki_head = 10.0;
float nav_kd_head = 0.0;

// ----- 到达阈值 -----
float nav_arrival_dist = 0.05;  // 5cm

// ----- 速度与输出限制 -----
float nav_base_speed = 0.3;     // 最大线速度 m/s
float nav_max_pwm = 255.0;
float nav_min_pwm = 30.0;

// ----- PWM与速度换算系数 -----
float nav_pwm_per_ms = 510.0;   // 255 / 0.5 = 510

// ----- 调试输出 -----
float nav_output_left = 0.0;
float nav_output_right = 0.0;
float nav_distance_to_target_last = 0.0;

// ----- PID内部状态变量 -----
static float nav_dist_integral = 0.0;
static float nav_dist_prev_error = 0.0;
static float nav_head_integral = 0.0;
static float nav_head_prev_error = 0.0;

// ----- 定时控制 -----
static unsigned long nav_last_update = 0;
static const unsigned long nav_update_interval = 100;  // 100ms

// ----- 导航控制标志：当导航激活时，阻止普通速度命令生效 -----
// 这个变量定义在movtion_module.h中，这里通过extern引用
// 实际在nav_pid_compute中直接调用leftCtrl/rightCtrl绕过setGoalSpeed

// ==================== 函数实现 ====================

float nav_wrapAngle(float angle) {
    while (angle > M_PI) angle -= 2.0 * M_PI;
    while (angle < -M_PI) angle += 2.0 * M_PI;
    return angle;
}

void nav_set_target(float x, float y) {
    nav_target_x = x;
    nav_target_y = y;
    nav_target_active = true;
    nav_goal_reached = false;

    // 如果开启了自动里程计，用当前里程计位置作为起点
    // （允许手动nav_update_position覆盖）
    if (nav_use_auto_odometry) {
        // 保持当前里程计累积值不变，nav_pid_compute中使用它
    }

    // 重置PID积分和上次误差，避免旧状态干扰新目标
    nav_dist_integral = 0.0;
    nav_dist_prev_error = 0.0;
    nav_head_integral = 0.0;
    nav_head_prev_error = 0.0;

    // 重置定时器，确保立即开始计算
    nav_last_update = 0;

    // 停止心跳检测中的电机停止，避免导航被心跳切断
    heartbeatStopFlag = false;

    if (InfoPrint == 1) {
        Serial.print("[NAV] 目标坐标已设置: (");
        Serial.print(x); Serial.print(", "); Serial.print(y);
        Serial.print(") 当前: ("); Serial.print(nav_current_x);
        Serial.print(", "); Serial.print(nav_current_y);
        Serial.println(")");
    }
}

void nav_update_position(float x, float y, float heading) {
    nav_current_x = x;
    nav_current_y = y;
    nav_current_heading = nav_wrapAngle(heading);

    // 同步里程计内部状态，避免手动更新后跳变
    nav_odom_x = x;
    nav_odom_y = y;
    nav_odom_heading = nav_wrapAngle(heading);
}

void nav_odometry_update() {
    // 如果未启用自动里程计，直接返回
    if (!nav_use_auto_odometry) {
        return;
    }

    unsigned long now = micros();  // 使用微秒与编码器速度保持一致

    if (nav_odom_last_update == 0) {
        nav_odom_last_update = now;
        return;
    }

    // 时间间隔（秒）
    float dt = (now - nav_odom_last_update) / 1000000.0;

    // dt 过小或过大都不合理，做保护
    if (dt < 0.001 || dt > 1.0) {
        nav_odom_last_update = now;
        return;
    }

    nav_odom_last_update = now;

    // ----- 从全局速度变量读取左右轮线速度 (m/s) -----
    // speedGetA = 左轮线速度, speedGetB = 右轮线速度
    // 这些值由 getLeftSpeed() / getRightSpeed() 在loop中更新
    float left_speed  = speedGetA;   // m/s
    float right_speed = speedGetB;   // m/s

    // 车体线速度 = 左右轮速度的平均值
    float linear_velocity = (left_speed + right_speed) / 2.0;

    // 车体角速度 = (右轮-左轮) / 轮距
    // TRACK_WIDTH 定义在 ugv_config.h 中
    float angular_velocity = (right_speed - left_speed) / TRACK_WIDTH;

    // ----- 使用IMU yaw作为航向参考 -----
    // icm_yaw 来自 IMU_ctrl.h, 单位: 度（已在文件顶部 extern 声明）
    // 优先使用IMU航向（不受轮子打滑影响）
    float heading_from_imu = nav_wrapAngle(icm_yaw * M_PI / 180.0);  // 度转弧度

    // 使用IMU航向（更稳定，不受打滑影响）
    nav_odom_heading = heading_from_imu;

    // ----- 将车体速度投影到世界坐标系 -----
    // 使用当前航向角分解线速度
    float vx = linear_velocity * cos(nav_odom_heading);
    float vy = linear_velocity * sin(nav_odom_heading);

    // 积分位置
    nav_odom_x += vx * dt;
    nav_odom_y += vy * dt;

    // 更新全局当前位置（供导航PID和外部读取）
    nav_current_x = nav_odom_x;
    nav_current_y = nav_odom_y;
    nav_current_heading = nav_odom_heading;
}

void nav_odometry_reset() {
    nav_odom_x = 0.0;
    nav_odom_y = 0.0;
    nav_odom_heading = 0.0;
    nav_odom_last_update = 0;

    nav_current_x = 0.0;
    nav_current_y = 0.0;
    nav_current_heading = 0.0;

    if (InfoPrint == 1) {
        Serial.println("[NAV] 里程计已重置为零点");
    }
}

void nav_pid_compute() {
    // 未激活或已到达，直接返回（不输出任何电机控制）
    if (!nav_target_active || nav_goal_reached) {
        return;
    }

    // 节流控制：每100ms执行一次
    unsigned long now = millis();
    if (now - nav_last_update < nav_update_interval) {
        return;
    }
    nav_last_update = now;

    float dt = nav_update_interval / 1000.0;  // 控制周期转换为秒

    // ====== 第一步：计算距离误差 ======
    float dx = nav_target_x - nav_current_x;
    float dy = nav_target_y - nav_current_y;
    float distance_to_target = sqrt(dx * dx + dy * dy);
    nav_distance_to_target_last = distance_to_target;

    // 判断是否到达目标
    if (distance_to_target < nav_arrival_dist) {
        nav_goal_reached = true;
        nav_target_active = false;
        setGoalSpeed(0.0, 0.0);
        leftCtrl(0);
        rightCtrl(0);
        if (InfoPrint == 1) {
            Serial.print("[NAV] 已到达目标！最终位置: (");
            Serial.print(nav_current_x); Serial.print(", ");
            Serial.print(nav_current_y); Serial.println(")");
        }
        return;
    }

    // ====== 第二步：计算航向误差 ======
    // 从当前位置指向目标的方向角(世界坐标系)
    float bearing_to_target = atan2(dy, dx);
    // 航向误差 = 目标方向 - 当前车头朝向，归一化到[-PI, PI]
    float heading_error = nav_wrapAngle(bearing_to_target - nav_current_heading);

    // ====== 第三步：距离PID → 线速度 ======
    float dist_error = distance_to_target;
    nav_dist_integral += dist_error * dt;

    // 积分限幅，防止积分饱和(windup)
    if (nav_dist_integral > 2.0) nav_dist_integral = 2.0;
    if (nav_dist_integral < -2.0) nav_dist_integral = -2.0;

    float dist_derivative = (dist_error - nav_dist_prev_error) / dt;
    nav_dist_prev_error = dist_error;

    float linear_speed = nav_kp_dist * dist_error
                       + nav_ki_dist * nav_dist_integral
                       + nav_kd_dist * dist_derivative;

    // 线速度限幅
    if (linear_speed > nav_base_speed) linear_speed = nav_base_speed;
    if (linear_speed < -nav_base_speed) linear_speed = -nav_base_speed;

    // 接近目标时减速：距离0.5m内开始线性降速，最低降至20%
    float slow_factor = constrain(distance_to_target / 0.5, 0.2, 1.0);
    linear_speed *= slow_factor;

    // ====== 第四步：航向PID → 角速度 ======
    nav_head_integral += heading_error * dt;

    // 积分限幅
    if (nav_head_integral > 1.0) nav_head_integral = 1.0;
    if (nav_head_integral < -1.0) nav_head_integral = -1.0;

    float head_derivative = (heading_error - nav_head_prev_error) / dt;
    nav_head_prev_error = heading_error;

    float angular_speed = nav_kp_head * heading_error
                        + nav_ki_head * nav_head_integral
                        + nav_kd_head * head_derivative;

    // 角速度限幅
    float max_angular = 1.0;  // rad/s
    if (angular_speed > max_angular) angular_speed = max_angular;
    if (angular_speed < -max_angular) angular_speed = -max_angular;

    // ====== 第五步：差速运动学 → 左右轮速度 ======
    // 左轮线速度 = 线速度 - 角速度 × 轮距/2
    // 右轮线速度 = 线速度 + 角速度 × 轮距/2
    float left_speed = linear_speed - angular_speed * (TRACK_WIDTH / 2.0);
    float right_speed = linear_speed + angular_speed * (TRACK_WIDTH / 2.0);

    // ====== 第六步：速度换PWM → 电机输出 ======
    float left_pwm = left_speed * nav_pwm_per_ms;
    float right_pwm = right_speed * nav_pwm_per_ms;

    // PWM限幅
    left_pwm = constrain(left_pwm, -nav_max_pwm, nav_max_pwm);
    right_pwm = constrain(right_pwm, -nav_max_pwm, nav_max_pwm);

    // PWM死区处理：左右都很小时停止输出，避免电机嗡嗡响
    if (abs(left_pwm) < nav_min_pwm && abs(right_pwm) < nav_min_pwm) {
        left_pwm = 0.0;
        right_pwm = 0.0;
    }

    nav_output_left = left_pwm;
    nav_output_right = right_pwm;

    // ====== 第七步：输出到电机 ======
    // 导航激活时，直接控制电机，绕过心跳和速度命令
    // 根据车型选择控制方式
    if (!usePIDCompute) {
        // WAVE ROVER (mainType=1) / UGV02 (mainType=2): 直接PWM控制
        leftCtrl(left_pwm);
        rightCtrl(right_pwm);
    } else {
        // UGV01 (mainType=3): 有电机速度PID，设置速度目标值
        float max_speed_ms = nav_base_speed;
        float norm_left = constrain(left_speed / max_speed_ms, -1.0, 1.0);
        float norm_right = constrain(right_speed / max_speed_ms, -1.0, 1.0);
        setGoalSpeed(norm_left, norm_right);
    }

    // 重置心跳超时计时器，防止导航过程中被心跳超时强制停车
    lastCmdRecvTime = now;

    // 调试输出
    if (InfoPrint == 1) {
        static unsigned long last_debug_print = 0;
        if (now - last_debug_print >= 500) {  // 每500ms打印一次
            last_debug_print = now;
            Serial.print("[NAV] 当前:("); Serial.print(nav_current_x);
            Serial.print(","); Serial.print(nav_current_y);
            Serial.print(") 朝向:"); Serial.print(nav_current_heading);
            Serial.print(" 目标:("); Serial.print(nav_target_x);
            Serial.print(","); Serial.print(nav_target_y);
            Serial.print(") 距离:"); Serial.print(distance_to_target);
            Serial.print(" 航向差:"); Serial.print(heading_error);
            Serial.print(" 线速:"); Serial.print(linear_speed);
            Serial.print(" 角速:"); Serial.print(angular_speed);
            Serial.print(" L:"); Serial.print(left_pwm);
            Serial.print(" R:"); Serial.println(right_pwm);
        }
    }
}

void nav_stop() {
    nav_target_active = false;
    nav_goal_reached = false;
    nav_dist_integral = 0.0;
    nav_head_integral = 0.0;
    nav_dist_prev_error = 0.0;
    nav_head_prev_error = 0.0;
    setGoalSpeed(0.0, 0.0);
    leftCtrl(0);
    rightCtrl(0);

    if (InfoPrint == 1) {
        Serial.println("[NAV] 导航已停止");
    }
}

void nav_set_pid_params(float kp_dist, float ki_dist, float kd_dist,
                        float kp_head, float ki_head, float kd_head) {
    nav_kp_dist = kp_dist;
    nav_ki_dist = ki_dist;
    nav_kd_dist = kd_dist;
    nav_kp_head = kp_head;
    nav_ki_head = ki_head;
    nav_kd_head = kd_head;

    // 重置积分项，避免突变
    nav_dist_integral = 0.0;
    nav_head_integral = 0.0;

    if (InfoPrint == 1) {
        Serial.println("[NAV] PID参数已更新");
    }
}

void nav_set_max_speed(float max_speed) {
    if (max_speed > 0 && max_speed <= 1.0) {
        nav_base_speed = max_speed;
        if (InfoPrint == 1) {
            Serial.print("[NAV] 最大速度设置为: ");
            Serial.println(nav_base_speed);
        }
    }
}

bool nav_is_active() {
    return nav_target_active && !nav_goal_reached;
}

bool nav_is_reached() {
    return nav_goal_reached;
}

float nav_get_distance_to_target() {
    float dx = nav_target_x - nav_current_x;
    float dy = nav_target_y - nav_current_y;
    return sqrt(dx * dx + dy * dy);
}
