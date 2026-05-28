#include "app_task.h"

#include "app_route.h"

volatile uint8_t task1_state = TASK1_IDLE;
volatile float task1_distance_cm = 0.0f;
volatile float task1_target_yaw = 0.0f;
volatile float task1_heading_error = 0.0f;
volatile int16_t task1_base_cmd = 0;
volatile int16_t task1_turn_cmd = 0;
volatile int16_t task1_left_cmd = 0;
volatile int16_t task1_right_cmd = 0;
volatile uint8_t task1_black_detected = 0U;
volatile uint8_t task1_black_confirm_count = 0U;
volatile uint8_t task1_done = 0U;
volatile uint8_t task1_finish_reason = 0U;

void AppTask_Init(void)
{
    task1_state = TASK1_IDLE;
    task1_distance_cm = 0.0f;
    task1_target_yaw = 0.0f;
    task1_heading_error = 0.0f;
    task1_base_cmd = 0;
    task1_turn_cmd = 0;
    task1_left_cmd = 0;
    task1_right_cmd = 0;
    task1_black_detected = 0U;
    task1_black_confirm_count = 0U;
    task1_done = 0U;
    task1_finish_reason = 0U;
}

void AppTask_ResetForStart(uint8_t task_id)
{
    (void)task_id;

    task1_state = TASK1_IDLE;
    task1_distance_cm = 0.0f;
    task1_target_yaw = 0.0f;
    task1_heading_error = 0.0f;
    task1_base_cmd = 0;
    task1_turn_cmd = 0;
    task1_left_cmd = 0;
    task1_right_cmd = 0;
    task1_black_detected = 0U;
    task1_black_confirm_count = 0U;
    task1_done = 0U;
    task1_finish_reason = 0U;
}

void AppTask_MarkTask1Run(void)
{
    task1_state = TASK1_RUN;
    task1_black_detected = 0U;
    task1_black_confirm_count = 0U;
    task1_done = 0U;
    task1_finish_reason = 0U;
}

void AppTask_MarkTask1Done(void)
{
    task1_state = TASK1_DONE;
    task1_done = 1U;
}
