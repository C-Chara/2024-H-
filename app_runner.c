#include "app_runner.h"

#include "Algorithm/motor.h"
#include "alert.h"
#include "app_arc.h"
#include "app_route.h"
#include "app_straight.h"

volatile uint8_t run_active = 0U;
volatile uint8_t run_finished = 0U;

static uint8_t runner_segment_started = 0U;
static uint8_t runner_last_hint_node = 0xFFU;
static uint16_t runner_led_tick = 0U;

static void Runner_NodeHintOnce(uint8_t node_id)
{
    if (runner_last_hint_node != node_id) {
        Alert_NodeHint(node_id);
        runner_last_hint_node = node_id;
    }
}

void AppRunner_Init(void)
{
    run_active = 0U;
    run_finished = 0U;
    runner_segment_started = 0U;
    runner_last_hint_node = 0xFFU;
    runner_led_tick = 0U;
}

void AppRunner_Start(void)
{
    if (route_loaded == 0U) {
        run_active = 0U;
        run_finished = 0U;
        return;
    }

    run_active = 1U;
    run_finished = 0U;
    runner_segment_started = 0U;
    runner_last_hint_node = 0xFFU;
    runner_led_tick = 0U;
    LED_Task_On();
}

void AppRunner_Stop(void)
{
    Motor_Stop();
    run_active = 0U;
    runner_segment_started = 0U;
    LED_Task_Off();
}

void AppRunner_Task(void)
{
    const AppRouteSegment *segment;

    if (run_active == 0U) {
        return;
    }

    runner_led_tick++;
    if (runner_led_tick >= 100U) {
        runner_led_tick = 0U;
        LED_Task_Toggle();
    }

    segment = AppRoute_CurrentSegment();
    if (segment == 0) {
        AppRunner_Stop();
        run_finished = 1U;
        return;
    }

    if (runner_segment_started == 0U) {
        if (segment->type == SEG_STRAIGHT_BLIND ||
            segment->type == SEG_DIAGONAL_BLIND) {
            AppStraight_Start(segment);
        } else if (segment->type == SEG_ARC_LINE) {
            AppArc_Start(segment);
        }

        runner_segment_started = 1U;
    }

    if (segment->type == SEG_STRAIGHT_BLIND ||
        segment->type == SEG_DIAGONAL_BLIND) {
        AppStraight_Task();
        if (AppStraight_IsDone() != 0U) {
            Runner_NodeHintOnce(segment->to_node);
            AppRoute_AdvanceSegment();
            runner_segment_started = 0U;
        }
    } else if (segment->type == SEG_ARC_LINE) {
        AppArc_Task();
        if (AppArc_IsDone() != 0U) {
            Runner_NodeHintOnce(segment->to_node);
            AppRoute_AdvanceSegment();
            runner_segment_started = 0U;
        }
    } else if (segment->type == SEG_STOP_HINT) {
        Motor_Stop();
        Runner_NodeHintOnce(segment->to_node);
        AppRoute_AdvanceSegment();
        AppRunner_Stop();
        run_finished = 1U;
    } else {
        AppRunner_Stop();
        run_finished = 1U;
    }
}
