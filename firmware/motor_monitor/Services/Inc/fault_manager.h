#ifndef FAULT_MANAGER_H
#define FAULT_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    SYSTEM_STATE_NORMAL = 0,
    SYSTEM_STATE_WARNING,
    SYSTEM_STATE_FAULT

} SystemState_t;


typedef enum
{
    FAULT_NONE = 0,

    FAULT_SENSOR_FAILURE = (1U << 0),
    FAULT_OVER_TEMPERATURE = (1U << 1)

} FaultCode_t;


void FaultManager_Init(void);

void FaultManager_Process(void);

SystemState_t FaultManager_GetSystemState(void);

uint32_t FaultManager_GetActiveFaults(void);

bool FaultManager_IsFaultActive(FaultCode_t fault);

const char *FaultManager_GetStateString(void);

#endif