#include "gyro.h"

#include <stdint.h>

#include "ti_msp_dl_config.h"

#define GYRO_FRAME_HEADER        (0x55U)
#define GYRO_FRAME_ID_ATTITUDE   (0x01U)
#define GYRO_FRAME_DATA_LENGTH   (0x06U)
#define GYRO_FRAME_SIZE          (11U)

volatile float gyro_roll = 0.0f;
volatile float gyro_pitch = 0.0f;
volatile float gyro_yaw = 0.0f;

static uint8_t gyro_frame[GYRO_FRAME_SIZE];
static uint8_t gyro_index = 0U;

static float Gyro_BytesToAngle(uint8_t low, uint8_t high)
{
    int16_t raw = (int16_t)(((uint16_t)high << 8) | (uint16_t)low);

    return ((float)raw / 32768.0f) * 180.0f;
}

static void Gyro_ParseFrame(void)
{
    uint8_t sum = 0U;

    if ((gyro_frame[2] != GYRO_FRAME_ID_ATTITUDE) ||
        (gyro_frame[3] != GYRO_FRAME_DATA_LENGTH)) {
        return;
    }

    for (uint8_t i = 0U; i < (GYRO_FRAME_SIZE - 1U); i++) {
        sum = (uint8_t)(sum + gyro_frame[i]);
    }

    if (sum != gyro_frame[GYRO_FRAME_SIZE - 1U]) {
        return;
    }

    gyro_roll = Gyro_BytesToAngle(gyro_frame[4], gyro_frame[5]);
    gyro_pitch = Gyro_BytesToAngle(gyro_frame[6], gyro_frame[7]);
    gyro_yaw = Gyro_BytesToAngle(gyro_frame[8], gyro_frame[9]);
}

static void Gyro_ParseByte(uint8_t byte)
{
    if (gyro_index == 0U) {
        if (byte == GYRO_FRAME_HEADER) {
            gyro_frame[gyro_index++] = byte;
        }
        return;
    }

    if (gyro_index == 1U) {
        if (byte == GYRO_FRAME_HEADER) {
            gyro_frame[gyro_index++] = byte;
        } else {
            gyro_index = 0U;
        }
        return;
    }

    gyro_frame[gyro_index++] = byte;

    if (gyro_index >= GYRO_FRAME_SIZE) {
        Gyro_ParseFrame();
        gyro_index = 0U;
    }
}

void Gyro_Task(void)
{
    while (!DL_UART_Main_isRXFIFOEmpty(GYRO_UART_INST)) {
        Gyro_ParseByte(DL_UART_Main_receiveData(GYRO_UART_INST));
    }
}
