#ifndef HCSR04_H
#define HCSR04_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#define HCSR04_TRIG_GPIO_Port GPIOB
#define HCSR04_TRIG_Pin       GPIO_PIN_5

typedef enum
{
    HCSR04_OK = 0,
    HCSR04_BUSY,
    HCSR04_ERROR_TIMEOUT,
    HCSR04_ERROR_NOT_READY,
    HCSR04_ERROR_PARAM
} HCSR04_Status_t;

void HCSR04_Init(void);
bool HCSR04_Trigger(void);
HCSR04_Status_t HCSR04_GetResult(uint32_t *echo_us);

#endif