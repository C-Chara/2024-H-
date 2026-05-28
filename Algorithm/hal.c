#include "hal.h"
#include "sensor.h"
#include "ti_msp_dl_config.h"
#include "hal_board_config.h"

#define HAL_I2C_TIMEOUT  100000UL

static int32_t HAL_Abs32(int32_t value)
{
    return (value >= 0) ? value : -value;
}

static int HAL_I2C_WaitIdle(I2C_Regs *i2c)
{
    uint32_t timeout = HAL_I2C_TIMEOUT;

    while ((DL_I2C_getControllerStatus(i2c) & DL_I2C_CONTROLLER_STATUS_IDLE) == 0U) {
        if (timeout-- == 0U) {
            return 0;
        }
    }

    return 1;
}

static int HAL_I2C_WaitBusFree(I2C_Regs *i2c)
{
    uint32_t timeout = HAL_I2C_TIMEOUT;

    while ((DL_I2C_getControllerStatus(i2c) & DL_I2C_CONTROLLER_STATUS_BUSY_BUS) != 0U) {
        if (timeout-- == 0U) {
            return 0;
        }
    }

    return ((DL_I2C_getControllerStatus(i2c) & DL_I2C_CONTROLLER_STATUS_ERROR) == 0U);
}

static int HAL_I2C_WriteByte(I2C_Regs *i2c, uint8_t target_addr, uint8_t data)
{
    uint8_t tx_data = data;

    if (!HAL_I2C_WaitIdle(i2c)) {
        return 0;
    }

    DL_I2C_flushControllerTXFIFO(i2c);
    DL_I2C_flushControllerRXFIFO(i2c);
    DL_I2C_fillControllerTXFIFO(i2c, &tx_data, 1U);
    DL_I2C_startControllerTransfer(i2c, target_addr,
        DL_I2C_CONTROLLER_DIRECTION_TX, 1U);

    return HAL_I2C_WaitBusFree(i2c);
}

static int HAL_I2C_ReadBytes(I2C_Regs *i2c, uint8_t target_addr,
    uint8_t *buffer, uint8_t length)
{
    if (!HAL_I2C_WaitIdle(i2c)) {
        return 0;
    }

    DL_I2C_flushControllerRXFIFO(i2c);
    DL_I2C_startControllerTransfer(i2c, target_addr,
        DL_I2C_CONTROLLER_DIRECTION_RX, length);

    for (uint8_t i = 0; i < length; i++) {
        uint32_t timeout = HAL_I2C_TIMEOUT;

        while (DL_I2C_isControllerRXFIFOEmpty(i2c)) {
            if (timeout-- == 0U) {
                return 0;
            }
        }

        buffer[i] = DL_I2C_receiveControllerData(i2c);
    }

    return HAL_I2C_WaitBusFree(i2c);
}

void HAL_Beep_On(void)
{
#if defined(GPIO_BEEP_PORT) && defined(GPIO_BEEP_PIN)
    DL_GPIO_setPins(GPIO_BEEP_PORT, GPIO_BEEP_PIN);
#endif
}

void HAL_Beep_Off(void)
{
#if defined(GPIO_BEEP_PORT) && defined(GPIO_BEEP_PIN)
    DL_GPIO_clearPins(GPIO_BEEP_PORT, GPIO_BEEP_PIN);
#endif
}

int32_t HAL_Get_Left_Encoder_Delta(void)
{
    return 0;
}

int32_t HAL_Get_Right_Encoder_Delta(void)
{
    return 0;
}

void HAL_Sensor_Read_I2C(uint8_t *buffer, uint8_t length)
{
    if (buffer == 0 || length == 0U) {
        return;
    }

    for (uint8_t i = 0; i < length; i++) {
        buffer[i] = 0U;
    }

    if (!HAL_I2C_WriteByte(GRAY_I2C_INST, SENSOR_I2C_ADDR, SENSOR_CMD_READ_ANALOG)) {
        return;
    }

    (void)HAL_I2C_ReadBytes(GRAY_I2C_INST, SENSOR_I2C_ADDR, buffer, length);
}

void HAL_Motor_Set_Left(int32_t pwm_value)
{
    int32_t duty = HAL_Abs32(pwm_value);

#if defined(LEFT_MOTOR_IN1_PORT) && defined(LEFT_MOTOR_IN1_PIN) && \
    defined(LEFT_MOTOR_IN2_PORT) && defined(LEFT_MOTOR_IN2_PIN)
    if (pwm_value >= 0) {
        DL_GPIO_setPins(LEFT_MOTOR_IN1_PORT, LEFT_MOTOR_IN1_PIN);
        DL_GPIO_clearPins(LEFT_MOTOR_IN2_PORT, LEFT_MOTOR_IN2_PIN);
    } else {
        DL_GPIO_clearPins(LEFT_MOTOR_IN1_PORT, LEFT_MOTOR_IN1_PIN);
        DL_GPIO_setPins(LEFT_MOTOR_IN2_PORT, LEFT_MOTOR_IN2_PIN);
    }
#endif

    (void)duty;
}

void HAL_Motor_Set_Right(int32_t pwm_value)
{
    int32_t duty = HAL_Abs32(pwm_value);

#if defined(RIGHT_MOTOR_IN1_PORT) && defined(RIGHT_MOTOR_IN1_PIN) && \
    defined(RIGHT_MOTOR_IN2_PORT) && defined(RIGHT_MOTOR_IN2_PIN)
    if (pwm_value >= 0) {
        DL_GPIO_setPins(RIGHT_MOTOR_IN1_PORT, RIGHT_MOTOR_IN1_PIN);
        DL_GPIO_clearPins(RIGHT_MOTOR_IN2_PORT, RIGHT_MOTOR_IN2_PIN);
    } else {
        DL_GPIO_clearPins(RIGHT_MOTOR_IN1_PORT, RIGHT_MOTOR_IN1_PIN);
        DL_GPIO_setPins(RIGHT_MOTOR_IN2_PORT, RIGHT_MOTOR_IN2_PIN);
    }
#endif

    (void)duty;
}

float HAL_IMU_Get_Yaw(void)
{
    return 0.0f;
}
