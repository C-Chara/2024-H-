#ifndef APP_STRAIGHT_H_
#define APP_STRAIGHT_H_

#include <stdint.h>

#include "app_route.h"

#define STRAIGHT_ENCODER_COUNTS_PER_REV     (1560.0f)
#define STRAIGHT_WHEEL_CIRCUMFERENCE_CM     (20.4f)
#define STRAIGHT_BASE_SPEED_FAST            (260)
#define STRAIGHT_BASE_SPEED_SLOW            (180)
#define STRAIGHT_MAX_FORWARD_SPEED          (500)
#define STRAIGHT_KP_YAW                     (4.0f)
#define STRAIGHT_KD_YAW                     (1.0f)

extern volatile float straight_distance_cm;
extern volatile float straight_target_distance_cm;
extern volatile float straight_target_yaw;
extern volatile float straight_heading_error;
extern volatile int16_t straight_base_cmd;
extern volatile int16_t straight_turn_cmd;
extern volatile int16_t straight_left_cmd;
extern volatile int16_t straight_right_cmd;

void AppStraight_Init(void);
void AppStraight_Start(const AppRouteSegment *segment);
void AppStraight_Task(void);
uint8_t AppStraight_IsDone(void);

#endif /* APP_STRAIGHT_H_ */
