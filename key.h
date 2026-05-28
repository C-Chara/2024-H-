#ifndef KEY_H_
#define KEY_H_

#include <stdint.h>

void Key_Init(void);
void Key_Task(void);
uint8_t Key_GetModeShortPress(void);
uint8_t Key_GetStartShortPress(void);
uint8_t Key_GetStartLongPress(void);

#endif /* KEY_H_ */
