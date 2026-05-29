#ifndef APP_LINE_H_
#define APP_LINE_H_

#include <stdint.h>

extern volatile int16_t line_error;
extern volatile uint8_t line_found;
extern volatile uint16_t line_confidence;
extern volatile uint16_t line_dark_sum;
extern volatile uint8_t black_detected;
extern volatile uint8_t white_detected;
extern volatile uint8_t black_confirm_count;
extern volatile uint8_t white_confirm_count;

void AppLine_Init(void);
void AppLine_Task(void);

#endif /* APP_LINE_H_ */
