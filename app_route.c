#include "app_route.h"

#include "app_config.h"

#define ROUTE_TASK1_COUNT       (2U)
#define ROUTE_TASK2_COUNT       (5U)
#define ROUTE_TASK3_COUNT       (5U)
#define ROUTE_TASK4_COUNT       (5U)

volatile uint8_t route_loaded = 0U;
volatile uint8_t current_segment_index = 0U;
volatile uint8_t current_lap = 1U;
volatile uint8_t route_finished = 0U;

static volatile uint8_t route_segment_count = 0U;
static volatile uint8_t route_lap_target = 1U;
static const AppRouteSegment *route_segments = 0;

static const AppRouteSegment route_task1[ROUTE_TASK1_COUNT] = {
    { SEG_STRAIGHT_BLIND, NODE_A, NODE_B, TASK1_TARGET_DISTANCE_CM, YAW_OFFSET_AB, 0, SEG_FLAG_TASK1_BLACK_STOP },
    { SEG_STOP_HINT,      NODE_B, NODE_B, 0.0f,       0.0f,          0, 0U },
};

static const AppRouteSegment route_task2[ROUTE_TASK2_COUNT] = {
    { SEG_STRAIGHT_BLIND, NODE_A, NODE_B, DIST_AB_CM,          YAW_OFFSET_AB, 0, 0U },
    { SEG_ARC_LINE,       NODE_B, NODE_C, ARC_HALF_LENGTH_CM,  0.0f,          1, 0U },
    { SEG_STRAIGHT_BLIND, NODE_C, NODE_D, DIST_CD_CM,          YAW_OFFSET_CD, 0, 0U },
    { SEG_ARC_LINE,       NODE_D, NODE_A, ARC_HALF_LENGTH_CM,  0.0f,          1, 0U },
    { SEG_STOP_HINT,      NODE_A, NODE_A, 0.0f,                0.0f,          0, 0U },
};

static const AppRouteSegment route_task3[ROUTE_TASK3_COUNT] = {
    { SEG_DIAGONAL_BLIND, NODE_A, NODE_C, DIST_AC_CM,          YAW_OFFSET_AC, 0, 0U },
    { SEG_ARC_LINE,       NODE_C, NODE_B, ARC_HALF_LENGTH_CM,  0.0f,         -1, 0U },
    { SEG_DIAGONAL_BLIND, NODE_B, NODE_D, DIST_BD_CM,          YAW_OFFSET_BD, 0, 0U },
    { SEG_ARC_LINE,       NODE_D, NODE_A, ARC_HALF_LENGTH_CM,  0.0f,          1, 0U },
    { SEG_STOP_HINT,      NODE_A, NODE_A, 0.0f,                0.0f,          0, 0U },
};

static const AppRouteSegment route_task4[ROUTE_TASK4_COUNT] = {
    { SEG_DIAGONAL_BLIND, NODE_A, NODE_C, DIST_AC_CM,          YAW_OFFSET_AC, 0, 0U },
    { SEG_ARC_LINE,       NODE_C, NODE_B, ARC_HALF_LENGTH_CM,  0.0f,         -1, 0U },
    { SEG_DIAGONAL_BLIND, NODE_B, NODE_D, DIST_BD_CM,          YAW_OFFSET_BD, 0, 0U },
    { SEG_ARC_LINE,       NODE_D, NODE_A, ARC_HALF_LENGTH_CM,  0.0f,          1, 0U },
    { SEG_STOP_HINT,      NODE_A, NODE_A, 0.0f,                0.0f,          0, 0U },
};

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
    current_segment_index = 0U;
    current_lap = 1U;
    route_finished = 0U;
    return 1U;
}

void AppRoute_Clear(void)
{
    route_loaded = 0U;
    current_segment_index = 0U;
    current_lap = 1U;
    route_finished = 0U;
    route_segment_count = 0U;
    route_lap_target = 1U;
    route_segments = 0;
}

const AppRouteSegment *AppRoute_CurrentSegment(void)
{
    if (route_loaded == 0U || route_segments == 0 ||
        current_segment_index >= route_segment_count) {
        return 0;
    }

    return &route_segments[current_segment_index];
}

void AppRoute_AdvanceSegment(void)
{
    uint8_t last_move_index;

    if (route_loaded == 0U || route_segment_count == 0U) {
        route_finished = 1U;
        return;
    }

    last_move_index = (uint8_t)(route_segment_count - 2U);

    if (current_segment_index == last_move_index &&
        current_lap < route_lap_target) {
        current_lap++;
        current_segment_index = 0U;
        return;
    }

    current_segment_index++;
    if (current_segment_index >= route_segment_count) {
        route_finished = 1U;
    }
}
