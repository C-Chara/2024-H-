#include "atk_ms901m.h"

#include "atk_ms901m_uart.h"
#include "gyro.h"

#define ATK_MS901M_READ_REG_ID(id)        ((uint8_t)((id) | 0x80U))
#define ATK_MS901M_WRITE_REG_ID(id)       ((uint8_t)(id))

volatile uint8_t gyro_ok = 0U;
volatile uint8_t gyro_init_result = 0U;
volatile uint32_t gyro_attitude_ok_count = 0U;
volatile uint32_t gyro_attitude_fail_count = 0U;
volatile uint32_t gyro_rx_count = 0U;
volatile uint32_t gyro_frame_ok_count = 0U;
volatile uint32_t gyro_frame_checksum_error = 0U;
volatile uint8_t gyro_last_frame_id = 0U;
volatile uint8_t gyro_last_frame_type = 0U;
volatile uint8_t gyro_last_frame_len = 0U;

static void Atk_DelayTick(void)
{
    volatile uint32_t i;

    for (i = 0U; i < 320U; i++) {
    }
}

static uint8_t Atk_Checksum(const uint8_t *buf, uint8_t len)
{
    uint8_t sum = 0U;

    for (uint8_t i = 0U; i < len; i++) {
        sum = (uint8_t)(sum + buf[i]);
    }

    return sum;
}

static uint8_t Atk_ReadByteWithTimeout(uint8_t *byte, uint32_t *timeout)
{
    while (*timeout > 0U) {
        if (atk_ms901m_uart_read_byte(byte) != 0U) {
            gyro_rx_count++;
            return ATK_MS901M_EOK;
        }

        (*timeout)--;
        Atk_DelayTick();
    }

    return ATK_MS901M_ETIMEOUT;
}

uint8_t atk_ms901m_get_frame_by_id(atk_ms901m_frame_t *frame, uint8_t id,
    uint8_t frame_type, uint32_t timeout)
{
    uint8_t state = 0U;
    uint8_t dat_index = 0U;
    uint8_t checksum = 0U;
    uint8_t byte = 0U;

    if (frame == 0 ||
        (frame_type != ATK_MS901M_FRAME_ID_TYPE_UPLOAD &&
         frame_type != ATK_MS901M_FRAME_ID_TYPE_ACK)) {
        return ATK_MS901M_EINVAL;
    }

    while (timeout > 0U) {
        if (Atk_ReadByteWithTimeout(&byte, &timeout) != ATK_MS901M_EOK) {
            return ATK_MS901M_ETIMEOUT;
        }

        switch (state) {
            case 0:
                if (byte == ATK_MS901M_FRAME_HEAD_L) {
                    frame->head_l = byte;
                    checksum = byte;
                    state = 1U;
                }
                break;

            case 1:
                if (byte == ATK_MS901M_FRAME_HEAD_UPLOAD_H ||
                    byte == ATK_MS901M_FRAME_HEAD_ACK_H) {
                    frame->head_h = byte;
                    checksum = (uint8_t)(checksum + byte);
                    state = 2U;
                } else if (byte == ATK_MS901M_FRAME_HEAD_L) {
                    frame->head_l = byte;
                    checksum = byte;
                    state = 1U;
                } else {
                    state = 0U;
                }
                break;

            case 2:
                frame->id = byte;
                checksum = (uint8_t)(checksum + byte);
                state = 3U;
                break;

            case 3:
                frame->len = byte;
                gyro_last_frame_len = byte;
                checksum = (uint8_t)(checksum + byte);
                dat_index = 0U;

                if (frame->len > ATK_MS901M_FRAME_DAT_MAX_LEN) {
                    state = 0U;
                } else if (frame->len == 0U) {
                    state = 5U;
                } else {
                    state = 4U;
                }
                break;

            case 4:
                frame->dat[dat_index] = byte;
                dat_index++;
                checksum = (uint8_t)(checksum + byte);
                if (dat_index >= frame->len) {
                    state = 5U;
                }
                break;

            case 5:
                frame->sum = byte;
                gyro_last_frame_id = frame->id;
                gyro_last_frame_type = frame->head_h;
                gyro_last_frame_len = frame->len;

                if (checksum == frame->sum) {
                    gyro_frame_ok_count++;
                    if (frame->id == id && frame->head_h == frame_type) {
                        return ATK_MS901M_EOK;
                    }
                } else {
                    gyro_frame_checksum_error++;
                }

                state = (byte == ATK_MS901M_FRAME_HEAD_L) ? 1U : 0U;
                checksum = (byte == ATK_MS901M_FRAME_HEAD_L) ? byte : 0U;
                break;

            default:
                state = 0U;
                break;
        }
    }

    return ATK_MS901M_ETIMEOUT;
}

uint8_t atk_ms901m_read_reg_by_id(uint8_t id, uint8_t *dat, uint32_t timeout)
{
    uint8_t buf[6];
    atk_ms901m_frame_t frame;

    if (dat == 0) {
        return 0U;
    }

    buf[0] = ATK_MS901M_FRAME_HEAD_L;
    buf[1] = ATK_MS901M_FRAME_HEAD_ACK_H;
    buf[2] = ATK_MS901M_READ_REG_ID(id);
    buf[3] = 1U;
    buf[4] = 0U;
    buf[5] = Atk_Checksum(buf, 5U);
    atk_ms901m_uart_send(buf, 6U);

    if (atk_ms901m_get_frame_by_id(&frame, id,
        ATK_MS901M_FRAME_ID_TYPE_ACK, timeout) != ATK_MS901M_EOK) {
        return 0U;
    }

    for (uint8_t i = 0U; i < frame.len; i++) {
        dat[i] = frame.dat[i];
    }

    return frame.len;
}

uint8_t atk_ms901m_write_reg_by_id(uint8_t id, const uint8_t *dat,
    uint8_t len)
{
    uint8_t buf[7];

    if (dat == 0 || (len != 1U && len != 2U)) {
        return ATK_MS901M_EINVAL;
    }

    buf[0] = ATK_MS901M_FRAME_HEAD_L;
    buf[1] = ATK_MS901M_FRAME_HEAD_ACK_H;
    buf[2] = ATK_MS901M_WRITE_REG_ID(id);
    buf[3] = len;
    buf[4] = dat[0];

    if (len == 1U) {
        buf[5] = Atk_Checksum(buf, 5U);
        atk_ms901m_uart_send(buf, 6U);
    } else {
        buf[5] = dat[1];
        buf[6] = Atk_Checksum(buf, 6U);
        atk_ms901m_uart_send(buf, 7U);
    }

    return ATK_MS901M_EOK;
}

uint8_t atk_ms901m_get_attitude(atk_ms901m_attitude_data_t *attitude_dat,
    uint32_t timeout)
{
    atk_ms901m_frame_t frame;

    if (attitude_dat == 0) {
        return ATK_MS901M_EINVAL;
    }

    if (atk_ms901m_get_frame_by_id(&frame, ATK_MS901M_FRAME_ID_ATTITUDE,
        ATK_MS901M_FRAME_ID_TYPE_UPLOAD, timeout) != ATK_MS901M_EOK ||
        frame.len != 6U) {
        return ATK_MS901M_ERROR;
    }

    attitude_dat->roll = (float)((int16_t)
        (((uint16_t)frame.dat[1] << 8) | frame.dat[0])) / 32768.0f * 180.0f;
    attitude_dat->pitch = (float)((int16_t)
        (((uint16_t)frame.dat[3] << 8) | frame.dat[2])) / 32768.0f * 180.0f;
    attitude_dat->yaw = (float)((int16_t)
        (((uint16_t)frame.dat[5] << 8) | frame.dat[4])) / 32768.0f * 180.0f;

    return ATK_MS901M_EOK;
}

uint8_t atk_ms901m_init(uint32_t baudrate)
{
#if defined(ATK_MS901M_FRAME_ID_REG_GYROFSR) && \
    defined(ATK_MS901M_FRAME_ID_REG_ACCFSR)
    uint8_t gyro_fsr[2];
    uint8_t acc_fsr[2];
#endif

    gyro_ok = 0U;
    gyro_init_result = ATK_MS901M_ERROR;
    atk_ms901m_uart_init(baudrate);
    atk_ms901m_uart_flush_rx();

#if defined(ATK_MS901M_FRAME_ID_REG_GYROFSR) && \
    defined(ATK_MS901M_FRAME_ID_REG_ACCFSR)
    if (atk_ms901m_read_reg_by_id(ATK_MS901M_FRAME_ID_REG_GYROFSR,
        gyro_fsr, 60000U) == 0U) {
        gyro_init_result = 2U;
        return ATK_MS901M_ERROR;
    }

    if (atk_ms901m_read_reg_by_id(ATK_MS901M_FRAME_ID_REG_ACCFSR,
        acc_fsr, 60000U) == 0U) {
        gyro_init_result = 3U;
        return ATK_MS901M_ERROR;
    }

    gyro_init_result = ATK_MS901M_EOK;
    gyro_ok = 1U;
    return ATK_MS901M_EOK;
#else
    /*
     * Need official register ID macro values before enabling register-based
     * init:
     * - ATK_MS901M_FRAME_ID_REG_GYROFSR
     * - ATK_MS901M_FRAME_ID_REG_ACCFSR
     *
     * Until those IDs are added, use one valid attitude upload frame as the
     * communication proof.
     */
    atk_ms901m_attitude_data_t attitude;
    if (atk_ms901m_get_attitude(&attitude, 120000U) == ATK_MS901M_EOK) {
        gyro_roll = attitude.roll;
        gyro_pitch = attitude.pitch;
        gyro_yaw = attitude.yaw;
        gyro_init_result = ATK_MS901M_EOK;
        gyro_ok = 1U;
        return ATK_MS901M_EOK;
    }

    gyro_init_result = 4U;
    return ATK_MS901M_ERROR;
#endif
}
