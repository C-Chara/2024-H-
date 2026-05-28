#ifndef HAL_H_
#define HAL_H_

#include <stdint.h>

// =======================================================
// 硬件抽象层 (Hardware Abstraction Layer) 接口定义
// =======================================================

// --- 1. 声光指示接口 (适配题目：经过节点需有声光提示) ---
void HAL_Beep_On(void);
void HAL_Beep_Off(void);

// --- 2. 编码器接口 (适配题目：精准计算 125.66cm 和 128.06cm) ---
// 返回自上次调用以来，左/右轮编码器增加的脉冲数
int32_t HAL_Get_Left_Encoder_Delta(void);
int32_t HAL_Get_Right_Encoder_Delta(void);

// --- 3. 灰度传感器接口 (适配题目：循线) ---
// 从 I2C 总线读取 8 个字节的模拟量数据存入 buffer
void HAL_Sensor_Read_I2C(uint8_t *buffer, uint8_t length);

// --- 4. 电机 PWM 与方向控制接口 ---
// 传入带有正负号的 PWM 值 (例如 -10000 到 10000)
void HAL_Motor_Set_Left(int32_t pwm_value);
void HAL_Motor_Set_Right(int32_t pwm_value);

// --- 5. 陀螺仪 IMU 接口 (适配题目：任务3/4对角线盲走) ---
// 获取当前车头的绝对偏航角 (Yaw)
float HAL_IMU_Get_Yaw(void);

#endif /* HAL_H_ */