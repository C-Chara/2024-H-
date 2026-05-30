#include "app_mode.h"

#include "Algorithm/motor.h"
#include "alert.h"
#include "app_event.h"
#include "app_runner.h"
#include "app_sensor.h"
#include "key.h"

volatile uint8_t selected_task = 1U;
volatile uint8_t system_mode = SYS_SELECT;

static uint8_t start_pending = 0U;
static uint8_t pending_task = 1U;

static void AppMode_StartPendingTask(void)
{
    if (AppRunner_Start(pending_task) != 0U) {
        system_mode = SYS_RUN;
    } else {
        system_mode = SYS_FAULT;
    }
    start_pending = 0U;
}

void AppMode_Init(void)
{
    selected_task = 1U;
    system_mode = SYS_SELECT;
    start_pending = 0U;
    pending_task = 1U;
    Motor_Stop();
    LED_Task_Off();
    LED_ShowTask(selected_task);
}

void AppMode_Task(void)
{
    if (system_mode == SYS_SELECT) {
        Motor_Stop();
        (void)Key_GetStartShortPress();

        if (start_pending != 0U) {
            if (AppEvent_IsBusy() == 0U) {
                AppMode_StartPendingTask();
            }
            return;
        }

        if (Key_GetModeShortPress() != 0U) {
            selected_task++;
            if (selected_task > 4U) {
                selected_task = 1U;
            }

            LED_ShowTask(selected_task);
            AppEvent_SelectHint();
        }

        if (Key_GetStartLongPress() != 0U) {
            pending_task = selected_task;
            start_pending = 1U;
            Sensor_StartYawCaptureReset();
            AppEvent_StartHint();
        }

        return;
    }

    if (system_mode == SYS_RUN) {
        (void)Key_GetModeShortPress();
        (void)Key_GetStartShortPress();
        (void)Key_GetStartLongPress();

        if (run_finished != 0U) {
            Motor_Stop();
            system_mode = SYS_FINISHED;
        }

        return;
    }

    if (system_mode == SYS_FINISHED) {
        Motor_Stop();
        (void)Key_GetStartShortPress();

        if (start_pending != 0U) {
            if (AppEvent_IsBusy() == 0U) {
                AppMode_StartPendingTask();
            }
            return;
        }

        if (Key_GetModeShortPress() != 0U) {
            AppRunner_Stop();
            start_pending = 0U;
            system_mode = SYS_SELECT;
            LED_ShowTask(selected_task);
            AppEvent_SelectHint();
            return;
        }

        if (Key_GetStartLongPress() != 0U) {
            pending_task = selected_task;
            start_pending = 1U;
            Sensor_StartYawCaptureReset();
            AppEvent_StartHint();
        }

        return;
    }

    if (system_mode == SYS_FAULT) {
        Motor_Stop();
    }
}
