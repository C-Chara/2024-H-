#ifndef APP_TASK_H_
#define APP_TASK_H_

#include <stdint.h>

#define TASK1_IDLE      (0U)
#define TASK1_RUN       (1U)
#define TASK1_STOP      (2U)
#define TASK1_DONE      (3U)

extern volatile uint8_t task1_state;
extern volatile float task1_distance_cm;
extern volatile float task1_target_yaw;
extern volatile float task1_heading_error;
extern volatile int16_t task1_base_cmd;
extern volatile int16_t task1_turn_cmd;
extern volatile int16_t task1_left_cmd;
extern volatile int16_t task1_right_cmd;
extern volatile uint8_t task1_black_detected;
extern volatile uint8_t task1_black_confirm_count;
extern volatile uint8_t task1_done;
extern volatile uint8_t task1_finish_reason;

void AppTask_Init(void);
void AppTask_ResetForStart(uint8_t task_id);
void AppTask_MarkTask1Run(void);
void AppTask_MarkTask1Done(void);

#endif /* APP_TASK_H_ */
