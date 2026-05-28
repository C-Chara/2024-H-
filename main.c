#include "ti_msp_dl_config.h"

#include "Algorithm/gray.h"
#include "Algorithm/motor.h"
#include "alert.h"
#include "app_arc.h"
#include "app_blind.h"
#include "app_event.h"
#include "app_line.h"
#include "app_mode.h"
#include "app_route.h"
#include "app_runner.h"
#include "app_task.h"
#include "encoder.h"
#include "gyro.h"
#include "key.h"

int main(void)
{
    SYSCFG_DL_init();

    Motor_Init();
    Encoder_Init();
    Gray_Init();
    /* Gyro_Init() is not present in the current project; Gyro_Task() is used. */
    Alert_Init();
    Key_Init();
    AppMode_Init();
    AppLine_Init();
    AppBlind_Init();
    AppArc_Init();
    AppEvent_Init();
    AppRoute_Init();
    AppRunner_Init();
    AppTask_Init();

    while (1) {
        Key_Task();
        Gray_Task();
        Gyro_Task();
        Encoder_Task();
        AppLine_Task();
        AppMode_Task();
        AppRunner_Task();
    }
}
