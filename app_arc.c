#include "app_arc.h"

#include "Algorithm/hal.h"
#include "app_line.h"
#include "app_motion.h"

#define ARC_LINE_BASE_SPEED       (120)
#define ARC_LINE_CONF_THRESHOLD   (120U)
#define ARC_ENCODER_COUNTS_PER_REV     (1560.0f)
#define ARC_WHEEL_CIRCUMFERENCE_CM     (20.4f)

volatile uint8_t arc_active = 0U;
volatile uint8_t arc_done_flag = 0U;
volatile float arc_distance_cm = 0.0f;
volatile float arc_target_length_cm = 0.0f;
volatile uint16_t arc_line_confidence = 0U;

static int32_t Arc_Abs32(int32_t value)
{
    return (value >= 0) ? value : -value;
}

void AppArc_Init(void)
{
    arc_active = 0U;
    arc_done_flag = 0U;
    arc_distance_cm = 0.0f;
    arc_target_length_cm = 0.0f;
    arc_line_confidence = 0U;
}

void AppArc_Start(const AppRouteSegment *segment)
{
    arc_distance_cm = 0.0f;
    arc_target_length_cm = (segment != 0) ? segment->distance_cm : 0.0f;
    arc_line_confidence = 0U;
    arc_active = 1U;
    arc_done_flag = 0U;

    /* Discard any old encoder delta before measuring arc length. */
    (void)HAL_Get_Left_Encoder_Delta();
    (void)HAL_Get_Right_Encoder_Delta();

    Motion_SetBaseSpeed(ARC_LINE_BASE_SPEED);
}

void AppArc_Task(void)
{
    int32_t left_delta;
    int32_t right_delta;
    float avg_counts;

    if (arc_active == 0U || arc_done_flag != 0U) {
        return;
    }

    left_delta = HAL_Get_Left_Encoder_Delta();
    right_delta = HAL_Get_Right_Encoder_Delta();
    avg_counts = ((float)Arc_Abs32(left_delta) +
        (float)Arc_Abs32(right_delta)) * 0.5f;
    arc_distance_cm += avg_counts * ARC_WHEEL_CIRCUMFERENCE_CM /
        ARC_ENCODER_COUNTS_PER_REV;

    arc_line_confidence = line_confidence;

    /*
     * The black arc is only about 1.8 cm wide, so line control must be gentle:
     * use normalized gray values, black-strength centroid, and confidence guard.
     */
    if (line_found != 0U && line_confidence >= ARC_LINE_CONF_THRESHOLD) {
        Motion_UpdateByLineError(line_error);
    } else {
        Motion_Stop();
    }

    if (arc_target_length_cm > 0.0f &&
        arc_distance_cm >= arc_target_length_cm) {
        Motion_Stop();
        arc_done_flag = 1U;
        arc_active = 0U;
    }
}

uint8_t AppArc_IsDone(void)
{
    return arc_done_flag;
}
