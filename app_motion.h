#ifndef APP_MOTION_H_
#define APP_MOTION_H_

#include <stdint.h>

extern volatile int16_t base_speed;
extern volatile int16_t left_cmd;
extern volatile int16_t right_cmd;
extern volatile int16_t turn_cmd;

void Motion_Init(void);
void Motion_SetBaseSpeed(int16_t base);
void Motion_UpdateByLineError(int16_t error);
void Motion_Stop(void);

#endif /* APP_MOTION_H_ */
