#include "gray.h"

#include <stdint.h>

#include "ti_msp_dl_config.h"

#define GRAY_I2C_ADDR          (0x4FU)
#define GRAY_CMD_PING          (0xAAU)
#define GRAY_PING_RESPONSE     (0x66U)
#define GRAY_CMD_READ_ANALOG   (0xB0U)
#define GRAY_CHANNEL_COUNT     (8U)
#define GRAY_INIT_RETRY_COUNT  (5U)
#define GRAY_I2C_TIMEOUT       (100000UL)

#if !defined(I2C_0_INST)
#error "I2C_0_INST is not defined. Check ti_msp_dl_config.h for the real SysConfig I2C instance macro name."
#endif

#define GRAY_I2C_INST          I2C_0_INST

volatile uint8_t gray_value_0 = 0U;
volatile uint8_t gray_value_1 = 0U;
volatile uint8_t gray_value_2 = 0U;
volatile uint8_t gray_value_3 = 0U;
volatile uint8_t gray_value_4 = 0U;
volatile uint8_t gray_value_5 = 0U;
volatile uint8_t gray_value_6 = 0U;
volatile uint8_t gray_value_7 = 0U;
volatile uint8_t gray_ping_ok = 0U;
volatile uint8_t gray_last_cmd = 0U;

static int Gray_I2C_WaitIdle(I2C_Regs *i2c)
{
    uint32_t timeout = GRAY_I2C_TIMEOUT;

    while ((DL_I2C_getControllerStatus(i2c) & DL_I2C_CONTROLLER_STATUS_IDLE) == 0U) {
        if (timeout-- == 0U) {
            return 0;
        }
    }

    return 1;
}

static int Gray_I2C_WaitBusFree(I2C_Regs *i2c)
{
    uint32_t timeout = GRAY_I2C_TIMEOUT;

    while ((DL_I2C_getControllerStatus(i2c) & DL_I2C_CONTROLLER_STATUS_BUSY_BUS) != 0U) {
        if (timeout-- == 0U) {
            return 0;
        }
    }

    return ((DL_I2C_getControllerStatus(i2c) & DL_I2C_CONTROLLER_STATUS_ERROR) == 0U);
}

static int Gray_I2C_WriteCommand(uint8_t command)
{
    uint8_t tx_data = command;

    gray_last_cmd = command;

    if (!Gray_I2C_WaitIdle(GRAY_I2C_INST)) {
        return 0;
    }

    DL_I2C_flushControllerTXFIFO(GRAY_I2C_INST);
    DL_I2C_flushControllerRXFIFO(GRAY_I2C_INST);
    DL_I2C_fillControllerTXFIFO(GRAY_I2C_INST, &tx_data, 1U);
    DL_I2C_startControllerTransfer(GRAY_I2C_INST, GRAY_I2C_ADDR,
        DL_I2C_CONTROLLER_DIRECTION_TX, 1U);

    return Gray_I2C_WaitBusFree(GRAY_I2C_INST);
}

static int Gray_I2C_ReadBytes(uint8_t *buffer, uint8_t length)
{
    if (buffer == 0 || length == 0U) {
        return 0;
    }

    if (!Gray_I2C_WaitIdle(GRAY_I2C_INST)) {
        return 0;
    }

    DL_I2C_flushControllerRXFIFO(GRAY_I2C_INST);
    DL_I2C_startControllerTransfer(GRAY_I2C_INST, GRAY_I2C_ADDR,
        DL_I2C_CONTROLLER_DIRECTION_RX, length);

    for (uint8_t i = 0; i < length; i++) {
        uint32_t timeout = GRAY_I2C_TIMEOUT;

        while (DL_I2C_isControllerRXFIFOEmpty(GRAY_I2C_INST)) {
            if (timeout-- == 0U) {
                return 0;
            }
        }

        buffer[i] = DL_I2C_receiveControllerData(GRAY_I2C_INST);
    }

    return Gray_I2C_WaitBusFree(GRAY_I2C_INST);
}

static int Gray_I2C_CommandRead(uint8_t command, uint8_t *buffer, uint8_t length)
{
    if (!Gray_I2C_WriteCommand(command)) {
        return 0;
    }

    return Gray_I2C_ReadBytes(buffer, length);
}

void Gray_Init(void)
{
    uint8_t response = 0U;

    gray_value_0 = 0U;
    gray_value_1 = 0U;
    gray_value_2 = 0U;
    gray_value_3 = 0U;
    gray_value_4 = 0U;
    gray_value_5 = 0U;
    gray_value_6 = 0U;
    gray_value_7 = 0U;
    gray_ping_ok = 0U;
    gray_last_cmd = 0U;

    for (uint8_t i = 0U; i < GRAY_INIT_RETRY_COUNT; i++) {
        response = 0U;

        if (Gray_I2C_CommandRead(GRAY_CMD_PING, &response, 1U) &&
            response == GRAY_PING_RESPONSE) {
            gray_ping_ok = 1U;
            break;
        }
    }
}

void Gray_Task(void)
{
    uint8_t raw[GRAY_CHANNEL_COUNT] = {0U};

    if (!Gray_I2C_CommandRead(GRAY_CMD_READ_ANALOG, raw, GRAY_CHANNEL_COUNT)) {
        return;
    }

    gray_value_0 = raw[0];
    gray_value_1 = raw[1];
    gray_value_2 = raw[2];
    gray_value_3 = raw[3];
    gray_value_4 = raw[4];
    gray_value_5 = raw[5];
    gray_value_6 = raw[6];
    gray_value_7 = raw[7];
}
