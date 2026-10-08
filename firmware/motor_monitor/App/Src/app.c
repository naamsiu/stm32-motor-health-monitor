#include "app.h"
#include "bsp_led.h"
#include "stm32f1xx_hal.h"
#include "logger.h"

static AppLedMode_t led_mode = APP_LED_OFF;

static uint32_t last_blink_tick = 0;

void App_Init(void)
{
    BSP_LED_Init();

    led_mode = APP_LED_OFF;

    Logger_Print("Motor Monitor started\r\n");
}

void App_SetLedMode(AppLedMode_t mode)
{
    led_mode = mode;

    switch (led_mode)
    {
        case APP_LED_OFF:
            BSP_LED_Off();
            break;

        case APP_LED_ON:
            BSP_LED_On();
            break;

        case APP_LED_BLINK:
            last_blink_tick = HAL_GetTick();
            break;

        default:
            break;
    }
}

void App_Run(void)
{
    if (led_mode == APP_LED_BLINK)
    {
        uint32_t now = HAL_GetTick();

        if ((now - last_blink_tick) >= 500)
        {
            BSP_LED_Toggle();

            last_blink_tick = now;
        }
    }
}