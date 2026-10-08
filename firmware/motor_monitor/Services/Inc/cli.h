#ifndef CLI_H
#define CLI_H

#include "stm32f1xx_hal.h"
#include "app.h"

void CLI_Init(UART_HandleTypeDef *huart);

void CLI_Process(void);

void CLI_UART_RxCallback(UART_HandleTypeDef *huart);

#endif