#include "gray.h"

#include <stdint.h>

#include "ti_msp_dl_config.h"

#define GRAY_I2C_ADDR              (0x4CU)
#define GRAY_CMD_PING              (0xAAU)
#define GRAY_PING_RESPONSE         (0x66U)
#define GRAY_CMD_READ_ANALOG       (0xB0U)
#define GRAY_REG_CHANNEL_ENABLE    (0xCEU)
#define GRAY_REG_NORMALIZE_ENABLE  (0xCFU)
#define GRAY_ENABLE_ALL_CHANNELS   (0xFFU)
#define GRAY_CHANNEL_COUNT         (8U)
#define GRAY_PING_MAX_TRIES        (5U)
#define GRAY_I2C_TIMEOUT           (100000UL)

#if !defined(I2C_0_INST)
/* Check ti_msp_dl_config.h for the real SysConfig I2C instance macro name. */
#error "I2C_0_INST is not defined."
#endif

#define GRAY_I2C_INST              I2C_0_INST

volatile uint8_t gray_ping_ok = 0U;
volatile uint8_t gray_ping_resp = 0U;
volatile uint8_t gray_ce_readback = 0U;
volatile uint8_t gray_cf_readback = 0U;
volatile uint8_t gray_read_ok = 0U;
volatile uint8_t gray_last_cmd = 0U;
volatile uint8_t gray_fail_step = 0U;
volatile uint32_t gray_task_count = 0U;
volatile uint32_t gray_read_count = 0U;
volatile uint32_t gray_read_fail_count = 0U;
volatile uint8_t gray_value_0 = 0U;
volatile uint8_t gray_value_1 = 0U;
volatile uint8_t gray_value_2 = 0U;
volatile uint8_t gray_value_3 = 0U;
volatile uint8_t gray_value_4 = 0U;
volatile uint8_t gray_value_5 = 0U;
volatile uint8_t gray_value_6 = 0U;
volatile uint8_t gray_value_7 = 0U;

static void Gray_DelayMs(uint32_t ms)
{
    while (ms > 0U) {
        DL_Common_delayCycles(CPUCLK_FREQ / 1000U);
        ms--;
    }
}

static int Gray_I2C_WaitIdle(void)
{
    uint32_t timeout = GRAY_I2C_TIMEOUT;

    while ((DL_I2C_getControllerStatus(GRAY_I2C_INST) & DL_I2C_CONTROLLER_STATUS_IDLE) == 0U) {
        if (timeout-- == 0U) {
            return 0;
        }
    }

    return 1;
}

static int Gray_I2C_WaitBusFree(void)
{
    uint32_t timeout = GRAY_I2C_TIMEOUT;

    while ((DL_I2C_getControllerStatus(GRAY_I2C_INST) & DL_I2C_CONTROLLER_STATUS_BUSY_BUS) != 0U) {
        if (timeout-- == 0U) {
            return 0;
        }
    }

    return ((DL_I2C_getControllerStatus(GRAY_I2C_INST) & DL_I2C_CONTROLLER_STATUS_ERROR) == 0U);
}

static int Gray_I2C_WriteBytes(const uint8_t *buffer, uint8_t length)
{
    if (buffer == 0 || length == 0U) {
        gray_fail_step = 1U;
        return 0;
    }

    if (!Gray_I2C_WaitIdle()) {
        gray_fail_step = 2U;
        return 0;
    }

    DL_I2C_flushControllerTXFIFO(GRAY_I2C_INST);
    DL_I2C_flushControllerRXFIFO(GRAY_I2C_INST);
    DL_I2C_fillControllerTXFIFO(GRAY_I2C_INST, (uint8_t *)buffer, length);
    DL_I2C_startControllerTransfer(GRAY_I2C_INST, GRAY_I2C_ADDR,
        DL_I2C_CONTROLLER_DIRECTION_TX, length);

    if (!Gray_I2C_WaitBusFree()) {
        gray_fail_step = 3U;
        return 0;
    }

    return 1;
}

static int Gray_I2C_ReadBytes(uint8_t *buffer, uint8_t length)
{
    if (buffer == 0 || length == 0U) {
        gray_fail_step = 4U;
        return 0;
    }

    if (!Gray_I2C_WaitIdle()) {
        gray_fail_step = 5U;
        return 0;
    }

    DL_I2C_flushControllerRXFIFO(GRAY_I2C_INST);
    DL_I2C_startControllerTransfer(GRAY_I2C_INST, GRAY_I2C_ADDR,
        DL_I2C_CONTROLLER_DIRECTION_RX, length);

    for (uint8_t i = 0U; i < length; i++) {
        uint32_t timeout = GRAY_I2C_TIMEOUT;

        while (DL_I2C_isControllerRXFIFOEmpty(GRAY_I2C_INST)) {
            if (timeout-- == 0U) {
                gray_fail_step = 6U;
                return 0;
            }
        }

        buffer[i] = DL_I2C_receiveControllerData(GRAY_I2C_INST);
    }

    if (!Gray_I2C_WaitBusFree()) {
        gray_fail_step = 7U;
        return 0;
    }

    return 1;
}

static int Gray_WriteCommand(uint8_t command)
{
    gray_last_cmd = command;

    return Gray_I2C_WriteBytes(&command, 1U);
}

static int Gray_WriteRegister(uint8_t reg, uint8_t value)
{
    uint8_t tx_data[2] = {reg, value};

    gray_last_cmd = reg;

    return Gray_I2C_WriteBytes(tx_data, 2U);
}

static int Gray_ReadRegister(uint8_t reg, uint8_t *value)
{
    if (!Gray_WriteCommand(reg)) {
        return 0;
    }

    return Gray_I2C_ReadBytes(value, 1U);
}

static int Gray_CommandRead(uint8_t command, uint8_t *buffer, uint8_t length)
{
    if (!Gray_WriteCommand(command)) {
        return 0;
    }

    return Gray_I2C_ReadBytes(buffer, length);
}

void Gray_Init(void)
{
    uint8_t response = 0U;
    uint8_t readback = 0U;

    gray_ping_ok = 0U;
    gray_ping_resp = 0U;
    gray_ce_readback = 0U;
    gray_cf_readback = 0U;
    gray_read_ok = 0U;
    gray_last_cmd = 0U;
    gray_fail_step = 0U;
    gray_task_count = 0U;
    gray_read_count = 0U;
    gray_read_fail_count = 0U;
    gray_value_0 = 0U;
    gray_value_1 = 0U;
    gray_value_2 = 0U;
    gray_value_3 = 0U;
    gray_value_4 = 0U;
    gray_value_5 = 0U;
    gray_value_6 = 0U;
    gray_value_7 = 0U;

    for (uint8_t i = 0U; i < GRAY_PING_MAX_TRIES; i++) {
        response = 0U;
        Gray_DelayMs(10U);

        if (Gray_CommandRead(GRAY_CMD_PING, &response, 1U)) {
            gray_ping_resp = response;
        }

        if (gray_ping_resp == GRAY_PING_RESPONSE) {
            gray_ping_ok = 1U;
            break;
        }
    }

    if (gray_ping_ok == 0U) {
        return;
    }

    (void)Gray_WriteRegister(GRAY_REG_CHANNEL_ENABLE, GRAY_ENABLE_ALL_CHANNELS);
    Gray_DelayMs(2U);
    readback = 0U;
    if (Gray_ReadRegister(GRAY_REG_CHANNEL_ENABLE, &readback)) {
        gray_ce_readback = readback;
    }

    (void)Gray_WriteRegister(GRAY_REG_NORMALIZE_ENABLE, GRAY_ENABLE_ALL_CHANNELS);
    Gray_DelayMs(2U);
    readback = 0U;
    if (Gray_ReadRegister(GRAY_REG_NORMALIZE_ENABLE, &readback)) {
        gray_cf_readback = readback;
    }
}

void Gray_Task(void)
{
    uint8_t raw[GRAY_CHANNEL_COUNT] = {0U};

    gray_task_count++;
    gray_read_ok = 0U;
    gray_fail_step = 0U;

    if (!Gray_CommandRead(GRAY_CMD_READ_ANALOG, raw, GRAY_CHANNEL_COUNT)) {
        gray_read_fail_count++;
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
    gray_read_ok = 1U;
    gray_read_count++;
}
