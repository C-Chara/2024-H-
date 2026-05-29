#include "app_sensor.h"

#include "atk_ms901m.h"
#include "app_time.h"
#include "gyro.h"

#define IMU_STALE_MS       (200U)

volatile float gyro_yaw_filtered = 0.0f;
volatile uint32_t imu_last_update_tick = 0U;
volatile uint8_t imu_valid = 0U;
volatile uint32_t gyro_stale_count = 0U;

static uint8_t imu_was_valid = 0U;

void Sensor_Fusion_Init(void)
{
    gyro_yaw_filtered = 0.0f;
    imu_last_update_tick = 0U;
    imu_valid = 0U;
    gyro_stale_count = 0U;
    imu_was_valid = 0U;
}

void Sensor_Fusion_Task(void)
{
    atk_ms901m_attitude_data_t att;
    uint32_t now = app_millis();

    if (gyro_ok != 0U &&
        atk_ms901m_get_latest_attitude(&att) == ATK_MS901M_EOK) {
        gyro_roll = att.roll;
        gyro_pitch = att.pitch;
        gyro_yaw = att.yaw;

        if (imu_valid == 0U && imu_was_valid == 0U) {
            gyro_yaw_filtered = gyro_yaw;
        } else {
            gyro_yaw_filtered = gyro_yaw_filtered * 0.8f + gyro_yaw * 0.2f;
        }

        imu_last_update_tick = now;
        imu_valid = 1U;
        imu_was_valid = 1U;
    }

    if (imu_valid != 0U && (uint32_t)(now - imu_last_update_tick) >
        IMU_STALE_MS) {
        imu_valid = 0U;
        imu_was_valid = 0U;
        gyro_stale_count++;
        gyro_attitude_fail_count++;
    }
}
