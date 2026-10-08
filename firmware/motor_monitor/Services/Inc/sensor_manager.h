#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "stm32f1xx_hal.h"
#include "dht11.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint8_t temperature;
    uint8_t humidity;

    bool valid;

    uint32_t last_update_ms;

    DHT11_Status_t last_error;

} SensorData_t;


void SensorManager_Init(void);

void SensorManager_Process(void);

const SensorData_t *SensorManager_GetData(void);

#endif