#ifndef MOTOR_MANAGER_H
#define MOTOR_MANAGER_H

#include "motor_driver.h"

#include <stdint.h>
#include <stdbool.h>


typedef enum
{
    MOTOR_STATE_STOPPED = 0,
    MOTOR_STATE_RUNNING

} MotorState_t;


typedef struct
{
    MotorState_t state;

    MotorDirection_t direction;

    uint8_t speed_percent;

} MotorStatus_t;


void MotorManager_Init(void);

void MotorManager_Process(void);

void MotorManager_Start(void);

void MotorManager_Stop(void);

void MotorManager_SetSpeed(uint8_t percent);

void MotorManager_SetDirection(
    MotorDirection_t direction
);

const MotorStatus_t *
MotorManager_GetStatus(void);

#endif