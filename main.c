#include <stdint.h>

#include "ti_msp_dl_config.h"
#include "alert.h"
#include "Algorithm/gray.h"
#include "Algorithm/motor.h"
#include "app_arc.h"
#include "app_line.h"
#include "app_mode.h"
#include "app_motion.h"
#include "app_runner.h"
#include "app_straight.h"
#include "app_task.h"
#include "gyro.h"
#include "key.h"

int main(void)
{
    SYSCFG_DL_init();

    Alert_Init();
    Key_Init();
    Motor_Init();
    Gray_Init();
    AppLine_Init();
    Motion_Init();
    AppStraight_Init();
    AppArc_Init();
    AppTask_Init();
    AppMode_Init();

    while (1) {
        Key_Task();
        Gray_Task();
        Gyro_Task();
        /* If a real Encoder_Task() exists later, call it here. */
        AppLine_Task();
        AppMode_Task();
        AppTask_Task();
        AppRunner_Task();
    }
}
