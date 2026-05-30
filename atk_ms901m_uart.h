#ifndef ATK_MS901M_UART_H_
#define ATK_MS901M_UART_H_

#include <stdint.h>

void atk_ms901m_uart_init(uint32_t baudrate);
void atk_ms901m_uart_send(const uint8_t *buf, uint8_t len);
uint8_t atk_ms901m_uart_read_byte(uint8_t *byte);
void atk_ms901m_uart_flush_rx(void);

extern volatile uint32_t imu_uart_irq_rx_count;
extern volatile uint32_t imu_uart_ring_count;
extern volatile uint32_t imu_uart_overrun_count;
extern volatile uint32_t imu_uart_max_fill_level;

#endif /* ATK_MS901M_UART_H_ */
