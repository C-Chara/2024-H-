#include "app_sensor.h"

#include "app_mode.h"
#include "atk_ms901m.h"
#include "app_time.h"
#include "gyro.h"

#define IMU_STALE_MS       (100U)

volatile float gyro_yaw_filtered = 0.0f;
volatile uint32_t imu_last_update_tick = 0U;
volatile uint32_t imu_update_dt_ms = 0U;
volatile uint32_t imu_update_slow_count = 0U;
volatile uint8_t imu_valid = 0U;
volatile uint32_t gyro_stale_count = 0U;
volatile float gyro_yaw_fusion_dbg = 0.0f;
volatile uint32_t gyro_fusion_update_count = 0U;
volatile uint8_t imu_protection_enabled_dbg = 0U;
volatile float start_yaw_snapshot = 0.0f;
volatile uint32_t start_yaw_sample_count = 0U;
volatile uint8_t start_yaw_ready = 0U;

static uint8_t imu_was_valid = 0U;
static uint8_t imu_stale_latched = 0U;
static uint32_t imu_health_last_tick = 0U;
static uint32_t imu_health_last_poll_count = 0U;
static uint32_t imu_health_last_rx_count = 0U;
static uint32_t imu_health_last_upload_count = 0U;
static uint32_t imu_health_last_attitude_count = 0U;
static uint8_t start_yaw_capture_active = 0U;
static float start_yaw_sum = 0.0f;
static float start_yaw_last_raw = 0.0f;
static float start_yaw_unwrapped = 0.0f;

static float Sensor_NormalizeAngle(float angle)
{
    while (angle > 180.0f) {
        angle -= 360.0f;
    }
    while (angle < -180.0f) {
        angle += 360.0f;
    }
    return angle;
}

static void Sensor_CaptureStartYawSample(float yaw)
{
    float delta;

    if (start_yaw_capture_active == 0U) {
        return;
    }

    if (start_yaw_sample_count == 0U) {
        start_yaw_unwrapped = yaw;
        start_yaw_sum = yaw;
    } else {
        delta = Sensor_NormalizeAngle(yaw - start_yaw_last_raw);
        start_yaw_unwrapped += delta;
        start_yaw_sum += start_yaw_unwrapped;
    }

    start_yaw_last_raw = yaw;
    start_yaw_sample_count++;
    start_yaw_snapshot =
        Sensor_NormalizeAngle(start_yaw_sum / (float)start_yaw_sample_count);
    start_yaw_ready = (start_yaw_sample_count >= 5U) ? 1U : 0U;
}

void Sensor_Fusion_Init(void)
{
    gyro_yaw_filtered = 0.0f;
    imu_last_update_tick = 0U;
    imu_update_dt_ms = 0U;
    imu_update_slow_count = 0U;
    imu_valid = 0U;
    gyro_stale_count = 0U;
    gyro_yaw_fusion_dbg = 0.0f;
    gyro_fusion_update_count = 0U;
    imu_protection_enabled_dbg = 0U;
    imu_was_valid = 0U;
    imu_stale_latched = 0U;
    imu_health_last_tick = 0U;
    imu_health_last_poll_count = 0U;
    imu_health_last_rx_count = 0U;
    imu_health_last_upload_count = 0U;
    imu_health_last_attitude_count = 0U;
    start_yaw_snapshot = 0.0f;
    start_yaw_sample_count = 0U;
    start_yaw_ready = 0U;
    start_yaw_capture_active = 0U;
    start_yaw_sum = 0.0f;
    start_yaw_last_raw = 0.0f;
    start_yaw_unwrapped = 0.0f;
}

void Sensor_Fusion_Task(void)
{
    atk_ms901m_attitude_data_t att;
    uint32_t now = app_millis();
    uint32_t last_tick = imu_last_update_tick;

    if (atk_ms901m_attitude_available() != 0U &&
        atk_ms901m_get_latest_attitude(&att) == ATK_MS901M_EOK) {
        gyro_roll = att.roll;
        gyro_pitch = att.pitch;
        gyro_yaw = att.yaw;
        gyro_yaw_fusion_dbg = gyro_yaw;
        gyro_fusion_update_count++;
        Sensor_CaptureStartYawSample(gyro_yaw);

        if (last_tick != 0U) {
            imu_update_dt_ms = now - last_tick;
            if (imu_update_dt_ms > 50U) {
                imu_update_slow_count++;
            }
        } else {
            imu_update_dt_ms = 0U;
        }
        imu_last_update_tick = now;

        if (imu_valid == 0U && imu_was_valid == 0U) {
            gyro_yaw_filtered = gyro_yaw;
        } else {
            gyro_yaw_filtered = gyro_yaw_filtered * 0.5f + gyro_yaw * 0.5f;
        }

        imu_valid = 1U;
        imu_was_valid = 1U;
        imu_stale_latched = 0U;
    }

    if (imu_last_update_tick != 0U &&
        (uint32_t)(now - imu_last_update_tick) > IMU_STALE_MS) {
        imu_valid = 0U;
        imu_was_valid = 0U;
        if (imu_stale_latched == 0U) {
            gyro_stale_count++;
            imu_stale_latched = 1U;
        }
    }
}

void Sensor_StartYawCaptureReset(void)
{
    start_yaw_snapshot = gyro_yaw;
    start_yaw_sample_count = 0U;
    start_yaw_ready = 0U;
    start_yaw_capture_active = 1U;
    start_yaw_sum = 0.0f;
    start_yaw_last_raw = gyro_yaw;
    start_yaw_unwrapped = gyro_yaw;
}

float Sensor_GetStartYawSnapshot(void)
{
    if (start_yaw_ready == 0U) {
        return gyro_yaw;
    }

    return start_yaw_snapshot;
}

void Imu_Health_Task(void)
{
    uint32_t now = app_millis();
    uint8_t attitude_updated;
    uint8_t upload_updated;
    uint8_t rx_updated;
    uint8_t poll_updated;

    if (system_mode != SYS_RUN) {
        imu_run_source_state = 0U;
        imu_health_last_tick = now;
        imu_health_last_poll_count = gyro_poll_count;
        imu_health_last_rx_count = gyro_rx_count;
        imu_health_last_upload_count = imu_valid_upload_frame_count;
        imu_health_last_attitude_count = imu_valid_attitude_frame_count;
        return;
    }

    if (imu_health_last_tick == 0U) {
        imu_health_last_tick = now;
        imu_health_last_poll_count = gyro_poll_count;
        imu_health_last_rx_count = gyro_rx_count;
        imu_health_last_upload_count = imu_valid_upload_frame_count;
        imu_health_last_attitude_count = imu_valid_attitude_frame_count;
        return;
    }

    if ((uint32_t)(now - imu_health_last_tick) < 100U) {
        return;
    }

    poll_updated = (gyro_poll_count != imu_health_last_poll_count) ? 1U : 0U;
    rx_updated = (gyro_rx_count != imu_health_last_rx_count) ? 1U : 0U;
    upload_updated =
        (imu_valid_upload_frame_count != imu_health_last_upload_count) ?
        1U : 0U;
    attitude_updated =
        (imu_valid_attitude_frame_count != imu_health_last_attitude_count) ?
        1U : 0U;

    if (poll_updated == 0U) {
        imu_run_source_state = 3U;
    } else if (attitude_updated != 0U) {
        imu_run_source_state = 4U;
    } else if (rx_updated != 0U && upload_updated != 0U) {
        imu_run_source_state = 1U;
        imu_attitude_miss_while_run_count++;
    } else if (rx_updated != 0U) {
        imu_run_source_state = 2U;
    }

    imu_health_last_tick = now;
    imu_health_last_poll_count = gyro_poll_count;
    imu_health_last_rx_count = gyro_rx_count;
    imu_health_last_upload_count = imu_valid_upload_frame_count;
    imu_health_last_attitude_count = imu_valid_attitude_frame_count;
}
