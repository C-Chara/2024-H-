#include "app_event.h"

#include "alert.h"
#include "app_config.h"
#include "app_time.h"

#define EVENT_PHASE_IDLE        (0U)
#define EVENT_PHASE_ON          (1U)
#define EVENT_PHASE_GAP         (2U)
#define EVENT_PHASE_POST_DELAY  (3U)

volatile uint8_t last_node_hint = 0xFFU;
volatile uint8_t event_busy = 0U;

static uint8_t event_phase = EVENT_PHASE_IDLE;
static uint8_t event_remaining_pulses = 0U;
static uint16_t event_post_delay_ms = 0U;
static uint32_t event_phase_end_tick = 0U;

static void AppEvent_RequestPulse(uint8_t times, uint16_t post_delay_ms)
{
    if (times == 0U) {
        return;
    }

    event_busy = 1U;
    event_remaining_pulses = times;
    event_post_delay_ms = post_delay_ms;
    event_phase = EVENT_PHASE_ON;
    event_phase_end_tick = app_millis() + EVENT_FLASH_MS;
    LED_Task_On();
    Buzzer_On();
}

void AppEvent_Init(void)
{
    last_node_hint = 0xFFU;
    event_busy = 0U;
    event_phase = EVENT_PHASE_IDLE;
    event_remaining_pulses = 0U;
    event_post_delay_ms = 0U;
    event_phase_end_tick = 0U;
    LED_Task_Off();
    Buzzer_Off();
}

void AppEvent_SelectHint(void)
{
    AppEvent_RequestPulse(1U, 0U);
}

void AppEvent_StartHint(void)
{
    AppEvent_RequestPulse(1U, EVENT_START_DELAY_MS);
}

void AppEvent_NodeHint(uint8_t node_id)
{
    last_node_hint = node_id;
    AppEvent_RequestPulse(1U, 0U);
}

void AppEvent_FinishHint(void)
{
    AppEvent_RequestPulse(3U, 0U);
}

void AppEvent_FaultHint(void)
{
    AppEvent_RequestPulse(2U, 0U);
}

void Event_Task(void)
{
    uint32_t now = app_millis();

    if (event_phase == EVENT_PHASE_IDLE) {
        event_busy = 0U;
        return;
    }

    if ((int32_t)(now - event_phase_end_tick) < 0) {
        return;
    }

    if (event_phase == EVENT_PHASE_ON) {
        LED_Task_Off();
        Buzzer_Off();

        if (event_remaining_pulses > 0U) {
            event_remaining_pulses--;
        }

        if (event_remaining_pulses > 0U) {
            event_phase = EVENT_PHASE_GAP;
            event_phase_end_tick = now + EVENT_GAP_MS;
        } else if (event_post_delay_ms > 0U) {
            event_phase = EVENT_PHASE_POST_DELAY;
            event_phase_end_tick = now + event_post_delay_ms;
        } else {
            event_phase = EVENT_PHASE_IDLE;
            event_busy = 0U;
        }
        return;
    }

    if (event_phase == EVENT_PHASE_GAP) {
        LED_Task_On();
        Buzzer_On();
        event_phase = EVENT_PHASE_ON;
        event_phase_end_tick = now + EVENT_FLASH_MS;
        return;
    }

    if (event_phase == EVENT_PHASE_POST_DELAY) {
        event_phase = EVENT_PHASE_IDLE;
        event_busy = 0U;
    }
}

uint8_t AppEvent_IsBusy(void)
{
    return event_busy;
}

void AppEvent_StartupHint(void)
{
    AppEvent_StartHint();
}

void AppEvent_StopHint(void)
{
    AppEvent_FinishHint();
}
