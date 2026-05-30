#include "atk_ms901m_uart.h"

#include "ti_msp_dl_config.h"

#define ATK_MS901M_UART_RX_RING_SIZE     (256U)

volatile uint32_t imu_uart_irq_rx_count = 0U;
volatile uint32_t imu_uart_ring_count = 0U;
volatile uint32_t imu_uart_overrun_count = 0U;
volatile uint32_t imu_uart_max_fill_level = 0U;

static volatile uint8_t imu_uart_rx_ring[ATK_MS901M_UART_RX_RING_SIZE];
static volatile uint16_t imu_uart_rx_head = 0U;
static volatile uint16_t imu_uart_rx_tail = 0U;

static void AtkUart_RingClear(void)
{
    imu_uart_rx_head = 0U;
    imu_uart_rx_tail = 0U;
    imu_uart_ring_count = 0U;
    imu_uart_max_fill_level = 0U;
}

static void AtkUart_RingPush(uint8_t byte)
{
    uint16_t next_head =
        (uint16_t)((imu_uart_rx_head + 1U) % ATK_MS901M_UART_RX_RING_SIZE);

    if (next_head == imu_uart_rx_tail) {
        imu_uart_overrun_count++;
        return;
    }

    imu_uart_rx_ring[imu_uart_rx_head] = byte;
    imu_uart_rx_head = next_head;
    imu_uart_ring_count++;
    if (imu_uart_ring_count > imu_uart_max_fill_level) {
        imu_uart_max_fill_level = imu_uart_ring_count;
    }
}

static uint8_t AtkUart_RingPop(uint8_t *byte)
{
    if (byte == 0 || imu_uart_rx_head == imu_uart_rx_tail) {
        return 0U;
    }

    *byte = imu_uart_rx_ring[imu_uart_rx_tail];
    imu_uart_rx_tail =
        (uint16_t)((imu_uart_rx_tail + 1U) % ATK_MS901M_UART_RX_RING_SIZE);
    if (imu_uart_ring_count > 0U) {
        imu_uart_ring_count--;
    }

    return 1U;
}

void atk_ms901m_uart_init(uint32_t baudrate)
{
    (void)baudrate;
    atk_ms901m_uart_flush_rx();

#if defined(GYRO_UART_INST) && defined(GYRO_UART_INST_INT_IRQN)
    DL_UART_Main_setRXFIFOThreshold(GYRO_UART_INST,
        DL_UART_MAIN_RX_FIFO_LEVEL_ONE_ENTRY);
    DL_UART_Main_clearInterruptStatus(GYRO_UART_INST,
        DL_UART_MAIN_INTERRUPT_RX | DL_UART_MAIN_INTERRUPT_OVERRUN_ERROR |
        DL_UART_MAIN_INTERRUPT_FRAMING_ERROR |
        DL_UART_MAIN_INTERRUPT_PARITY_ERROR |
        DL_UART_MAIN_INTERRUPT_RX_TIMEOUT_ERROR);
    DL_UART_Main_enableInterrupt(GYRO_UART_INST, DL_UART_MAIN_INTERRUPT_RX);
    NVIC_ClearPendingIRQ(GYRO_UART_INST_INT_IRQN);
    NVIC_EnableIRQ(GYRO_UART_INST_INT_IRQN);
#endif
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
#if defined(GYRO_UART_INST) && defined(GYRO_UART_INST_INT_IRQN)
    uint8_t result;

    if (byte == 0) {
        return 0U;
    }

    NVIC_DisableIRQ(GYRO_UART_INST_INT_IRQN);
    result = AtkUart_RingPop(byte);
    NVIC_EnableIRQ(GYRO_UART_INST_INT_IRQN);
    return result;
#else
    (void)byte;
    return 0U;
#endif
}

void atk_ms901m_uart_flush_rx(void)
{
#if defined(GYRO_UART_INST)
    AtkUart_RingClear();
    while (!DL_UART_Main_isRXFIFOEmpty(GYRO_UART_INST)) {
        (void)DL_UART_Main_receiveData(GYRO_UART_INST);
    }
#endif
}

#if defined(GYRO_UART_INST) && defined(GYRO_UART_INST_IRQHandler)
void GYRO_UART_INST_IRQHandler(void)
{
    switch (DL_UART_Main_getPendingInterrupt(GYRO_UART_INST)) {
        case DL_UART_MAIN_IIDX_RX:
        case DL_UART_MAIN_IIDX_RX_TIMEOUT_ERROR:
            while (!DL_UART_Main_isRXFIFOEmpty(GYRO_UART_INST)) {
                AtkUart_RingPush(DL_UART_Main_receiveData(GYRO_UART_INST));
                imu_uart_irq_rx_count++;
            }
            break;

        default:
            DL_UART_Main_clearInterruptStatus(GYRO_UART_INST,
                DL_UART_MAIN_INTERRUPT_OVERRUN_ERROR |
                DL_UART_MAIN_INTERRUPT_FRAMING_ERROR |
                DL_UART_MAIN_INTERRUPT_PARITY_ERROR |
                DL_UART_MAIN_INTERRUPT_RX_TIMEOUT_ERROR);
            break;
    }
}
#endif
