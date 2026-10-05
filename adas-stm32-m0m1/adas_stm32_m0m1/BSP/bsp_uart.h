/**
 * @file    bsp_uart.h
 * @brief   Console UART: byte-wise RX interrupt + blocking TX.
 *          The RX callback runs in ISR context: keep it tiny and use only
 *          *FromISR RTOS calls inside it.
 */
#ifndef BSP_UART_H
#define BSP_UART_H

#include <stdint.h>
#include "status.h"

typedef void (*bsp_uart_rx_cb_t)(uint8_t byte);

status_t bsp_uart_start(bsp_uart_rx_cb_t cb);
status_t bsp_uart_write(const uint8_t *data, uint16_t len);

#endif /* BSP_UART_H */
