#include "app_runner.h"

#include "Algorithm/motor.h"
#include "app_arc.h"
#include "app_blind.h"
#include "app_event.h"
#include "app_mode.h"
#include "app_route.h"
#include "app_task.h"

volatile uint8_t run_active = 0U;
volatile uint8_t run_finished = 0U;

static uint8_t runner_segment_started = 0U;
static uint8_t runner_task_id = 1U;

static uint8_t Runner_IsBlindSegment(uint8_t type)
{
    return (type == SEG_STRAIGHT_BLIND || type == SEG_DIAGONAL_BLIND) ?
        1U : 0U;
}

static void Runner_ArriveNode(uint8_t node_id)
{
    const AppRouteSegment *next_segment;

    AppRoute_AdvanceSegment();
    next_segment = AppRoute_CurrentSegment();

    if (next_segment != 0 &&
        next_segment->type == SEG_STOP_HINT &&
        next_segment->to_node == node_id) {
        return;
    }

    AppEvent_NodeHint(node_id);
}

void AppRunner_Init(void)
{
    run_active = 0U;
    run_finished = 0U;
    runner_segment_started = 0U;
    runner_task_id = 1U;
}

uint8_t AppRunner_Start(uint8_t task_id)
{
    if (AppRoute_LoadTask(task_id) == 0U) {
        run_active = 0U;
        run_finished = 0U;
        return 0U;
    }

    AppTask_ResetForStart(task_id);
    run_active = 1U;
    run_finished = 0U;
    runner_segment_started = 0U;
    runner_task_id = task_id;
    return 1U;
}

void AppRunner_Stop(void)
{
    Motor_Stop();
    AppBlind_Stop();
    run_active = 0U;
    runner_segment_started = 0U;
}

void AppRunner_Task(void)
{
    const AppRouteSegment *segment;

    if (system_mode != SYS_RUN || run_active == 0U || run_finished != 0U) {
        return;
    }

    segment = AppRoute_CurrentSegment();
    if (segment == 0) {
        AppRunner_Stop();
        route_finished = 1U;
        run_finished = 1U;
        system_mode = SYS_FINISHED;
        return;
    }

    if (runner_segment_started == 0U) {
        if (Runner_IsBlindSegment(segment->type) != 0U) {
            AppBlind_EnableTask1BlackStop(
                (segment->flags & SEG_FLAG_TASK1_BLACK_STOP) != 0U);
            AppBlind_Start(segment->target_distance_cm,
                segment->yaw_offset_deg);
        } else if (segment->type == SEG_ARC_LINE) {
            AppArc_Start(segment);
        }

        runner_segment_started = 1U;
    }

    if (Runner_IsBlindSegment(segment->type) != 0U) {
        AppBlind_Task();
        if (AppBlind_IsDone() != 0U) {
            Runner_ArriveNode(segment->to_node);
            runner_segment_started = 0U;
        }
    } else if (segment->type == SEG_ARC_LINE) {
        AppArc_Task();
        if (AppArc_IsDone() != 0U) {
            Runner_ArriveNode(segment->to_node);
            runner_segment_started = 0U;
        }
    } else if (segment->type == SEG_STOP_HINT) {
        Motor_Stop();
        AppEvent_FinishHint();
        if (runner_task_id == 1U) {
            AppTask_MarkTask1Done();
        }
        AppRoute_AdvanceSegment();
        AppRunner_Stop();
        route_finished = 1U;
        run_finished = 1U;
        system_mode = SYS_FINISHED;
    } else {
        AppRunner_Stop();
        route_finished = 1U;
        run_finished = 1U;
        system_mode = SYS_FAULT;
    }
}
