#include "ti_msp_dl_config.h"

#include <stdint.h>

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

volatile uint32_t app_main_loop_count = 0U;
volatile uint32_t key_task_count = 0U;
volatile uint32_t imu_poll_enter_count = 0U;
volatile uint32_t gray_poll_enter_count = 0U;
volatile uint32_t runner_task_count = 0U;
volatile uint32_t event_task_count = 0U;
volatile uint8_t runtime_block_source_dbg = 0U;

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
        app_main_loop_count++;

        runtime_block_source_dbg = 1U;
        key_task_count++;
        Key_Task();

        runtime_block_source_dbg = 2U;
        imu_poll_enter_count++;
        atk_ms901m_poll();

        runtime_block_source_dbg = 3U;
        gray_poll_enter_count++;
        gray_poll();

        AppLine_Task();

        Encoder_Task();

        runtime_block_source_dbg = 4U;
        Sensor_Fusion_Task();
        Imu_Health_Task();

        runtime_block_source_dbg = 5U;
        AppMode_Task();

        runtime_block_source_dbg = 6U;
        runner_task_count++;
        AppRunner_Task();

        runtime_block_source_dbg = 7U;
        event_task_count++;
        Event_Task();

        runtime_block_source_dbg = 0U;
    }
}
