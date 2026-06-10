/**
 * ============================================================
 * linktrack_p_as2.h — LinkTrack UWB 定位数据接收（精简版）
 * ============================================================
 *
 * 【功能】
 *   通过 ESP32 Serial (GPIO3/1, 921600bps) 接收 LinkTrack TAG 的
 *   NLink TAG_Frame0 协议帧，提取实时坐标与偏航角，自动注入导航 PID。
 *
 * 【依赖】
 *   - ESP32 Arduino 框架 (HardwareSerial Serial)
 *   - nav_pid_ctrl.h: nav_update_position() 函数
 *
 * 【数据流】
 *   LinkTrack TAG --UART--> ESP32 Serial
 *       --> 状态机逐字节解析 TAG_Frame0 (128字节)
 *       --> 校验通过后更新 lt_pos_x/y、lt_yaw、lt_eop_x/y
 *       --> EOP 过滤后自动调用 nav_update_position() 覆盖里程计
 *
 * 【坐标系说明】
 *   确保 LinkTrack 坐标系 X/Y 轴与车辆坐标系对齐，
 *   否则需修改 lt_pos_x/y 的赋值逻辑或外部做坐标变换。
 * ============================================================
 */

#ifndef LINKTRACK_P_AS2_H
#define LINKTRACK_P_AS2_H

#include <Arduino.h>
#include "nav_pid_ctrl.h"

// ==================== UART 配置 ====================
// Serial (UART0): RX=GPIO3 ← LinkTrack TX, TX=GPIO1 → LinkTrack RX
#define LINKTRACK_BAUDRATE  921600

// ==================== 协议常量 ====================
#define LINKTRACK_HEADER       0x55
#define LINKTRACK_FUNC_TAG     0x01   // TAG_Frame0 功能码
#define LINKTRACK_FRAME_LEN    128

// 倍率（来自 NLink 协议规范）
#define LINKTRACK_POS_SCALE    1000.0f   // 位置 ×1000 → 米
#define LINKTRACK_ANG_SCALE    100.0f    // 欧拉角 ×100 → 度
#define LINKTRACK_EOP_SCALE    100.0f    // 精度估计 ×100 → 米

// 无效 ID
#define LINKTRACK_INVALID_ID   0xFF

// ==================== 全局变量 ====================

// 位置 (m)
float lt_pos_x = 0.0f;
float lt_pos_y = 0.0f;

// 偏航角 (度)
float lt_yaw = 0.0f;

// 精度估计 EOP (m)，值越小精度越高
float lt_eop_x = 0.0f;
float lt_eop_y = 0.0f;

// 统计
unsigned long lt_frame_count = 0;   // 累计有效帧数
unsigned long lt_error_count = 0;   // 校验错误帧数

// 控制
bool  lt_auto_nav_update = true;    // 是否自动调用 nav_update_position()
float lt_eop_xy_max      = 0.5f;    // EOP 超过此值不注入导航 (0=不过滤)

// ==================== 内部解析状态 ====================
static uint8_t lt_rx_buf[LINKTRACK_FRAME_LEN];
static uint8_t lt_rx_idx = 0;
static bool    lt_synced = false;

// ==================== 辅助函数 ====================

/**
 * 解析 3 字节小端序 int24 为 int32（符号扩展）
 * 验证示例（官方文档）:
 *   正数: {0xe6,0x0e,0x00} → 3814  (3.814m)
 *   负数: {0xa5,0xff,0xff} → -91   (-0.091m)
 */
static inline int32_t lt_parse_int24(const uint8_t *data) {
  uint32_t raw = (uint32_t)data[0] |
                 ((uint32_t)data[1] << 8) |
                 ((uint32_t)data[2] << 16);
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

/**
 * 累加和校验：前面所有字节相加，取低 8 位与帧尾最后一字节比较
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
    Serial.print("[LinkTrack] UART init: RX=GPIO3, Baud=");
    Serial.println(LINKTRACK_BAUDRATE);
  }
}

/**
 * 接收并解析 TAG_Frame0，更新全局变量
 *
 * 状态机流程:
 *   1. 寻找帧头 0x55
 *   2. 验证功能码 0x01 (TAG_Frame0)
 *   3. 接收完整 128 字节
 *   4. 累加和校验
 *   5. 检查 ID 有效性
 *   6. 解析位置、偏航角、EOP
 *   7. EOP 过滤后自动调用 nav_update_position()
 *
 * 在 loop() 中每次调用
 */
void updateLinkTrack() {
  while (Serial.available() > 0) {
    uint8_t b = Serial.read();

    if (!lt_synced) {
      // 等待帧头 0x55
      if (b == LINKTRACK_HEADER) {
        lt_rx_buf[0] = b;
        lt_rx_idx = 1;
        lt_synced = true;
      }
    } else if (lt_rx_idx == 1) {
      // 验证功能码
      if (b == LINKTRACK_FUNC_TAG) {
        lt_rx_buf[1] = b;
        lt_rx_idx = 2;
      } else {
        lt_synced = false;
        lt_rx_idx = 0;
        if (b == LINKTRACK_HEADER) {
          lt_rx_buf[0] = b;
          lt_rx_idx = 1;
          lt_synced = true;
        }
      }
    } else {
      // 填满 128 字节
      lt_rx_buf[lt_rx_idx++] = b;

      if (lt_rx_idx >= LINKTRACK_FRAME_LEN) {
        lt_synced = false;
        lt_rx_idx = 0;

        // 累加和校验
        if (!lt_verify_checksum(lt_rx_buf, LINKTRACK_FRAME_LEN)) {
          lt_error_count++;
          continue;
        }

        // 检查 ID 有效性
        if (lt_rx_buf[2] == LINKTRACK_INVALID_ID) {
          continue;
        }

        // 解析位置 (m): int24/1000, 偏移 4,7
        lt_pos_x = (float)lt_parse_int24(&lt_rx_buf[4])  / LINKTRACK_POS_SCALE;
        lt_pos_y = (float)lt_parse_int24(&lt_rx_buf[7])  / LINKTRACK_POS_SCALE;

        // 解析偏航角 (度): int16/100, 偏移 86
        lt_yaw = (float)lt_parse_s16(&lt_rx_buf[86]) / LINKTRACK_ANG_SCALE;

        // 解析 EOP (m): uint8/100, 偏移 117,118
        lt_eop_x = (float)lt_rx_buf[117] / LINKTRACK_EOP_SCALE;
        lt_eop_y = (float)lt_rx_buf[118] / LINKTRACK_EOP_SCALE;

        lt_frame_count++;

        // 自动注入导航 PID（EOP 过滤）
        if (lt_auto_nav_update) {
          bool eop_ok = (lt_eop_xy_max <= 0.0f) ||
                        ((lt_eop_x <= lt_eop_xy_max || lt_eop_x >= 2.54f) &&
                         (lt_eop_y <= lt_eop_xy_max || lt_eop_y >= 2.54f));
          if (eop_ok) {
            float heading_rad = lt_yaw * M_PI / 180.0f;
            nav_update_position(lt_pos_x, lt_pos_y, heading_rad);
          }
        }

        // 调试输出（每 50 帧一次）
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
          Serial.print(") err=");
          Serial.print(lt_error_count);
          Serial.println();
        }
      }
    }
  }
}

#endif // LINKTRACK_P_AS2_H
