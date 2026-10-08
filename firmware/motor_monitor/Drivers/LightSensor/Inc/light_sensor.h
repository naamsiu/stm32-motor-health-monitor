#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

typedef enum
{
    LIGHT_SENSOR_OK = 0,
    LIGHT_SENSOR_ERROR,
    LIGHT_SENSOR_ERROR_PARAM

} LightSensor_Status_t;


void LightSensor_Init(
    ADC_HandleTypeDef *hadc
);

LightSensor_Status_t LightSensor_Read(
    uint16_t *raw
);

uint16_t LightSensor_Invert(
    uint16_t raw
);

uint8_t LightSensor_ToPercent(
    uint16_t raw
);

#endif