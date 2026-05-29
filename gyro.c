#include "gyro.h"

#include <stdint.h>

#include "ti_msp_dl_config.h"

#define GYRO_HEADER_0           (0x55U)
#define GYRO_HEADER_1           (0x55U)
#define GYRO_FRAME_ID_ATTITUDE  (0x01U)
#define GYRO_FRAME_DATA_LEN     (0x06U)
#define GYRO_FRAME_SIZE         (11U)

volatile uint32_t gyro_rx_count = 0U;
volatile uint32_t gyro_frame_count = 0U;
volatile uint32_t gyro_angle_frame_count = 0U;
volatile uint32_t gyro_checksum_error = 0U;
volatile uint8_t gyro_last_byte = 0U;
volatile float gyro_roll = 0.0f;
volatile float gyro_pitch = 0.0f;
volatile float gyro_yaw = 0.0f;

static volatile float gyro_yaw_zero = 0.0f;
static uint8_t gyro_frame[GYRO_FRAME_SIZE];
static uint8_t gyro_index = 0U;

static float Gyro_NormalizeAngle(float angle)
{
    while (angle > 180.0f) {
        angle -= 360.0f;
    }

    while (angle < -180.0f) {
        angle += 360.0f;
    }

    return angle;
}

static float Gyro_RawToAngle(uint8_t low, uint8_t high)
{
    int16_t raw = (int16_t)(((uint16_t)high << 8) | (uint16_t)low);

    return ((float)raw / 32768.0f) * 180.0f;
}

static void Gyro_ResetParser(uint8_t keep_header)
{
    if (keep_header != 0U) {
        gyro_frame[0] = GYRO_HEADER_0;
        gyro_index = 1U;
    } else {
        gyro_index = 0U;
    }
}

static void Gyro_ParseCompleteFrame(void)
{
    uint8_t sum = 0U;

    gyro_frame_count++;

    for (uint8_t i = 0U; i < (GYRO_FRAME_SIZE - 1U); i++) {
        sum = (uint8_t)(sum + gyro_frame[i]);
    }

    if (sum != gyro_frame[GYRO_FRAME_SIZE - 1U]) {
        gyro_checksum_error++;
        return;
    }

    if (gyro_frame[2] != GYRO_FRAME_ID_ATTITUDE ||
        gyro_frame[3] != GYRO_FRAME_DATA_LEN) {
        return;
    }

    gyro_roll = Gyro_RawToAngle(gyro_frame[4], gyro_frame[5]);
    gyro_pitch = Gyro_RawToAngle(gyro_frame[6], gyro_frame[7]);
    gyro_yaw = Gyro_RawToAngle(gyro_frame[8], gyro_frame[9]);
    gyro_angle_frame_count++;
}

static void Gyro_ParseByte(uint8_t byte)
{
    if (gyro_index == 0U) {
        if (byte == GYRO_HEADER_0) {
            gyro_frame[0] = byte;
            gyro_index = 1U;
        }
        return;
    }

    if (gyro_index == 1U) {
        if (byte == GYRO_HEADER_1) {
            gyro_frame[1] = byte;
            gyro_index = 2U;
        } else {
            Gyro_ResetParser((byte == GYRO_HEADER_0) ? 1U : 0U);
        }
        return;
    }

    gyro_frame[gyro_index] = byte;
    gyro_index++;

    if (gyro_index >= GYRO_FRAME_SIZE) {
        Gyro_ParseCompleteFrame();
        Gyro_ResetParser(0U);
    }
}

void Gyro_Task(void)
{
#if defined(GYRO_UART_INST)
    while (!DL_UART_Main_isRXFIFOEmpty(GYRO_UART_INST)) {
        uint8_t byte = DL_UART_Main_receiveData(GYRO_UART_INST);

        gyro_rx_count++;
        gyro_last_byte = byte;
        Gyro_ParseByte(byte);
    }
#else
    /*
     * Check ti_msp_dl_config.h for the real UART instance macro, such as:
     * GYRO_UART_INST, GYRO_UART_INST_IRQHandler, GYRO_UART_INST_INT_IRQN.
     */
#endif
}

float Gyro_GetYaw(void)
{
    return gyro_yaw;
}

void Gyro_SetSoftwareZero(void)
{
    gyro_yaw_zero = gyro_yaw;
}

float Gyro_GetRelativeYaw(void)
{
    return Gyro_NormalizeAngle(gyro_yaw - gyro_yaw_zero);
}
