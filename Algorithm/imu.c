#include "imu.h"

// 实例化全局 IMU 对象
IMU_TypeDef my_car_imu;

// --- 串口解析状态机所需变量 ---
typedef enum {
    STEP_HEADER1 = 0,
    STEP_HEADER2,
    STEP_ID,
    STEP_LEN,
    STEP_DATA,
    STEP_SUM
} ParserState_e;

static ParserState_e rx_state = STEP_HEADER1;
static uint8_t rx_id = 0;
static uint8_t rx_len = 0;
static uint8_t rx_cnt = 0;
static uint8_t rx_sum = 0;
static uint8_t rx_buf[20]; // 最大数据长度缓存

void IMU_Init(IMU_TypeDef *imu) {
    imu->yaw = 0.0f;
    imu->pitch = 0.0f;
    imu->roll = 0.0f;
}

/**
 * @brief  解析 ATK-MS901M 传来的单个字节 (串口接收中断内调用)
 * @param  byte: 串口收到的一个字节 (8位)
 */
void IMU_ParseByte(uint8_t byte) {
    switch(rx_state) {
        case STEP_HEADER1:
            if(byte == 0x55) { 
                rx_state = STEP_HEADER2; 
                rx_sum = byte; 
            }
            break;
            
        case STEP_HEADER2:
            if(byte == 0x55) { 
                rx_state = STEP_ID; 
                rx_sum += byte; 
            } else { 
                rx_state = STEP_HEADER1; // 头不对，重新找
            }
            break;
            
        case STEP_ID:
            rx_id = byte; 
            rx_sum += byte; 
            rx_state = STEP_LEN;
            break;
            
        case STEP_LEN:
            rx_len = byte; 
            rx_sum += byte; 
            rx_cnt = 0;
            // 如果数据长度异常，放弃该包防止数组越界
            if(rx_len > 20) rx_state = STEP_HEADER1; 
            else rx_state = STEP_DATA;
            break;
            
        case STEP_DATA:
            rx_buf[rx_cnt++] = byte; 
            rx_sum += byte;
            if(rx_cnt >= rx_len) {
                rx_state = STEP_SUM; // 数据收够了，等校验和
            }
            break;
            
        case STEP_SUM:
            // 校验和匹配，确保数据没有在传输中发生错乱
            if(rx_sum == byte) {
                
                // 协议类型 0x01：主动上报姿态角数据帧
                if(rx_id == 0x01) {
                    // 根据手册公式：Yaw(float) = (float)((int16_t)(YawH << 8) | YawL) / 32768 * 180
                    // 数据段顺序: RollL, RollH, PitchL, PitchH, YawL(buf[4]), YawH(buf[5])
                    
                    int16_t yaw_raw = (int16_t)((rx_buf[5] << 8) | rx_buf[4]);
                    my_car_imu.yaw = (float)yaw_raw / 32768.0f * 180.0f;
                    
                    // 可选：如果你后续想用车身倾斜做防护，也可以解算 Roll 和 Pitch
                    // int16_t roll_raw = (int16_t)((rx_buf[1] << 8) | rx_buf[0]);
                    // my_car_imu.roll = (float)roll_raw / 32768.0f * 180.0f;
                }
                // 如果以后需要处理 0x03 陀螺仪角速度数据，在此处添加 else if (rx_id == 0x03) 即可
            }
            
            // 无论校验对错，状态机重置，准备迎接下一帧
            rx_state = STEP_HEADER1; 
            break;
    }
}