#include "app_task1.h"

#include "alert.h"
#include "app_line.h"
#include "app_mode.h"
#include "app_motion.h"

volatile uint8_t task1_state = TASK1_IDLE;
volatile uint8_t stop_flag = 0U;

static uint8_t stop_feedback_done = 0U;

static uint8_t AppTask1_ShouldStop(void)
{
    /*
     * Placeholder for B-point detection.
     * For now, set stop_flag = 1 in CCS Watch to force the stop path.
     */
    return stop_flag;
}

void AppTask1_Init(void)
{
    task1_state = TASK1_IDLE;
    stop_flag = 0U;
    stop_feedback_done = 0U;
}

void AppTask1_Task(void)
{
    if (selected_task != 1U || system_mode != SYS_RUN) {
        if (task1_state == TASK1_IDLE) {
            Motion_Stop();
        }
        return;
    }

    switch (task1_state) {
        case TASK1_IDLE:
            Motion_SetBaseSpeed(250);
            task1_state = TASK1_RUN_TO_B;
            break;

        case TASK1_RUN_TO_B:
            if (AppTask1_ShouldStop() != 0U) {
                task1_state = TASK1_STOP;
                break;
            }

            if (line_found != 0U) {
                Motion_UpdateByLineError(line_error);
            } else {
                Motion_Stop();
            }
            break;

        case TASK1_STOP:
            Motion_Stop();
            stop_flag = 1U;
            if (stop_feedback_done == 0U) {
                LED_Task_On();
                Beep(150U);
                LED_Task_Off();
                Beep(150U);
                LED_Task_On();
                Beep(150U);
                LED_Task_Off();
                stop_feedback_done = 1U;
            }
            task1_state = TASK1_DONE;
            break;

        case TASK1_DONE:
        default:
            Motion_Stop();
            break;
    }
}
