#ifndef APP_ARC_H_
#define APP_ARC_H_

#include <stdint.h>

#include "app_route.h"

#define ARC_STATE_IDLE      (0U)
#define ARC_STATE_RUN       (1U)
#define ARC_STATE_DONE      (2U)
#define ARC_STATE_LOST      (3U)

extern volatile uint8_t arc_state;
extern volatile float arc_distance_cm;
extern volatile int16_t arc_left_cmd;
extern volatile int16_t arc_right_cmd;

void AppArc_Init(void);
void AppArc_Start(const AppRouteSegment *segment);
void AppArc_Task(void);
uint8_t AppArc_IsDone(void);

#endif /* APP_ARC_H_ */
