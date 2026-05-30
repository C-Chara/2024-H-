#ifndef ALGORITHM_MOTOR_H_
#define ALGORITHM_MOTOR_H_

#include <stdint.h>

extern volatile int16_t motor_left_cmd;
extern volatile int16_t motor_right_cmd;
extern volatile uint16_t motor_left_compare;
extern volatile uint16_t motor_right_compare;
extern volatile uint8_t motor_left_dir;
extern volatile uint8_t motor_right_dir;
extern volatile uint8_t motor_output_enabled_dbg;

void Motor_Init(void);

/* speed > 0 is the car forward direction after the direction swap. */
void Motor_SetLeft(int16_t speed);
void Motor_SetRight(int16_t speed);
void Motor_Stop(void);

/* Compatibility for existing project files that are still compiled. */
void Motor_SetSpeed(float left_speed, float right_speed);

#endif /* ALGORITHM_MOTOR_H_ */
