#ifndef ALGORITHM_SENSOR_H_
#define ALGORITHM_SENSOR_H_

#include <stdint.h>

#define SENSOR_CHANNEL_COUNT     8U
#define SENSOR_I2C_ADDR          0x4C
#define SENSOR_CMD_READ_ANALOG   0x0A

typedef struct {
    uint8_t raw_data[SENSOR_CHANNEL_COUNT];
    float line_offset;
    float last_line_offset;
    uint8_t active_count;
    int is_lost;
} Sensor_TypeDef;

void Sensor_Init(Sensor_TypeDef *sensor);
void Sensor_ProcessData(Sensor_TypeDef *sensor);
void Sensor_Update(Sensor_TypeDef *sensor);

#endif
