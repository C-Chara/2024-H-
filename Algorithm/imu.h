#ifndef ALGORITHM_IMU_H_
#define ALGORITHM_IMU_H_

#include <stdint.h>

// IMU 结构体，专门存储姿态数据
typedef struct {
    float yaw;      // 偏航角：车头偏左偏右的角度
    float pitch;    // 俯仰角
    float roll;     // 横滚角
} IMU_TypeDef;

// 外部声明一个全局的 IMU 变量
extern IMU_TypeDef my_car_imu;

// 初始化函数
void IMU_Init(IMU_TypeDef *imu);

// 🔥 新增：核心字节解析函数（供硬件串口接收中断逐字节调用）
void IMU_ParseByte(uint8_t byte);

#endif /* ALGORITHM_IMU_H_ */