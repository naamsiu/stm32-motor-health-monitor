#include "light_sensor.h"

#define ADC_MAX_VALUE 4095U

static ADC_HandleTypeDef *light_adc = NULL;


void LightSensor_Init(
    ADC_HandleTypeDef *hadc)
{
    light_adc = hadc;
}


LightSensor_Status_t LightSensor_Read(
    uint16_t *raw)
{
    if ((light_adc == NULL) ||
        (raw == NULL))
    {
        return LIGHT_SENSOR_ERROR_PARAM;
    }


    if (HAL_ADC_Start(light_adc) != HAL_OK)
    {
        return LIGHT_SENSOR_ERROR;
    }


    if (HAL_ADC_PollForConversion(
            light_adc,
            10) != HAL_OK)
    {
        HAL_ADC_Stop(light_adc);

        return LIGHT_SENSOR_ERROR;
    }


    *raw =
        (uint16_t)HAL_ADC_GetValue(
            light_adc
        );


    HAL_ADC_Stop(light_adc);


    return LIGHT_SENSOR_OK;
}
uint16_t LightSensor_Invert(
    uint16_t raw)
{
    if (raw > ADC_MAX_VALUE)
    {
        raw = ADC_MAX_VALUE;
    }

    return ADC_MAX_VALUE - raw;
}
uint8_t LightSensor_ToPercent(
    uint16_t raw)
{
    uint16_t inverted =
        LightSensor_Invert(raw);

    uint32_t percent =
        ((uint32_t)inverted * 100U)
        / ADC_MAX_VALUE;

    if (percent > 100U)
    {
        percent = 100U;
    }

    return (uint8_t)percent;
}