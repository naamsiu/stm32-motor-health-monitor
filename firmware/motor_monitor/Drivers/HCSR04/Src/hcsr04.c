#include "hcsr04.h"
#include "main.h"

extern TIM_HandleTypeDef htim4;

#define HCSR04_TRIGGER_PULSE_US  10U
#define HCSR04_TIMEOUT_MS        60U

typedef enum
{
    STATE_IDLE = 0,
    STATE_WAIT_RISING,
    STATE_WAIT_FALLING,
    STATE_DONE,
    STATE_TIMEOUT
} HCSR04_State_t;

static volatile HCSR04_State_t state = STATE_IDLE;
static volatile uint16_t capture_rise = 0;
static volatile uint16_t capture_fall = 0;

static uint32_t trigger_tick = 0;
static bool initialized = false;

void HCSR04_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(
        HCSR04_TRIG_GPIO_Port,
        HCSR04_TRIG_Pin,
        GPIO_PIN_RESET
    );

    GPIO_InitStruct.Pin = HCSR04_TRIG_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*
     * TIM4 is configured by STM32CubeMX.
     * Counter must run at 1 MHz.
     */
    if (HAL_TIM_IC_Start(&htim4, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }

    __HAL_TIM_DISABLE_IT(&htim4, TIM_IT_CC1);
    __HAL_TIM_CLEAR_FLAG(&htim4, TIM_FLAG_CC1);

    state = STATE_IDLE;
    initialized = true;
}

bool HCSR04_Trigger(void)
{
    uint16_t start;

    if (!initialized)
    {
        return false;
    }

    if ((state == STATE_WAIT_RISING) ||
        (state == STATE_WAIT_FALLING))
    {
        return false;
    }

    __HAL_TIM_DISABLE_IT(&htim4, TIM_IT_CC1);
    __HAL_TIM_SET_CAPTUREPOLARITY(
        &htim4,
        TIM_CHANNEL_1,
        TIM_INPUTCHANNELPOLARITY_RISING
    );
    __HAL_TIM_CLEAR_FLAG(&htim4, TIM_FLAG_CC1);

    state = STATE_WAIT_RISING;
    trigger_tick = HAL_GetTick();

    __HAL_TIM_ENABLE_IT(&htim4, TIM_IT_CC1);

    HAL_GPIO_WritePin(
        HCSR04_TRIG_GPIO_Port,
        HCSR04_TRIG_Pin,
        GPIO_PIN_SET
    );

    start = (uint16_t)__HAL_TIM_GET_COUNTER(&htim4);

    while ((uint16_t)(
        (uint16_t)__HAL_TIM_GET_COUNTER(&htim4) - start
    ) < HCSR04_TRIGGER_PULSE_US)
    {
    }

    HAL_GPIO_WritePin(
        HCSR04_TRIG_GPIO_Port,
        HCSR04_TRIG_Pin,
        GPIO_PIN_RESET
    );

    return true;
}

HCSR04_Status_t HCSR04_GetResult(uint32_t *echo_us)
{
    if (echo_us == NULL)
    {
        return HCSR04_ERROR_PARAM;
    }

    if (!initialized)
    {
        return HCSR04_ERROR_NOT_READY;
    }

    if (((state == STATE_WAIT_RISING) ||
         (state == STATE_WAIT_FALLING)) &&
        ((HAL_GetTick() - trigger_tick) >= HCSR04_TIMEOUT_MS))
    {
        __HAL_TIM_DISABLE_IT(&htim4, TIM_IT_CC1);

        if ((state == STATE_WAIT_RISING) ||
            (state == STATE_WAIT_FALLING))
        {
            state = STATE_TIMEOUT;
        }
    }

    switch (state)
    {
        case STATE_WAIT_RISING:
        case STATE_WAIT_FALLING:
            return HCSR04_BUSY;

        case STATE_DONE:
            *echo_us = (uint16_t)(capture_fall - capture_rise);
            state = STATE_IDLE;
            return HCSR04_OK;

        case STATE_TIMEOUT:
            state = STATE_IDLE;
            return HCSR04_ERROR_TIMEOUT;

        default:
            return HCSR04_ERROR_NOT_READY;
    }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    uint16_t value;

    if ((htim->Instance != TIM4) ||
        (htim->Channel != HAL_TIM_ACTIVE_CHANNEL_1))
    {
        return;
    }

    value = (uint16_t)HAL_TIM_ReadCapturedValue(
        htim,
        TIM_CHANNEL_1
    );

    if (state == STATE_WAIT_RISING)
    {
        capture_rise = value;

        __HAL_TIM_SET_CAPTUREPOLARITY(
            htim,
            TIM_CHANNEL_1,
            TIM_INPUTCHANNELPOLARITY_FALLING
        );

        state = STATE_WAIT_FALLING;
    }
    else if (state == STATE_WAIT_FALLING)
    {
        capture_fall = value;

        __HAL_TIM_DISABLE_IT(htim, TIM_IT_CC1);
        state = STATE_DONE;
    }
}