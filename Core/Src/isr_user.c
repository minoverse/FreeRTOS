#include "main.h"
#include "app_rtos.h"

extern uint8_t uart_rx_byte;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        BaseType_t xWoken = pdFALSE;
        xQueueSendFromISR(xRxQueue, &uart_rx_byte, &xWoken);
        HAL_UART_Receive_IT(&huart2, &uart_rx_byte, 1);
        portYIELD_FROM_ISR(xWoken);
    }
}
