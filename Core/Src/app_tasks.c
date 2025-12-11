#include "app_tasks.h"
#include "app_rtos.h"
#include "main.h"
#include <string.h>
#include <stdio.h>

void Debug_Transmit(const char *msg)
{
    char buf[64];
    strncpy(buf, msg, 63);
    xQueueSend(xTxQueue, &buf, portMAX_DELAY);
}

void Task_UART_RX(void *argument)
{
    uint8_t byte;
    char cmd[64];
    int idx = 0;

    HAL_UART_Receive_IT(&huart2, &byte, 1);

    for (;;)
    {
        if (xQueueReceive(xRxQueue, &byte, portMAX_DELAY) == pdPASS)
        {
            if (byte == '\n')
            {
                cmd[idx] = '\0';
                xQueueSend(xCmdQueue, &cmd, portMAX_DELAY);
                idx = 0;
            }
            else if (idx < 63)
                cmd[idx++] = byte;

            HAL_UART_Receive_IT(&huart2, &byte, 1);
        }
    }
}

void Task_UART_TX(void *argument)
{
    char msg[64];

    for (;;)
    {
        if (xQueueReceive(xTxQueue, &msg, portMAX_DELAY) == pdPASS)
        {
            HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        }
    }
}

void Task_CommandProcessor(void *argument)
{
    char cmd[64];

    for (;;)
    {
        if (xQueueReceive(xCmdQueue, &cmd, portMAX_DELAY) == pdPASS)
        {
            ProcessCommand(cmd);
        }
    }
}

void ProcessCommand(const char *cmd)
{
    if (strcmp(cmd, "PING") == 0)
        Debug_Transmit("PONG\n");
}

void Task_SensorLogger(void *argument)
{
    for (;;)
    {
        Debug_Transmit("Sensor tick\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void Task_Monitor(void *argument)
{
    for (;;)
    {
        UBaseType_t stack = uxTaskGetStackHighWaterMark(NULL);
        char buf[64];
        sprintf(buf, "Stack HWM: %lu\n", stack);
        Debug_Transmit(buf);

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
