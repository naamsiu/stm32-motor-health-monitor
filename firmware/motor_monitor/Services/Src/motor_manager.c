#include "motor_manager.h"
#include "fault_manager.h"

static MotorStatus_t motor_status;


void MotorManager_Init(void)
{
    motor_status.state =
        MOTOR_STATE_STOPPED;

    motor_status.direction =
        MOTOR_DIR_FORWARD;

    motor_status.speed_percent = 0;

    MotorDriver_Stop();

    MotorDriver_SetDirection(
        MOTOR_DIR_FORWARD
    );
}

void MotorManager_Start(void)
{
    /*
     * Do not start motor during system fault
     */
    if (FaultManager_GetSystemState()
        == SYSTEM_STATE_FAULT)
    {
        return;
    }

    if (motor_status.speed_percent == 0U)
    {
        motor_status.speed_percent = 30U;
    }

    MotorDriver_SetDirection(
        motor_status.direction
    );

    MotorDriver_SetSpeed(
        motor_status.speed_percent
    );

    motor_status.state =
        MOTOR_STATE_RUNNING;
}
void MotorManager_Stop(void)
{
    MotorDriver_Stop();

    motor_status.state =
        MOTOR_STATE_STOPPED;
}

void MotorManager_SetSpeed(uint8_t percent)
{
    if (percent > 100U)
    {
        percent = 100U;
    }

    motor_status.speed_percent =
        percent;

    if (motor_status.state ==
        MOTOR_STATE_RUNNING)
    {
        MotorDriver_SetSpeed(percent);

        if (percent == 0U)
        {
            motor_status.state =
                MOTOR_STATE_STOPPED;
        }
    }
}

void MotorManager_SetDirection(
    MotorDirection_t direction)
{
    if (motor_status.direction ==
        direction)
    {
        return;
    }

    bool was_running =
        motor_status.state ==
        MOTOR_STATE_RUNNING;

    /*
     * Remove PWM before changing H-bridge direction.
     */
    MotorDriver_SetSpeed(0);

    MotorDriver_SetDirection(direction);

    motor_status.direction =
        direction;

    if (was_running)
    {
        MotorDriver_SetSpeed(
            motor_status.speed_percent
        );
    }
}

void MotorManager_Process(void)
{
    if (FaultManager_GetSystemState()
        == SYSTEM_STATE_FAULT)
    {
        if (motor_status.state ==
            MOTOR_STATE_RUNNING)
        {
            MotorManager_Stop();
        }
    }
}

const MotorStatus_t *
MotorManager_GetStatus(void)
{
    return &motor_status;
}