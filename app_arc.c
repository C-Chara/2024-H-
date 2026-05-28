#include "app_arc.h"

#include "Algorithm/motor.h"
#include "app_config.h"
#include "app_line.h"
#include "encoder.h"

volatile uint8_t arc_state = ARC_STATE_IDLE;
volatile float arc_distance_cm = 0.0f;
volatile int16_t arc_left_cmd = 0;
volatile int16_t arc_right_cmd = 0;

static float arc_target_length_cm = 0.0f;
static int16_t arc_last_error = 0;
static int16_t arc_turn_cmd = 0;
static uint8_t arc_lost_count = 0U;

static int16_t Arc_ClampForward(int32_t value)
{
    if (value < 0) {
        return 0;
    }

    if (value > ARC_MAX_SPEED) {
        return ARC_MAX_SPEED;
    }

    return (int16_t)value;
}

void AppArc_Init(void)
{
    arc_state = ARC_STATE_IDLE;
    arc_distance_cm = 0.0f;
    arc_left_cmd = 0;
    arc_right_cmd = 0;
    arc_target_length_cm = 0.0f;
    arc_last_error = 0;
    arc_turn_cmd = 0;
    arc_lost_count = 0U;
}

void AppArc_Start(const AppRouteSegment *segment)
{
    arc_state = ARC_STATE_RUN;
    arc_distance_cm = 0.0f;
    arc_left_cmd = 0;
    arc_right_cmd = 0;
    arc_target_length_cm = (segment != 0) ? segment->target_distance_cm : 0.0f;
    arc_last_error = 0;
    arc_turn_cmd = 0;
    arc_lost_count = 0U;

    Encoder_ResetDistance();
}

void AppArc_Task(void)
{
    int16_t d_error;
    float turn_float;

    if (arc_state != ARC_STATE_RUN) {
        return;
    }

    Encoder_Task();
    arc_distance_cm = Encoder_GetDistanceCm();
    AppLine_Task();

    if (arc_distance_cm >= ARC_EXIT_ENABLE_CM &&
        arc_target_length_cm > 0.0f &&
        arc_distance_cm >= arc_target_length_cm) {
        Motor_Stop();
        arc_left_cmd = 0;
        arc_right_cmd = 0;
        arc_state = ARC_STATE_DONE;
        return;
    }

    /*
     * Arc segments are the only line-following parts.
     * Use low speed and black-strength centroid from app_line for the 1.8 cm line.
     */
    if (line_found != 0U && line_dark_sum >= LINE_FOUND_THRESHOLD) {
        arc_lost_count = 0U;
        d_error = (int16_t)(line_error - arc_last_error);
        arc_last_error = line_error;

        turn_float = ARC_KP_LINE * (float)line_error +
            ARC_KD_LINE * (float)d_error;
        arc_turn_cmd = (int16_t)turn_float;

        arc_left_cmd = Arc_ClampForward((int32_t)ARC_BASE_SPEED -
            (int32_t)arc_turn_cmd);
        arc_right_cmd = Arc_ClampForward((int32_t)ARC_BASE_SPEED +
            (int32_t)arc_turn_cmd);
        Motor_SetLeft(arc_left_cmd);
        Motor_SetRight(arc_right_cmd);
    } else {
        if (arc_lost_count < 255U) {
            arc_lost_count++;
        }

        if (arc_lost_count >= ARC_LOST_LINE_LIMIT) {
            Motor_Stop();
            arc_left_cmd = 0;
            arc_right_cmd = 0;
            arc_state = ARC_STATE_LOST;
        }
    }
}

uint8_t AppArc_IsDone(void)
{
    return (arc_state == ARC_STATE_DONE) ? 1U : 0U;
}
