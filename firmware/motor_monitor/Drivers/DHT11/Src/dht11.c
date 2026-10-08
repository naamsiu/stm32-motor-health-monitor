#include "dht11.h"
#include "main.h"

static TIM_HandleTypeDef *dht_timer = NULL;


/* =========================================================
 * Delay microsecond
 * TIM2 must run at 1 MHz -> 1 tick = 1 us
 * ========================================================= */
static void DHT11_DelayUs(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(dht_timer, 0);

    while (__HAL_TIM_GET_COUNTER(dht_timer) < us)
    {
    }
}


/* =========================================================
 * Configure DATA pin as output
 * ========================================================= */
static void DHT11_PinOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_DATA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(
        DHT11_DATA_GPIO_Port,
        &GPIO_InitStruct
    );
}


/* =========================================================
 * Configure DATA pin as input
 * ========================================================= */
static void DHT11_PinInput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_DATA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;

    /* Internal pull-up for reliability while testing */
    GPIO_InitStruct.Pull = GPIO_PULLUP;

    HAL_GPIO_Init(
        DHT11_DATA_GPIO_Port,
        &GPIO_InitStruct
    );
}


/* =========================================================
 * Read DATA pin
 * ========================================================= */
static GPIO_PinState DHT11_ReadPin(void)
{
    return HAL_GPIO_ReadPin(
        DHT11_DATA_GPIO_Port,
        DHT11_DATA_Pin
    );
}


/* =========================================================
 * Wait until DATA reaches expected state
 * ========================================================= */
static DHT11_Status_t DHT11_WaitForState(
    GPIO_PinState state,
    uint16_t timeout_us)
{
    __HAL_TIM_SET_COUNTER(dht_timer, 0);

    while (DHT11_ReadPin() != state)
    {
        if (__HAL_TIM_GET_COUNTER(dht_timer) >= timeout_us)
        {
            return DHT11_ERROR_TIMEOUT;
        }
    }

    return DHT11_OK;
}


/* =========================================================
 * Initialize driver
 * ========================================================= */
void DHT11_Init(TIM_HandleTypeDef *htim)
{
    if (htim == NULL)
    {
        return;
    }

    dht_timer = htim;

    HAL_TIM_Base_Start(dht_timer);

    DHT11_PinOutput();

    /* DATA idle HIGH */
    HAL_GPIO_WritePin(
        DHT11_DATA_GPIO_Port,
        DHT11_DATA_Pin,
        GPIO_PIN_SET
    );
}


/* =========================================================
 * Send start signal
 * ========================================================= */
static void DHT11_StartSignal(void)
{
    DHT11_PinOutput();

    /* MCU pulls DATA LOW */
    HAL_GPIO_WritePin(
        DHT11_DATA_GPIO_Port,
        DHT11_DATA_Pin,
        GPIO_PIN_RESET
    );

    HAL_Delay(20);

    /* Release DATA */
    HAL_GPIO_WritePin(
        DHT11_DATA_GPIO_Port,
        DHT11_DATA_Pin,
        GPIO_PIN_SET
    );

    DHT11_DelayUs(25);

    /*
     * From now on DHT11 controls DATA.
     */
    DHT11_PinInput();
}


/* =========================================================
 * Check initial response from DHT11
 *
 * Sensor response:
 *
 * LOW  ~80 us
 * HIGH ~80 us
 * LOW  -> first data bit
 * ========================================================= */
static DHT11_Status_t DHT11_CheckResponse(void)
{
    if (DHT11_WaitForState(
            GPIO_PIN_RESET,
            150) != DHT11_OK)
    {
        return DHT11_ERROR_RESPONSE_LOW;
    }

    if (DHT11_WaitForState(
            GPIO_PIN_SET,
            150) != DHT11_OK)
    {
        return DHT11_ERROR_RESPONSE_HIGH;
    }

    if (DHT11_WaitForState(
            GPIO_PIN_RESET,
            150) != DHT11_OK)
    {
        return DHT11_ERROR_RESPONSE_END;
    }

    return DHT11_OK;
}


/* =========================================================
 * Read one data bit
 *
 * Each bit:
 *
 * LOW ~50 us
 *
 * HIGH ~26 us -> bit 0
 * HIGH ~70 us -> bit 1
 *
 * Sample at 40 us
 * ========================================================= */
static DHT11_Status_t DHT11_ReadBit(uint8_t *bit)
{
    if (bit == NULL)
    {
        return DHT11_ERROR_PARAM;
    }

    /* Wait until LOW period finishes */
    if (DHT11_WaitForState(
            GPIO_PIN_SET,
            120) != DHT11_OK)
    {
        return DHT11_ERROR_BIT_START;
    }

    /*
     * After 40 us:
     *
     * LOW  -> bit 0
     * HIGH -> bit 1
     */
    DHT11_DelayUs(40);

    if (DHT11_ReadPin() == GPIO_PIN_SET)
    {
        *bit = 1;

        /*
         * Wait for HIGH pulse to finish
         */
        if (DHT11_WaitForState(
                GPIO_PIN_RESET,
                120) != DHT11_OK)
        {
            return DHT11_ERROR_BIT_END;
        }
    }
    else
    {
        *bit = 0;
    }

    return DHT11_OK;
}


/* =========================================================
 * Read complete 40-bit DHT11 frame
 * ========================================================= */
DHT11_Status_t DHT11_Read(DHT11_Data_t *data)
{
    if ((data == NULL) ||
        (dht_timer == NULL))
    {
        return DHT11_ERROR_PARAM;
    }

    uint8_t bytes[5] = {0};

    DHT11_StartSignal();

    DHT11_Status_t status =
        DHT11_CheckResponse();

    if (status != DHT11_OK)
    {
        return status;
    }

    /*
     * Read 40 bits = 5 bytes
     */
    for (uint8_t i = 0; i < 40; i++)
    {
        uint8_t bit = 0;

        status = DHT11_ReadBit(&bit);

        if (status != DHT11_OK)
        {
            return status;
        }

        bytes[i / 8] <<= 1;

        bytes[i / 8] |= bit;
    }

    /*
     * DHT11 checksum:
     *
     * byte0 + byte1 + byte2 + byte3
     * lower 8 bits == byte4
     */
    uint8_t checksum =
        (uint8_t)(
            bytes[0] +
            bytes[1] +
            bytes[2] +
            bytes[3]
        );

    if (checksum != bytes[4])
    {
        return DHT11_ERROR_CHECKSUM;
    }

    data->humidity = bytes[0];
    data->temperature = bytes[2];
    data->checksum = bytes[4];

    return DHT11_OK;
}