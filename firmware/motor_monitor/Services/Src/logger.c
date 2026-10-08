#include "logger.h"
#include <string.h>

static UART_HandleTypeDef *logger_uart = NULL;

void Logger_Init(UART_HandleTypeDef *huart)
{
    logger_uart = huart;
}

void Logger_Print(const char *message)
{
    if ((logger_uart == NULL) || (message == NULL))
    {
        return;
    }

    HAL_UART_Transmit(
        logger_uart,
        (uint8_t *)message,
        strlen(message),
        HAL_MAX_DELAY
    );
}