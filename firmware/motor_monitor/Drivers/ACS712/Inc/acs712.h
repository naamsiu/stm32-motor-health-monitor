
#ifndef ACS712_H
#define ACS712_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/*
 * ACS712 current sensor
 *
 * MCU     : STM32F103C8T6
 * ADC     : ADC1
 * Channel : ADC_CHANNEL_1
 * Pin     : PA1
 *
 * ADC1 is shared with the LightSensor on PA0.
 */

/* Driver status */
typedef enum
{
    ACS712_OK = 0,
    ACS712_ERROR_PARAM,
    ACS712_ERROR_NOT_INITIALIZED,
    ACS712_ERROR_ADC

} ACS712_Status_t;

/* Initialize ACS712 driver */
void ACS712_Init(ADC_HandleTypeDef *hadc);

/* Read one ADC sample */
ACS712_Status_t ACS712_ReadRaw(uint16_t *raw);

/* Read average ADC value */
ACS712_Status_t ACS712_ReadAverage(
    uint16_t sample_count,
    uint16_t *average
);

/* Calibrate zero-current offset */
ACS712_Status_t ACS712_CalibrateZero(void);

/* Check whether zero calibration is done */
bool ACS712_IsCalibrated(void);

/* Get zero-current ADC value */
uint16_t ACS712_GetZeroRaw(void);

#endif /* ACS712_H */
