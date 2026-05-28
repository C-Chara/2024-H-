#ifndef ALERT_H_
#define ALERT_H_

#include <stdint.h>

void Alert_Init(void);
void Buzzer_On(void);
void Buzzer_Off(void);
void LED_All_Off(void);
void LED_All_On(void);
void LED_ShowTask(uint8_t task_id);
void LED_Task_On(void);
void LED_Task_Off(void);
void LED_Task_Toggle(void);
void Beep(uint16_t ms);
void LED_BlinkAll(uint8_t times, uint16_t on_ms, uint16_t off_ms);
void Alert_NodeHint(uint8_t node_id);

#endif /* ALERT_H_ */
