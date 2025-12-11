#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f4xx_hal.h"

/* Exported HAL handles */
extern UART_HandleTypeDef huart2;

/* System init functions */
void Error_Handler(void);

#endif
