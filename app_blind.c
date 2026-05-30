#include "app_blind.h"

#include "Algorithm/gray.h"
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
volatile uint8_t blind_segment_started = 0U;
volatile float blind_last_error_dbg = 0.0f;
volatile uint8_t blind_ctrl_active = 0U;
volatile float blind_yaw_now_dbg = 0.0f;
volatile uint8_t blind_finish_enabled_dbg = 1U;
volatile uint8_t blind_stop_on_black_active = 0U;
volatile uint8_t blind_finish_by_gray = 0U;
volatile uint8_t blind_black_confirm_count = 0U;
volatile uint8_t blind_raw_black_count = 0U;
volatile uint8_t blind_raw_black_hit = 0U;
volatile uint8_t blind_abs_yaw_active = 0U;
volatile uint8_t blind_aligning_dbg = 0U;

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

static int16_t AppBlind_ClampTurn(int32_t value)
{
    if (value > BLIND_MAX_SPEED) {
        return BLIND_MAX_SPEED;
    }

    if (value < -BLIND_MAX_SPEED) {
        return (int16_t)(-BLIND_MAX_SPEED);
    }

    return (int16_t)value;
}

static uint8_t AppBlind_CheckRawBlack(void)
{
    const uint8_t gray_values[8] = {
        gray_value_0, gray_value_1, gray_value_2, gray_value_3,
        gray_value_4, gray_value_5, gray_value_6, gray_value_7
    };

    blind_raw_black_count = 0U;
    if (gray_valid == 0U) {
        blind_raw_black_hit = 0U;
        return 0U;
    }

    for (uint8_t i = 0U; i < 8U; i++) {
        uint8_t dark;
#if GRAY_BLACK_IS_LOW
        dark = (uint8_t)(255U - gray_values[i]);
#else
        dark = gray_values[i];
#endif
        if (dark >= T2_BLACK_SENSOR_DARK_TH) {
            blind_raw_black_count++;
        }
    }

    blind_raw_black_hit =
        (blind_raw_black_count >= T2_BLACK_SENSOR_NEED) ? 1U : 0U;
    return blind_raw_black_hit;
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
    blind_segment_started = 0U;
    blind_last_error_dbg = 0.0f;
    blind_ctrl_active = 0U;
    blind_yaw_now_dbg = 0.0f;
    blind_finish_enabled_dbg = 1U;
    blind_stop_on_black_active = 0U;
    blind_finish_by_gray = 0U;
    blind_black_confirm_count = 0U;
    blind_raw_black_count = 0U;
    blind_raw_black_hit = 0U;
    blind_abs_yaw_active = 0U;
    blind_aligning_dbg = 0U;
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
    blind_abs_yaw_active = (segment != 0 &&
        ((segment->flags & SEG_FLAG_ABS_ROUTE_YAW) != 0U)) ? 1U : 0U;
    blind_target_yaw = (blind_abs_yaw_active != 0U) ?
        AppBlind_NormalizeAngle(route_start_yaw + yaw_offset_deg) :
        AppBlind_NormalizeAngle(gyro_yaw + yaw_offset_deg);
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
    blind_segment_started = 1U;
    blind_ctrl_active = 0U;
    blind_last_error_dbg = blind_last_heading_error;
    blind_stop_on_black_active = (segment != 0 &&
        ((segment->flags & SEG_FLAG_BLIND_STOP_ON_BLACK) != 0U)) ? 1U : 0U;
    blind_finish_by_gray = 0U;
    blind_black_confirm_count = 0U;
    blind_raw_black_count = 0U;
    blind_raw_black_hit = 0U;
    blind_aligning_dbg = blind_abs_yaw_active;

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
    int32_t turn_i32;
    int32_t left_i32;
    int32_t right_i32;

    if (blind_active == 0U || blind_done != 0U) {
        blind_ctrl_active = 0U;
        return;
    }

    blind_ctrl_active = 1U;
    Encoder_Task();
    blind_distance_cm = Encoder_GetDistanceCm();

    blind_yaw_now_dbg = gyro_yaw;
    blind_heading_error = AppBlind_NormalizeAngle(blind_target_yaw - gyro_yaw);
    d_error = blind_heading_error - blind_last_heading_error;
    blind_last_heading_error = blind_heading_error;
    blind_last_error_dbg = blind_last_heading_error;

    if (blind_stop_on_black_active != 0U) {
        if (AppBlind_CheckRawBlack() != 0U) {
            if (blind_black_confirm_count < 255U) {
                blind_black_confirm_count++;
            }
        } else {
            blind_black_confirm_count = 0U;
        }

        if (blind_black_confirm_count >= T2_BLIND_BLACK_CONFIRM_NEED) {
            Motor_Stop();
            blind_left_cmd = 0;
            blind_right_cmd = 0;
            blind_turn_cmd = 0;
            blind_finish_by_gray = 1U;
            blind_done = 1U;
            blind_active = 0U;
            blind_segment_started = 0U;
            blind_ctrl_active = 0U;
            return;
        }
    }

    if (blind_finish_enabled_dbg != 0U && blind_timeout_ms > 0U &&
        (uint32_t)(app_millis() - blind_start_tick) > blind_timeout_ms) {
        Motor_Stop();
        blind_left_cmd = 0;
        blind_right_cmd = 0;
        blind_turn_cmd = 0;
        blind_done = 1U;
        blind_active = 0U;
        blind_segment_started = 0U;
        blind_ctrl_active = 0U;
        task1_finish_reason = 3U;
        return;
    }

    if (imu_valid == 0U) {
        blind_base_cmd = BLIND_SLOW_SPEED;
    }

    if (blind_abs_yaw_active != 0U &&
        (blind_aligning_dbg != 0U ||
         blind_heading_error > BLIND_ALIGN_ENTER_DEG ||
         blind_heading_error < -BLIND_ALIGN_ENTER_DEG)) {
        if (blind_heading_error <= BLIND_ALIGN_EXIT_DEG &&
            blind_heading_error >= -BLIND_ALIGN_EXIT_DEG) {
            blind_aligning_dbg = 0U;
        } else {
            turn_float = BLIND_ALIGN_TURN_GAIN * blind_heading_error;
            blind_turn_cmd = AppBlind_ClampTurn((int32_t)turn_float);
            blind_base_cmd = BLIND_ALIGN_BASE_CMD;
            blind_left_cmd = AppBlind_ClampForward(
                (int32_t)blind_base_cmd - (int32_t)blind_turn_cmd);
            blind_right_cmd = AppBlind_ClampForward(
                (int32_t)blind_base_cmd + (int32_t)blind_turn_cmd);
            Motor_SetLeft(blind_left_cmd);
            Motor_SetRight(blind_right_cmd);
            return;
        }
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

    if (blind_finish_enabled_dbg != 0U &&
        (blind_distance_cm >= blind_target_distance_cm ||
         (blind_task1_black_stop_enable != 0U &&
          task1_black_confirm_count >= BLACK_STOP_CONFIRM_COUNT))) {
        Motor_Stop();
        blind_left_cmd = 0;
        blind_right_cmd = 0;
        blind_turn_cmd = 0;
        blind_done = 1U;
        blind_active = 0U;
        blind_segment_started = 0U;
        blind_ctrl_active = 0U;

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

    turn_float = BLIND_KP_YAW * blind_heading_error +
        BLIND_KD_YAW * d_error;
    turn_i32 = (int32_t)turn_float;
    blind_turn_cmd = AppBlind_ClampTurn(turn_i32);

    left_i32 = (int32_t)blind_base_cmd - (int32_t)blind_turn_cmd;
    right_i32 = (int32_t)blind_base_cmd + (int32_t)blind_turn_cmd;
    blind_left_cmd = AppBlind_ClampForward(left_i32);
    blind_right_cmd = AppBlind_ClampForward(right_i32);

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
    blind_segment_started = 0U;
    blind_ctrl_active = 0U;
    blind_left_cmd = 0;
    blind_right_cmd = 0;
    blind_turn_cmd = 0;
    blind_stop_on_black_active = 0U;
    blind_aligning_dbg = 0U;
}
