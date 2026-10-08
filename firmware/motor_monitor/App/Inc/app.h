#ifndef APP_H
#define APP_H

typedef enum
{
    APP_LED_OFF = 0,
    APP_LED_ON,
    APP_LED_BLINK
} AppLedMode_t;

void App_Init(void);
void App_Run(void);

void App_SetLedMode(AppLedMode_t mode);
AppLedMode_t App_GetLedMode(void);

#endif