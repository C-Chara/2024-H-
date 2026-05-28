#ifndef ALGORITHM_PID_H_
#define ALGORITHM_PID_H_

// 定义 PID 控制器结构体
typedef struct {
    // 基础参数 (需要根据实车调试)
    float Kp;           // 比例系数
    float Ki;           // 积分系数
    float Kd;           // 微分系数

    // 状态变量
    float target;       // 目标设定值 (例如：目标速度，或目标偏差为0)
    float current;      // 当前实际测量值
    float error;        // 当前误差
    float last_error;   // 上一次计算时的误差
    float integral;     // 误差的累积积分值

    // 保护限制 (防止积分爆炸或电机满载烧毁)
    float max_integral; // 积分限幅
    float max_output;   // 输出限幅
    
    // 最终计算结果
    float output;       // 输出给电机的控制量
} PID_TypeDef;

// 外部可调用的函数声明
void PID_Init(PID_TypeDef *pid, float kp, float ki, float kd, float max_out, float max_i);
float PID_Calculate(PID_TypeDef *pid, float target, float current);

#endif /* ALGORITHM_PID_H_ */