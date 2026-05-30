#ifndef APP_LINE_H_
#define APP_LINE_H_

#include <stdint.h>

#include "app_route.h"

extern volatile int16_t line_error;
extern volatile uint8_t line_found;
extern volatile uint16_t line_confidence;
extern volatile uint16_t line_dark_sum;
extern volatile uint8_t black_detected;
extern volatile uint8_t white_detected;
extern volatile uint8_t black_confirm_count;
extern volatile uint8_t white_confirm_count;
extern volatile uint16_t line_max_dark;
extern volatile uint8_t line_active_sensor_count;
extern volatile uint8_t line_lost_count;
extern volatile int16_t last_line_error;
extern volatile int16_t line_turn_cmd;
extern volatile int16_t line_left_cmd;
extern volatile int16_t line_right_cmd;
extern volatile uint8_t line_black_sensor_count;
extern volatile uint8_t line_gray_trigger_hit;
extern volatile uint8_t line_ever_found;
extern volatile uint32_t line_lost_start_tick;
extern volatile uint8_t line_exit_by_white;
extern volatile uint8_t line_end_on_lost_active;
extern volatile uint8_t line_exit_distance_ok;
extern volatile uint8_t line_ctrl_active;
extern volatile uint8_t line_fail_reason;
extern volatile uint8_t line_exit_forward_active;
extern volatile uint32_t line_exit_forward_start_tick;

void AppLine_Init(void);
void AppLine_Task(void);
void AppLine_StartSegment(const AppRouteSegment *segment);
void AppLine_ControllerTask(void);
uint8_t AppLine_IsDone(void);
uint8_t AppLine_IsFailed(void);
void AppLine_Stop(void);

#endif /* APP_LINE_H_ */
