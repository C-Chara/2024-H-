#ifndef APP_TASK1_H_
#define APP_TASK1_H_

#include <stdint.h>

#define TASK1_IDLE      (0U)
#define TASK1_RUN_TO_B  (1U)
#define TASK1_STOP      (2U)
#define TASK1_DONE      (3U)

extern volatile uint8_t task1_state;
extern volatile uint8_t stop_flag;

void AppTask1_Init(void);
void AppTask1_Task(void);

#endif /* APP_TASK1_H_ */
