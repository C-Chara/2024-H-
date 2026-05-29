#ifndef ATK_MS901M_UART_H_
#define ATK_MS901M_UART_H_

#include <stdint.h>

void atk_ms901m_uart_init(uint32_t baudrate);
void atk_ms901m_uart_send(const uint8_t *buf, uint8_t len);
uint8_t atk_ms901m_uart_read_byte(uint8_t *byte);
void atk_ms901m_uart_flush_rx(void);

#endif /* ATK_MS901M_UART_H_ */
