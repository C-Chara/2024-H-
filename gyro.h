#ifndef GYRO_H_
#define GYRO_H_

#include <stdint.h>

extern volatile int16_t gyro_yaw_raw;
extern volatile float gyro_roll;
extern volatile float gyro_pitch;
extern volatile float gyro_yaw;

float Gyro_GetYaw(void);
void Gyro_SetSoftwareZero(void);
float Gyro_GetRelativeYaw(void);

#endif /* GYRO_H_ */
