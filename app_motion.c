#include "app_motion.h"

#include "Algorithm/motor.h"

#define MOTION_SPEED_LIMIT  (500)
#define MOTION_KP           (1)
#define MOTION_KD           (0)

volatile int16_t base_speed = 250;
volatile int16_t left_cmd = 0;
volatile int16_t right_cmd = 0;
volatile int16_t turn_cmd = 0;

static int16_t last_line_error = 0;

static int16_t Motion_Clamp(int32_t value)
{
    if (value > MOTION_SPEED_LIMIT) {
        return MOTION_SPEED_LIMIT;
    }

    if (value < -MOTION_SPEED_LIMIT) {
        return -MOTION_SPEED_LIMIT;
    }

    return (int16_t)value;
}

static int16_t Motion_ClampForward(int32_t value)
{
    if (value < 0) {
        return 0;
    }

    if (value > MOTION_SPEED_LIMIT) {
        return MOTION_SPEED_LIMIT;
    }

    return (int16_t)value;
}

void Motion_Init(void)
{
    base_speed = 250;
    left_cmd = 0;
    right_cmd = 0;
    turn_cmd = 0;
    last_line_error = 0;
    Motion_Stop();
}

void Motion_SetBaseSpeed(int16_t base)
{
    base_speed = Motion_ClampForward(base);
}

void Motion_UpdateByLineError(int16_t error)
{
    int16_t d_error = (int16_t)(error - last_line_error);
    int32_t turn = ((int32_t)MOTION_KP * error) +
        ((int32_t)MOTION_KD * d_error);

    last_line_error = error;
    turn_cmd = Motion_Clamp(turn);
    left_cmd = Motion_ClampForward((int32_t)base_speed - turn_cmd);
    right_cmd = Motion_ClampForward((int32_t)base_speed + turn_cmd);

    Motor_SetLeft(left_cmd);
    Motor_SetRight(right_cmd);
}

void Motion_Stop(void)
{
    left_cmd = 0;
    right_cmd = 0;
    turn_cmd = 0;
    Motor_Stop();
}
