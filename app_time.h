#ifndef APP_TIME_H_
#define APP_TIME_H_

#include <stdint.h>

extern volatile uint32_t g_sys_tick_ms;

void AppTime_Init(void);
void SysTick_Task_1ms(void);
uint32_t app_millis(void);

#endif /* APP_TIME_H_ */
