#include "distance_manager.h"
#include "hcsr04.h"
#include "stm32f1xx_hal.h"

#define DISTANCE_UPDATE_PERIOD_MS  100U
#define DISTANCE_MIN_MM            20U
#define DISTANCE_MAX_MM            4000U

static DistanceData_t distance_data;
static uint32_t last_trigger_tick = 0;
static bool measuring = false;

void DistanceManager_Init(void)
{
    distance_data.distance_mm = 0;
    distance_data.echo_us = 0;
    distance_data.valid = false;
    distance_data.last_update_ms = 0;
    distance_data.last_sample_ms = 0;
    distance_data.sample_sequence = 0;
    distance_data.last_error = DISTANCE_OK;

    last_trigger_tick = HAL_GetTick();
    measuring = false;
}

void DistanceManager_Process(void)
{
    uint32_t now = HAL_GetTick();
    uint32_t echo_us = 0;

    if (!measuring)
    {
        if ((now - last_trigger_tick) <
            DISTANCE_UPDATE_PERIOD_MS)
        {
            return;
        }

        if (HCSR04_Trigger())
        {
            last_trigger_tick = now;
            measuring = true;
        }

        return;
    }

    HCSR04_Status_t status =
        HCSR04_GetResult(&echo_us);

    if (status == HCSR04_BUSY)
    {
        return;
    }

    measuring = false;

    distance_data.sample_sequence++;
    distance_data.last_sample_ms = now;

    if (status == HCSR04_ERROR_TIMEOUT)
    {
        distance_data.valid = false;
        distance_data.last_error = DISTANCE_ERROR_NO_ECHO;
        return;
    }

    if (status != HCSR04_OK)
    {
        distance_data.valid = false;
        distance_data.last_error = DISTANCE_ERROR_DRIVER;
        return;
    }

    /*
     * Approximate speed of sound at 20 C = 343 m/s.
     * Distance in mm = echo_us * 343 / 2000.
     */
    uint32_t distance_mm = (echo_us * 343U) / 2000U;

    distance_data.echo_us = echo_us;

    if ((distance_mm < DISTANCE_MIN_MM) ||
        (distance_mm > DISTANCE_MAX_MM))
    {
        distance_data.valid = false;
        distance_data.last_error =
            DISTANCE_ERROR_OUT_OF_RANGE;
        return;
    }

    distance_data.distance_mm = (uint16_t)distance_mm;
    distance_data.valid = true;
    distance_data.last_error = DISTANCE_OK;
    distance_data.last_update_ms = now;
}

const DistanceData_t *DistanceManager_GetData(void)
{
    return &distance_data;
}

const char *DistanceManager_GetErrorString(
    DistanceError_t error)
{
    switch (error)
    {
        case DISTANCE_OK:
            return "NONE";

        case DISTANCE_ERROR_NO_ECHO:
            return "NO_ECHO";

        case DISTANCE_ERROR_OUT_OF_RANGE:
            return "OUT_OF_RANGE";

        case DISTANCE_ERROR_DRIVER:
            return "DRIVER";

        default:
            return "UNKNOWN";
    }
}