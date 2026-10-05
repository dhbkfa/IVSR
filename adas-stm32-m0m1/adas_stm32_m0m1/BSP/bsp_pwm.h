/**
 * @file    bsp_pwm.h
 * @brief   PWM duty output. Duty is given in permille (0..1000) and converted
 *          with the ARR actually programmed in the timer, so the PWM period can
 *          be changed in CubeMX without touching this code.
 */
#ifndef BSP_PWM_H
#define BSP_PWM_H

#include <stdint.h>
#include "status.h"

status_t bsp_pwm_start(uint8_t wheel);
status_t bsp_pwm_set_permille(uint8_t wheel, uint16_t permille);

#endif /* BSP_PWM_H */
