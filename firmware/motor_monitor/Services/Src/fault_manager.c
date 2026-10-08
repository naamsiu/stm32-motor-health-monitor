 #include "fault_manager.h"
#include "sensor_manager.h"

#define TEMP_WARNING_THRESHOLD_C 25U
#define TEMP_FAULT_THRESHOLD_C   30U

static uint32_t active_faults = FAULT_NONE;

static SystemState_t system_state =
    SYSTEM_STATE_NORMAL;


void FaultManager_Init(void)
{
    active_faults = FAULT_NONE;
    system_state = SYSTEM_STATE_NORMAL;
}


void FaultManager_Process(void)
{
    const SensorData_t *sensor =
        SensorManager_GetData();

    uint32_t new_faults = FAULT_NONE;


    /*
     * Sensor failure
     */
    if (!sensor->valid)
    {
        new_faults |= FAULT_SENSOR_FAILURE;
    }


    /*
     * Temperature fault
     */
    if (sensor->valid)
    {
        if (sensor->temperature >=
            TEMP_FAULT_THRESHOLD_C)
        {
            new_faults |=
                FAULT_OVER_TEMPERATURE;
        }
    }


    active_faults = new_faults;


    /*
     * Determine overall system state
     */
    if (active_faults != FAULT_NONE)
    {
        system_state = SYSTEM_STATE_FAULT;
    }
    else if (sensor->valid &&
             sensor->temperature >=
             TEMP_WARNING_THRESHOLD_C)
    {
        system_state = SYSTEM_STATE_WARNING;
    }
    else
    {
        system_state = SYSTEM_STATE_NORMAL;
    }
}


SystemState_t FaultManager_GetSystemState(void)
{
    return system_state;
}


uint32_t FaultManager_GetActiveFaults(void)
{
    return active_faults;
}


bool FaultManager_IsFaultActive(
    FaultCode_t fault)
{
    return
        (active_faults & fault) != 0U;
}


const char *FaultManager_GetStateString(void)
{
    switch (system_state)
    {
        case SYSTEM_STATE_NORMAL:
            return "NORMAL";

        case SYSTEM_STATE_WARNING:
            return "WARNING";

        case SYSTEM_STATE_FAULT:
            return "FAULT";

        default:
            return "UNKNOWN";
    }
}