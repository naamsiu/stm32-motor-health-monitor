#ifndef DHT11_H
#define DHT11_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

typedef enum
{
    DHT11_OK = 0,
    DHT11_ERROR_TIMEOUT,
    DHT11_ERROR_RESPONSE_LOW,
    DHT11_ERROR_RESPONSE_HIGH,
    DHT11_ERROR_RESPONSE_END,
    DHT11_ERROR_BIT_START,
    DHT11_ERROR_BIT_END,
    DHT11_ERROR_CHECKSUM,
    DHT11_ERROR_PARAM

} DHT11_Status_t;

typedef struct
{
    uint8_t temperature;
    uint8_t humidity;
    uint8_t checksum;

} DHT11_Data_t;

void DHT11_Init(TIM_HandleTypeDef *htim);

DHT11_Status_t DHT11_Read(DHT11_Data_t *data);

#endif