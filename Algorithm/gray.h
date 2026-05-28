#ifndef ALGORITHM_GRAY_H_
#define ALGORITHM_GRAY_H_

#include <stdint.h>

extern volatile uint8_t gray_ping_ok;
extern volatile uint8_t gray_ping_resp;
extern volatile uint8_t gray_ce_readback;
extern volatile uint8_t gray_cf_readback;
extern volatile uint8_t gray_read_ok;
extern volatile uint8_t gray_last_cmd;
extern volatile uint8_t gray_value_0;
extern volatile uint8_t gray_value_1;
extern volatile uint8_t gray_value_2;
extern volatile uint8_t gray_value_3;
extern volatile uint8_t gray_value_4;
extern volatile uint8_t gray_value_5;
extern volatile uint8_t gray_value_6;
extern volatile uint8_t gray_value_7;

void Gray_Init(void);
void Gray_Task(void);

#endif /* ALGORITHM_GRAY_H_ */
