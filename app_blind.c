#include "app_blind.h"

#include "Algorithm/motor.h"
#include "app_config.h"
#include "app_line.h"
#include "app_sensor.h"
#include "app_task.h"
#include "app_time.h"
#include "encoder.h"
#include "gyro.h"

volatile float blind_distance_cm = 0.0f;
volatile float blind_target_distance_cm = 0.0f;
volatile float blind_target_yaw = 0.0f;
volatile float blind_heading_error = 0.0f;
volatile int16_t blind_base_cmd = 0;
volatile int16_t blind_turn_cmd = 0;
volatile int16_t blind_left_cmd = 0;
volatile int16_t blind_right_cmd = 0;

static uint8_t blind_active = 0U;
static uint8_t blind_done = 0U;
static uint8_t blind_task1_black_stop_enable = 0U;
static uint8_t blind_next_task1_black_stop_enable = 0U;
static float blind_last_heading_error = 0.0f;
static int16_t blind_segment_base_speed = BLIND_FAST_SPEED;
static uint32_t blind_timeout_ms = SEG_TIMEOUT_BLIND_MS;
static uint32_t blind_start_tick = 0U;

static float AppBlind_NormalizeAngle(float angle)
{
    while (angle > 180.0f) {
        angle -= 360.0f;
    }

    while (angle < -180.0f) {
        angle += 360.0f;
    }

    return angle;
}

static int16_t AppBlind_ClampForward(int32_t value)
{
    if (value < 0) {
        return 0;
    }

    if (value > BLIND_MAX_SPEED) {
        return BLIND_MAX_SPEED;
    }

    return (int16_t)value;
}

void AppBlind_Init(void)
{
    blind_active = 0U;
    blind_done = 0U;
    blind_task1_black_stop_enable = 0U;
    blind_next_task1_black_stop_enable = 0U;
    blind_distance_cm = 0.0f;
    blind_target_distance_cm = 0.0f;
    blind_target_yaw = 0.0f;
    blind_heading_error = 0.0f;
    blind_base_cmd = 0;
    blind_turn_cmd = 0;
    blind_left_cmd = 0;
    blind_right_cmd = 0;
    blind_last_heading_error = 0.0f;
    blind_segment_base_speed = BLIND_FAST_SPEED;
    blind_timeout_ms = SEG_TIMEOUT_BLIND_MS;
    blind_start_tick = 0U;
}

void AppBlind_EnableTask1BlackStop(uint8_t enable)
{
    blind_next_task1_black_stop_enable = (enable != 0U) ? 1U : 0U;
}

void AppBlind_Start(float target_distance_cm, float yaw_offset_deg)
{
    blind_segment_base_speed = BLIND_FAST_SPEED;
    blind_timeout_ms = SEG_TIMEOUT_BLIND_MS;
    blind_start_tick = 0U;
    AppBlind_StartSegment(&(AppRouteSegment) {
        SEG_BLIND, NODE_NONE, NODE_NONE, target_distance_cm, yaw_offset_deg,
        BLIND_FAST_SPEED, SEG_TIMEOUT_BLIND_MS, LINE_EXIT_BY_DISTANCE,
        0.0f, 0U, 0U
    });
}

void AppBlind_StartSegment(const AppRouteSegment *segment)
{
    float target_distance_cm = (segment != 0) ? segment->distance_cm : 0.0f;
    float yaw_offset_deg = (segment != 0) ? segment->yaw_offset_deg : 0.0f;

    Encoder_ResetDistance();

    blind_distance_cm = 0.0f;
    blind_target_distance_cm = target_distance_cm;
    blind_target_yaw = AppBlind_NormalizeAngle(gyro_yaw_filtered +
        yaw_offset_deg);
    blind_heading_error = 0.0f;
    blind_base_cmd = BLIND_FAST_SPEED;
    blind_turn_cmd = 0;
    blind_left_cmd = 0;
    blind_right_cmd = 0;
    blind_last_heading_error = 0.0f;
    blind_segment_base_speed = (segment != 0 && segment->base_speed > 0.0f) ?
        (int16_t)segment->base_speed : BLIND_FAST_SPEED;
    blind_timeout_ms = (segment != 0) ? segment->timeout_ms :
        SEG_TIMEOUT_BLIND_MS;
    blind_start_tick = app_millis();
    blind_task1_black_stop_enable = blind_next_task1_black_stop_enable;
    blind_next_task1_black_stop_enable = 0U;
    blind_done = 0U;
    blind_active = 1U;

    if (blind_task1_black_stop_enable != 0U) {
        AppTask_MarkTask1Run();
        task1_distance_cm = blind_distance_cm;
        task1_target_yaw = blind_target_yaw;
        task1_heading_error = blind_heading_error;
        task1_base_cmd = blind_base_cmd;
        task1_turn_cmd = blind_turn_cmd;
        task1_left_cmd = blind_left_cmd;
        task1_right_cmd = blind_right_cmd;
        AppTask_UpdateTask1RunStats();
    }
}

void AppBlind_Task(void)
{
    float d_error;
    float turn_float;

    if (blind_active == 0U || blind_done != 0U) {
        return;
    }

    Encoder_Task();
    blind_distance_cm = Encoder_GetDistanceCm();

    if (blind_timeout_ms > 0U &&
        (uint32_t)(app_millis() - blind_start_tick) > blind_timeout_ms) {
        Motor_Stop();
        blind_left_cmd = 0;
        blind_right_cmd = 0;
        blind_turn_cmd = 0;
        blind_done = 1U;
        blind_active = 0U;
        task1_finish_reason = 3U;
        return;
    }

    if (imu_valid == 0U) {
        blind_base_cmd = BLIND_SLOW_SPEED;
    }

    if ((blind_target_distance_cm - blind_distance_cm) <=
        BLIND_SLOW_DOWN_DISTANCE_CM) {
        blind_base_cmd = BLIND_SLOW_SPEED;
    } else {
        blind_base_cmd = blind_segment_base_speed;
    }

    if (blind_task1_black_stop_enable != 0U &&
        blind_distance_cm >= TASK1_BLACK_STOP_ENABLE_CM &&
        black_detected != 0U) {
        if (task1_black_confirm_count < 255U) {
            task1_black_confirm_count++;
        }
        task1_black_detected = 1U;
    } else if (blind_task1_black_stop_enable != 0U) {
        task1_black_confirm_count = 0U;
        task1_black_detected = 0U;
    }

    if (blind_distance_cm >= blind_target_distance_cm ||
        (blind_task1_black_stop_enable != 0U &&
         task1_black_confirm_count >= BLACK_STOP_CONFIRM_COUNT)) {
        Motor_Stop();
        blind_left_cmd = 0;
        blind_right_cmd = 0;
        blind_turn_cmd = 0;
        blind_done = 1U;
        blind_active = 0U;

        if (blind_task1_black_stop_enable != 0U) {
            task1_state = TASK1_STOP;
            task1_distance_cm = blind_distance_cm;
            task1_target_yaw = blind_target_yaw;
            task1_heading_error = blind_heading_error;
            task1_base_cmd = blind_base_cmd;
            task1_turn_cmd = blind_turn_cmd;
            task1_left_cmd = blind_left_cmd;
            task1_right_cmd = blind_right_cmd;
            task1_finish_reason =
                (blind_distance_cm >= blind_target_distance_cm) ? 1U : 2U;
        }
        return;
    }

    blind_heading_error = AppBlind_NormalizeAngle(blind_target_yaw -
        gyro_yaw_filtered);
    d_error = blind_heading_error - blind_last_heading_error;
    blind_last_heading_error = blind_heading_error;
    turn_float = BLIND_KP_YAW * blind_heading_error +
        BLIND_KD_YAW * d_error;
    blind_turn_cmd = (int16_t)turn_float;

    blind_left_cmd = AppBlind_ClampForward((int32_t)blind_base_cmd -
        (int32_t)blind_turn_cmd);
    blind_right_cmd = AppBlind_ClampForward((int32_t)blind_base_cmd +
        (int32_t)blind_turn_cmd);

    Motor_SetLeft(blind_left_cmd);
    Motor_SetRight(blind_right_cmd);

    if (blind_task1_black_stop_enable != 0U) {
        task1_distance_cm = blind_distance_cm;
        task1_target_yaw = blind_target_yaw;
        task1_heading_error = blind_heading_error;
        task1_base_cmd = blind_base_cmd;
        task1_turn_cmd = blind_turn_cmd;
        task1_left_cmd = blind_left_cmd;
        task1_right_cmd = blind_right_cmd;
    }
}

uint8_t AppBlind_IsDone(void)
{
    return blind_done;
}

void AppBlind_Stop(void)
{
    Motor_Stop();
    blind_active = 0U;
    blind_done = 1U;
    blind_left_cmd = 0;
    blind_right_cmd = 0;
    blind_turn_cmd = 0;
}
