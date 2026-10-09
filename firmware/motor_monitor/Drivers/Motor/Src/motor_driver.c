#include "motor_driver.h"
#include "main.h"

static TIM_HandleTypeDef *motor_timer = NULL;
static uint32_t motor_channel = 0;


void MotorDriver_Init(
    TIM_HandleTypeDef *htim,
    uint32_t channel)
{
    motor_timer = htim;
    motor_channel = channel;

    /* Safe startup */
    HAL_GPIO_WritePin(
        MOTOR_IN1_GPIO_Port,
        MOTOR_IN1_Pin,
        GPIO_PIN_RESET
    );

    HAL_GPIO_WritePin(
        MOTOR_IN2_GPIO_Port,
        MOTOR_IN2_Pin,
        GPIO_PIN_RESET
    );

    __HAL_TIM_SET_COMPARE(
        motor_timer,
        motor_channel,
        0
    );

    HAL_TIM_PWM_Start(
        motor_timer,
        motor_channel
    );
}
void MotorDriver_SetSpeed(uint8_t percent)
{
    if (motor_timer == NULL)
    {
        return;
    }

    if (percent > 100U)
    {
        percent = 100U;
    }

    uint32_t arr =
        __HAL_TIM_GET_AUTORELOAD(
            motor_timer
        );

    uint32_t compare =
        ((arr + 1U) * percent) / 100U;

    if (compare > arr)
    {
        compare = arr;
    }

    __HAL_TIM_SET_COMPARE(
        motor_timer,
        motor_channel,
        compare
    );
}
void MotorDriver_SetDirection(
    MotorDirection_t direction)
{
    if (direction == MOTOR_DIR_FORWARD)
    {
        HAL_GPIO_WritePin(
            MOTOR_IN1_GPIO_Port,
            MOTOR_IN1_Pin,
            GPIO_PIN_SET
        );

        HAL_GPIO_WritePin(
            MOTOR_IN2_GPIO_Port,
            MOTOR_IN2_Pin,
            GPIO_PIN_RESET
        );
    }
    else
    {
        HAL_GPIO_WritePin(
            MOTOR_IN1_GPIO_Port,
            MOTOR_IN1_Pin,
            GPIO_PIN_RESET
        );

        HAL_GPIO_WritePin(
            MOTOR_IN2_GPIO_Port,
            MOTOR_IN2_Pin,
            GPIO_PIN_SET
        );
    }
}
void MotorDriver_Stop(void)
{
    MotorDriver_SetSpeed(0);

    HAL_GPIO_WritePin(
        MOTOR_IN1_GPIO_Port,
        MOTOR_IN1_Pin,
        GPIO_PIN_RESET
    );

    HAL_GPIO_WritePin(
        MOTOR_IN2_GPIO_Port,
        MOTOR_IN2_Pin,
        GPIO_PIN_RESET
    );
}