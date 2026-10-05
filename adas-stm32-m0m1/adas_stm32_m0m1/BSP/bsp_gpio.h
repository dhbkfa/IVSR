/**
 * @file    bsp_gpio.h
 * @brief   LED, global bridge enable and per-wheel direction gates.
 *
 * Bridge model (BTS7960 + 74HC08):
 *   FWD   : ENR=1, ENL=0 -> PWM reaches RPWM only
 *   REV   : ENR=0, ENL=1 -> PWM reaches LPWM only
 *   BRAKE : ENR=0, ENL=0 -> both inputs low, both low-side MOSFETs on (EN high)
 *   COAST : global EN low -> both half bridges off (free wheeling)
 */
#ifndef BSP_GPIO_H
#define BSP_GPIO_H

#include <stdbool.h>
#include <stdint.h>
#include "status.h"

typedef enum
{
    BRIDGE_DIR_BRAKE = 0,
    BRIDGE_DIR_FWD,
    BRIDGE_DIR_REV
} bridge_dir_t;

void     bsp_gpio_init(void);                     /* safe state: EN low, BRAKE gates */
void     bsp_led_toggle(void);
void     bsp_bridge_enable(bool enable);
status_t bsp_bridge_set_dir(uint8_t wheel, bridge_dir_t dir);

#endif /* BSP_GPIO_H */
