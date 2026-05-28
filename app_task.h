#ifndef APP_TASK_H_
#define APP_TASK_H_

#include <stdint.h>

void AppTask_Init(void);
uint8_t AppTask_Load(uint8_t task_id);
void AppTask_Start(void);
void AppTask_Cancel(void);
void AppTask_Task(void);

#endif /* APP_TASK_H_ */
