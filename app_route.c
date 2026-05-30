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
    { SEG_BLIND, NODE_A, NODE_B, FIELD_AB_CM, YAW_A_TO_B_DEG,
      BLIND_FAST_SPEED, TASK1_TIMEOUT_BLIND_MS, LINE_EXIT_BY_DISTANCE_OR_NODE,
      TASK1_BLACK_STOP_ENABLE_CM, 0U, SEG_FLAG_TASK1_BLACK_STOP },
    { SEG_STOP, NODE_B, NODE_B, 0.0f, 0.0f, 0.0f, 0U,
      LINE_EXIT_BY_DISTANCE, 0.0f, 0U, 0U },
};

static const AppRouteSegment route_task2[ROUTE_TASK2_COUNT] = {
    { SEG_BLIND, NODE_A, NODE_B, 9999.0f, 0.0f,
      BLIND_BASE_SPEED_T2, SEG_TIMEOUT_BLIND_MS, LINE_EXIT_BY_DISTANCE,
      0.0f, 1U, SEG_FLAG_BLIND_STOP_ON_BLACK | SEG_FLAG_ABS_ROUTE_YAW },
    { SEG_LINE, NODE_B, NODE_C, 0.0f, 0.0f,
      LINE_BASE_SPEED, SEG_TIMEOUT_LINE_MS, LINE_EXIT_BY_TIMEOUT,
      0.0f, 1U, SEG_FLAG_LINE_END_ON_LOST },
    { SEG_BLIND, NODE_C, NODE_D, 9999.0f, 180.0f,
      BLIND_BASE_SPEED_T2, SEG_TIMEOUT_BLIND_MS, LINE_EXIT_BY_DISTANCE,
      0.0f, 1U, SEG_FLAG_BLIND_STOP_ON_BLACK | SEG_FLAG_ABS_ROUTE_YAW },
    { SEG_LINE, NODE_D, NODE_A, 0.0f, 0.0f,
      LINE_BASE_SPEED, SEG_TIMEOUT_LINE_MS, LINE_EXIT_BY_TIMEOUT,
      0.0f, 1U, SEG_FLAG_LINE_END_ON_LOST },
    { SEG_STOP, NODE_A, NODE_A, 0.0f, 0.0f, 0.0f, 0U,
      LINE_EXIT_BY_DISTANCE, 0.0f, 0U, 0U },
};

static const AppRouteSegment route_task3[ROUTE_TASK3_COUNT] = {
    { SEG_BLIND, NODE_A, NODE_C, FIELD_AC_CM, YAW_A_TO_C_DEG,
      BLIND_BASE_SPEED_T3, SEG_TIMEOUT_BLIND_MS, LINE_EXIT_BY_DISTANCE,
      0.0f, 1U, 0U },
    { SEG_LINE, NODE_C, NODE_B, LINE_C_TO_B_CM, YAW_C_TO_B_LINE_DEG,
      LINE_BASE_SPEED, SEG_TIMEOUT_LINE_MS, LINE_EXIT_BY_DISTANCE_OR_NODE,
      30.0f, 1U, 0U },
    { SEG_BLIND, NODE_B, NODE_D, FIELD_BD_CM, YAW_B_TO_D_DEG,
      BLIND_BASE_SPEED_T3, SEG_TIMEOUT_BLIND_MS, LINE_EXIT_BY_DISTANCE,
      0.0f, 1U, 0U },
    { SEG_LINE, NODE_D, NODE_A, LINE_D_TO_A_CM, YAW_D_TO_A_LINE_DEG,
      LINE_BASE_SPEED, SEG_TIMEOUT_LINE_MS, LINE_EXIT_BY_DISTANCE_OR_NODE,
      30.0f, 1U, 0U },
    { SEG_STOP, NODE_A, NODE_A, 0.0f, 0.0f, 0.0f, 0U,
      LINE_EXIT_BY_DISTANCE, 0.0f, 0U, 0U },
};

static const AppRouteSegment route_task4[ROUTE_TASK4_COUNT] = {
    { SEG_BLIND, NODE_A, NODE_C, FIELD_AC_CM, YAW_A_TO_C_DEG,
      BLIND_BASE_SPEED_T4, SEG_TIMEOUT_BLIND_MS, LINE_EXIT_BY_DISTANCE,
      0.0f, 1U, 0U },
    { SEG_LINE, NODE_C, NODE_B, LINE_C_TO_B_CM, YAW_C_TO_B_LINE_DEG,
      LINE_SLOW_SPEED, SEG_TIMEOUT_LINE_MS, LINE_EXIT_BY_DISTANCE_OR_NODE,
      30.0f, 1U, 0U },
    { SEG_BLIND, NODE_B, NODE_D, FIELD_BD_CM, YAW_B_TO_D_DEG,
      BLIND_BASE_SPEED_T4, SEG_TIMEOUT_BLIND_MS, LINE_EXIT_BY_DISTANCE,
      0.0f, 1U, 0U },
    { SEG_LINE, NODE_D, NODE_A, LINE_D_TO_A_CM, YAW_D_TO_A_LINE_DEG,
      LINE_SLOW_SPEED, SEG_TIMEOUT_LINE_MS, LINE_EXIT_BY_DISTANCE_OR_NODE,
      30.0f, 1U, 0U },
    { SEG_STOP, NODE_A, NODE_A, 0.0f, 0.0f, 0.0f, 0U,
      LINE_EXIT_BY_DISTANCE, 0.0f, 0U, 0U },
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
