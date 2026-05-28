#include "alert.h"

#include <stdint.h>

#include "ti_msp_dl_config.h"

#if defined(ALERT_GPIO_BUZZER_PORT) && defined(ALERT_GPIO_BUZZER_PIN)
#define BUZZER_PORT       ALERT_GPIO_BUZZER_PORT
#define BUZZER_PIN        ALERT_GPIO_BUZZER_PIN
#else
/* Here needs replacement with the real BUZZER macros from ti_msp_dl_config.h. */
#define BUZZER_PORT       GPIOB
#define BUZZER_PIN        DL_GPIO_PIN_0
#endif

#if defined(ALERT_GPIO_LED1_PORT) && defined(ALERT_GPIO_LED1_PIN)
#define LED1_PORT         ALERT_GPIO_LED1_PORT
#define LED1_PIN          ALERT_GPIO_LED1_PIN
#else
/* Here needs replacement with the real LED1 macros from ti_msp_dl_config.h. */
#define LED1_PORT         GPIOB
#define LED1_PIN          DL_GPIO_PIN_9
#endif

#if defined(ALERT_GPIO_LED2_PORT) && defined(ALERT_GPIO_LED2_PIN)
#define LED2_PORT         ALERT_GPIO_LED2_PORT
#define LED2_PIN          ALERT_GPIO_LED2_PIN
#else
/* Here needs replacement with the real LED2 macros from ti_msp_dl_config.h. */
#define LED2_PORT         GPIOB
#define LED2_PIN          DL_GPIO_PIN_2
#endif

#if defined(ALERT_GPIO_LED3_PORT) && defined(ALERT_GPIO_LED3_PIN)
#define LED3_PORT         ALERT_GPIO_LED3_PORT
#define LED3_PIN          ALERT_GPIO_LED3_PIN
#else
/* Here needs replacement with the real LED3 macros from ti_msp_dl_config.h. */
#define LED3_PORT         GPIOB
#define LED3_PIN          DL_GPIO_PIN_3
#endif

#if defined(ALERT_GPIO_LED4_PORT) && defined(ALERT_GPIO_LED4_PIN)
#define LED4_PORT         ALERT_GPIO_LED4_PORT
#define LED4_PIN          ALERT_GPIO_LED4_PIN
#else
/* Here needs replacement with the real LED4 macros from ti_msp_dl_config.h. */
#define LED4_PORT         GPIOA
#define LED4_PIN          DL_GPIO_PIN_24
#endif

#if defined(ALERT_GPIO_LED_TASK_PORT) && defined(ALERT_GPIO_LED_TASK_PIN)
#define LED_TASK_PORT     ALERT_GPIO_LED_TASK_PORT
#define LED_TASK_PIN      ALERT_GPIO_LED_TASK_PIN
#else
/* Here needs replacement with the real LED_TASK macros from ti_msp_dl_config.h. */
#define LED_TASK_PORT     GPIOA
#define LED_TASK_PIN      DL_GPIO_PIN_17
#endif

static void Alert_DelayMs(uint16_t ms)
{
    while (ms > 0U) {
        DL_Common_delayCycles(CPUCLK_FREQ / 1000U);
        ms--;
    }
}

static void Alert_Write(GPIO_Regs *port, uint32_t pin, uint8_t on)
{
    if (on != 0U) {
        DL_GPIO_setPins(port, pin);
    } else {
        DL_GPIO_clearPins(port, pin);
    }
}

void Alert_Init(void)
{
    DL_GPIO_enableOutput(BUZZER_PORT, BUZZER_PIN);
    DL_GPIO_enableOutput(LED1_PORT, LED1_PIN);
    DL_GPIO_enableOutput(LED2_PORT, LED2_PIN);
    DL_GPIO_enableOutput(LED3_PORT, LED3_PIN);
    DL_GPIO_enableOutput(LED4_PORT, LED4_PIN);
    DL_GPIO_enableOutput(LED_TASK_PORT, LED_TASK_PIN);

    Buzzer_Off();
    LED_All_Off();
}

void Buzzer_On(void)
{
    Alert_Write(BUZZER_PORT, BUZZER_PIN, 1U);
}

void Buzzer_Off(void)
{
    Alert_Write(BUZZER_PORT, BUZZER_PIN, 0U);
}

void LED_All_Off(void)
{
    Alert_Write(LED1_PORT, LED1_PIN, 0U);
    Alert_Write(LED2_PORT, LED2_PIN, 0U);
    Alert_Write(LED3_PORT, LED3_PIN, 0U);
    Alert_Write(LED4_PORT, LED4_PIN, 0U);
    Alert_Write(LED_TASK_PORT, LED_TASK_PIN, 0U);
}

void LED_All_On(void)
{
    Alert_Write(LED1_PORT, LED1_PIN, 1U);
    Alert_Write(LED2_PORT, LED2_PIN, 1U);
    Alert_Write(LED3_PORT, LED3_PIN, 1U);
    Alert_Write(LED4_PORT, LED4_PIN, 1U);
    Alert_Write(LED_TASK_PORT, LED_TASK_PIN, 1U);
}

void LED_ShowTask(uint8_t task_id)
{
    Alert_Write(LED1_PORT, LED1_PIN, (task_id == 1U) ? 1U : 0U);
    Alert_Write(LED2_PORT, LED2_PIN, (task_id == 2U) ? 1U : 0U);
    Alert_Write(LED3_PORT, LED3_PIN, (task_id == 3U) ? 1U : 0U);
    Alert_Write(LED4_PORT, LED4_PIN, (task_id == 4U) ? 1U : 0U);
}

void LED_Task_On(void)
{
    Alert_Write(LED_TASK_PORT, LED_TASK_PIN, 1U);
}

void LED_Task_Off(void)
{
    Alert_Write(LED_TASK_PORT, LED_TASK_PIN, 0U);
}

void LED_Task_Toggle(void)
{
    DL_GPIO_togglePins(LED_TASK_PORT, LED_TASK_PIN);
}

void Beep(uint16_t ms)
{
    Buzzer_On();
    Alert_DelayMs(ms);
    Buzzer_Off();
}

void LED_BlinkAll(uint8_t times, uint16_t on_ms, uint16_t off_ms)
{
    for (uint8_t i = 0U; i < times; i++) {
        LED_All_On();
        Alert_DelayMs(on_ms);
        LED_All_Off();
        Alert_DelayMs(off_ms);
    }
}

void Alert_NodeHint(uint8_t node_id)
{
    (void)node_id;

    for (uint8_t i = 0U; i < 3U; i++) {
        LED_Task_On();
        Beep(60U);
        LED_Task_Off();
        Alert_DelayMs(60U);
    }
}
