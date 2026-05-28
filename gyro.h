#ifndef GYRO_H_
#define GYRO_H_

extern volatile float gyro_roll;
extern volatile float gyro_pitch;
extern volatile float gyro_yaw;

void Gyro_Task(void);

#endif /* GYRO_H_ */
