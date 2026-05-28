#include "app_route.h"

#define ROUTE_TASK1_COUNT   (2U)
#define ROUTE_TASK2_COUNT   (5U)
#define ROUTE_TASK3_COUNT   (5U)
#define ROUTE_TASK4_COUNT   (5U)

#define ROUTE_AB_DISTANCE_CM       (100.0f)
#define ROUTE_CD_DISTANCE_CM       (100.0f)
#define ROUTE_AC_DISTANCE_CM       (141.4f)
#define ROUTE_BD_DISTANCE_CM       (141.4f)
#define ROUTE_ARC_HALF_LENGTH_CM   (125.7f)

/* Tune these headings after confirming the field coordinate direction. */
#define ROUTE_YAW_A_TO_C_DEG       (45.0f)
#define ROUTE_YAW_B_TO_D_DEG       (45.0f)

volatile uint8_t route_loaded = 0U;
volatile uint8_t route_segment_index = 0U;
volatile uint8_t route_segment_count = 0U;
volatile uint8_t route_lap_index = 1U;
volatile uint8_t route_lap_target = 1U;
volatile uint8_t route_finished = 0U;

static const AppRouteSegment route_task1[ROUTE_TASK1_COUNT] = {
    { SEG_STRAIGHT_BLIND, NODE_A, NODE_B, ROUTE_AB_DISTANCE_CM, ROUTE_YAW_KEEP_CURRENT, 0 },
    { SEG_STOP_HINT,      NODE_B, NODE_B, 0.0f,                 ROUTE_YAW_KEEP_CURRENT, 0 },
};

static const AppRouteSegment route_task2[ROUTE_TASK2_COUNT] = {
    { SEG_STRAIGHT_BLIND, NODE_A, NODE_B, ROUTE_AB_DISTANCE_CM,     ROUTE_YAW_KEEP_CURRENT, 0 },
    { SEG_ARC_LINE,       NODE_B, NODE_C, ROUTE_ARC_HALF_LENGTH_CM, ROUTE_YAW_KEEP_CURRENT, 1 },
    { SEG_STRAIGHT_BLIND, NODE_C, NODE_D, ROUTE_CD_DISTANCE_CM,     ROUTE_YAW_KEEP_CURRENT, 0 },
    { SEG_ARC_LINE,       NODE_D, NODE_A, ROUTE_ARC_HALF_LENGTH_CM, ROUTE_YAW_KEEP_CURRENT, 1 },
    { SEG_STOP_HINT,      NODE_A, NODE_A, 0.0f,                     ROUTE_YAW_KEEP_CURRENT, 0 },
};

static const AppRouteSegment route_task3[ROUTE_TASK3_COUNT] = {
    { SEG_DIAGONAL_BLIND, NODE_A, NODE_C, ROUTE_AC_DISTANCE_CM,     ROUTE_YAW_A_TO_C_DEG,   0 },
    { SEG_ARC_LINE,       NODE_C, NODE_B, ROUTE_ARC_HALF_LENGTH_CM, ROUTE_YAW_KEEP_CURRENT, -1 },
    { SEG_DIAGONAL_BLIND, NODE_B, NODE_D, ROUTE_BD_DISTANCE_CM,     ROUTE_YAW_B_TO_D_DEG,   0 },
    { SEG_ARC_LINE,       NODE_D, NODE_A, ROUTE_ARC_HALF_LENGTH_CM, ROUTE_YAW_KEEP_CURRENT, 1 },
    { SEG_STOP_HINT,      NODE_A, NODE_A, 0.0f,                     ROUTE_YAW_KEEP_CURRENT, 0 },
};

static const AppRouteSegment route_task4[ROUTE_TASK4_COUNT] = {
    { SEG_DIAGONAL_BLIND, NODE_A, NODE_C, ROUTE_AC_DISTANCE_CM,     ROUTE_YAW_A_TO_C_DEG,   0 },
    { SEG_ARC_LINE,       NODE_C, NODE_B, ROUTE_ARC_HALF_LENGTH_CM, ROUTE_YAW_KEEP_CURRENT, -1 },
    { SEG_DIAGONAL_BLIND, NODE_B, NODE_D, ROUTE_BD_DISTANCE_CM,     ROUTE_YAW_B_TO_D_DEG,   0 },
    { SEG_ARC_LINE,       NODE_D, NODE_A, ROUTE_ARC_HALF_LENGTH_CM, ROUTE_YAW_KEEP_CURRENT, 1 },
    { SEG_STOP_HINT,      NODE_A, NODE_A, 0.0f,                     ROUTE_YAW_KEEP_CURRENT, 0 },
};

static const AppRouteSegment *route_segments = 0;

void AppRoute_Init(void)
{
    AppRoute_Clear();
}

uint8_t AppRoute_LoadTask(uint8_t task_id)
{
    AppRoute_Clear();

    if (task_id == 1U) {
        route_segments = route_task1;
        route_segment_count = ROUTE_TASK1_COUNT;
        route_lap_target = 1U;
    } else if (task_id == 2U) {
        route_segments = route_task2;
        route_segment_count = ROUTE_TASK2_COUNT;
        route_lap_target = 1U;
    } else if (task_id == 3U) {
        route_segments = route_task3;
        route_segment_count = ROUTE_TASK3_COUNT;
        route_lap_target = 1U;
    } else if (task_id == 4U) {
        route_segments = route_task4;
        route_segment_count = ROUTE_TASK4_COUNT;
        route_lap_target = 4U;
    } else {
        return 0U;
    }

    route_loaded = 1U;
    route_finished = 0U;
    route_segment_index = 0U;
    route_lap_index = 1U;
    return 1U;
}

void AppRoute_Clear(void)
{
    route_segments = 0;
    route_loaded = 0U;
    route_finished = 0U;
    route_segment_index = 0U;
    route_segment_count = 0U;
    route_lap_index = 1U;
    route_lap_target = 1U;
}

const AppRouteSegment *AppRoute_CurrentSegment(void)
{
    if (route_loaded == 0U || route_segments == 0 ||
        route_segment_index >= route_segment_count) {
        return 0;
    }

    return &route_segments[route_segment_index];
}

void AppRoute_AdvanceSegment(void)
{
    uint8_t last_move_index;

    if (route_loaded == 0U || route_segment_count == 0U) {
        route_finished = 1U;
        return;
    }

    last_move_index = (uint8_t)(route_segment_count - 2U);

    if (route_segment_index == last_move_index &&
        route_lap_index < route_lap_target) {
        route_lap_index++;
        route_segment_index = 0U;
        return;
    }

    route_segment_index++;
    if (route_segment_index >= route_segment_count) {
        route_finished = 1U;
    }
}

uint8_t AppRoute_IsFinished(void)
{
    return route_finished;
}
