#include "main.h"
#include "app_rtos.h"
#include "app_tasks.h"

extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef  htim2;
extern uint8_t uartRxByte;

/* ===== USART2 IRQ Handler ===== */
void USART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart2);
}

/* UART Rx Complete Callback (ISR context) */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        xQueueSendFromISR(xRxByteQueue,
                          &uartRxByte,
                          &xHigherPriorityTaskWoken);

        HAL_UART_Receive_IT(&huart2, &uartRxByte, 1);

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/* ===== EXTI IRQ Handler (e.g. User Button on PC13) ===== */
void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_13)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        vTaskNotifyGiveFromISR(xButtonTaskHandle, &xHigherPriorityTaskWoken);

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/* ===== TIM2 IRQ Handler – periodic interrupt ===== */
void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2);
}

/* Called by HAL on timer period event */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        /* Example: set bit 0 & 1 for BitNotify task */
        uint32_t bits = (1U << 0) | (1U << 1);

        xTaskNotifyFromISR(
            xBitNotifyTaskHandle,
            bits,
            eSetBits,
            &xHigherPriorityTaskWoken);

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
