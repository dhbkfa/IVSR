#ifndef MAIN_H
#define MAIN_H
#include <stdint.h>
typedef struct { int dummy; } TIM_HandleTypeDef;
typedef struct { int dummy; } UART_HandleTypeDef;
typedef struct { int dummy; } GPIO_TypeDef;
#define GPIOB ((GPIO_TypeDef *)0)
#define GPIOC ((GPIO_TypeDef *)0)
#define GPIO_PIN_12 (1U)
#define GPIO_PIN_13 (2U)
#define GPIO_PIN_14 (4U)
#endif
