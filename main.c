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
#include "app_task.h"
#include "encoder.h"
#include "gyro.h"
#include "key.h"

#define GYRO_POLL_DIV_THRESHOLD (200U)
#define GYRO_ATTITUDE_TIMEOUT   (60000U)

volatile uint32_t gyro_poll_count = 0U;

int main(void)
{
    static atk_ms901m_attitude_data_t gyro_attitude;
    static uint32_t imu_div = 0U;

    SYSCFG_DL_init();

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

    if (atk_ms901m_init(115200U) == 0U) {
        gyro_ok = 1U;
        Gyro_SetSoftwareZero();
    } else {
        gyro_ok = 0U;
    }

    while (1) {
        Key_Task();
        Gray_Task();
        if (gyro_ok != 0U) {
            imu_div++;
            if (imu_div >= GYRO_POLL_DIV_THRESHOLD) {
                imu_div = 0U;
                gyro_poll_count++;

                if (atk_ms901m_get_attitude(&gyro_attitude,
                        GYRO_ATTITUDE_TIMEOUT) == ATK_MS901M_EOK) {
                    gyro_roll = gyro_attitude.roll;
                    gyro_pitch = gyro_attitude.pitch;
                    gyro_yaw = gyro_attitude.yaw;
                    gyro_attitude_ok_count++;
                } else {
                    gyro_attitude_fail_count++;
                }
            }
        }
        Encoder_Task();
        AppLine_Task();
        AppMode_Task();
        AppRunner_Task();
    }
}
