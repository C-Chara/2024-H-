#ifndef APP_ARC_H_
#define APP_ARC_H_

#include <stdint.h>

#include "app_route.h"

extern volatile uint8_t arc_active;
extern volatile uint8_t arc_done_flag;
extern volatile float arc_distance_cm;
extern volatile float arc_target_length_cm;
extern volatile uint16_t arc_line_confidence;

void AppArc_Init(void);
void AppArc_Start(const AppRouteSegment *segment);
void AppArc_Task(void);
uint8_t AppArc_IsDone(void);

#endif /* APP_ARC_H_ */
