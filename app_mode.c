#include "app_mode.h"

#include "Algorithm/motor.h"
#include "alert.h"
#include "app_event.h"
#include "app_runner.h"
#include "key.h"

volatile uint8_t selected_task = 1U;
volatile uint8_t system_mode = SYS_SELECT;

void AppMode_Init(void)
{
    selected_task = 1U;
    system_mode = SYS_SELECT;
    Motor_Stop();
    LED_Task_Off();
    LED_ShowTask(selected_task);
}

void AppMode_Task(void)
{
    if (system_mode == SYS_SELECT) {
        Motor_Stop();
        LED_Task_Off();
        (void)Key_GetStartShortPress();

        if (Key_GetModeShortPress() != 0U) {
            selected_task++;
            if (selected_task > 4U) {
                selected_task = 1U;
            }

            LED_ShowTask(selected_task);
            Beep(60U);
        }

        if (Key_GetStartLongPress() != 0U) {
            AppEvent_StartHint();
            if (AppRunner_Start(selected_task) != 0U) {
                system_mode = SYS_RUN;
            } else {
                system_mode = SYS_FAULT;
            }
        }

        return;
    }

    if (system_mode == SYS_RUN) {
        (void)Key_GetModeShortPress();
        (void)Key_GetStartShortPress();
        (void)Key_GetStartLongPress();

        if (run_finished != 0U) {
            Motor_Stop();
            LED_Task_Off();
            system_mode = SYS_FINISHED;
        }

        return;
    }

    if (system_mode == SYS_FINISHED) {
        Motor_Stop();
        LED_Task_Off();
        (void)Key_GetStartShortPress();

        if (Key_GetModeShortPress() != 0U) {
            AppRunner_Stop();
            system_mode = SYS_SELECT;
            LED_ShowTask(selected_task);
            Beep(60U);
            return;
        }

        if (Key_GetStartLongPress() != 0U) {
            AppEvent_StartHint();
            if (AppRunner_Start(selected_task) != 0U) {
                system_mode = SYS_RUN;
            } else {
                system_mode = SYS_FAULT;
            }
        }

        return;
    }

    if (system_mode == SYS_FAULT) {
        Motor_Stop();
        LED_Task_Off();
    }
}
