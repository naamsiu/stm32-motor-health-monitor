#include "app.h"
#include "bsp_led.h"
#include "stm32f1xx_hal.h"

void App_Init(void)
{
    BSP_LED_Init();
}

void App_Run(void)
{
    BSP_LED_Toggle();
    HAL_Delay(500);
}