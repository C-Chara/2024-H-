#include <stdint.h>

#include "ti_msp_dl_config.h"
#include "Algorithm/gray.h"
#include "Algorithm/motor.h"

int main(void)
{
    SYSCFG_DL_init();
    Gray_Init();

    while (1) {
        Motor_Stop();
        Gray_Task();
    }
}
