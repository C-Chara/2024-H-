#ifndef APP_RUNNER_H_
#define APP_RUNNER_H_

#include <stdint.h>

extern volatile uint8_t run_active;
extern volatile uint8_t run_finished;

void AppRunner_Init(void);
void AppRunner_Start(void);
void AppRunner_Stop(void);
void AppRunner_Task(void);

#endif /* APP_RUNNER_H_ */
