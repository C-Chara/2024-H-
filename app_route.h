#ifndef APP_ROUTE_H_
#define APP_ROUTE_H_

#include <stdint.h>

typedef enum {
    NODE_NONE = 0,
    NODE_A,
    NODE_B,
    NODE_C,
    NODE_D
} route_node_t;

typedef enum {
    SEG_NONE = 0,
    SEG_BLIND,
    SEG_LINE,
    SEG_TURN_TO_YAW,
    SEG_NODE_EVENT,
    SEG_STOP
} route_segment_type_t;

typedef enum {
    LINE_EXIT_BY_DISTANCE = 0,
    LINE_EXIT_BY_NODE_BLACK,
    LINE_EXIT_BY_DISTANCE_OR_NODE,
    LINE_EXIT_BY_TIMEOUT
} line_exit_mode_t;

#define SEG_STRAIGHT_BLIND      SEG_BLIND
#define SEG_DIAGONAL_BLIND      SEG_BLIND
#define SEG_ARC_LINE            SEG_LINE
#define SEG_STOP_HINT           SEG_STOP

#define SEG_FLAG_TASK1_BLACK_STOP   (0x01U)

typedef struct {
    route_segment_type_t type;
    route_node_t from_node;
    route_node_t to_node;
    float distance_cm;
    float yaw_offset_deg;
    float base_speed;
    uint32_t timeout_ms;
    line_exit_mode_t line_exit_mode;
    float min_black_detect_cm;
    uint8_t node_hint_enable;
    uint8_t flags;
} route_segment_t;

typedef route_segment_t AppRouteSegment;

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
