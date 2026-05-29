#ifndef GYRO_H_
#define GYRO_H_

#include <stdint.h>

extern volatile uint32_t gyro_rx_count;
extern volatile uint32_t gyro_frame_count;
extern volatile uint32_t gyro_angle_frame_count;
extern volatile uint32_t gyro_checksum_error;
extern volatile uint8_t gyro_last_byte;
extern volatile float gyro_roll;
extern volatile float gyro_pitch;
extern volatile float gyro_yaw;

void Gyro_Task(void);
float Gyro_GetYaw(void);
void Gyro_SetSoftwareZero(void);
float Gyro_GetRelativeYaw(void);

#endif /* GYRO_H_ */
