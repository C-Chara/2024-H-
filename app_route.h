#ifndef APP_ROUTE_H_
#define APP_ROUTE_H_

#include <stdint.h>

#define NODE_A                  (0U)
#define NODE_B                  (1U)
#define NODE_C                  (2U)
#define NODE_D                  (3U)

#define SEG_STRAIGHT_BLIND      (0U)
#define SEG_DIAGONAL_BLIND      (1U)
#define SEG_ARC_LINE            (2U)
#define SEG_STOP_HINT           (3U)

#define SEG_FLAG_TASK1_BLACK_STOP   (0x01U)

typedef struct {
    uint8_t type;
    uint8_t from_node;
    uint8_t to_node;
    float target_distance_cm;
    float yaw_offset_deg;
    int8_t arc_dir;
    uint8_t flags;
} AppRouteSegment;

extern volatile uint8_t route_loaded;
extern volatile uint8_t current_segment_index;
extern volatile uint8_t current_lap;
extern volatile uint8_t route_finished;

void AppRoute_Init(void);
uint8_t AppRoute_LoadTask(uint8_t task_id);
void AppRoute_Clear(void);
const AppRouteSegment *AppRoute_CurrentSegment(void);
void AppRoute_AdvanceSegment(void);

#endif /* APP_ROUTE_H_ */
