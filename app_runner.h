#ifndef APP_RUNNER_H_
#define APP_RUNNER_H_

#include <stdint.h>

extern volatile uint8_t run_active;
extern volatile uint8_t run_finished;
extern volatile uint8_t route_finish_reason;
extern volatile uint8_t route_error_code;
extern volatile uint32_t route_run_ticks;
extern volatile float route_final_distance_cm;
extern volatile float route_final_yaw;
extern volatile int16_t route_max_heading_error;
extern volatile uint8_t route_last_node;
extern volatile uint8_t runner_state_dbg;

extern volatile uint8_t t2_test_done;
extern volatile uint8_t t2_finish_reason;
extern volatile float t2_final_distance_cm;
extern volatile float t2_final_yaw;
extern volatile int16_t t2_max_heading_error;
extern volatile uint8_t t3_test_done;
extern volatile uint8_t t3_finish_reason;
extern volatile float t3_final_distance_cm;
extern volatile float t3_final_yaw;
extern volatile int16_t t3_max_heading_error;
extern volatile uint8_t t4_test_done;
extern volatile uint8_t t4_finish_reason;
extern volatile float t4_final_distance_cm;
extern volatile float t4_final_yaw;
extern volatile int16_t t4_max_heading_error;

void AppRunner_Init(void);
uint8_t AppRunner_Start(uint8_t task_id);
void AppRunner_Stop(void);
void AppRunner_Task(void);

#endif /* APP_RUNNER_H_ */
