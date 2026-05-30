#ifndef APP_BLIND_H_
#define APP_BLIND_H_

#include <stdint.h>

#include "app_route.h"

extern volatile float blind_distance_cm;
extern volatile float blind_target_distance_cm;
extern volatile float blind_target_yaw;
extern volatile float blind_heading_error;
extern volatile int16_t blind_base_cmd;
extern volatile int16_t blind_turn_cmd;
extern volatile int16_t blind_left_cmd;
extern volatile int16_t blind_right_cmd;
extern volatile uint8_t blind_segment_started;
extern volatile float blind_last_error_dbg;
extern volatile uint8_t blind_ctrl_active;
extern volatile float blind_yaw_now_dbg;
extern volatile uint8_t blind_finish_enabled_dbg;

void AppBlind_Init(void);
void AppBlind_EnableTask1BlackStop(uint8_t enable);
void AppBlind_Start(float target_distance_cm, float yaw_offset_deg);
void AppBlind_StartSegment(const AppRouteSegment *segment);
void AppBlind_Task(void);
uint8_t AppBlind_IsDone(void);
void AppBlind_Stop(void);

#endif /* APP_BLIND_H_ */
