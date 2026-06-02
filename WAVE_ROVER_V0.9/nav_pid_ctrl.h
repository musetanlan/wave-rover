#ifndef NAV_PID_CTRL_H
#define NAV_PID_CTRL_H

#include <Arduino.h>
#include <math.h>

// ============================================================
// 导航PID控制器 - 驱动车辆从当前坐标向目标坐标行驶
// 使用两个PID控制器：
// 1. 距离PID → 线速度 (m/s)
// 2. 航向PID → 角速度 (rad/s)
// 通过差速运动学模型转换为左右轮PWM控制
//
// 位置估计来源：
//  - 轮式里程计：编码器速度 → 积分得到位移
//  - IMU航向：IMU偏航角作为车头朝向
// ============================================================

// ----- 当前位置（由轮式里程计+IMU自动更新）-----
extern float nav_current_x;        // 当前X坐标 (m)
extern float nav_current_y;        // 当前Y坐标 (m)
extern float nav_current_heading;  // 当前车头朝向 (rad), 0 = +X轴方向

// ----- 目标位置（由web/串口JSON指令设置）-----
extern float nav_target_x;         // 目标X坐标 (m)
extern float nav_target_y;         // 目标Y坐标 (m)

// ----- 状态标志 -----
extern bool nav_target_active;     // 导航是否激活
extern bool nav_goal_reached;      // 是否已到达目标
extern bool nav_use_auto_odometry; // 是否使用自动里程计（默认true）

// ----- 里程计内部状态 -----
extern float nav_odom_x;           // 里程计X累计 (m)
extern float nav_odom_y;           // 里程计Y累计 (m)
extern float nav_odom_heading;     // 里程计航向累计 (rad)
extern unsigned long nav_odom_last_update; // 上次里程计更新时间

// ----- 距离PID参数 -----
// 输入: 距离误差(m), 输出: 线速度(m/s)
extern float nav_kp_dist;          // 距离比例系数
extern float nav_ki_dist;          // 距离积分系数
extern float nav_kd_dist;          // 距离微分系数

// ----- 航向PID参数 -----
// 输入: 航向误差(rad), 输出: 角速度(rad/s)
extern float nav_kp_head;          // 航向比例系数
extern float nav_ki_head;          // 航向积分系数
extern float nav_kd_head;          // 航向微分系数

// ----- 到达阈值 -----
extern float nav_arrival_dist;     // 距离目标小于此值视为到达 (m)

// ----- 速度与输出限制 -----
extern float nav_base_speed;       // 最大线速度 (m/s)
extern float nav_max_pwm;          // PWM最大输出值
extern float nav_min_pwm;          // PWM死区，小于此值不输出

// ----- PWM与速度换算系数 -----
// WAVE ROVER实测: 满PWM(255) ≈ 0.5 m/s → 255/0.5 ≈ 510
extern float nav_pwm_per_ms;

// ----- 调试输出 -----
extern float nav_output_left;      // 最终左轮PWM
extern float nav_output_right;     // 最终右轮PWM
extern float nav_distance_to_target_last; // 距目标距离（用于反馈）

// ==================== 函数声明 ====================

// 角度归一化到 [-PI, PI] 区间
float nav_wrapAngle(float angle);

// 设置目标坐标（由web JSON指令调用）
// 参数: x - 目标X坐标(m), y - 目标Y坐标(m)
void nav_set_target(float x, float y);

// 手动更新当前位姿（由外部源如串口/外部定位系统调用）
// 调用此函数后，自动里程计会以此为基准继续累积
// 参数: x - 当前X坐标(m), y - 当前Y坐标(m), heading - 车头朝向(rad)
void nav_update_position(float x, float y, float heading);

// 轮式里程计自动更新（由loop()每轮调用，使用编码器速度和IMU航向）
// 内部自动积分速度和航向，更新nav_current_x/y/heading
void nav_odometry_update();

// 重置里程计原点（将当前位置清零）
void nav_odometry_reset();

// PID核心计算函数
// 由loop()每轮调用（约10ms一次），内部节流到100ms执行一次
void nav_pid_compute();

// 停止导航，关闭所有电机输出
void nav_stop();

// 设置PID参数（动态调整）
void nav_set_pid_params(float kp_dist, float ki_dist, float kd_dist,
                        float kp_head, float ki_head, float kd_head);

// 设置最大速度
void nav_set_max_speed(float max_speed);

// 获取当前导航状态
bool nav_is_active();
bool nav_is_reached();
float nav_get_distance_to_target();

#endif // NAV_PID_CTRL_H
