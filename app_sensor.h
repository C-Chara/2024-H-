#ifndef APP_SENSOR_H_
#define APP_SENSOR_H_

#include <stdint.h>

extern volatile float gyro_yaw_filtered;
extern volatile uint32_t imu_last_update_tick;
extern volatile uint32_t imu_update_dt_ms;
extern volatile uint32_t imu_update_slow_count;
extern volatile uint8_t imu_valid;
extern volatile uint32_t gyro_stale_count;
extern volatile float gyro_yaw_fusion_dbg;
extern volatile uint32_t gyro_fusion_update_count;
extern volatile uint8_t imu_protection_enabled_dbg;
extern volatile float start_yaw_snapshot;
extern volatile uint32_t start_yaw_sample_count;
extern volatile uint8_t start_yaw_ready;

void Sensor_Fusion_Init(void);
void Sensor_Fusion_Task(void);
void Imu_Health_Task(void);
void Sensor_StartYawCaptureReset(void);
float Sensor_GetStartYawSnapshot(void);

#endif /* APP_SENSOR_H_ */
