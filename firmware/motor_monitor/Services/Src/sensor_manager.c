#include "sensor_manager.h"

#define SENSOR_UPDATE_PERIOD_MS 2000U

static SensorData_t sensor_data;

static uint32_t last_sample_tick = 0;


void SensorManager_Init(void)
{
    sensor_data.temperature = 0;
    sensor_data.humidity = 0;

    sensor_data.valid = false;

    sensor_data.last_update_ms = 0;
    sensor_data.last_sample_ms = 0;

    sensor_data.sample_sequence = 0;

    sensor_data.last_error = DHT11_OK;

    last_sample_tick = 0;
}


void SensorManager_Process(void)
{
    uint32_t now = HAL_GetTick();

    if ((now - last_sample_tick) <
        SENSOR_UPDATE_PERIOD_MS)
    {
        return;
    }

    last_sample_tick = now;

    DHT11_Data_t dht_data;

    DHT11_Status_t status =
        DHT11_Read(&dht_data);

    /*
     * New sensor acquisition occurred
     */
    sensor_data.sample_sequence++;
    sensor_data.last_sample_ms = now;
    sensor_data.last_error = status;

    if (status == DHT11_OK)
    {
        sensor_data.temperature =
            dht_data.temperature;

        sensor_data.humidity =
            dht_data.humidity;

        sensor_data.valid = true;

        sensor_data.last_update_ms = now;
    }
    else
    {
        sensor_data.valid = false;
    }
}


const SensorData_t *SensorManager_GetData(void)
{
    return &sensor_data;
}