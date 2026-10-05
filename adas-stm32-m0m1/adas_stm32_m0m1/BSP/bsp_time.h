/**
 * @file    bsp_time.h
 * @brief   Cycle counter (DWT) used to measure task execution time and jitter.
 */
#ifndef BSP_TIME_H
#define BSP_TIME_H

#include <stdint.h>

void     bsp_time_init(void);
uint32_t bsp_time_cycles(void);
uint32_t bsp_time_cycles_to_us(uint32_t cycles);

#endif /* BSP_TIME_H */
