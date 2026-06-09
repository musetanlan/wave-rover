/**
 * ============================================================
 * linktrack_p_as2.h — LinkTrack P_AS2 UWB 定位模块 (增强版)
 * ============================================================
 *
 * 【功能概述】
 *   针对 Nooploop LinkTrack P_AS2 器件的完整定位模块，通过 ESP32 Serial
 *   (GPIO3/1) 与 P_AS2 标签通信，支持：
 *     - NLink TAG_Frame0 协议解析（定位核心数据）
 *     - NLink NODE_FRAME2 协议解析（基站距离、IMU 等扩展数据）
 *     - 下行配置命令发送（LP_MODE0、波特率等设备参数）
 *     - FreeRTOS 任务模式（可选，防止串口 FIFO 溢出）
 *
 * 【LP 模式说明】
 *   LP（Local Positioning）是 LinkTrack 的局部定位模式，标签测量到各基站的
 *   距离并解算坐标。LP_MODE0 提供最高的定位频率，适合实时导航。
 *
 * 【配置策略】
 *   默认在 initLinkTrack() 时自动发送 LP_MODE0 配置命令（lt_auto_config=true），
 *   也可通过 NAssistant 预先配置。设备配置参数持久化存储在 Flash 中。
 *
 * 【依赖】
 *   - ESP32 Arduino 框架 (HardwareSerial Serial)
 *   - nav_pid_ctrl.h: nav_update_position() 函数
 *
 * 【数据流】
 *   P_AS2 TAG --UART(921600bps)--> ESP32 Serial
 *       --> 状态机逐字节解析 NLink 帧
 *       --> TAG_Frame0 / NODE_FRAME2 分类解析
 *       --> 统一后处理 → 更新全局变量
 *       --> 自动调用 nav_update_position() 覆盖里程计
 *
 * 【坐标系说明】
 *   LinkTrack 输出坐标系与基站标定坐标系一致。
 *   需确保 LinkTrack 坐标系 X/Y 轴方向与车辆坐标系对齐，
 *   否则需要在外部做坐标变换或修改 lt_pos_x/y 的赋值逻辑。
 *
 * 【与原 linktrack_ctrl.h 的差异】
 *   + NODE_FRAME2 协议解析支持
 *   + 下行配置命令发送（LP_MODE0 / 波特率）
 *   + FreeRTOS 任务模式选项
 *   + lt_frame_type / lt_anchors[] 等新全局变量
 *   + initLinkTrack() 自动配置选项
 *   - 核心 TAG_Frame0 解析逻辑完全保留，向后兼容
 * ============================================================
 */

#ifndef LINKTRACK_P_AS2_H
#define LINKTRACK_P_AS2_H

#include <Arduino.h>
#include "nav_pid_ctrl.h"

// ==================== 编译选项 ====================
// 取消注释以启用 FreeRTOS 任务模式
// #define LT_USE_FREERTOS

// ==================== UART 配置 ====================
// Serial (UART0) 默认引脚: RX=GPIO3, TX=GPIO1
// ESP32 GPIO3  ← P_AS2 TX
// ESP32 GPIO1  → P_AS2 RX
// LinkTrack 默认波特率（需与 NAssistant 中配置一致）
#define LINKTRACK_BAUDRATE  921600

// ==================== 协议常量 ====================
// 帧头标识
#define LINKTRACK_HEADER     0x55

// 功能码
#define LINKTRACK_FUNC_TAG   0x01   // TAG_Frame0 功能码
#define LINKTRACK_FUNC_NODE2 0x02   // NODE_FRAME2 功能码（待 NLink 文档确认）

// TAG_Frame0 帧长度（固定 128 字节）
#define LINKTRACK_FRAME_LEN  128

// NODE_FRAME2 帧长度（待 NLink 文档确认，通常为变长或约 224 字节）
#define NODE_FRAME2_LEN      224

// 坐标与速度的倍率（来自 NLink 协议规范）
#define LINKTRACK_POS_SCALE  1000.0f   // 位置 ×1000 → 除以 1000 得到米
#define LINKTRACK_VEL_SCALE  10000.0f  // 速度 ×10000 → 除以 10000 得到米/秒
#define LINKTRACK_ANG_SCALE  100.0f    // 欧拉角 ×100 → 除以 100 得到度
#define LINKTRACK_EOP_SCALE  100.0f    // 精度估计 ×100
#define LINKTRACK_VOL_SCALE  1000.0f   // 电压 ×1000

// 无效 ID（LinkTrack 协议规定 0xFF 为无效节点）
#define LINKTRACK_INVALID_ID 0xFF

// 最大基站数量（NODE_FRAME2 数据）
#define LT_MAX_ANCHORS       8

// 下行命令缓冲区大小
#define LT_DOWNLINK_BUF_SIZE 64

// ==================== NLink 下行命令常量 ====================
// 【重要】以下功能码和参数索引均为基于 NLink 协议框架的预估定义，
//         实际值需对照 NLink_V1.4.pdf 协议文档确认。
//         所有命令相关的宏集中在此处，便于根据官方文档一键修正。

// 系统配置命令功能码（预估，请对照协议文档）
#define LT_DL_FUNC_SYSTEM    0x05   // 系统配置功能码
#define LT_DL_CMD_LP_MODE    0x01   // 设置 LP 模式
#define LT_DL_CMD_BAUD       0x03   // 设置波特率
#define LT_DL_CMD_QUERY      0x10   // 查询设备版本

// LP 模式参数
#define LT_DL_LP_MODE0       0x00   // LP_MODE0 — 最高定位频率

// 波特率索引（固件相关，请对照协议文档）
#define LT_DL_BAUD_921600    0x06   // 921600 波特率索引
#define LT_DL_BAUD_115200    0x01   // 115200 波特率索引（备用）

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
// yaw = 偏航角(Yaw), pitch = 俯仰角(Pitch), roll = 翻滚角(Roll)
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
bool lt_data_updated       = false;   // 本循环是否有新数据到来
unsigned long lt_last_recv_time = 0;  // 最后收到有效帧的时间 (ms)
unsigned long lt_frame_count   = 0;   // 累计收到的有效帧数
unsigned long lt_error_count   = 0;   // 校验错误帧数

// --- 控制开关 ---
bool lt_auto_nav_update  = true;      // 是否自动调用 nav_update_position()
float lt_eop_xy_max      = 0.5f;      // EOP 超过此值 (m) 时不注入导航（0=不过滤）

// --- NODE_FRAME2 扩展数据 ---
// 基站距离数据: lt_anchors[i][0]=基站ID, lt_anchors[i][1]=距离(m)
float   lt_anchors[LT_MAX_ANCHORS][2] = {{0}};
uint8_t lt_anchor_count = 0;          // 有效基站数量

// IMU 数据（来自 NODE_FRAME2，与板载 ICM-20948 互为备份）
float lt_accel_x = 0.0f, lt_accel_y = 0.0f, lt_accel_z = 0.0f;  // m/s²
float lt_gyro_x  = 0.0f, lt_gyro_y  = 0.0f, lt_gyro_z  = 0.0f;  // rad/s

// 帧类型（最后收到帧的功能码）：0=无, 1=TAG_Frame0, 2=NODE_FRAME2
uint8_t lt_frame_type = 0;

// --- 新增控制变量 ---
bool lt_auto_config   = true;         // 初始化时自动发送 LP_MODE0 配置
bool lt_use_task_mode = false;        // 是否使用 FreeRTOS 任务模式（需编译时启用）
bool lt_configured    = false;        // 是否已完成 LP_MODE0 配置

// ==================== 内部解析状态（静态变量） ====================
static uint8_t  lt_rx_buf[LINKTRACK_FRAME_LEN > NODE_FRAME2_LEN ?
                           LINKTRACK_FRAME_LEN : NODE_FRAME2_LEN];  // 取两者较大值
static uint8_t  lt_rx_idx     = 0;       // 当前缓冲区写入位置
static uint8_t  lt_rx_func    = 0;       // 当前帧的功能码
static uint16_t lt_rx_frame_len = 0;     // 当前帧期望长度
static bool     lt_synced     = false;   // 是否已同步到帧头

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
static bool lt_verify_checksum(const uint8_t *data, uint16_t length) {
  uint8_t sum = 0;
  for (uint16_t i = 0; i < length - 1; i++) {
    sum += data[i];
  }
  return sum == data[length - 1];
}

/**
 * 根据功能码返回对应的帧长度
 */
static uint16_t lt_get_frame_len_for_func(uint8_t func) {
  switch (func) {
    case LINKTRACK_FUNC_TAG:   return LINKTRACK_FRAME_LEN;   // 128
    case LINKTRACK_FUNC_NODE2: return NODE_FRAME2_LEN;       // 224 (待确认)
    default:                   return 0;                     // 未知协议
  }
}

// ==================== NLink 下行命令帧基础设施 ====================

/**
 * 构造并发送 NLink 下行命令帧
 *
 * 帧格式:
 *   [0] 0x55       帧头
 *   [1] func_code  功能码
 *   [2..N-1]       负载数据
 *   [N]            checksum（累加和低8位）
 *
 * @param func_code   功能码
 * @param payload     负载数据指针
 * @param payload_len 负载数据长度（字节）
 * @return  true: 发送成功, false: 负载过长
 */
static bool lt_build_and_send_frame(uint8_t func_code,
                                     const uint8_t *payload,
                                     uint8_t payload_len) {
  if (payload_len > LT_DOWNLINK_BUF_SIZE - 3) return false;

  uint8_t frame[LT_DOWNLINK_BUF_SIZE];
  uint8_t total_len = 2 + payload_len;  // header + func + payload

  frame[0] = LINKTRACK_HEADER;          // 0x55
  frame[1] = func_code;
  if (payload_len > 0) {
    memcpy(&frame[2], payload, payload_len);
  }

  // 计算累加和校验
  uint8_t sum = 0;
  for (uint8_t i = 0; i < total_len; i++) {
    sum += frame[i];
  }
  frame[total_len] = sum;
  total_len++;

  // 发送帧
  size_t written = Serial.write(frame, total_len);
  Serial.flush();  // 确保发送完成

  if (InfoPrint >= 1) {
    Serial.print("[LK] Sent downlink frame: func=0x");
    Serial.print(func_code, HEX);
    Serial.print(" len=");
    Serial.print(total_len);
    Serial.print(" written=");
    Serial.println(written);
  }

  return (written == (size_t)total_len);
}

/**
 * 发送 LP_MODE0 配置命令
 *
 * 向 P_AS2 设备发送指令，切换到 LP（定位）模式的 MODE0 子模式。
 * LP_MODE0 提供最高的定位更新频率。
 */
static bool lt_send_lp_mode0_config() {
  // 命令负载: [CMD_ID=LT_DL_CMD_LP_MODE, PARAM=LT_DL_LP_MODE0]
  // 待对照 NLink_V1.4.pdf 确认正确字节序后修正
  uint8_t payload[] = { LT_DL_CMD_LP_MODE, LT_DL_LP_MODE0 };
  return lt_build_and_send_frame(LT_DL_FUNC_SYSTEM, payload, 2);
}

/**
 * 发送波特率配置命令
 *
 * @param baud_idx 波特率索引（LT_DL_BAUD_921600 / LT_DL_BAUD_115200）
 * @note  修改波特率后设备可能需要重启才能生效
 */
static bool lt_send_baud_config(uint8_t baud_idx) {
  uint8_t payload[] = { LT_DL_CMD_BAUD, baud_idx };
  return lt_build_and_send_frame(LT_DL_FUNC_SYSTEM, payload, 2);
}

/**
 * 查询设备版本信息
 */
static bool lt_send_query_version() {
  uint8_t payload[] = { LT_DL_CMD_QUERY };
  return lt_build_and_send_frame(LT_DL_FUNC_SYSTEM, payload, 1);
}

// ==================== 帧数据解析器 ====================

/**
 * 统一后处理：帧校验通过并解析字段后调用
 *
 * 负责更新状态标志、自动导航注入、调试输出等公共逻辑。
 * TAG_Frame0 和 NODE_FRAME2 解析完成后都调用此函数。
 */
static void lt_post_parse_common() {
  lt_data_updated    = true;
  lt_last_recv_time  = millis();
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
    Serial.print(" type=");
    Serial.print(lt_frame_type);
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
    Serial.print("V");

    // NODE_FRAME2 额外信息
    if (lt_frame_type == LINKTRACK_FUNC_NODE2 && lt_anchor_count > 0) {
      Serial.print(" anchors=");
      Serial.print(lt_anchor_count);
      Serial.print(" dist=[");
      for (uint8_t i = 0; i < lt_anchor_count && i < 4; i++) {
        if (i > 0) Serial.print(",");
        Serial.print(lt_anchors[i][1], 2);
      }
      Serial.print("]");
    }

    Serial.print(" err=");
    Serial.print(lt_error_count);
    Serial.println();
  }
}

/**
 * 解析 TAG_Frame0 数据帧
 *
 * 从 128 字节帧缓冲区中提取位置、速度、欧拉角、EOP、电压等字段。
 * 字段偏移和比例因子遵循 NLink 协议规范。
 *
 * 【完整保留自 linktrack_ctrl.h — 已验证可用】
 *
 * @param buf 128 字节帧缓冲区
 */
static void lt_parse_tag_frame0(const uint8_t *buf) {
  lt_frame_type = LINKTRACK_FUNC_TAG;

  // 位置 (m): int24 / 1000, 偏移 4,7,10
  lt_pos_x = (float)lt_parse_int24(&buf[4])  / LINKTRACK_POS_SCALE;
  lt_pos_y = (float)lt_parse_int24(&buf[7])  / LINKTRACK_POS_SCALE;
  lt_pos_z = (float)lt_parse_int24(&buf[10]) / LINKTRACK_POS_SCALE;

  // 速度 (m/s): int24 / 10000, 偏移 13,16,19
  lt_vel_x = (float)lt_parse_int24(&buf[13]) / LINKTRACK_VEL_SCALE;
  lt_vel_y = (float)lt_parse_int24(&buf[16]) / LINKTRACK_VEL_SCALE;
  lt_vel_z = (float)lt_parse_int24(&buf[19]) / LINKTRACK_VEL_SCALE;

  // 欧拉角 (度): int16 / 100, 偏移 82,84,86
  lt_roll  = (float)lt_parse_s16(&buf[82]) / LINKTRACK_ANG_SCALE;
  lt_pitch = (float)lt_parse_s16(&buf[84]) / LINKTRACK_ANG_SCALE;
  lt_yaw   = (float)lt_parse_s16(&buf[86]) / LINKTRACK_ANG_SCALE;

  // 精度估计 (m): uint8 / 100, 偏移 117,118,119
  lt_eop_x = (float)buf[117] / LINKTRACK_EOP_SCALE;
  lt_eop_y = (float)buf[118] / LINKTRACK_EOP_SCALE;
  lt_eop_z = (float)buf[119] / LINKTRACK_EOP_SCALE;

  // 电压 (V): uint16 / 1000, 偏移 120
  lt_voltage = (float)lt_parse_u16(&buf[120]) / LINKTRACK_VOL_SCALE;

  // 共用后处理
  lt_post_parse_common();
}

/**
 * 解析 NODE_FRAME2 数据帧
 *
 * NODE_FRAME2 提供比 TAG_Frame0 更丰富的数据：
 *   - 位置 / 速度 / 欧拉角（同 TAG_Frame0）
 *   - 加速度计和陀螺仪数据
 *   - 各基站的距离和信号强度
 *   - 网络状态信息
 *
 * 【注意】以下字段偏移为基于 NLink 协议框架的预估，
 *         实际值需对照 NLink_V1.4.pdf 协议文档确认后修正。
 *
 * @param buf  帧缓冲区
 * @param len  帧长度
 */
static void lt_parse_node_frame2(const uint8_t *buf, uint16_t len) {
  lt_frame_type = LINKTRACK_FUNC_NODE2;

  // --- 基础定位数据（字段可能与 TAG_Frame0 位置不同，待确认）---
  // 以下偏移为预估，需对照 NLink_V1.4.pdf 确认

  // 位置 (m): 偏移 4,7,10 — int24 / 1000
  lt_pos_x = (float)lt_parse_int24(&buf[4])  / LINKTRACK_POS_SCALE;
  lt_pos_y = (float)lt_parse_int24(&buf[7])  / LINKTRACK_POS_SCALE;
  lt_pos_z = (float)lt_parse_int24(&buf[10]) / LINKTRACK_POS_SCALE;

  // 速度 (m/s): 偏移 13,16,19 — int24 / 10000
  lt_vel_x = (float)lt_parse_int24(&buf[13]) / LINKTRACK_VEL_SCALE;
  lt_vel_y = (float)lt_parse_int24(&buf[16]) / LINKTRACK_VEL_SCALE;
  lt_vel_z = (float)lt_parse_int24(&buf[19]) / LINKTRACK_VEL_SCALE;

  // 欧拉角 (度): 偏移 82,84,86 — int16 / 100
  lt_roll  = (float)lt_parse_s16(&buf[82]) / LINKTRACK_ANG_SCALE;
  lt_pitch = (float)lt_parse_s16(&buf[84]) / LINKTRACK_ANG_SCALE;
  lt_yaw   = (float)lt_parse_s16(&buf[86]) / LINKTRACK_ANG_SCALE;

  // --- IMU 数据 ---
  // 加速度计: 通常紧随欧拉角之后
  // 以下偏移为占位，待 NLink_V1.4.pdf 确认
  if (len >= 110) {
    lt_accel_x = (float)lt_parse_s16(&buf[88])  / 100.0f;  // m/s²
    lt_accel_y = (float)lt_parse_s16(&buf[90])  / 100.0f;
    lt_accel_z = (float)lt_parse_s16(&buf[92])  / 100.0f;

    lt_gyro_x  = (float)lt_parse_s16(&buf[94])  / 100.0f;  // rad/s
    lt_gyro_y  = (float)lt_parse_s16(&buf[96])  / 100.0f;
    lt_gyro_z  = (float)lt_parse_s16(&buf[98])  / 100.0f;
  }

  // --- 基站距离数据 ---
  // 通常在帧中后部，格式为 [基站数量(1B)] [基站ID(1B)+距离(float)]*N
  // 以下偏移为占位，待 NLink_V1.4.pdf 确认
  if (len >= 114) {
    uint8_t anchor_count = buf[112];
    lt_anchor_count = (anchor_count > LT_MAX_ANCHORS) ?
                       LT_MAX_ANCHORS : anchor_count;

    uint16_t anchor_offset = 113;
    for (uint8_t i = 0; i < lt_anchor_count; i++) {
      if (anchor_offset + 5 > len) break;  // 边界保护
      lt_anchors[i][0] = buf[anchor_offset];         // 基站 ID
      lt_anchors[i][1] = lt_parse_float(&buf[anchor_offset + 1]); // 距离 (m)
      anchor_offset += 5;
    }
  }

  // --- EOP (精度估计) ---
  // EOP 通常在帧尾附近，偏移需对照协议文档
  if (len >= 212) {
    lt_eop_x = (float)buf[209] / LINKTRACK_EOP_SCALE;
    lt_eop_y = (float)buf[210] / LINKTRACK_EOP_SCALE;
    lt_eop_z = (float)buf[211] / LINKTRACK_EOP_SCALE;
  }

  // --- 电压 ---
  if (len >= 220) {
    lt_voltage = (float)lt_parse_u16(&buf[218]) / LINKTRACK_VOL_SCALE;
  }

  // 共用后处理
  lt_post_parse_common();
}

// ==================== FreeRTOS 任务模式 ====================

#ifdef LT_USE_FREERTOS

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// 数据包：从解析任务传递到主循环
struct lt_parsed_packet_t {
  uint8_t  frame_type;
  float    pos_x, pos_y, pos_z;
  float    vel_x, vel_y, vel_z;
  float    yaw, pitch, roll;
  float    eop_x, eop_y, eop_z;
  float    voltage;
  float    anchors[LT_MAX_ANCHORS][2];
  uint8_t  anchor_count;
  float    accel_x, accel_y, accel_z;
  float    gyro_x, gyro_y, gyro_z;
  unsigned long recv_time;
};

static TaskHandle_t  lt_task_handle  = NULL;
static QueueHandle_t lt_data_queue   = NULL;
static const uint8_t LT_QUEUE_LENGTH = 16;

/**
 * FreeRTOS 解析任务函数
 *
 * 在独立任务中循环读取 Serial，逐字节解析 NLink 帧。
 * 完整的帧数据打包后通过队列发送给主循环。
 */
static void lt_parser_task(void *pvParameters) {
  lt_parsed_packet_t packet;
  // 帧接收缓冲区（任务栈中分配，避免与轮询模式共用）
  uint8_t  rx_buf[LINKTRACK_FRAME_LEN > NODE_FRAME2_LEN ?
                   LINKTRACK_FRAME_LEN : NODE_FRAME2_LEN];
  uint8_t  rx_idx     = 0;
  uint8_t  rx_func    = 0;
  uint16_t rx_frame_len = 0;
  bool     synced     = false;

  while (1) {
    // 检查是否有数据
    if (Serial.available() == 0) {
      vTaskDelay(pdMS_TO_TICKS(1));  // 1ms 让出 CPU
      continue;
    }

    uint8_t b = Serial.read();

    if (!synced) {
      // ===== 状态1: 等待帧头 0x55 =====
      if (b == LINKTRACK_HEADER) {
        rx_buf[0] = b;
        rx_idx    = 1;
        synced    = true;
      }
    } else if (rx_idx == 1) {
      // ===== 状态2: 验证功能码 =====
      rx_frame_len = lt_get_frame_len_for_func(b);
      if (rx_frame_len > 0) {
        rx_buf[1] = b;
        rx_func   = b;
        rx_idx    = 2;
      } else {
        // 未知功能码，重新同步
        synced = false;
        rx_idx = 0;
        if (b == LINKTRACK_HEADER) {
          rx_buf[0] = b;
          rx_idx    = 1;
          synced    = true;
        }
      }
    } else {
      // ===== 状态3: 继续接收，填满帧 =====
      rx_buf[rx_idx++] = b;

      if (rx_idx >= rx_frame_len) {
        // 帧已收满，重置状态机
        synced = false;
        rx_idx = 0;

        // 校验
        if (!lt_verify_checksum(rx_buf, rx_frame_len)) {
          lt_error_count++;
          continue;
        }

        // 检查 ID 有效性
        if (rx_buf[2] == LINKTRACK_INVALID_ID) {
          continue;
        }

        // --- 填充数据包 ---
        memset(&packet, 0, sizeof(packet));
        packet.frame_type = rx_func;
        packet.recv_time  = millis();

        // 位置 (偏移 4,7,10)
        packet.pos_x = (float)lt_parse_int24(&rx_buf[4])  / LINKTRACK_POS_SCALE;
        packet.pos_y = (float)lt_parse_int24(&rx_buf[7])  / LINKTRACK_POS_SCALE;
        packet.pos_z = (float)lt_parse_int24(&rx_buf[10]) / LINKTRACK_POS_SCALE;

        // 速度 (偏移 13,16,19)
        packet.vel_x = (float)lt_parse_int24(&rx_buf[13]) / LINKTRACK_VEL_SCALE;
        packet.vel_y = (float)lt_parse_int24(&rx_buf[16]) / LINKTRACK_VEL_SCALE;
        packet.vel_z = (float)lt_parse_int24(&rx_buf[19]) / LINKTRACK_VEL_SCALE;

        // 欧拉角 (偏移 82,84,86)
        packet.roll  = (float)lt_parse_s16(&rx_buf[82]) / LINKTRACK_ANG_SCALE;
        packet.pitch = (float)lt_parse_s16(&rx_buf[84]) / LINKTRACK_ANG_SCALE;
        packet.yaw   = (float)lt_parse_s16(&rx_buf[86]) / LINKTRACK_ANG_SCALE;

        // EOP (偏移 117,118,119)
        packet.eop_x = (float)rx_buf[117] / LINKTRACK_EOP_SCALE;
        packet.eop_y = (float)rx_buf[118] / LINKTRACK_EOP_SCALE;
        packet.eop_z = (float)rx_buf[119] / LINKTRACK_EOP_SCALE;

        // 电压 (偏移 120)
        packet.voltage = (float)lt_parse_u16(&rx_buf[120]) / LINKTRACK_VOL_SCALE;

        // 发送到队列（非阻塞，队列满则丢弃最旧数据）
        xQueueSend(lt_data_queue, &packet, 0);
      }
    }
  }
}

/**
 * 启动 FreeRTOS 任务模式
 *
 * 创建独立解析任务和通信队列。调用后 updateLinkTrack()
 * 自动切换为队列读取模式。
 *
 * @return true: 启动成功, false: 任务创建失败
 */
bool lt_start_task_mode() {
  if (lt_task_handle != NULL) return true;  // 已在运行

  lt_data_queue = xQueueCreate(LT_QUEUE_LENGTH, sizeof(lt_parsed_packet_t));
  if (lt_data_queue == NULL) return false;

  BaseType_t result = xTaskCreatePinnedToCore(
    lt_parser_task,           // 任务函数
    "lt_parser",              // 任务名称
    4096,                     // 栈大小（字节）
    NULL,                     // 参数
    10,                       // 优先级（高于 loop 的优先级 1）
    &lt_task_handle,           // 任务句柄
    0                         // 绑定到 Core 0（loop 在 Core 1）
  );

  if (result != pdPASS) {
    vQueueDelete(lt_data_queue);
    lt_data_queue = NULL;
    return false;
  }

  lt_use_task_mode = true;

  if (InfoPrint >= 1) {
    Serial.println("[LK] FreeRTOS task mode started (core 0, prio 10)");
  }
  return true;
}

/**
 * 停止 FreeRTOS 任务模式
 *
 * 删除解析任务和队列，恢复为轮询模式。
 */
void lt_stop_task_mode() {
  if (lt_task_handle != NULL) {
    vTaskDelete(lt_task_handle);
    lt_task_handle = NULL;
  }
  if (lt_data_queue != NULL) {
    vQueueDelete(lt_data_queue);
    lt_data_queue = NULL;
  }
  lt_use_task_mode = false;

  if (InfoPrint >= 1) {
    Serial.println("[LK] FreeRTOS task mode stopped, back to polling");
  }
}

/**
 * 从任务队列应用最新解析数据到全局变量
 */
static void lt_apply_parsed_packet(const lt_parsed_packet_t *pkt) {
  lt_frame_type = pkt->frame_type;

  lt_pos_x = pkt->pos_x;
  lt_pos_y = pkt->pos_y;
  lt_pos_z = pkt->pos_z;

  lt_vel_x = pkt->vel_x;
  lt_vel_y = pkt->vel_y;
  lt_vel_z = pkt->vel_z;

  lt_yaw   = pkt->yaw;
  lt_pitch = pkt->pitch;
  lt_roll  = pkt->roll;

  lt_eop_x = pkt->eop_x;
  lt_eop_y = pkt->eop_y;
  lt_eop_z = pkt->eop_z;

  lt_voltage     = pkt->voltage;
  lt_anchor_count = pkt->anchor_count;
  memcpy(lt_anchors, pkt->anchors, sizeof(pkt->anchors));

  lt_accel_x = pkt->accel_x;
  lt_accel_y = pkt->accel_y;
  lt_accel_z = pkt->accel_z;

  lt_gyro_x  = pkt->gyro_x;
  lt_gyro_y  = pkt->gyro_y;
  lt_gyro_z  = pkt->gyro_z;

  // 共用后处理
  lt_post_parse_common();
}

#else  // !LT_USE_FREERTOS

// 无 FreeRTOS 时提供空实现
bool lt_start_task_mode() {
  if (InfoPrint >= 1) {
    Serial.println("[LK] FreeRTOS task mode not compiled (enable LT_USE_FREERTOS)");
  }
  return false;
}
void lt_stop_task_mode() {}

#endif // LT_USE_FREERTOS

// ==================== 核心函数 ====================

/**
 * 初始化 LinkTrack P_AS2 UART 通信
 *
 * 在 setup() 中调用一次。执行顺序：
 * 1. 切换 Serial 波特率 (921600 bps, GPIO3/1)
 * 2. 可选：发送 LP_MODE0 配置命令（lt_auto_config=true 时）
 * 3. 可选：启动 FreeRTOS 解析任务（lt_use_task_mode=true 时）
 */
void initLinkTrack() {
  Serial.flush();
  Serial.updateBaudRate(LINKTRACK_BAUDRATE);

  if (InfoPrint >= 1) {
    Serial.print("[LinkTrack] UART init: Serial RX=GPIO3, TX=GPIO1, Baud=");
    Serial.println(LINKTRACK_BAUDRATE);
  }

  // 自动配置 LP_MODE0
  if (lt_auto_config) {
    if (InfoPrint >= 1) {
      Serial.println("[LinkTrack] Sending LP_MODE0 config...");
    }
    lt_send_lp_mode0_config();
    // 可选：同时重设波特率以确保一致
    // lt_send_baud_config(LT_DL_BAUD_921600);

    delay(200);  // 等待设备处理配置命令

    // 清空串口缓冲区，丢弃配置期间可能到达的部分帧
    while (Serial.available()) {
      Serial.read();
    }

    lt_configured = true;

    if (InfoPrint >= 1) {
      Serial.println("[LinkTrack] LP_MODE0 config sent");
    }
  }

  // 启动 FreeRTOS 任务模式（如果启用）
  if (lt_use_task_mode) {
    lt_start_task_mode();
  }
}

/**
 * 接收并解析一帧定位数据，更新全局变量
 *
 * 在 loop() 中每次调用。行为取决于运行模式：
 * - 轮询模式 (lt_use_task_mode=false): 逐字节从 Serial 读取并解析
 * - 任务模式 (lt_use_task_mode=true): 从 FreeRTOS 队列读取最新解析结果
 *
 * 状态机流程（轮询模式）:
 *   1. 寻找帧头 0x55
 *   2. 验证功能码 → 确定帧类型和长度
 *   3. 接收完整帧
 *   4. 累加和校验
 *   5. 检查 ID 有效性（!= 0xFF）
 *   6. 按功能码调度解析器
 *   7. 共用后处理（导航注入 + 调试输出）
 */
void updateLinkTrack() {
#ifdef LT_USE_FREERTOS
  // ===== FreeRTOS 任务模式：从队列读取 =====
  if (lt_use_task_mode && lt_data_queue != NULL) {
    lt_parsed_packet_t packet;
    bool got_data = false;

    // 排空队列，只保留最新一帧
    while (xQueueReceive(lt_data_queue, &packet, 0) == pdTRUE) {
      got_data = true;
    }

    if (got_data) {
      lt_apply_parsed_packet(&packet);
    }
    return;
  }
#endif

  // ===== 轮询模式：逐字节解析（与原 linktrack_ctrl.h 完全一致）=====
  while (Serial.available() > 0) {
    uint8_t b = Serial.read();

    if (!lt_synced) {
      // ===== 状态1: 等待帧头 0x55 =====
      if (b == LINKTRACK_HEADER) {
        lt_rx_buf[0] = b;
        lt_rx_idx    = 1;
        lt_synced    = true;
      }
    } else if (lt_rx_idx == 1) {
      // ===== 状态2: 验证功能码 =====
      lt_rx_frame_len = lt_get_frame_len_for_func(b);
      if (lt_rx_frame_len > 0) {
        // 已知功能码：TAG_Frame0 (0x01) 或 NODE_FRAME2 (0x02)
        lt_rx_buf[1] = b;
        lt_rx_func   = b;
        lt_rx_idx    = 2;
      } else {
        // 未知功能码，重新同步
        lt_synced = false;
        lt_rx_idx = 0;
        // 当前字节可能是新的帧头
        if (b == LINKTRACK_HEADER) {
          lt_rx_buf[0] = b;
          lt_rx_idx    = 1;
          lt_synced    = true;
        }
      }
    } else {
      // ===== 状态3: 继续接收，填满帧 =====
      lt_rx_buf[lt_rx_idx++] = b;

      if (lt_rx_idx >= lt_rx_frame_len) {
        // 帧已收满，重置状态机
        lt_synced = false;
        lt_rx_idx = 0;

        // --- 校验 ---
        if (!lt_verify_checksum(lt_rx_buf, lt_rx_frame_len)) {
          lt_error_count++;
          continue;  // 校验失败，丢弃
        }

        // --- 检查 ID 有效性 ---
        uint8_t tag_id = lt_rx_buf[2];
        if (tag_id == LINKTRACK_INVALID_ID) {
          continue;  // 无效节点
        }

        // --- 按功能码调度解析器 ---
        switch (lt_rx_func) {
          case LINKTRACK_FUNC_TAG:
            lt_parse_tag_frame0(lt_rx_buf);
            break;

          case LINKTRACK_FUNC_NODE2:
            lt_parse_node_frame2(lt_rx_buf, lt_rx_frame_len);
            break;

          default:
            // 不应到达此处（已在状态2过滤）
            break;
        }
      }
    }
  }
}

/**
 * 获取数据新鲜度
 *
 * @param max_age_ms 最大允许的数据年龄 (ms)
 * @return true: 在 max_age_ms 内收到过有效数据
 */
bool lt_is_data_fresh(unsigned long max_age_ms) {
  if (lt_last_recv_time == 0) return false;
  return (millis() - lt_last_recv_time) <= max_age_ms;
}

/**
 * 估算帧率 (Hz)
 *
 * @return 基于总帧数和运行时间的平均帧率
 */
float lt_get_fps() {
  static unsigned long start_time = 0;
  static unsigned long start_count = 0;

  if (start_time == 0) {
    start_time  = millis();
    start_count = lt_frame_count;
    return 0.0f;
  }

  unsigned long elapsed = millis() - start_time;
  if (elapsed < 1000) return 0.0f;  // 至少运行 1 秒

  unsigned long frames = lt_frame_count - start_count;
  return (float)frames * 1000.0f / (float)elapsed;
}

/**
 * 重置统计计数器
 */
void lt_reset_stats() {
  lt_frame_count = 0;
  lt_error_count = 0;
  lt_last_recv_time = 0;
}

#endif // LINKTRACK_P_AS2_H
