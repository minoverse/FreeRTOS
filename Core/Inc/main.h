
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/* Global HAL handles (defined in main.c) */
extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef  htim2;

/* System init functions (must be implemented in main.c or CubeMX generated files) */
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_USART2_UART_Init(void);
void MX_TIM2_Init(void);

/* Error handler */
void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
