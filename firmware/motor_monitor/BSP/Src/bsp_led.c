#include "bsp_led.h"
#include "main.h"

void BSP_LED_Init(void)
{
    BSP_LED_Off();
}

void BSP_LED_On(void)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
}

void BSP_LED_Off(void)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

void BSP_LED_Toggle(void)
{
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}