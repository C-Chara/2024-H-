#include "app_line.h"

#include "Algorithm/gray.h"
#include "Algorithm/motor.h"
#include "app_config.h"
#include "app_time.h"
#include "encoder.h"

volatile int16_t line_error = 0;
volatile uint8_t line_found = 0U;
volatile uint16_t line_confidence = 0U;
volatile uint16_t line_dark_sum = 0U;
volatile uint8_t black_detected = 0U;
volatile uint8_t white_detected = 0U;
volatile uint8_t black_confirm_count = 0U;
volatile uint8_t white_confirm_count = 0U;
volatile uint16_t line_max_dark = 0U;
volatile uint8_t line_active_sensor_count = 0U;
volatile uint8_t line_lost_count = 0U;
volatile int16_t last_line_error = 0;
volatile int16_t line_turn_cmd = 0;
volatile int16_t line_left_cmd = 0;
volatile int16_t line_right_cmd = 0;
volatile uint8_t line_black_sensor_count = 0U;
volatile uint8_t line_gray_trigger_hit = 0U;
volatile uint8_t line_ever_found = 0U;
volatile uint32_t line_lost_start_tick = 0U;
volatile uint8_t line_exit_by_white = 0U;
volatile uint8_t line_end_on_lost_active = 0U;
volatile uint8_t line_ctrl_active = 0U;
volatile uint8_t line_fail_reason = 0U;

static uint8_t line_segment_active = 0U;
static uint8_t line_segment_done = 0U;
static uint8_t line_segment_failed = 0U;
static float line_target_distance_cm = 0.0f;
static float line_min_black_detect_cm = 0.0f;
static float line_base_speed = LINE_BASE_SPEED;
static uint32_t line_timeout_ms = SEG_TIMEOUT_LINE_MS;
static uint32_t line_start_tick = 0U;
static line_exit_mode_t line_exit_mode = LINE_EXIT_BY_DISTANCE_OR_NODE;
static int16_t line_prev_control_error = 0;

static const int16_t line_weights[8] = {
    -350, -250, -150, -50, 50, 150, 250, 350
};

static int16_t AppLine_ClampForward(int32_t value)
{
    if (value < 0) {
        return 0;
    }

    if (value > LINE_MAX_SPEED) {
        return LINE_MAX_SPEED;
    }

    return (int16_t)value;
}

void AppLine_Init(void)
{
    line_error = 0;
    line_found = 0U;
    line_confidence = 0U;
    line_dark_sum = 0U;
    black_detected = 0U;
    white_detected = 0U;
    black_confirm_count = 0U;
    white_confirm_count = 0U;
    line_max_dark = 0U;
    line_active_sensor_count = 0U;
    line_lost_count = 0U;
    last_line_error = 0;
    line_turn_cmd = 0;
    line_left_cmd = 0;
    line_right_cmd = 0;
    line_black_sensor_count = 0U;
    line_gray_trigger_hit = 0U;
    line_ever_found = 0U;
    line_lost_start_tick = 0U;
    line_exit_by_white = 0U;
    line_end_on_lost_active = 0U;
    line_ctrl_active = 0U;
    line_fail_reason = 0U;
    line_prev_control_error = 0;
    line_segment_active = 0U;
    line_segment_done = 0U;
    line_segment_failed = 0U;
}

void AppLine_Task(void)
{
    const uint8_t gray_values[8] = {
        gray_value_0, gray_value_1, gray_value_2, gray_value_3,
        gray_value_4, gray_value_5, gray_value_6, gray_value_7
    };
    int32_t weighted_sum = 0;
    uint16_t sum = 0U;
    uint16_t max_dark = 0U;
    uint8_t active_count = 0U;
    uint8_t black_sensor_count = 0U;

    if (gray_valid == 0U) {
        line_found = 0U;
        line_error = 0;
        line_confidence = 0U;
        line_dark_sum = 0U;
        line_max_dark = 0U;
        line_active_sensor_count = 0U;
        line_black_sensor_count = 0U;
        line_gray_trigger_hit = 0U;
        black_detected = 0U;
        white_detected = 0U;
        black_confirm_count = 0U;
        white_confirm_count = 0U;
        return;
    }

    for (uint8_t i = 0U; i < 8U; i++) {
        /*
         * I2C gray values are 0..255: white is high, black is low.
         * Use black strength for the centroid because the line is black.
         */
        uint8_t dark;

#if GRAY_BLACK_IS_LOW
        dark = (uint8_t)(255U - gray_values[i]);
#else
        dark = gray_values[i];
#endif

        weighted_sum += (int32_t)dark * line_weights[i];
        sum = (uint16_t)(sum + dark);
        if (dark > max_dark) {
            max_dark = dark;
        }
        if (dark >= T2_BLACK_SENSOR_DARK_TH) {
            black_sensor_count++;
        }
        if (dark > SINGLE_SENSOR_BLACK_TH) {
            active_count++;
        }
    }

    line_dark_sum = sum;
    line_confidence = sum;
    line_max_dark = max_dark;
    line_active_sensor_count = active_count;
    line_black_sensor_count = black_sensor_count;
    line_gray_trigger_hit =
        (line_black_sensor_count >= T2_BLACK_SENSOR_NEED) ? 1U : 0U;

    if (line_gray_trigger_hit != 0U || sum >= LINE_DARK_SUM_MIN) {
        line_found = 1U;
        line_error = (sum != 0U) ?
            (int16_t)(weighted_sum / (int32_t)sum) : 0;
        last_line_error = line_error;
    } else {
        line_found = 0U;
        line_error = 0;
    }

    if (line_gray_trigger_hit != 0U) {
        if (black_confirm_count < 255U) {
            black_confirm_count++;
        }
    } else {
        black_confirm_count = 0U;
    }
    black_detected =
        (black_confirm_count >= T2_BLACK_SENSOR_NEED) ? 1U : 0U;

    if (line_black_sensor_count == 0U) {
        if (white_confirm_count < 255U) {
            white_confirm_count++;
        }
    } else {
        white_confirm_count = 0U;
    }
    white_detected =
        (white_confirm_count >= WHITE_CONFIRM_COUNT) ? 1U : 0U;
}

void AppLine_StartSegment(const AppRouteSegment *segment)
{
    line_segment_active = 1U;
    line_segment_done = 0U;
    line_segment_failed = 0U;
    line_lost_count = 0U;
    last_line_error = 0;
    line_prev_control_error = 0;
    line_turn_cmd = 0;
    line_left_cmd = 0;
    line_right_cmd = 0;
    line_black_sensor_count = 0U;
    line_gray_trigger_hit = 0U;
    line_ever_found = 0U;
    line_lost_start_tick = 0U;
    line_exit_by_white = 0U;
    line_ctrl_active = 0U;
    line_fail_reason = 0U;
    black_confirm_count = 0U;
    white_confirm_count = 0U;
    black_detected = 0U;
    white_detected = 0U;
    line_start_tick = app_millis();

    if (segment != 0) {
        line_target_distance_cm = segment->distance_cm;
        line_min_black_detect_cm = segment->min_black_detect_cm;
        line_base_speed = segment->base_speed;
        line_timeout_ms = segment->timeout_ms;
        line_exit_mode = segment->line_exit_mode;
        line_end_on_lost_active =
            ((segment->flags & SEG_FLAG_LINE_END_ON_LOST) != 0U) ? 1U : 0U;
    } else {
        line_target_distance_cm = LINE_B_TO_C_CM;
        line_min_black_detect_cm = 30.0f;
        line_base_speed = LINE_BASE_SPEED;
        line_timeout_ms = SEG_TIMEOUT_LINE_MS;
        line_exit_mode = LINE_EXIT_BY_DISTANCE_OR_NODE;
        line_end_on_lost_active = 0U;
    }

    Encoder_ResetDistance();
}

void AppLine_ControllerTask(void)
{
    int16_t error_for_control;
    int16_t d_error;
    float turn_float;
    float distance = Encoder_GetDistanceCm();
    uint8_t distance_done = 0U;
    uint8_t node_done = 0U;

    if (line_segment_active == 0U || line_segment_done != 0U ||
        line_segment_failed != 0U) {
        line_ctrl_active = 0U;
        return;
    }

    line_ctrl_active = 1U;

    if (line_timeout_ms > 0U &&
        (uint32_t)(app_millis() - line_start_tick) > line_timeout_ms) {
        line_segment_failed = 1U;
        line_fail_reason = 2U;
        Motor_Stop();
        return;
    }

    if (line_end_on_lost_active != 0U) {
        if (line_found != 0U) {
            line_ever_found = 1U;
            line_lost_start_tick = 0U;
            line_exit_by_white = 0U;
        } else if (line_ever_found == 0U) {
            line_turn_cmd = 0;
            line_left_cmd = T2_LINE_LOST_SEARCH_SPEED;
            line_right_cmd = T2_LINE_LOST_SEARCH_SPEED;
            Motor_SetLeft(line_left_cmd);
            Motor_SetRight(line_right_cmd);
            return;
        } else if (white_detected != 0U) {
            if (line_lost_start_tick == 0U) {
                line_lost_start_tick = app_millis();
            }
            if ((uint32_t)(app_millis() - line_lost_start_tick) >=
                T2_LINE_EXIT_WHITE_MS) {
                line_exit_by_white = 1U;
                line_segment_done = 1U;
                Motor_Stop();
                line_left_cmd = 0;
                line_right_cmd = 0;
                line_turn_cmd = 0;
                return;
            }
        } else {
            line_lost_start_tick = 0U;
        }
    }

    if (line_target_distance_cm > 0.0f && distance >= line_target_distance_cm) {
        distance_done = 1U;
    }

    if (distance >= line_min_black_detect_cm && black_detected != 0U) {
        node_done = 1U;
    }

    if ((line_exit_mode == LINE_EXIT_BY_DISTANCE && distance_done != 0U) ||
        (line_exit_mode == LINE_EXIT_BY_NODE_BLACK && node_done != 0U) ||
        (line_exit_mode == LINE_EXIT_BY_DISTANCE_OR_NODE &&
         (distance_done != 0U || node_done != 0U)) ||
        (line_exit_mode == LINE_EXIT_BY_TIMEOUT &&
         (uint32_t)(app_millis() - line_start_tick) > line_timeout_ms)) {
        line_segment_done = 1U;
        Motor_Stop();
        line_left_cmd = 0;
        line_right_cmd = 0;
        return;
    }

    if (line_found != 0U) {
        line_lost_count = 0U;
        error_for_control = line_error;
    } else {
        if (line_lost_count < 255U) {
            line_lost_count++;
        }

        if (line_lost_count >= LINE_LOST_STOP_COUNT) {
            if (line_end_on_lost_active != 0U) {
                error_for_control = last_line_error;
            } else {
            line_segment_failed = 1U;
            line_fail_reason = 1U;
            Motor_Stop();
            line_left_cmd = 0;
            line_right_cmd = 0;
            return;
            }
        } else {
            line_turn_cmd = (last_line_error >= 0) ?
                T2_LINE_SEARCH_TURN_CMD : (int16_t)(-T2_LINE_SEARCH_TURN_CMD);
            line_left_cmd = AppLine_ClampForward(
                T2_LINE_LOST_SEARCH_SPEED + (int32_t)line_turn_cmd);
            line_right_cmd = AppLine_ClampForward(
                T2_LINE_LOST_SEARCH_SPEED - (int32_t)line_turn_cmd);
            Motor_SetLeft(line_left_cmd);
            Motor_SetRight(line_right_cmd);
            return;
        }

        error_for_control = last_line_error;
    }

    d_error = (int16_t)(error_for_control - line_prev_control_error);
    turn_float = (float)LINE_TURN_SIGN *
        (LINE_KP * (float)error_for_control + LINE_KD * (float)d_error);
    line_turn_cmd = (int16_t)turn_float;
    line_prev_control_error = error_for_control;
    last_line_error = error_for_control;

    line_left_cmd = AppLine_ClampForward((int32_t)line_base_speed +
        (int32_t)line_turn_cmd);
    line_right_cmd = AppLine_ClampForward((int32_t)line_base_speed -
        (int32_t)line_turn_cmd);
    Motor_SetLeft(line_left_cmd);
    Motor_SetRight(line_right_cmd);
}

uint8_t AppLine_IsDone(void)
{
    return line_segment_done;
}

uint8_t AppLine_IsFailed(void)
{
    return line_segment_failed;
}

void AppLine_Stop(void)
{
    line_segment_active = 0U;
    line_segment_done = 1U;
    line_left_cmd = 0;
    line_right_cmd = 0;
    line_turn_cmd = 0;
    line_ctrl_active = 0U;
    Motor_Stop();
}
