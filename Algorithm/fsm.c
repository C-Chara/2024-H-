#include "fsm.h"
#include "hal.h"
#include "imu.h"
#include "motor.h"

#include <math.h>

#define DIST_CURVE_AB        125.66f
#define DIST_DIAGONAL_AC     128.06f
#define BASE_SPEED           50.0f
#define TARGET_ANGLE         -38.66f
#define ANGLE_TOLERANCE_DEG  1.0f

static void FSM_UpdateDistance(FSM_TypeDef *fsm)
{
    int32_t left_delta = HAL_Get_Left_Encoder_Delta();
    int32_t right_delta = HAL_Get_Right_Encoder_Delta();
    float left_pulse = (left_delta >= 0) ? (float)left_delta : (float)(-left_delta);
    float right_pulse = (right_delta >= 0) ? (float)right_delta : (float)(-right_delta);
    float mean_pulse = (left_pulse + right_pulse) * 0.5f;

    fsm->total_distance_cm += mean_pulse * PULSE_TO_CM;
}

void FSM_Init(FSM_TypeDef *fsm)
{
    fsm->current_state = STATE_INIT;
    fsm->target_task = TASK_1_AB;
    fsm->node_count = 0;
    fsm->lap_count = 0;
    fsm->total_distance_cm = 0.0f;
    fsm->beep_counter = 0;

    Sensor_Init(&fsm->sensor);
    PID_Init(&fsm->pid, 1.5f, 0.0f, 0.2f, 100.0f, 50.0f);
    PID_Init(&fsm->angle_pid, 2.0f, 0.0f, 0.5f, 100.0f, 40.0f);
}

void FSM_SetTask(FSM_TypeDef *fsm, TaskType_e task)
{
    fsm->target_task = task;
    fsm->current_state = STATE_LINE_TRACKING;
    fsm->node_count = 0;
    fsm->lap_count = 0;
    fsm->total_distance_cm = 0.0f;
    fsm->beep_counter = 0;
}

void FSM_Run(FSM_TypeDef *fsm)
{
    float current_yaw = HAL_IMU_Get_Yaw();
    my_car_imu.yaw = current_yaw;

    FSM_UpdateDistance(fsm);

    if (fsm->beep_counter > 0) {
        fsm->beep_counter--;
        if (fsm->beep_counter == 0) {
            HAL_Beep_Off();
        }
    }

    switch (fsm->current_state) {
    case STATE_INIT:
        fsm->current_state = STATE_LINE_TRACKING;
        fsm->total_distance_cm = 0.0f;
        break;

    case STATE_LINE_TRACKING:
        Sensor_Update(&fsm->sensor);

        if ((fsm->target_task == TASK_1_AB || fsm->target_task == TASK_2_LOOP) &&
            fsm->total_distance_cm >= DIST_CURVE_AB) {
            fsm->current_state = STATE_NODE_DETECTED;
            break;
        }

        if (fsm->sensor.is_lost) {
            fsm->current_state = STATE_STOP;
        } else {
            float turn_comp = PID_Calculate(&fsm->pid, 0.0f, fsm->sensor.line_offset);
            Motor_SetSpeed(BASE_SPEED - turn_comp, BASE_SPEED + turn_comp);
        }
        break;

    case STATE_NODE_DETECTED:
        fsm->node_count++;
        fsm->total_distance_cm = 0.0f;
        HAL_Beep_On();
        fsm->beep_counter = 50;

        if (fsm->target_task == TASK_1_AB) {
            fsm->current_state = STATE_STOP;
        } else if (fsm->target_task == TASK_3_DIAGONAL ||
                   fsm->target_task == TASK_4_FOUR_LAPS) {
            fsm->current_state = STATE_ROTATING;
        } else {
            fsm->current_state = STATE_LINE_TRACKING;
        }
        break;

    case STATE_ROTATING: {
        float angle_error = TARGET_ANGLE - current_yaw;

        if (fabsf(angle_error) < ANGLE_TOLERANCE_DEG) {
            Motor_SetSpeed(0.0f, 0.0f);
            fsm->current_state = STATE_BLIND_STRAIGHT;
            fsm->total_distance_cm = 0.0f;
        } else {
            float rotate_comp = PID_Calculate(&fsm->angle_pid, TARGET_ANGLE, current_yaw);
            Motor_SetSpeed(-rotate_comp, rotate_comp);
        }
        break;
    }

    case STATE_BLIND_STRAIGHT:
        if (fsm->total_distance_cm >= DIST_DIAGONAL_AC) {
            fsm->current_state = STATE_NODE_DETECTED;
        } else {
            float straight_comp = PID_Calculate(&fsm->angle_pid, TARGET_ANGLE, current_yaw);
            Motor_SetSpeed(BASE_SPEED - straight_comp, BASE_SPEED + straight_comp);
        }
        break;

    case STATE_STOP:
    default:
        Motor_SetSpeed(0.0f, 0.0f);
        break;
    }
}
