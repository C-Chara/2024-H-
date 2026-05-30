#ifndef ATK_MS901M_H_
#define ATK_MS901M_H_

#include <stdint.h>

#define ATK_MS901M_FRAME_HEAD_L             (0x55U)
#define ATK_MS901M_FRAME_HEAD_UPLOAD_H      (0x55U)
#define ATK_MS901M_FRAME_HEAD_ACK_H         (0xAFU)

#define ATK_MS901M_FRAME_ID_TYPE_UPLOAD     (0x55U)
#define ATK_MS901M_FRAME_ID_TYPE_ACK        (0xAFU)

#define ATK_MS901M_EOK                      (0U)
#define ATK_MS901M_ERROR                    (1U)
#define ATK_MS901M_EINVAL                   (2U)
#define ATK_MS901M_ETIMEOUT                 (3U)

#define ATK_MS901M_FRAME_DAT_MAX_LEN        (32U)
#define ATK_MS901M_FRAME_ID_ATTITUDE        (0x01U)

/*
 * The current project does not contain official register ID macros:
 * ATK_MS901M_FRAME_ID_REG_GYROFSR / ATK_MS901M_FRAME_ID_REG_ACCFSR.
 * Add them from the ATK-MS901M manual before enabling register-based init.
 */

typedef struct
{
    uint8_t head_l;
    uint8_t head_h;
    uint8_t id;
    uint8_t len;
    uint8_t dat[ATK_MS901M_FRAME_DAT_MAX_LEN];
    uint8_t sum;
} atk_ms901m_frame_t;

typedef struct
{
    float roll;
    float pitch;
    float yaw;
} atk_ms901m_attitude_data_t;

extern volatile uint8_t gyro_ok;
extern volatile uint8_t gyro_init_result;
extern volatile uint32_t gyro_attitude_ok_count;
extern volatile uint32_t gyro_attitude_fail_count;
extern volatile uint32_t gyro_rx_count;
extern volatile uint32_t gyro_frame_ok_count;
extern volatile uint32_t gyro_frame_checksum_error;
extern volatile uint8_t gyro_last_frame_id;
extern volatile uint8_t gyro_last_frame_type;
extern volatile uint8_t gyro_last_frame_len;
extern volatile uint32_t gyro_poll_count;
extern volatile uint8_t gyro_latest_valid;
extern volatile float gyro_yaw_parser_dbg;
extern volatile uint32_t imu_valid_upload_frame_count;
extern volatile uint32_t imu_valid_attitude_frame_count;
extern volatile uint32_t imu_valid_other_frame_count;
extern volatile uint8_t imu_last_valid_upload_id;
extern volatile uint8_t imu_last_valid_upload_len;
extern volatile uint32_t imu_parser_state_reset_count;
extern volatile uint32_t imu_attitude_miss_while_run_count;
extern volatile uint8_t imu_run_source_state;

uint8_t atk_ms901m_init(uint32_t baudrate);
uint8_t atk_ms901m_read_reg_by_id(uint8_t id, uint8_t *dat, uint32_t timeout);
uint8_t atk_ms901m_write_reg_by_id(uint8_t id, const uint8_t *dat,
    uint8_t len);
uint8_t atk_ms901m_get_frame_by_id(atk_ms901m_frame_t *frame, uint8_t id,
    uint8_t frame_type, uint32_t timeout);
uint8_t atk_ms901m_get_attitude(atk_ms901m_attitude_data_t *attitude_dat,
    uint32_t timeout);
void atk_ms901m_poll(void);
uint8_t atk_ms901m_attitude_available(void);
uint8_t atk_ms901m_get_latest_attitude(atk_ms901m_attitude_data_t *att);

#endif /* ATK_MS901M_H_ */
