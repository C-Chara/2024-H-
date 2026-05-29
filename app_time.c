#include "app_time.h"

#include "ti_msp_dl_config.h"

volatile uint32_t g_sys_tick_ms = 0U;

void AppTime_Init(void)
{
    g_sys_tick_ms = 0U;
    (void)SysTick_Config(CPUCLK_FREQ / 1000U);
}

void SysTick_Handler(void)
{
    SysTick_Task_1ms();
}

void SysTick_Task_1ms(void)
{
    g_sys_tick_ms++;
}

uint32_t app_millis(void)
{
    return g_sys_tick_ms;
}
