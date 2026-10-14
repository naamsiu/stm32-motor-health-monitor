
#include "acs712.h"
#include <stddef.h>

/* ACS712 configuration */
#define ACS712_CHANNEL          ADC_CHANNEL_1
#define ACS712_ADC_TIMEOUT_MS   10U
#define ACS712_MAX_SAMPLES      256U
#define ACS712_ZERO_SAMPLES     128U

/* Private variables */
static ADC_HandleTypeDef *acs_adc = NULL;

static uint16_t zero_raw = 0U;
static bool zero_calibrated = false;


/*
 * Initialize ACS712 driver
 */
void ACS712_Init(ADC_HandleTypeDef *hadc)
{
    acs_adc = hadc;

    zero_raw = 0U;
    zero_calibrated = false;
}


/*
 * Select ADC1 Channel 1 (PA1)
 */
static ACS712_Status_t ACS712_SelectChannel(void)
{
    ADC_ChannelConfTypeDef config = {0};

    if (acs_adc == NULL)
    {
        return ACS712_ERROR_NOT_INITIALIZED;
    }

    config.Channel = ACS712_CHANNEL;
    config.Rank = ADC_REGULAR_RANK_1;
    config.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;

    if (HAL_ADC_ConfigChannel(acs_adc, &config) != HAL_OK)
    {
        return ACS712_ERROR_ADC;
    }

    return ACS712_OK;
}


/*
 * Perform one ADC conversion.
 * ADC channel must be selected before calling.
 */
static ACS712_Status_t ACS712_ConvertOnce(uint16_t *raw)
{
    HAL_StatusTypeDef stop_status;

    if (HAL_ADC_Start(acs_adc) != HAL_OK)
    {
        return ACS712_ERROR_ADC;
    }

    if (HAL_ADC_PollForConversion(
            acs_adc,
            ACS712_ADC_TIMEOUT_MS) != HAL_OK)
    {
        (void)HAL_ADC_Stop(acs_adc);
        return ACS712_ERROR_ADC;
    }

    *raw = (uint16_t)HAL_ADC_GetValue(acs_adc);

    stop_status = HAL_ADC_Stop(acs_adc);

    if (stop_status != HAL_OK)
    {
        return ACS712_ERROR_ADC;
    }

    return ACS712_OK;
}


/*
 * Read one ADC sample
 */
ACS712_Status_t ACS712_ReadRaw(uint16_t *raw)
{
    ACS712_Status_t status;

    if (raw == NULL)
    {
        return ACS712_ERROR_PARAM;
    }

    status = ACS712_SelectChannel();

    if (status != ACS712_OK)
    {
        return status;
    }

    return ACS712_ConvertOnce(raw);
}


/*
 * Read multiple ADC samples and calculate average.
 *
 * Blocking function.
 * Only call from main loop, not inside ISR.
 */
ACS712_Status_t ACS712_ReadAverage(
    uint16_t sample_count,
    uint16_t *average)
{
    ACS712_Status_t status;

    uint32_t sum = 0U;
    uint16_t sample;
    uint16_t i;

    if ((average == NULL) ||
        (sample_count == 0U) ||
        (sample_count > ACS712_MAX_SAMPLES))
    {
        return ACS712_ERROR_PARAM;
    }

    status = ACS712_SelectChannel();

    if (status != ACS712_OK)
    {
        return status;
    }

    for (i = 0U; i < sample_count; i++)
    {
        status = ACS712_ConvertOnce(&sample);

        if (status != ACS712_OK)
        {
            return status;
        }

        sum += sample;
    }

    *average = (uint16_t)(
        (sum + (sample_count / 2U)) / sample_count
    );

    return ACS712_OK;
}


/*
 * Zero-current calibration.
 *
 * IMPORTANT:
 * No current must flow through ACS712 during calibration.
 */
ACS712_Status_t ACS712_CalibrateZero(void)
{
    uint16_t baseline;

    zero_calibrated = false;

    ACS712_Status_t status =
        ACS712_ReadAverage(
            ACS712_ZERO_SAMPLES,
            &baseline
        );

    if (status != ACS712_OK)
    {
        return status;
    }

    zero_raw = baseline;
    zero_calibrated = true;

    return ACS712_OK;
}


/*
 * Return calibration status
 */
bool ACS712_IsCalibrated(void)
{
    return zero_calibrated;
}


/*
 * Return ADC zero offset
 */
uint16_t ACS712_GetZeroRaw(void)
{
    return zero_raw;
}
