#include "cli.h"
#include "ring_buffer.h"
#include "logger.h"
#include "bsp_led.h"
#include "app.h"
#include "sensor_manager.h"
#include "fault_manager.h"


#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define CLI_COMMAND_SIZE 64
#define CLI_MAX_ARGS         4

static UART_HandleTypeDef *cli_uart;

static RingBuffer_t rx_buffer;

static uint8_t rx_byte;

static char command_buffer[CLI_COMMAND_SIZE];

static uint16_t command_index = 0;

static bool command_overflow = false;
static bool previous_was_cr = false;

static volatile uint32_t rx_drop_count = 0;

typedef void (*CLI_CommandHandler_t)(int argc, char *argv[]);

typedef struct
{
    const char *name;
    const char *usage;
    const char *description;
    CLI_CommandHandler_t handler;
} CLI_Command_t;

static void CLI_CmdHelp(int argc, char *argv[]);
static void CLI_CmdVersion(int argc, char *argv[]);
static void CLI_CmdStatus(int argc, char *argv[]);
static void CLI_CmdUptime(int argc, char *argv[]);
static void CLI_CmdLed(int argc, char *argv[]);
static void CLI_CmdClear(int argc, char *argv[]);
static void CLI_CmdReset(int argc, char *argv[]);
static void CLI_CmdSensor(int argc,char *argv[]);
static void CLI_CmdFault(int argc,char *argv[]);

static const CLI_Command_t commands[] =
{
    {
        "help",
        "help",
        "Show available commands",
        CLI_CmdHelp
    },
    {
        "version",
        "version",
        "Show firmware version",
        CLI_CmdVersion
    },
    {
        "status",
        "status",
        "Show system status",
        CLI_CmdStatus
    },
    {
        "uptime",
        "uptime",
        "Show system uptime",
        CLI_CmdUptime
    },
    {
        "led",
        "led <on|off|blink>",
        "Control status LED",
        CLI_CmdLed
    },
    {
    "sensor",
    "sensor",
    "Read DHT11 temperature and humidity",
    CLI_CmdSensor
    },
    {
    "fault",
    "fault [clear]",
    "Show or clear fault history",
    CLI_CmdFault
},
    {
        "clear",
        "clear",
        "Clear terminal",
        CLI_CmdClear
    },
    {
        "reset",
        "reset",
        "Reset the MCU",
        CLI_CmdReset
    }
};

#define CLI_COMMAND_COUNT \
    (sizeof(commands) / sizeof(commands[0]))


static void CLI_CmdFault(
    int argc,
    char *argv[])
{
    if (argc == 2)
    {
        if (strcmp(argv[1], "clear") == 0)
        {
            FaultManager_ClearHistory();

            Logger_Print(
                "Fault history cleared\r\n"
            );

            return;
        }

        Logger_Print(
            "Usage: fault [clear]\r\n"
        );

        return;
    }


    if (argc != 1)
    {
        Logger_Print(
            "Usage: fault [clear]\r\n"
        );

        return;
    }


    Logger_Print(
        "Fault Status\r\n"
        "------------\r\n"
    );

    Logger_Print("System : ");

    Logger_Print(
        FaultManager_GetStateString()
    );

    Logger_Print("\r\n");


    uint32_t active =
        FaultManager_GetActiveFaults();


    if (active == FAULT_NONE)
    {
        Logger_Print(
            "Active faults : NONE\r\n"
        );
    }
    else
    {
        Logger_Print(
            "Active faults:\r\n"
        );

        if (FaultManager_IsFaultActive(
                FAULT_SENSOR_FAILURE))
        {
            Logger_Print(
                "- SENSOR_FAILURE\r\n"
            );
        }

        if (FaultManager_IsFaultActive(
                FAULT_OVER_TEMPERATURE))
        {
            Logger_Print(
                "- OVER_TEMPERATURE\r\n"
            );
        }
    }


    uint32_t history =
        FaultManager_GetFaultHistory();


    if (history == FAULT_NONE)
    {
        Logger_Print(
            "History       : NONE\r\n"
        );
    }
    else
    {
        Logger_Print(
            "History:\r\n"
        );

        if ((history &
             FAULT_SENSOR_FAILURE) != 0U)
        {
            Logger_Print(
                "- SENSOR_FAILURE\r\n"
            );
        }

        if ((history &
             FAULT_OVER_TEMPERATURE) != 0U)
        {
            Logger_Print(
                "- OVER_TEMPERATURE\r\n"
            );
        }
    }
}

static int CLI_Tokenize(
    char *line,
    char *argv[],
    int max_args)
{
    int argc = 0;
    char *p = line;

    while (*p != '\0')
    {
        while ((*p == ' ') || (*p == '\t'))
        {
            p++;
        }

        if (*p == '\0')
        {
            break;
        }

        if (argc >= max_args)
        {
            break;
        }

        argv[argc++] = p;

        while ((*p != '\0') &&
               (*p != ' ') &&
               (*p != '\t'))
        {
            p++;
        }

        if (*p != '\0')
        {
            *p = '\0';
            p++;
        }
    }

    return argc;
}

static void CLI_CmdSensor(
    int argc,
    char *argv[])
{
    (void)argc;
    (void)argv;

    const SensorData_t *data =
        SensorManager_GetData();

    char buffer[160];

    if (!data->valid)
    {
        snprintf(
            buffer,
            sizeof(buffer),
            "Sensor data unavailable\r\n"
            "Last error : %u\r\n",
            (unsigned int)data->last_error
        );

        Logger_Print(buffer);

        return;
    }

    snprintf(
        buffer,
        sizeof(buffer),
        "DHT11\r\n"
        "-----\r\n"
        "Temperature : %u C\r\n"
        "Humidity    : %u %%\r\n"
        "Age         : %lu ms\r\n",
        data->temperature,
        data->humidity,
        (unsigned long)(
            HAL_GetTick()
            - data->last_update_ms
        )
    );

    Logger_Print(buffer);
}

static void CLI_ExecuteCommand(char *line)
{
    char *argv[CLI_MAX_ARGS];

    int argc = CLI_Tokenize(
        line,
        argv,
        CLI_MAX_ARGS
    );

    if (argc == 0)
    {
        return;
    }

    for (uint32_t i = 0;
         i < CLI_COMMAND_COUNT;
         i++)
    {
        if (strcmp(
                argv[0],
                commands[i].name) == 0)
        {
            commands[i].handler(argc, argv);
            return;
        }
    }

    Logger_Print(
        "Unknown command. Type 'help'.\r\n"
    );
}


static void CLI_CmdHelp(
    int argc,
    char *argv[])
{
    (void)argc;
    (void)argv;

    Logger_Print(
        "Available commands:\r\n"
    );

    for (uint32_t i = 0;
         i < CLI_COMMAND_COUNT;
         i++)
    {
        Logger_Print("  ");
        Logger_Print(commands[i].usage);
        Logger_Print("\r\n      ");
        Logger_Print(commands[i].description);
        Logger_Print("\r\n");
    }
}

static void CLI_CmdVersion(
    int argc,
    char *argv[])
{
    (void)argc;
    (void)argv;

    Logger_Print(
        "Motor Monitor v0.4.0\r\n"
    );
}

static void CLI_CmdUptime(
    int argc,
    char *argv[])
{
    (void)argc;
    (void)argv;

    char buffer[64];

    uint32_t uptime =
        HAL_GetTick() / 1000U;

    snprintf(
        buffer,
        sizeof(buffer),
        "Uptime: %lu seconds\r\n",
        (unsigned long)uptime
    );

    Logger_Print(buffer);
}

static void CLI_CmdLed(
    int argc,
    char *argv[])
{
    if (argc != 2)
    {
        Logger_Print(
            "Usage: led <on|off|blink>\r\n"
        );
        return;
    }

    if (strcmp(argv[1], "on") == 0)
    {
        App_SetLedMode(APP_LED_ON);

        Logger_Print("OK\r\n");
    }
    else if (strcmp(argv[1], "off") == 0)
    {
        App_SetLedMode(APP_LED_OFF);

        Logger_Print("OK\r\n");
    }
    else if (strcmp(argv[1], "blink") == 0)
    {
        App_SetLedMode(APP_LED_BLINK);

        Logger_Print("OK\r\n");
    }
    else
    {
        Logger_Print(
            "Usage: led <on|off|blink>\r\n"
        );
    }
}

static const char *CLI_GetLedModeString(void)
{
    switch (App_GetLedMode())
    {
        case APP_LED_OFF:
            return "OFF";

        case APP_LED_ON:
            return "ON";

        case APP_LED_BLINK:
            return "BLINK";

        default:
            return "UNKNOWN";
    }
}

static void CLI_CmdStatus(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    char buffer[192];

    const SensorData_t *sensor =
        SensorManager_GetData();

    if (sensor->valid)
    {
        snprintf(
            buffer,
            sizeof(buffer),
            "System Status\r\n"
            "-------------\r\n"
            "LED mode    : %s\r\n"
            "Temperature : %u C\r\n"
            "Humidity    : %u %%\r\n"
            "Fault       : %s\r\n"
            "Uptime      : %lu s\r\n"
            "RX drops    : %lu\r\n",
            CLI_GetLedModeString(),
            sensor->temperature,
            sensor->humidity,
            FaultManager_GetStateString(),
            (unsigned long)(
                HAL_GetTick() / 1000U
            ),
            (unsigned long)rx_drop_count
        );
    }
    else
    {
        snprintf(
            buffer,
            sizeof(buffer),
            "System Status\r\n"
            "-------------\r\n"
            "LED mode    : %s\r\n"
            "Sensor      : INVALID\r\n"
            "Fault       : %s\r\n"
            "Uptime      : %lu s\r\n"
            "RX drops    : %lu\r\n",
            CLI_GetLedModeString(),
            FaultManager_GetStateString(),
            (unsigned long)(
                HAL_GetTick() / 1000U
            ),
            (unsigned long)rx_drop_count
        );
    }

    Logger_Print(buffer);
}

static void CLI_CmdClear(
    int argc,
    char *argv[])
{
    (void)argc;
    (void)argv;

    Logger_Print("\033[2J\033[H");
}

static void CLI_CmdReset(
    int argc,
    char *argv[])
{
    (void)argc;
    (void)argv;

    Logger_Print(
        "Resetting system...\r\n"
    );

    HAL_Delay(50);

    NVIC_SystemReset();
}

void CLI_UART_RxCallback(
    UART_HandleTypeDef *huart)
{
    if (huart != cli_uart)
    {
        return;
    }

    if (!RingBuffer_Push(
            &rx_buffer,
            rx_byte))
    {
        rx_drop_count++;
    }

    HAL_UART_Receive_IT(
        cli_uart,
        &rx_byte,
        1
    );
}

static void CLI_ProcessLine(void)
{
    if (command_overflow)
    {
        Logger_Print(
            "Error: command too long\r\n"
        );

        command_overflow = false;
        command_index = 0;

        Logger_Print("> ");
        return;
    }

    command_buffer[command_index] = '\0';

    if (command_index > 0)
    {
        CLI_ExecuteCommand(
            command_buffer
        );
    }

    command_index = 0;

    Logger_Print("> ");
}
void CLI_Process(void)
{
    uint8_t data;

    while (RingBuffer_Pop(
        &rx_buffer,
        &data))
    {
        if (data == '\r')
        {
            Logger_Print("\r\n");

            CLI_ProcessLine();

            previous_was_cr = true;
        }
        else if (data == '\n')
        {
            if (!previous_was_cr)
            {
                Logger_Print("\r\n");

                CLI_ProcessLine();
            }

            previous_was_cr = false;
        }
        else
        {
            previous_was_cr = false;

            if ((data == 0x08) ||
                (data == 0x7F))
            {
                if (command_index > 0)
                {
                    command_index--;

                    Logger_Print(
                        "\b \b"
                    );
                }
            }
            else if ((data >= 32) &&
                     (data <= 126))
            {
                if (command_index <
                    CLI_COMMAND_SIZE - 1)
                {
                    command_buffer[
                        command_index++] =
                        (char)data;

                    char echo[2];

                    echo[0] = (char)data;
                    echo[1] = '\0';

                    Logger_Print(echo);
                }
                else
                {
                    command_overflow = true;
                }
            }
        }
    }
}

void CLI_Init(
    UART_HandleTypeDef *huart)
{
    if (huart == NULL)
    {
        return;
    }

    cli_uart = huart;

    RingBuffer_Init(&rx_buffer);

    command_index = 0;
    command_overflow = false;
    previous_was_cr = false;
    rx_drop_count = 0;

    HAL_UART_Receive_IT(
        cli_uart,
        &rx_byte,
        1
    );

    Logger_Print(
        "\r\n"
        "Motor Monitor CLI v0.4.0\r\n"
        "Type 'help' for commands\r\n"
        "> "
    );
}

