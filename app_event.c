#include "app_event.h"

#include "alert.h"
#include "app_config.h"
#include "ti_msp_dl_config.h"

volatile uint8_t last_node_hint = 0xFFU;
volatile uint8_t event_busy = 0U;

static void AppEvent_DelayMs(uint16_t ms)
{
    while (ms > 0U) {
        DL_Common_delayCycles(CPUCLK_FREQ / 1000U);
        ms--;
    }
}

static void AppEvent_Pulse(uint8_t times)
{
    event_busy = 1U;

    for (uint8_t i = 0U; i < times; i++) {
        LED_Task_On();
        Beep(EVENT_BEEP_MS);
        AppEvent_DelayMs(EVENT_FLASH_MS);
        LED_Task_Off();
        AppEvent_DelayMs(EVENT_GAP_MS);
    }

    event_busy = 0U;
}

void AppEvent_Init(void)
{
    last_node_hint = 0xFFU;
    event_busy = 0U;
    LED_Task_Off();
}

void AppEvent_StartHint(void)
{
    AppEvent_Pulse(1U);
    AppEvent_DelayMs(EVENT_START_DELAY_MS);
}

void AppEvent_NodeHint(uint8_t node_id)
{
    last_node_hint = node_id;
    AppEvent_Pulse(1U);
}

void AppEvent_FinishHint(void)
{
    AppEvent_Pulse(3U);
}

void AppEvent_FaultHint(void)
{
    AppEvent_Pulse(2U);
}

void AppEvent_StartupHint(void)
{
    AppEvent_StartHint();
}

void AppEvent_StopHint(void)
{
    AppEvent_FinishHint();
}
