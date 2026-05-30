#include "app_runner.h"

#include "Algorithm/motor.h"
#include "app_blind.h"
#include "app_event.h"
#include "app_line.h"
#include "app_mode.h"
#include "app_route.h"
#include "app_sensor.h"
#include "app_task.h"
#include "app_time.h"
#include "encoder.h"

typedef enum {
    RUNNER_IDLE = 0,
    RUNNER_SEG_START,
    RUNNER_SEG_RUNNING,
    RUNNER_SEG_EVENT,
    RUNNER_FINISHED,
    RUNNER_ERROR
} runner_state_t;

#define ROUTE_FINISH_NONE       (0U)
#define ROUTE_FINISH_DONE       (1U)
#define ROUTE_FINISH_TIMEOUT    (2U)
#define ROUTE_FINISH_LINE_LOST  (3U)
#define ROUTE_FINISH_IMU_LOST   (4U)
#define ROUTE_FAIL_IMU          (ROUTE_FINISH_IMU_LOST)

volatile uint8_t run_active = 0U;
volatile uint8_t run_finished = 0U;
volatile uint8_t route_finish_reason = ROUTE_FINISH_NONE;
volatile uint8_t route_error_code = 0U;
volatile uint32_t route_run_ticks = 0U;
volatile float route_final_distance_cm = 0.0f;
volatile float route_final_yaw = 0.0f;
volatile int16_t route_max_heading_error = 0;
volatile uint8_t route_last_node = NODE_NONE;

volatile uint8_t t2_test_done = 0U;
volatile uint8_t t2_finish_reason = 0U;
volatile float t2_final_distance_cm = 0.0f;
volatile float t2_final_yaw = 0.0f;
volatile int16_t t2_max_heading_error = 0;
volatile uint8_t t3_test_done = 0U;
volatile uint8_t t3_finish_reason = 0U;
volatile float t3_final_distance_cm = 0.0f;
volatile float t3_final_yaw = 0.0f;
volatile int16_t t3_max_heading_error = 0;
volatile uint8_t t4_test_done = 0U;
volatile uint8_t t4_finish_reason = 0U;
volatile float t4_final_distance_cm = 0.0f;
volatile float t4_final_yaw = 0.0f;
volatile int16_t t4_max_heading_error = 0;

static runner_state_t runner_state = RUNNER_IDLE;
static uint8_t runner_task_id = 1U;
static uint32_t runner_start_tick = 0U;
static uint32_t runner_segment_start_tick = 0U;
static uint32_t runner_imu_invalid_start_tick = 0U;
static const AppRouteSegment *runner_event_segment = 0;
static uint8_t runner_event_started = 0U;

static int16_t Runner_AbsI16(int16_t value)
{
    return (value >= 0) ? value : (int16_t)(-value);
}

static void Runner_ResetResults(void)
{
    route_finish_reason = ROUTE_FINISH_NONE;
    route_error_code = 0U;
    route_run_ticks = 0U;
    route_final_distance_cm = 0.0f;
    route_final_yaw = 0.0f;
    route_max_heading_error = 0;
    route_last_node = NODE_NONE;

    t2_test_done = 0U;
    t2_finish_reason = 0U;
    t2_final_distance_cm = 0.0f;
    t2_final_yaw = 0.0f;
    t2_max_heading_error = 0;
    t3_test_done = 0U;
    t3_finish_reason = 0U;
    t3_final_distance_cm = 0.0f;
    t3_final_yaw = 0.0f;
    t3_max_heading_error = 0;
    t4_test_done = 0U;
    t4_finish_reason = 0U;
    t4_final_distance_cm = 0.0f;
    t4_final_yaw = 0.0f;
    t4_max_heading_error = 0;
}

static void Runner_UpdateStats(void)
{
    int16_t abs_error = Runner_AbsI16((int16_t)blind_heading_error);

    route_run_ticks = app_millis() - runner_start_tick;
    route_final_distance_cm = encoder_distance_cm;
    route_final_yaw = gyro_yaw_filtered;
    if (abs_error > route_max_heading_error) {
        route_max_heading_error = abs_error;
    }
}

static void Runner_WriteTaskResult(uint8_t finish_reason)
{
    if (runner_task_id == 2U) {
        t2_test_done = 1U;
        t2_finish_reason = finish_reason;
        t2_final_distance_cm = route_final_distance_cm;
        t2_final_yaw = route_final_yaw;
        t2_max_heading_error = route_max_heading_error;
    } else if (runner_task_id == 3U) {
        t3_test_done = 1U;
        t3_finish_reason = finish_reason;
        t3_final_distance_cm = route_final_distance_cm;
        t3_final_yaw = route_final_yaw;
        t3_max_heading_error = route_max_heading_error;
    } else if (runner_task_id == 4U) {
        t4_test_done = 1U;
        t4_finish_reason = finish_reason;
        t4_final_distance_cm = route_final_distance_cm;
        t4_final_yaw = route_final_yaw;
        t4_max_heading_error = route_max_heading_error;
    }
}

static void Runner_FinishRoute(uint8_t finish_reason, uint8_t error_code)
{
    Motor_Stop();
    AppBlind_Stop();
    AppLine_Stop();
    Runner_UpdateStats();
    route_finish_reason = finish_reason;
    route_error_code = error_code;
    route_finished = 1U;
    run_finished = 1U;
    run_active = 0U;
    runner_state = RUNNER_FINISHED;
    AppEvent_FinishHint();

    if (runner_task_id == 1U) {
        AppTask_MarkTask1Done();
    } else {
        Runner_WriteTaskResult(finish_reason);
    }

    system_mode = SYS_FINISHED;
}

static void Runner_AdvanceAfterSegment(void)
{
    AppRoute_AdvanceSegment();
    runner_state = (route_finished != 0U) ? RUNNER_FINISHED :
        RUNNER_SEG_START;
}

void AppRunner_Init(void)
{
    run_active = 0U;
    run_finished = 0U;
    runner_state = RUNNER_IDLE;
    runner_task_id = 1U;
    runner_start_tick = 0U;
    runner_segment_start_tick = 0U;
    runner_imu_invalid_start_tick = 0U;
    runner_event_segment = 0;
    runner_event_started = 0U;
    Runner_ResetResults();
}

uint8_t AppRunner_Start(uint8_t task_id)
{
    if (AppRoute_LoadTask(task_id) == 0U) {
        run_active = 0U;
        run_finished = 0U;
        return 0U;
    }

    AppTask_ResetForStart(task_id);
    Runner_ResetResults();
    run_active = 1U;
    run_finished = 0U;
    runner_state = RUNNER_SEG_START;
    runner_task_id = task_id;
    runner_start_tick = app_millis();
    runner_segment_start_tick = runner_start_tick;
    runner_imu_invalid_start_tick = 0U;
    runner_event_segment = 0;
    runner_event_started = 0U;
    return 1U;
}

void AppRunner_Stop(void)
{
    Motor_Stop();
    AppBlind_Stop();
    AppLine_Stop();
    run_active = 0U;
    runner_state = RUNNER_IDLE;
    runner_segment_start_tick = 0U;
    runner_imu_invalid_start_tick = 0U;
    runner_event_started = 0U;
}

void AppRunner_Task(void)
{
    const AppRouteSegment *segment;

    if (system_mode != SYS_RUN || run_active == 0U || run_finished != 0U) {
        return;
    }

    Runner_UpdateStats();
    segment = AppRoute_CurrentSegment();
    if (segment == 0) {
        Runner_FinishRoute(ROUTE_FINISH_DONE, 0U);
        return;
    }

    if (runner_state == RUNNER_SEG_START) {
        runner_event_started = 0U;
        runner_segment_start_tick = app_millis();
        runner_imu_invalid_start_tick = 0U;

        if (segment->type == SEG_BLIND) {
            AppBlind_EnableTask1BlackStop(
                (segment->flags & SEG_FLAG_TASK1_BLACK_STOP) != 0U);
            AppBlind_StartSegment(segment);
            runner_state = RUNNER_SEG_RUNNING;
        } else if (segment->type == SEG_LINE) {
            AppLine_StartSegment(segment);
            runner_state = RUNNER_SEG_RUNNING;
        } else if (segment->type == SEG_NODE_EVENT) {
            runner_event_segment = segment;
            runner_state = RUNNER_SEG_EVENT;
        } else if (segment->type == SEG_STOP) {
            Runner_FinishRoute(ROUTE_FINISH_DONE, 0U);
        } else {
            Runner_FinishRoute(ROUTE_FINISH_NONE, 1U);
        }
        return;
    }

    if (runner_state == RUNNER_SEG_RUNNING) {
        if (segment->type == SEG_BLIND) {
            AppBlind_Task();
            if (imu_valid == 0U) {
                route_error_code = ROUTE_FINISH_IMU_LOST;
                if (runner_imu_invalid_start_tick == 0U) {
                    runner_imu_invalid_start_tick = app_millis();
                }

                if (imu_protection_enabled_dbg != 0U &&
                    (uint32_t)(app_millis() -
                    runner_imu_invalid_start_tick) > 150U) {
                    Runner_FinishRoute(ROUTE_FAIL_IMU,
                        ROUTE_FINISH_IMU_LOST);
                    return;
                }
            } else {
                runner_imu_invalid_start_tick = 0U;
            }

            if (AppBlind_IsDone() != 0U) {
                Motor_Stop();
                if (segment->node_hint_enable != 0U) {
                    runner_event_segment = segment;
                    runner_state = RUNNER_SEG_EVENT;
                } else {
                    Runner_AdvanceAfterSegment();
                }
            }
        } else if (segment->type == SEG_LINE) {
            AppLine_Task();
            AppLine_ControllerTask();
            if (AppLine_IsFailed() != 0U) {
                Runner_FinishRoute(ROUTE_FINISH_LINE_LOST,
                    ROUTE_FINISH_LINE_LOST);
                return;
            }

            if (AppLine_IsDone() != 0U) {
                Motor_Stop();
                if (segment->node_hint_enable != 0U) {
                    runner_event_segment = segment;
                    runner_state = RUNNER_SEG_EVENT;
                } else {
                    Runner_AdvanceAfterSegment();
                }
            }
        }
        return;
    }

    if (runner_state == RUNNER_SEG_EVENT) {
        Motor_Stop();

        if (runner_event_started == 0U) {
            if (runner_event_segment != 0) {
                route_last_node = (uint8_t)runner_event_segment->to_node;
                AppEvent_NodeHint(route_last_node);
            }
            runner_event_started = 1U;
            return;
        }

        if (AppEvent_IsBusy() == 0U) {
            runner_event_started = 0U;
            Runner_AdvanceAfterSegment();
        }
        return;
    }
}
