#ifndef HAL_BOARD_CONFIG_H_
#define HAL_BOARD_CONFIG_H_

/*
 * CCS + SysConfig mapping file.
 *
 * Configure the gray sensor I2C bus in SysConfig first:
 *   - Controller mode I2C
 *   - SDA: PA0
 *   - SCL/SCK: PA1
 *   - 7-bit target address: 0x4C
 *
 * Then map the generated SysConfig symbols here. Keep algorithm files away
 * from pin names and DriverLib details.
 *
 * SysConfig instance in main.syscfg:
 *   name: I2C_0
 *   peripheral: I2C0
 */

#if defined(I2C_0_INST)
#define GRAY_I2C_INST I2C_0_INST
#else
#define GRAY_I2C_INST I2C0
#endif

/*
 * Map these after motor PWM/GPIO are created in SysConfig.
 * TB6612FNG needs two direction pins and one PWM channel per motor.
 */
/* #define LEFT_MOTOR_IN1_PORT   GPIO_MOTOR_PORT */
/* #define LEFT_MOTOR_IN1_PIN    GPIO_MOTOR_LEFT_IN1_PIN */
/* #define LEFT_MOTOR_IN2_PORT   GPIO_MOTOR_PORT */
/* #define LEFT_MOTOR_IN2_PIN    GPIO_MOTOR_LEFT_IN2_PIN */
/* #define LEFT_MOTOR_PWM_INST   PWM_MOTOR_INST */
/* #define LEFT_MOTOR_PWM_CC     DL_TIMER_CC_0_INDEX */

/* #define RIGHT_MOTOR_IN1_PORT  GPIO_MOTOR_PORT */
/* #define RIGHT_MOTOR_IN1_PIN   GPIO_MOTOR_RIGHT_IN1_PIN */
/* #define RIGHT_MOTOR_IN2_PORT  GPIO_MOTOR_PORT */
/* #define RIGHT_MOTOR_IN2_PIN   GPIO_MOTOR_RIGHT_IN2_PIN */
/* #define RIGHT_MOTOR_PWM_INST  PWM_MOTOR_INST */
/* #define RIGHT_MOTOR_PWM_CC    DL_TIMER_CC_1_INDEX */

#endif
