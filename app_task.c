#include "app_task.h"

#include "alert.h"
#include "app_mode.h"
#include "app_route.h"
#include "app_runner.h"

void AppTask_Init(void)
{
    AppRoute_Init();
    AppRunner_Init();
}

uint8_t AppTask_Load(uint8_t task_id)
{
    return AppRoute_LoadTask(task_id);
}

void AppTask_Start(void)
{
    if (route_loaded != 0U) {
        AppRunner_Start();
        Alert_NodeHint(NODE_A);
    }
}

void AppTask_Cancel(void)
{
    AppRunner_Stop();
    AppRoute_Clear();
}

void AppTask_Task(void)
{
    if (system_mode == SYS_RUN && run_finished != 0U) {
        system_mode = SYS_FINISHED;
        LED_Task_Off();
    }
}
