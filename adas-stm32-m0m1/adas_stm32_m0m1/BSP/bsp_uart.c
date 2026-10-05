#include "bsp_uart.h"
#include "board_config.h"

#define UART_TX_TIMEOUT_MS  (50U)

static uint8_t          s_rx_byte;
static bsp_uart_rx_cb_t s_rx_cb;

status_t bsp_uart_start(bsp_uart_rx_cb_t cb)
{
    status_t st = ST_OK;

    if (cb == NULL)
    {
        st = ST_ERR_PARAM;
    }
    else
    {
        s_rx_cb = cb;
        if (HAL_UART_Receive_IT(&huart2, &s_rx_byte, 1U) != HAL_OK)
        {
            st = ST_ERR_HW;
        }
    }
    return st;
}

status_t bsp_uart_write(const uint8_t *data, uint16_t len)
{
    status_t st = ST_OK;

    if (data == NULL)
    {
        st = ST_ERR_PARAM;
    }
    else
    {
        /* MISRA deviation D-001: HAL prototype takes a non-const pointer. */
        uint8_t *p = (uint8_t *)data;
        if (HAL_UART_Transmit(&huart2, p, len, UART_TX_TIMEOUT_MS) != HAL_OK)
        {
            st = ST_ERR_HW;
        }
    }
    return st;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == huart2.Instance)
    {
        if (s_rx_cb != NULL)
        {
            s_rx_cb(s_rx_byte);
        }
        (void)HAL_UART_Receive_IT(huart, &s_rx_byte, 1U);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    /* Overrun/noise/framing: clear and keep listening. */
    if (huart->Instance == huart2.Instance)
    {
        (void)HAL_UART_Receive_IT(huart, &s_rx_byte, 1U);
    }
}
