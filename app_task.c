#include "app_task.h"

#include "app_blind.h"
#include "app_route.h"
#include "app_sensor.h"
#include "app_time.h"
#include "gyro.h"

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
volatile uint8_t t1_test_done = 0U;
volatile uint8_t t1_finish_reason = 0U;
volatile float t1_final_distance_cm = 0.0f;
volatile float t1_final_yaw = 0.0f;
volatile int16_t t1_max_heading_error = 0;
volatile uint32_t t1_run_ticks = 0U;

static uint32_t t1_start_tick = 0U;

static int16_t AppTask_AbsI16(int16_t value)
{
    return (value >= 0) ? value : (int16_t)(-value);
}

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
    t1_test_done = 0U;
    t1_finish_reason = 0U;
    t1_final_distance_cm = 0.0f;
    t1_final_yaw = 0.0f;
    t1_max_heading_error = 0;
    t1_run_ticks = 0U;
    t1_start_tick = 0U;
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
    t1_test_done = 0U;
    t1_finish_reason = 0U;
    t1_final_distance_cm = 0.0f;
    t1_final_yaw = 0.0f;
    t1_max_heading_error = 0;
    t1_run_ticks = 0U;
    t1_start_tick = 0U;
}

void AppTask_MarkTask1Run(void)
{
    task1_state = TASK1_RUN;
    task1_black_detected = 0U;
    task1_black_confirm_count = 0U;
    task1_done = 0U;
    task1_finish_reason = 0U;
    t1_test_done = 0U;
    t1_finish_reason = 0U;
    t1_final_distance_cm = 0.0f;
    t1_final_yaw = 0.0f;
    t1_max_heading_error = 0;
    t1_run_ticks = 0U;
    t1_start_tick = app_millis();
}

void AppTask_MarkTask1Done(void)
{
    task1_state = TASK1_DONE;
    task1_done = 1U;
    t1_test_done = 1U;
    t1_finish_reason = task1_finish_reason;
    t1_final_distance_cm = blind_distance_cm;
    t1_final_yaw = gyro_yaw_filtered;
    t1_run_ticks = app_millis() - t1_start_tick;
}

void AppTask_UpdateTask1RunStats(void)
{
    int16_t abs_error = AppTask_AbsI16((int16_t)blind_heading_error);

    if (task1_state == TASK1_RUN) {
        t1_run_ticks = app_millis() - t1_start_tick;
        if (abs_error > t1_max_heading_error) {
            t1_max_heading_error = abs_error;
        }
    }
}
