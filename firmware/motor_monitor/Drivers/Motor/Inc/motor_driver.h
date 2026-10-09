#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

typedef enum
{
    MOTOR_DIR_FORWARD = 0,
    MOTOR_DIR_REVERSE

} MotorDirection_t;


void MotorDriver_Init(
    TIM_HandleTypeDef *htim,
    uint32_t channel
);

void MotorDriver_SetSpeed(uint8_t percent);

void MotorDriver_SetDirection(
    MotorDirection_t direction
);

void MotorDriver_Stop(void);

#endif