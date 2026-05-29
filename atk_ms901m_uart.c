#include "atk_ms901m_uart.h"

#include "ti_msp_dl_config.h"

void atk_ms901m_uart_init(uint32_t baudrate)
{
    (void)baudrate;
    atk_ms901m_uart_flush_rx();
}

void atk_ms901m_uart_send(const uint8_t *buf, uint8_t len)
{
#if defined(GYRO_UART_INST)
    if (buf == 0 || len == 0U) {
        return;
    }

    for (uint8_t i = 0U; i < len; i++) {
        DL_UART_Main_transmitDataBlocking(GYRO_UART_INST, buf[i]);
    }
#else
    (void)buf;
    (void)len;
#endif
}

uint8_t atk_ms901m_uart_read_byte(uint8_t *byte)
{
#if defined(GYRO_UART_INST)
    if (byte == 0 || DL_UART_Main_isRXFIFOEmpty(GYRO_UART_INST)) {
        return 0U;
    }

    *byte = DL_UART_Main_receiveData(GYRO_UART_INST);
    return 1U;
#else
    (void)byte;
    return 0U;
#endif
}

void atk_ms901m_uart_flush_rx(void)
{
#if defined(GYRO_UART_INST)
    while (!DL_UART_Main_isRXFIFOEmpty(GYRO_UART_INST)) {
        (void)DL_UART_Main_receiveData(GYRO_UART_INST);
    }
#endif
}
