/**
 * ============================================================
 * linktrack_ctrl.h — LinkTrack UWB 定位数据接收与 NLink 协议解析
 * ============================================================
 *
 * 【功能概述】
 *   通过 ESP32 Serial (GPIO3/1) 接收 LinkTrack TAG 的定位数据，
 *   解析 NLink TAG_Frame0 协议帧（固定128字节），提取实时坐标、
 *   速度、欧拉角、精度估计等信息，并自动注入导航 PID 系统。
 *
 * 【依赖】
 *   - ESP32 Arduino 框架 (HardwareSerial Serial)
 *   - nav_pid_ctrl.h: nav_update_position() 函数
 *
 * 【数据流】
 *   LinkTrack TAG --UART(921600bps)--> ESP32 Serial
 *       --> 状态机逐字节解析 NLink 帧
 *       --> 校验通过后更新全局变量
 *       --> 自动调用 nav_update_position() 覆盖里程计
 *
 * 【坐标系说明】
 *   LinkTrack 输出坐标系与基站标定坐标系一致。
 *   需确保 LinkTrack 坐标系 X/Y 轴方向与车辆坐标系对齐，
 *   否则需要在外部做坐标变换或修改 lt_pos_x/y 的赋值逻辑。
 * ============================================================
 */

#ifndef LINKTRACK_CTRL_H
#define LINKTRACK_CTRL_H

#include <Arduino.h>
#include "nav_pid_ctrl.h"

// ==================== UART 配置 ====================
// Serial (UART0) 默认引脚: RX=GPIO3, TX=GPIO1
// ESP32 GPIO3  ← LinkTrack TX
// ESP32 GPIO1  → LinkTrack RX
// LinkTrack 默认波特率（需与 NAssistant 中配置一致）
#define LINKTRACK_BAUDRATE  921600

// ==================== 协议常量 ====================
// TAG_Frame0 帧长度（固定128字节，非变长协议）
#define LINKTRACK_FRAME_LEN  128

// 帧头标识
#define LINKTRACK_HEADER     0x55
#define LINKTRACK_FUNC_TAG   0x01   // TAG_Frame0 功能码

// 坐标与速度的倍率（来自 NLink 协议规范）
#define LINKTRACK_POS_SCALE  1000.0f   // 位置 ×1000 → 除以1000得到米
#define LINKTRACK_VEL_SCALE  10000.0f  // 速度 ×10000 → 除以10000得到米/秒
#define LINKTRACK_ANG_SCALE  100.0f    // 欧拉角 ×100 → 除以100得到度
#define LINKTRACK_EOP_SCALE  100.0f    // 精度估计 ×100
#define LINKTRACK_VOL_SCALE  1000.0f   // 电压 ×1000

// 无效 ID（LinkTrack 协议规定 0xFF 为无效节点）
#define LINKTRACK_INVALID_ID 0xFF

// ==================== 全局变量：LinkTrack 实时数据 ====================

// --- 位置 (m) ---
float lt_pos_x = 0.0f;
float lt_pos_y = 0.0f;
float lt_pos_z = 0.0f;

// --- 速度 (m/s) ---
float lt_vel_x = 0.0f;
float lt_vel_y = 0.0f;
float lt_vel_z = 0.0f;

// --- 欧拉角 (度) ---
// angle.z = 偏航角(Yaw), angle.y = 俯仰角(Pitch), angle.x = 翻滚角(Roll)
float lt_yaw   = 0.0f;
float lt_pitch = 0.0f;
float lt_roll  = 0.0f;

// --- 精度估计 EOP (m)，值越小精度越高，2.55 表示无法估计 ---
float lt_eop_x = 0.0f;
float lt_eop_y = 0.0f;
float lt_eop_z = 0.0f;

// --- 标签供电电压 (V) ---
float lt_voltage = 0.0f;

// --- 状态标志 ---
bool lt_data_updated    = false;   // 本循环是否有新数据到来
unsigned long lt_last_recv_time = 0;  // 最后收到有效帧的时间 (ms)
unsigned long lt_frame_count   = 0;   // 累计收到的有效帧数
unsigned long lt_error_count   = 0;   // 校验错误帧数

// --- 控制开关 ---
bool lt_auto_nav_update  = true;   // 是否自动调用 nav_update_position()
float lt_eop_xy_max      = 0.5f;   // EOP超过此值(m)时不注入导航（0=不过滤）

// ==================== 内部解析状态（静态变量） ====================
static uint8_t  lt_rx_buf[LINKTRACK_FRAME_LEN];  // 接收缓冲区
static uint8_t  lt_rx_idx = 0;                    // 当前缓冲区写入位置
static bool     lt_synced = false;                // 是否已同步到帧头

// ==================== 辅助函数 ====================

/**
 * 解析 3 字节小端序 int24 为 int32（符号扩展）
 *
 * LinkTrack 使用 int24 有符号整数以节省带宽。
 * 解析时需检查 bit23（符号位），若为 1 则向高 8 位填充 0xFF。
 *
 * 验证示例（来自官方文档）:
 *   正数: byte[] = {0xe6, 0x0e, 0x00} → raw=0x000ee6 → signed=3814 (3.814m)
 *   负数: byte[] = {0xa5, 0xff, 0xff} → raw=0xffffa5 → signed=-91  (-0.091m)
 *   注: 官方文档的 shift-then-divide-by-256 公式对正数有效，
 *        但对负数依赖 C 未定义行为，本实现改用显式符号扩展确保跨平台正确。
 *
 * @param data 指向 3 字节小端序数据的指针
 * @return 符号扩展后的 int32 值
 */
static inline int32_t lt_parse_int24(const uint8_t *data) {
  uint32_t raw = (uint32_t)data[0] |
                 ((uint32_t)data[1] << 8) |
                 ((uint32_t)data[2] << 16);
  // 检查 24 位符号位，若为 1 则将高 8 位填充 0xFF 以完成符号扩展
  if (raw & 0x800000) {
    return (int32_t)(raw | 0xFF000000);
  }
  return (int32_t)raw;
}

/** 读取 2 字节小端序 uint16 */
static inline uint16_t lt_parse_u16(const uint8_t *data) {
  return (uint16_t)(data[0] | (data[1] << 8));
}

/** 读取 2 字节小端序 int16 */
static inline int16_t lt_parse_s16(const uint8_t *data) {
  return (int16_t)(data[0] | (data[1] << 8));
}

/** 读取 4 字节小端序 float */
static inline float lt_parse_float(const uint8_t *data) {
  float val;
  memcpy(&val, data, 4);
  return val;
}

/**
 * 累加和校验：前面所有字节相加，取低 8 位与帧尾最后一字节比较
 *
 * 来自官方文档:
 *   uint8_t sum = 0;
 *   for (i = 0; i < length-1; i++) sum += data[i];
 *   return sum == data[length-1];
 */
static bool lt_verify_checksum(const uint8_t *data, uint8_t length) {
  uint8_t sum = 0;
  for (uint8_t i = 0; i < length - 1; i++) {
    sum += data[i];
  }
  return sum == data[length - 1];
}

// ==================== 核心函数 ====================

/**
 * 初始化 LinkTrack UART 通信
 * 在 setup() 中调用一次
 */
void initLinkTrack() {
  Serial.flush();
  Serial.updateBaudRate(LINKTRACK_BAUDRATE);
  if (InfoPrint >= 1) {
    Serial.print("[LinkTrack] UART init: Serial RX=GPIO3, TX=GPIO1, Baud=");
    Serial.println(LINKTRACK_BAUDRATE);
  }
}

/**
 * 接收并解析一帧 TAG_Frame0，更新全局变量
 *
 * 状态机流程:
 *   1. 寻找帧头 0x55
 *   2. 验证功能码 0x01 (TAG_Frame0)
 *   3. 接收完整 128 字节
 *   4. 累加和校验
 *   5. 检查 ID 有效性（!= 0xFF）
 *   6. 解析各字段 → 更新 lt_* 全局变量
 *   7. 如 lt_auto_nav_update 为 true → 调用 nav_update_position()
 *
 * 在 loop() 中每次循环调用一次
 */
void updateLinkTrack() {
  while (Serial.available() > 0) {
    uint8_t b = Serial.read();

    if (!lt_synced) {
      // ===== 状态1: 等待帧头 0x55 =====
      if (b == LINKTRACK_HEADER) {
        lt_rx_buf[0] = b;
        lt_rx_idx = 1;
        lt_synced = true;
      }
      // 不是帧头则丢弃（LinkTrack 连续输出，下一字节可能是帧头）
    } else if (lt_rx_idx == 1) {
      // ===== 状态2: 验证功能码 =====
      if (b == LINKTRACK_FUNC_TAG) {
        lt_rx_buf[1] = b;
        lt_rx_idx = 2;
      } else {
        // 功能码不匹配，重新同步
        lt_synced = false;
        lt_rx_idx = 0;
        // 当前字节可能是新的帧头
        if (b == LINKTRACK_HEADER) {
          lt_rx_buf[0] = b;
          lt_rx_idx = 1;
          lt_synced = true;
        }
      }
    } else {
      // ===== 状态3: 继续接收，填满 128 字节 =====
      lt_rx_buf[lt_rx_idx++] = b;

      if (lt_rx_idx >= LINKTRACK_FRAME_LEN) {
        // 帧已收满，重置状态机
        lt_synced = false;
        lt_rx_idx = 0;

        // --- 校验 ---
        if (!lt_verify_checksum(lt_rx_buf, LINKTRACK_FRAME_LEN)) {
          lt_error_count++;
          continue;  // 校验失败，丢弃
        }

        // --- 检查 ID 有效性 ---
        uint8_t tag_id = lt_rx_buf[2];
        if (tag_id == LINKTRACK_INVALID_ID) {
          continue;  // 无效节点
        }

        // ===== 解析各字段 =====

        // 位置 (m): int24 / 1000, 偏移 4,7,10
        lt_pos_x = (float)lt_parse_int24(&lt_rx_buf[4])  / LINKTRACK_POS_SCALE;
        lt_pos_y = (float)lt_parse_int24(&lt_rx_buf[7])  / LINKTRACK_POS_SCALE;
        lt_pos_z = (float)lt_parse_int24(&lt_rx_buf[10]) / LINKTRACK_POS_SCALE;

        // 速度 (m/s): int24 / 10000, 偏移 13,16,19
        lt_vel_x = (float)lt_parse_int24(&lt_rx_buf[13]) / LINKTRACK_VEL_SCALE;
        lt_vel_y = (float)lt_parse_int24(&lt_rx_buf[16]) / LINKTRACK_VEL_SCALE;
        lt_vel_z = (float)lt_parse_int24(&lt_rx_buf[19]) / LINKTRACK_VEL_SCALE;

        // 欧拉角 (度): int16 / 100, 偏移 82,84,86
        lt_roll  = (float)lt_parse_s16(&lt_rx_buf[82]) / LINKTRACK_ANG_SCALE;
        lt_pitch = (float)lt_parse_s16(&lt_rx_buf[84]) / LINKTRACK_ANG_SCALE;
        lt_yaw   = (float)lt_parse_s16(&lt_rx_buf[86]) / LINKTRACK_ANG_SCALE;

        // 精度估计 (m): uint8 / 100, 偏移 117,118,119
        lt_eop_x = (float)lt_rx_buf[117] / LINKTRACK_EOP_SCALE;
        lt_eop_y = (float)lt_rx_buf[118] / LINKTRACK_EOP_SCALE;
        lt_eop_z = (float)lt_rx_buf[119] / LINKTRACK_EOP_SCALE;

        // 电压 (V): uint16 / 1000, 偏移 120
        lt_voltage = (float)lt_parse_u16(&lt_rx_buf[120]) / LINKTRACK_VOL_SCALE;

        // ===== 更新状态标志 =====
        lt_data_updated  = true;
        lt_last_recv_time = millis();
        lt_frame_count++;

        // ===== 自动注入导航 PID =====
        if (lt_auto_nav_update) {
          // EOP 过滤：精度太差时不注入导航，避免车辆失控
          bool eop_ok = (lt_eop_xy_max <= 0.0f) ||
                        ((lt_eop_x <= lt_eop_xy_max || lt_eop_x >= 2.54f) &&
                         (lt_eop_y <= lt_eop_xy_max || lt_eop_y >= 2.54f));
          // 注：EOP=2.55 表示无法估计（系统中<4个基站），此时不因EOP过滤而丢弃
          // 实际使用时建议 lt_eop_xy_max 设为 0.3~0.5

          if (eop_ok) {
            // 偏航角从度转换为弧度
            float heading_rad = lt_yaw * M_PI / 180.0f;
            nav_update_position(lt_pos_x, lt_pos_y, heading_rad);
          }
        }

        // ===== 调试输出 =====
        // 每 50 帧输出一次（200Hz 时约 4 次/秒，50Hz 时约 1 次/秒）
        if (InfoPrint >= 1 && (lt_frame_count % 50 == 0)) {
          Serial.print("[LK] #");
          Serial.print(lt_frame_count);
          Serial.print(" pos=(");
          Serial.print(lt_pos_x, 3);
          Serial.print(", ");
          Serial.print(lt_pos_y, 3);
          Serial.print(") yaw=");
          Serial.print(lt_yaw, 1);
          Serial.print("° eop=(");
          Serial.print(lt_eop_x, 2);
          Serial.print(", ");
          Serial.print(lt_eop_y, 2);
          Serial.print(") v=");
          Serial.print(lt_voltage, 2);
          Serial.print("V err=");
          Serial.print(lt_error_count);
          Serial.println();
        }
      }
    }
  }
}

#endif // LINKTRACK_CTRL_H
