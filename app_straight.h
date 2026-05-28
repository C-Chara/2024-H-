#ifndef APP_STRAIGHT_H_
#define APP_STRAIGHT_H_

#include <stdint.h>

#include "app_blind.h"
#include "app_route.h"

void AppStraight_Init(void);
void AppStraight_Start(const AppRouteSegment *segment);
void AppStraight_Task(void);
uint8_t AppStraight_IsDone(void);

#endif /* APP_STRAIGHT_H_ */
