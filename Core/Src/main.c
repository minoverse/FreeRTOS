#include "main.h"
#include "app_rtos.h"

UART_HandleTypeDef huart2;

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    MX_FREERTOS_Init();

    vTaskStartScheduler();

    while (1) {}
}
