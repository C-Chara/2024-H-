#include "ti_msp_dl_config.h"

#include "Algorithm/gray.h"
#include "Algorithm/motor.h"
#include "atk_ms901m.h"
#include "alert.h"
#include "app_arc.h"
#include "app_blind.h"
#include "app_event.h"
#include "app_line.h"
#include "app_mode.h"
#include "app_route.h"
#include "app_runner.h"
#include "app_sensor.h"
#include "app_task.h"
#include "app_time.h"
#include "encoder.h"
#include "gyro.h"
#include "key.h"

int main(void)
{
    SYSCFG_DL_init();
    AppTime_Init();

    Motor_Init();
    Encoder_Init();
    Gray_Init();
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
    Sensor_Fusion_Init();

    if (atk_ms901m_init(115200U) == 0U) {
        gyro_ok = 1U;
        Gyro_SetSoftwareZero();
    } else {
        gyro_ok = 0U;
    }

    while (1) {
        Key_Task();
        atk_ms901m_poll();
        gray_poll();
        Encoder_Task();
        Sensor_Fusion_Task();
        AppMode_Task();
        AppRunner_Task();
        Event_Task();
    }
}
