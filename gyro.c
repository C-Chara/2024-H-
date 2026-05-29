#include "gyro.h"

volatile int16_t gyro_yaw_raw = 0;
volatile float gyro_roll = 0.0f;
volatile float gyro_pitch = 0.0f;
volatile float gyro_yaw = 0.0f;

static volatile float gyro_yaw_zero = 0.0f;

static float Gyro_NormalizeAngle(float angle)
{
    while (angle > 180.0f) {
        angle -= 360.0f;
    }

    while (angle < -180.0f) {
        angle += 360.0f;
    }

    return angle;
}

float Gyro_GetYaw(void)
{
    return gyro_yaw;
}

void Gyro_SetSoftwareZero(void)
{
    gyro_yaw_zero = gyro_yaw;
}

float Gyro_GetRelativeYaw(void)
{
    return Gyro_NormalizeAngle(gyro_yaw - gyro_yaw_zero);
}
