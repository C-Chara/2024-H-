#ifndef ENCODER_H_
#define ENCODER_H_

#include <stdint.h>

#define ENCODER_LEFT_SIGN       (1)
#define ENCODER_RIGHT_SIGN      (1)

extern volatile int32_t encoder_left_total;
extern volatile int32_t encoder_right_total;
extern volatile int32_t encoder_left_start;
extern volatile int32_t encoder_right_start;
extern volatile int32_t encoder_left_delta;
extern volatile int32_t encoder_right_delta;
extern volatile float encoder_distance_cm;
extern volatile uint8_t encoder_distance_source;
extern volatile uint32_t encoder_left_irq_count;
extern volatile uint32_t encoder_right_irq_count;
extern volatile uint8_t encoder_valid;
extern volatile uint32_t encoder_last_update_tick;

void Encoder_Init(void);
void Encoder_ResetDistance(void);
int32_t Encoder_GetLeftTotal(void);
int32_t Encoder_GetRightTotal(void);
int32_t Encoder_GetLeftDelta(void);
int32_t Encoder_GetRightDelta(void);
float Encoder_GetDistanceCm(void);
void Encoder_Task(void);

#endif /* ENCODER_H_ */
