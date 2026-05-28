#ifndef APP_MODE_H_
#define APP_MODE_H_

#include <stdint.h>

#define SYS_BOOT        (0U)
#define SYS_SELECT      (1U)
#define SYS_ARMED       (2U)
#define SYS_RUN         (3U)
#define SYS_FINISHED    (4U)
#define SYS_FAULT       (5U)

extern volatile uint8_t selected_task;
extern volatile uint8_t system_mode;

void AppMode_Init(void);
void AppMode_Task(void);

#endif /* APP_MODE_H_ */
