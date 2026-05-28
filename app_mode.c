#include "app_mode.h"

#include "alert.h"
#include "app_task.h"
#include "key.h"

volatile uint8_t selected_task = 1U;
volatile uint8_t system_mode = SYS_BOOT;

void AppMode_Init(void)
{
    selected_task = 1U;
    system_mode = SYS_SELECT;
    LED_Task_Off();
    LED_ShowTask(selected_task);
}

void AppMode_Task(void)
{
    if (system_mode == SYS_SELECT) {
        (void)Key_GetStartLongPress();

        if (Key_GetModeShortPress() != 0U) {
            selected_task++;
            if (selected_task > 4U) {
                selected_task = 1U;
            }

            LED_ShowTask(selected_task);
            Beep(60U);
        }

        if (Key_GetStartShortPress() != 0U) {
            if (AppTask_Load(selected_task) != 0U) {
                system_mode = SYS_ARMED;
                LED_Task_On();
                Beep(120U);
            } else {
                system_mode = SYS_FAULT;
                LED_Task_Off();
                Beep(200U);
            }
        }

        return;
    }

    if (system_mode == SYS_ARMED) {
        (void)Key_GetStartShortPress();

        if (Key_GetModeShortPress() != 0U) {
            AppTask_Cancel();
            system_mode = SYS_SELECT;
            LED_Task_Off();
            LED_ShowTask(selected_task);
            Beep(60U);
            return;
        }

        if (Key_GetStartLongPress() != 0U) {
            AppTask_Start();
            system_mode = SYS_RUN;
            LED_Task_Off();
            Beep(80U);
            Beep(80U);
        }

        return;
    }

    if (system_mode == SYS_RUN) {
        (void)Key_GetModeShortPress();
        (void)Key_GetStartShortPress();
        (void)Key_GetStartLongPress();
        return;
    }

    if (system_mode == SYS_FINISHED) {
        (void)Key_GetStartLongPress();

        if (Key_GetStartShortPress() != 0U) {
            if (AppTask_Load(selected_task) != 0U) {
                AppTask_Start();
                system_mode = SYS_RUN;
                Beep(80U);
            }
            return;
        }

        if (Key_GetModeShortPress() != 0U) {
            AppTask_Cancel();
            system_mode = SYS_SELECT;
            LED_Task_Off();
            LED_ShowTask(selected_task);
            Beep(60U);
        }
    }
}
