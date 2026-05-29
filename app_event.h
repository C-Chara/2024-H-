#ifndef APP_EVENT_H_
#define APP_EVENT_H_

#include <stdint.h>

extern volatile uint8_t last_node_hint;
extern volatile uint8_t event_busy;

void AppEvent_Init(void);
void AppEvent_SelectHint(void);
void AppEvent_StartHint(void);
void AppEvent_NodeHint(uint8_t node_id);
void AppEvent_FinishHint(void);
void AppEvent_FaultHint(void);
void Event_Task(void);
uint8_t AppEvent_IsBusy(void);

/* Compatibility aliases for earlier framework versions. */
void AppEvent_StartupHint(void);
void AppEvent_StopHint(void);

#endif /* APP_EVENT_H_ */
