#include "sensor.h"
#include "hal.h"

static const float SENSOR_WEIGHTS[SENSOR_CHANNEL_COUNT] = {
    -4.0f, -3.0f, -2.0f, -1.0f, 1.0f, 2.0f, 3.0f, 4.0f
};

#define SENSOR_GRAY_THRESHOLD  50U

void Sensor_Init(Sensor_TypeDef *sensor)
{
    for (uint8_t i = 0; i < SENSOR_CHANNEL_COUNT; i++) {
        sensor->raw_data[i] = 0U;
    }

    sensor->line_offset = 0.0f;
    sensor->last_line_offset = 0.0f;
    sensor->active_count = 0U;
    sensor->is_lost = 1;
}

void Sensor_ProcessData(Sensor_TypeDef *sensor)
{
    float weighted_sum = 0.0f;
    float sum = 0.0f;
    uint8_t active_count = 0U;

    for (uint8_t i = 0; i < SENSOR_CHANNEL_COUNT; i++) {
        if (sensor->raw_data[i] > SENSOR_GRAY_THRESHOLD) {
            weighted_sum += SENSOR_WEIGHTS[i] * (float)sensor->raw_data[i];
            sum += (float)sensor->raw_data[i];
            active_count++;
        }
    }

    sensor->active_count = active_count;

    if (active_count == 0U || sum <= 0.0f) {
        sensor->is_lost = 1;
        sensor->line_offset = sensor->last_line_offset;
        return;
    }

    sensor->is_lost = 0;
    sensor->line_offset = weighted_sum / sum;
    sensor->last_line_offset = sensor->line_offset;
}

void Sensor_Update(Sensor_TypeDef *sensor)
{
    uint8_t rx_buffer[SENSOR_CHANNEL_COUNT] = {0U};

    HAL_Sensor_Read_I2C(rx_buffer, SENSOR_CHANNEL_COUNT);

    for (uint8_t i = 0; i < SENSOR_CHANNEL_COUNT; i++) {
        sensor->raw_data[i] = rx_buffer[i];
    }

    Sensor_ProcessData(sensor);
}
