#include "cli.h"
#include "ring_buffer.h"
#include "logger.h"
#include "bsp_led.h"

#include <string.h>

#define CLI_COMMAND_SIZE 64

static UART_HandleTypeDef *cli_uart;

static RingBuffer_t rx_buffer;

static uint8_t rx_byte;

static char command_buffer[CLI_COMMAND_SIZE];

static uint16_t command_index = 0;
void CLI_Init(UART_HandleTypeDef *huart)
{
    cli_uart = huart;

    RingBuffer_Init(&rx_buffer);

    HAL_UART_Receive_IT(
        cli_uart,
        &rx_byte,
        1
    );

    Logger_Print(
        "\r\nMotor Monitor CLI\r\n"
        "Type 'help' for commands\r\n"
        "> "
    );
}
void CLI_UART_RxCallback(
    UART_HandleTypeDef *huart)
{
    if (huart == cli_uart)
    {
        RingBuffer_Push(
            &rx_buffer,
            rx_byte
        );

        HAL_UART_Receive_IT(
            cli_uart,
            &rx_byte,
            1
        );
    }
}
static void CLI_ExecuteCommand(
    const char *command)
{
    Logger_Print("\r\nReceived: [");
    Logger_Print(command);
    Logger_Print("]\r\n");
    if (strcmp(command, "help") == 0)
    {
        Logger_Print(
            "\r\nAvailable commands:\r\n"
            "help\r\n"
            "version\r\n"
            "status\r\n"
            "led on\r\n"
            "led off\r\n"
            "led blink\r\n"
        );
    }

    else if (strcmp(command, "version") == 0)
    {
        Logger_Print(
            "\r\nMotor Monitor v0.3.0\r\n"
        );
    }

    else if (strcmp(command, "status") == 0)
    {
        Logger_Print(
            "\r\nSystem status: OK\r\n"
        );
    }
    else if (strcmp(command, "led on") == 0)
    {
        App_SetLedMode(APP_LED_ON);

        Logger_Print("\r\nOK\r\n");
    }

    

    else if (strcmp(command, "led off") == 0)
    {
        App_SetLedMode(APP_LED_OFF);

        Logger_Print("\r\nOK\r\n");
    }
    else if (strcmp(command, "led blink") == 0)
    {
        App_SetLedMode(APP_LED_BLINK);

        Logger_Print("\r\nOK\r\n");
    }

    else
    {
        Logger_Print(
            "\r\nUnknown command\r\n"
        );
    }

    Logger_Print("> ");
}
void CLI_Process(void)
{
    uint8_t data;

    while (RingBuffer_Pop(
        &rx_buffer,
        &data))
    {
        if ((data == '\r') ||
            (data == '\n'))
        {
            if (command_index > 0)
            {
                command_buffer[
                    command_index
                ] = '\0';

                CLI_ExecuteCommand(
                    command_buffer
                );

                command_index = 0;
            }
        }
        else
        {
            if (command_index <
                CLI_COMMAND_SIZE - 1)
            {
                command_buffer[
                    command_index++
                ] = data;
            }
        }
    }
}