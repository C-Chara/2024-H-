#include "pid.h"

// 1. 初始化 PID 参数
void PID_Init(PID_TypeDef *pid, float kp, float ki, float kd, float max_out, float max_i) {
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
    
    pid->target = 0.0f;
    pid->current = 0.0f;
    pid->error = 0.0f;
    pid->last_error = 0.0f;
    pid->integral = 0.0f;
    
    pid->max_output = max_out;
    pid->max_integral = max_i;
    pid->output = 0.0f;
}

// 2. 位置式 PID 计算函数
float PID_Calculate(PID_TypeDef *pid, float target, float current) {
    // 第一步：更新目标设定值与当前实际测量值
    pid->target = target;
    pid->current = current;
    
    // 第二步：计算当前误差
    pid->error = pid->target - pid->current;
    
    // 第三步：误差累积 (积分项) 并进行严格限幅，防止车子因为长时间小误差导致积分爆炸(Windup)
    pid->integral += pid->error;
    if (pid->integral > pid->max_integral) {
        pid->integral = pid->max_integral;
    } else if (pid->integral < -pid->max_integral) {
        pid->integral = -pid->max_integral;
    }
    
    // 第四步：PID 核心算式 —— 比例(P) + 积分(I) + 微分(D)
    pid->output = (pid->Kp * pid->error) + 
                  (pid->Ki * pid->integral) + 
                  (pid->Kd * (pid->error - pid->last_error));
                  
    // 第五步：保存当前误差，留给下个周期的微分(D)计算使用
    pid->last_error = pid->error;
    
    // 第六步：输出限幅，确保算出来的数值不会超过电机能接收的 PWM 极限
    if (pid->output > pid->max_output) {
        pid->output = pid->max_output;
    } else if (pid->output < -pid->max_output) {
        pid->output = -pid->max_output;
    }
    
    return pid->output;
}