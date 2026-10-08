#ifndef LOGGER_H
#define LOGGER_H

#include "stm32f1xx_hal.h"

void Logger_Init(UART_HandleTypeDef *huart);
void Logger_Print(const char *message);

#endif