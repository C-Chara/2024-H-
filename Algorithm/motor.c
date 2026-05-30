#include "motor.h"

#include <stdint.h>

#include "ti_msp_dl_config.h"

#define MOTOR_PWM_PERIOD_COUNT  (3200U)
#define MOTOR_SPEED_MAX         (1000)

#define MOTOR_DIR_STOP          (0U)
#define MOTOR_DIR_FORWARD       (1U)
#define MOTOR_DIR_REVERSE       (2U)

volatile int16_t motor_left_cmd = 0;
volatile int16_t motor_right_cmd = 0;
volatile uint16_t motor_left_compare = MOTOR_PWM_PERIOD_COUNT;
volatile uint16_t motor_right_compare = MOTOR_PWM_PERIOD_COUNT;
volatile uint8_t motor_stby_state = 0U;
volatile uint8_t motor_left_dir = MOTOR_DIR_STOP;
volatile uint8_t motor_right_dir = MOTOR_DIR_STOP;
volatile uint8_t motor_output_enabled_dbg = 1U;

static int16_t Motor_ClampSpeed(int16_t speed)
{
    if (speed > MOTOR_SPEED_MAX) {
        return MOTOR_SPEED_MAX;
    }

    if (speed < -MOTOR_SPEED_MAX) {
        return -MOTOR_SPEED_MAX;
    }

    return speed;
}

static uint16_t Motor_SpeedToCompare(int16_t speed)
{
    int16_t limited_speed = Motor_ClampSpeed(speed);
    int32_t abs_speed = limited_speed;

    if (abs_speed < 0) {
        abs_speed = -abs_speed;
    }

    return (uint16_t)(MOTOR_PWM_PERIOD_COUNT -
        (((uint32_t)abs_speed * MOTOR_PWM_PERIOD_COUNT) / MOTOR_SPEED_MAX));
}

static void Motor_WriteLeftPwm(int16_t speed)
{
    motor_left_compare = Motor_SpeedToCompare(speed);
    DL_TimerG_setCaptureCompareValue(MOTOR_PWM_INST, motor_left_compare,
        GPIO_MOTOR_PWM_C0_IDX);
}

static void Motor_WriteRightPwm(int16_t speed)
{
    motor_right_compare = Motor_SpeedToCompare(speed);
    DL_TimerG_setCaptureCompareValue(MOTOR_PWM_INST, motor_right_compare,
        GPIO_MOTOR_PWM_C1_IDX);
}

static void Motor_SetLeftDirection(int16_t speed)
{
    if (speed > 0) {
        motor_left_dir = MOTOR_DIR_FORWARD;
        DL_GPIO_clearPins(MOTOR_GPIO_AIN1_PORT, MOTOR_GPIO_AIN1_PIN);
        DL_GPIO_setPins(MOTOR_GPIO_AIN2_PORT, MOTOR_GPIO_AIN2_PIN);
    } else if (speed < 0) {
        motor_left_dir = MOTOR_DIR_REVERSE;
        DL_GPIO_setPins(MOTOR_GPIO_AIN1_PORT, MOTOR_GPIO_AIN1_PIN);
        DL_GPIO_clearPins(MOTOR_GPIO_AIN2_PORT, MOTOR_GPIO_AIN2_PIN);
    } else {
        motor_left_dir = MOTOR_DIR_STOP;
        DL_GPIO_clearPins(MOTOR_GPIO_AIN1_PORT, MOTOR_GPIO_AIN1_PIN);
        DL_GPIO_clearPins(MOTOR_GPIO_AIN2_PORT, MOTOR_GPIO_AIN2_PIN);
    }
}

static void Motor_SetRightDirection(int16_t speed)
{
    if (speed > 0) {
        motor_right_dir = MOTOR_DIR_FORWARD;
        DL_GPIO_clearPins(MOTOR_GPIO_BIN1_PORT, MOTOR_GPIO_BIN1_PIN);
        DL_GPIO_setPins(MOTOR_GPIO_BIN2_PORT, MOTOR_GPIO_BIN2_PIN);
    } else if (speed < 0) {
        motor_right_dir = MOTOR_DIR_REVERSE;
        DL_GPIO_setPins(MOTOR_GPIO_BIN1_PORT, MOTOR_GPIO_BIN1_PIN);
        DL_GPIO_clearPins(MOTOR_GPIO_BIN2_PORT, MOTOR_GPIO_BIN2_PIN);
    } else {
        motor_right_dir = MOTOR_DIR_STOP;
        DL_GPIO_clearPins(MOTOR_GPIO_BIN1_PORT, MOTOR_GPIO_BIN1_PIN);
        DL_GPIO_clearPins(MOTOR_GPIO_BIN2_PORT, MOTOR_GPIO_BIN2_PIN);
    }
}

void Motor_Init(void)
{
    motor_stby_state = 1U;
    /* TB6612 STBY is tied to 3.3V on the current wiring. */

    Motor_SetLeftDirection(0);
    Motor_SetRightDirection(0);
    Motor_WriteLeftPwm(0);
    Motor_WriteRightPwm(0);

    DL_TimerG_startCounter(MOTOR_PWM_INST);
}

void Motor_SetLeft(int16_t speed)
{
    int16_t limited_speed = Motor_ClampSpeed(speed);

    motor_left_cmd = limited_speed;
    if (motor_output_enabled_dbg != 0U) {
        Motor_SetLeftDirection(limited_speed);
        Motor_WriteLeftPwm(limited_speed);
    } else {
        Motor_SetLeftDirection(0);
        Motor_WriteLeftPwm(0);
    }
}

void Motor_SetRight(int16_t speed)
{
    int16_t limited_speed = Motor_ClampSpeed(speed);

    motor_right_cmd = limited_speed;
    if (motor_output_enabled_dbg != 0U) {
        Motor_SetRightDirection(limited_speed);
        Motor_WriteRightPwm(limited_speed);
    } else {
        Motor_SetRightDirection(0);
        Motor_WriteRightPwm(0);
    }
}

void Motor_Stop(void)
{
    Motor_SetLeft(0);
    Motor_SetRight(0);
}

void Motor_SetSpeed(float left_speed, float right_speed)
{
    Motor_SetLeft((int16_t)left_speed);
    Motor_SetRight((int16_t)right_speed);
}
