#ifndef ALGORITHM_FSM_H_
#define ALGORITHM_FSM_H_

#include "pid.h"
#include "sensor.h"

// 🔥 新增：编码器脉冲转距离的换算比例
// 算法：(6.5 * 3.14159) / (13 * 20 * 4) = 0.01963 cm/pulse
#define PULSE_TO_CM  0.01963f

typedef enum {
    TASK_1_AB = 1,
    TASK_2_LOOP = 2,
    TASK_3_DIAGONAL = 3,
    TASK_4_FOUR_LAPS = 4
} TaskType_e;

typedef enum {
    STATE_INIT = 0,         
    STATE_BLIND_STRAIGHT,   
    STATE_LINE_TRACKING,    
    STATE_NODE_DETECTED,    
    STATE_ROTATING,         // 🔥 新增：原地旋转状态 (用于A点转向)
    STATE_STOP              
} CarState_e;

typedef struct {
    CarState_e current_state;   
    TaskType_e target_task;     
    
    int node_count;             
    int lap_count;              
    float total_distance_cm;    
    
    int beep_counter;           // 🔥 新增：蜂鸣器非阻塞倒计时
    
    PID_TypeDef pid;            // 巡线大脑
    PID_TypeDef angle_pid;      // 走直线/旋转的角度大脑
    
    Sensor_TypeDef sensor;      // 眼睛
    // IMU_TypeDef imu;         // 预留的陀螺仪结构体（等确认型号后加上）
} FSM_TypeDef;

void FSM_Init(FSM_TypeDef *fsm);
void FSM_Run(FSM_TypeDef *fsm);
void FSM_SetTask(FSM_TypeDef *fsm, TaskType_e task);

#endif
