#include "app_straight.h"

#include "Algorithm/hal.h"
#include "Algorithm/motor.h"
#include "gyro.h"

volatile float straight_distance_cm = 0.0f;
volatile float straight_target_distance_cm = 0.0f;
volatile float straight_target_yaw = 0.0f;
volatile float straight_heading_error = 0.0f;
volatile int16_t straight_base_cmd = 0;
volatile int16_t straight_turn_cmd = 0;
volatile int16_t straight_left_cmd = 0;
volatile int16_t straight_right_cmd = 0;

static uint8_t straight_active = 0U;
static uint8_t straight_done = 0U;
static float straight_last_error = 0.0f;

static int32_t Straight_Abs32(int32_t value)
{
    return (value >= 0) ? value : -value;
}

static float Straight_WrapAngle(float angle)
{
    while (angle > 180.0f) {
        angle -= 360.0f;
    }

    while (angle < -180.0f) {
        angle += 360.0f;
    }

    return angle;
}

static int16_t Straight_ClampForward(int16_t value)
{
    if (value < 0) {
        return 0;
    }

    if (value > STRAIGHT_MAX_FORWARD_SPEED) {
        return STRAIGHT_MAX_FORWARD_SPEED;
    }

    return value;
}

void AppStraight_Init(void)
{
    straight_active = 0U;
    straight_done = 0U;
    straight_distance_cm = 0.0f;
    straight_target_distance_cm = 0.0f;
    straight_target_yaw = 0.0f;
    straight_heading_error = 0.0f;
    straight_base_cmd = 0;
    straight_turn_cmd = 0;
    straight_left_cmd = 0;
    straight_right_cmd = 0;
    straight_last_error = 0.0f;
}

void AppStraight_Start(const AppRouteSegment *segment)
{
    if (segment == 0) {
        straight_done = 1U;
        straight_active = 0U;
        return;
    }

    straight_distance_cm = 0.0f;
    straight_target_distance_cm = segment->distance_cm;
    if (segment->target_yaw_deg > 900.0f) {
        straight_target_yaw = gyro_yaw;
    } else {
        straight_target_yaw = segment->target_yaw_deg;
    }
    straight_heading_error = 0.0f;
    straight_base_cmd = STRAIGHT_BASE_SPEED_FAST;
    straight_turn_cmd = 0;
    straight_left_cmd = 0;
    straight_right_cmd = 0;
    straight_last_error = 0.0f;

    /* Discard any old encoder delta before starting this segment. */
    (void)HAL_Get_Left_Encoder_Delta();
    (void)HAL_Get_Right_Encoder_Delta();

    straight_done = 0U;
    straight_active = 1U;
}

void AppStraight_Task(void)
{
    int32_t left_delta;
    int32_t right_delta;
    float avg_counts;
    float step_cm;
    float d_error;
    float turn_float;
    int16_t left;
    int16_t right;

    if (straight_active == 0U || straight_done != 0U) {
        return;
    }

    left_delta = HAL_Get_Left_Encoder_Delta();
    right_delta = HAL_Get_Right_Encoder_Delta();
    avg_counts = ((float)Straight_Abs32(left_delta) +
        (float)Straight_Abs32(right_delta)) * 0.5f;
    step_cm = avg_counts * STRAIGHT_WHEEL_CIRCUMFERENCE_CM /
        STRAIGHT_ENCODER_COUNTS_PER_REV;
    straight_distance_cm += step_cm;

    if (straight_distance_cm >= straight_target_distance_cm) {
        Motor_Stop();
        straight_left_cmd = 0;
        straight_right_cmd = 0;
        straight_turn_cmd = 0;
        straight_done = 1U;
        straight_active = 0U;
        return;
    }

    if ((straight_target_distance_cm - straight_distance_cm) < 20.0f) {
        straight_base_cmd = STRAIGHT_BASE_SPEED_SLOW;
    } else {
        straight_base_cmd = STRAIGHT_BASE_SPEED_FAST;
    }

    straight_heading_error = Straight_WrapAngle(straight_target_yaw - gyro_yaw);
    d_error = straight_heading_error - straight_last_error;
    straight_last_error = straight_heading_error;

    turn_float = STRAIGHT_KP_YAW * straight_heading_error +
        STRAIGHT_KD_YAW * d_error;
    straight_turn_cmd = (int16_t)turn_float;

    left = (int16_t)(straight_base_cmd - straight_turn_cmd);
    right = (int16_t)(straight_base_cmd + straight_turn_cmd);
    straight_left_cmd = Straight_ClampForward(left);
    straight_right_cmd = Straight_ClampForward(right);

    Motor_SetLeft(straight_left_cmd);
    Motor_SetRight(straight_right_cmd);
}

uint8_t AppStraight_IsDone(void)
{
    return straight_done;
}
