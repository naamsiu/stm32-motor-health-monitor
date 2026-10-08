#include "fault_manager.h"
#include "sensor_manager.h"

#include "stm32f1xx_hal.h"


#define TEMP_WARNING_ON_C           35U
#define TEMP_WARNING_OFF_C          33U

#define TEMP_FAULT_ON_C             45U
#define TEMP_FAULT_OFF_C            42U


#define SENSOR_FAIL_ASSERT_COUNT    2U
#define SENSOR_FAIL_CLEAR_COUNT     2U

#define TEMP_FAULT_ASSERT_COUNT     2U
#define TEMP_FAULT_CLEAR_COUNT      2U


static uint32_t active_faults =
    FAULT_NONE;

static uint32_t fault_history =
    FAULT_NONE;

static SystemState_t system_state =
    SYSTEM_STATE_NORMAL;


static uint32_t last_processed_sequence = 0;

static uint32_t last_fault_change_ms = 0;


static uint8_t sensor_fail_count = 0;
static uint8_t sensor_ok_count = 0;

static uint8_t temp_fault_count = 0;
static uint8_t temp_clear_count = 0;

static bool warning_active = false;


/* =========================================================
 * Internal helpers
 * ========================================================= */

static void FaultManager_SetFault(
    FaultCode_t fault,
    uint32_t now)
{
    if ((active_faults & fault) == 0U)
    {
        active_faults |= fault;

        /*
         * History is latched.
         */
        fault_history |= fault;

        last_fault_change_ms = now;
    }
}


static void FaultManager_ClearFault(
    FaultCode_t fault,
    uint32_t now)
{
    if ((active_faults & fault) != 0U)
    {
        active_faults &=
            ~((uint32_t)fault);

        last_fault_change_ms = now;
    }
}


/* =========================================================
 * Init
 * ========================================================= */

void FaultManager_Init(void)
{
    active_faults = FAULT_NONE;
    fault_history = FAULT_NONE;

    system_state = SYSTEM_STATE_NORMAL;

    last_processed_sequence = 0;
    last_fault_change_ms = 0;

    sensor_fail_count = 0;
    sensor_ok_count = 0;

    temp_fault_count = 0;
    temp_clear_count = 0;

    warning_active = false;
}


/* =========================================================
 * Processing
 * ========================================================= */

void FaultManager_Process(void)
{
    const SensorData_t *sensor =
        SensorManager_GetData();

    /*
     * Do not process the same sample repeatedly.
     */
    if (sensor->sample_sequence ==
        last_processed_sequence)
    {
        return;
    }

    last_processed_sequence =
        sensor->sample_sequence;

    uint32_t now = HAL_GetTick();


    /* =====================================================
     * Sensor failure debounce
     * ===================================================== */

    if (!sensor->valid)
    {
        sensor_ok_count = 0;

        if (sensor_fail_count < 255U)
        {
            sensor_fail_count++;
        }

        if (sensor_fail_count >=
            SENSOR_FAIL_ASSERT_COUNT)
        {
            FaultManager_SetFault(
                FAULT_SENSOR_FAILURE,
                now
            );
        }
    }
    else
    {
        sensor_fail_count = 0;

        if (sensor_ok_count < 255U)
        {
            sensor_ok_count++;
        }

        if (sensor_ok_count >=
            SENSOR_FAIL_CLEAR_COUNT)
        {
            FaultManager_ClearFault(
                FAULT_SENSOR_FAILURE,
                now
            );
        }
    }


    /* =====================================================
     * Temperature processing
     * ===================================================== */

    if (sensor->valid)
    {
        /*
         * Warning hysteresis
         */
        if (!warning_active)
        {
            if (sensor->temperature >=
                TEMP_WARNING_ON_C)
            {
                warning_active = true;
            }
        }
        else
        {
            if (sensor->temperature <=
                TEMP_WARNING_OFF_C)
            {
                warning_active = false;
            }
        }


        /*
         * Temperature fault
         */
        if (!FaultManager_IsFaultActive(
                FAULT_OVER_TEMPERATURE))
        {
            temp_clear_count = 0;

            if (sensor->temperature >=
                TEMP_FAULT_ON_C)
            {
                if (temp_fault_count < 255U)
                {
                    temp_fault_count++;
                }
            }
            else
            {
                temp_fault_count = 0;
            }

            if (temp_fault_count >=
                TEMP_FAULT_ASSERT_COUNT)
            {
                FaultManager_SetFault(
                    FAULT_OVER_TEMPERATURE,
                    now
                );

                temp_fault_count = 0;
            }
        }
        else
        {
            temp_fault_count = 0;

            if (sensor->temperature <=
                TEMP_FAULT_OFF_C)
            {
                if (temp_clear_count < 255U)
                {
                    temp_clear_count++;
                }
            }
            else
            {
                temp_clear_count = 0;
            }

            if (temp_clear_count >=
                TEMP_FAULT_CLEAR_COUNT)
            {
                FaultManager_ClearFault(
                    FAULT_OVER_TEMPERATURE,
                    now
                );

                temp_clear_count = 0;
            }
        }
    }


    /* =====================================================
     * Overall system state
     * ===================================================== */

    if (active_faults != FAULT_NONE)
    {
        system_state =
            SYSTEM_STATE_FAULT;
    }
    else if (warning_active)
    {
        system_state =
            SYSTEM_STATE_WARNING;
    }
    else
    {
        system_state =
            SYSTEM_STATE_NORMAL;
    }
}


/* =========================================================
 * Getters
 * ========================================================= */

SystemState_t
FaultManager_GetSystemState(void)
{
    return system_state;
}


uint32_t
FaultManager_GetActiveFaults(void)
{
    return active_faults;
}


uint32_t
FaultManager_GetFaultHistory(void)
{
    return fault_history;
}


uint32_t
FaultManager_GetLastChangeMs(void)
{
    return last_fault_change_ms;
}


bool FaultManager_IsFaultActive(
    FaultCode_t fault)
{
    return
        (active_faults &
         (uint32_t)fault) != 0U;
}


const char *
FaultManager_GetStateString(void)
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


void FaultManager_ClearHistory(void)
{
    fault_history = FAULT_NONE;
}