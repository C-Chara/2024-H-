#include "app_task1.h"

/*
 * Compatibility wrapper for older project references.
 * The current task framework runs task 1 through app_route/app_runner/app_straight.
 */
void AppTask1_Init(void)
{
    AppTask_Init();
}

void AppTask1_Task(void)
{
    /* No standalone task-1 loop here. */
}
