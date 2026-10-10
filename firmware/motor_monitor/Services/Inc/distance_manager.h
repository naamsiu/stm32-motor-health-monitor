#ifndef DISTANCE_MANAGER_H
#define DISTANCE_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    DISTANCE_OK = 0,
    DISTANCE_ERROR_NO_ECHO,
    DISTANCE_ERROR_OUT_OF_RANGE,
    DISTANCE_ERROR_DRIVER
} DistanceError_t;

typedef struct
{
    uint16_t distance_mm;
    uint32_t echo_us;

    bool valid;

    uint32_t last_update_ms;
    uint32_t last_sample_ms;
    uint32_t sample_sequence;

    DistanceError_t last_error;
} DistanceData_t;

void DistanceManager_Init(void);
void DistanceManager_Process(void);

const DistanceData_t *DistanceManager_GetData(void);

const char *DistanceManager_GetErrorString(
    DistanceError_t error
);

#endif