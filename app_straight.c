#include "app_straight.h"

/*
 * Compatibility layer for older project references.
 * New code should use app_blind directly for no-line blind segments.
 */
void AppStraight_Init(void)
{
    AppBlind_Init();
}

void AppStraight_Start(const AppRouteSegment *segment)
{
    if (segment != 0) {
        AppBlind_Start(segment->target_distance_cm, segment->yaw_offset_deg);
    }
}

void AppStraight_Task(void)
{
    AppBlind_Task();
}

uint8_t AppStraight_IsDone(void)
{
    return AppBlind_IsDone();
}
