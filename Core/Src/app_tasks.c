#include "main.h"
#include "app_rtos.h"
#include "app_tasks.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern UART_HandleTypeDef huart2;
extern uint8_t uartRxByte;

/* ===== Helper: Centralized TX via Queue ===== */
void Debug_Transmit(const char *msg)
{
    TxMessage_t tx;
    size_t len = strlen(msg);
    if (len >= CMD_MAX_LEN) len = CMD_MAX_LEN - 1;

    memcpy(tx.text, msg, len);
    tx.text[len] = '\0';

    xQueueSend(xTxMsgQueue, &tx, portMAX_DELAY);
}

/* ===== Task: UART RX – ISR → Queue → Command strings ===== */
void Task_UART_RX(void *pvParameters)
{
    (void)pvParameters;
    uint8_t byte;
    char cmd_buf[CMD_MAX_LEN];
    size_t idx = 0;

    Debug_Transmit("RxTask started\r\n");

    for (;;)
    {
        if (xQueueReceive(xRxByteQueue, &byte, portMAX_DELAY) == pdPASS)
        {
            if (byte == '\r' || byte == '\n')
            {
                if (idx > 0)
                {
                    cmd_buf[idx] = '\0';

                    CmdMessage_t cmdMsg;
                    strncpy(cmdMsg.cmd, cmd_buf, CMD_MAX_LEN);
                    cmdMsg.cmd[CMD_MAX_LEN - 1] = '\0';

                    xQueueSend(xCmdMsgQueue, &cmdMsg, portMAX_DELAY);
                    idx = 0;
                }
            }
            else
            {
                if (idx < CMD_MAX_LEN - 1)
                {
                    cmd_buf[idx++] = (char)byte;
                }
            }
        }
    }
}

/* ===== Task: UART TX – Single point UART output ===== */
void Task_UART_TX(void *pvParameters)
{
    (void)pvParameters;
    TxMessage_t msg;

    Debug_Transmit("TxTask started\r\n");

    for (;;)
    {
        if (xQueueReceive(xTxMsgQueue, &msg, portMAX_DELAY) == pdPASS)
        {
            HAL_UART_Transmit(&huart2,
                              (uint8_t *)msg.text,
                              strlen(msg.text),
                              100);
        }
    }
}

/* ===== Task: Command Processor ===== */
void Task_CommandProcessor(void *pvParameters)
{
    (void)pvParameters;
    CmdMessage_t cmdMsg;

    Debug_Transmit("CmdTask started\r\n");

    for (;;)
    {
        if (xQueueReceive(xCmdMsgQueue, &cmdMsg, portMAX_DELAY) == pdPASS)
        {
            ProcessCommand(cmdMsg.cmd);

            /* Notify SensorLogger that a command was processed */
            xTaskNotifyGive(xSensorLoggerTaskHandle);
        }
    }
}

/* ===== ProcessCommand: handles STATS / READ / BITS ===== */
void ProcessCommand(const char *cmd)
{
    char response[CMD_MAX_LEN];

    if (strncmp(cmd, "STATS", 5) == 0)
    {
        uint32_t up, err;
        size_t free_heap, min_heap;
    // 단일 작업(Atomic Operation)
        taskENTER_CRITICAL();
        up  = system_uptime;
        err = error_count;
        taskEXIT_CRITICAL();

        free_heap = xPortGetFreeHeapSize();
        min_heap  = xPortGetMinimumEverFreeHeapSize();

        snprintf(response, sizeof(response),
                 "Uptime=%lu, Errors=%lu\r\nHeap: free=%u, minEver=%u\r\n",
                 (unsigned long)up,
                 (unsigned long)err,
                 (unsigned int)free_heap,
                 (unsigned int)min_heap);
        Debug_Transmit(response);
    }
    else if (strncmp(cmd, "READ", 4) == 0)
    {
        snprintf(response, sizeof(response),
                 "READ: I2C/SPI config OK (dummy)\r\n");
        Debug_Transmit(response);
    }
    else if (strncmp(cmd, "BITS", 4) == 0)
    {
        uint32_t bits = notify_bit_state;
        snprintf(response, sizeof(response),
                 "Notify bits: 0x%08lX\r\n",
                 (unsigned long)bits);
        Debug_Transmit(response);
    }
    else
    {
        snprintf(response, sizeof(response),
                 "ERROR: Unknown command: %s\r\n", cmd);
        Debug_Transmit(response);
    }
}

/* ===== Task: Sensor Logger – periodic + mutex + event + notify ===== */
void Task_SensorLogger(void *pvParameters)
{
    (void)pvParameters;
    char log_buf[CMD_MAX_LEN];
    uint16_t temp_val = 0;

    Debug_Transmit("SensorLogger started\r\n");

    for (;;)
    {
        /* Fake sensor read */
        temp_val = (uint16_t)(rand() % 40 + 20);
        Sensor_Reading_Count++;

        if (xSemaphoreTake(xSharedBufferMutex, portMAX_DELAY) == pdPASS)
        {
            snprintf((char *)Shared_Buffer, SHARED_BUFFER_SIZE,
                     "Latest Temp=%u, Count=%lu",
                     temp_val,
                     (unsigned long)Sensor_Reading_Count);
            xSemaphoreGive(xSharedBufferMutex);
        }

        xEventGroupSetBits(xSystemEventGroup, EVTBIT_SENSOR_READY);

        snprintf(log_buf, sizeof(log_buf),
                 "SENSOR: Log #%lu, Temp=%uC\r\n",
                 (unsigned long)Sensor_Reading_Count, temp_val);
        Debug_Transmit(log_buf);

        /* Wait (up to 100ms) for command-processed notification */
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100));

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

/* ===== Task: Monitor – EventGroup + Stack & Heap monitoring ===== */
void Task_Monitor(void *pvParameters)
{
    (void)pvParameters;
    char buf[CMD_MAX_LEN];

    Debug_Transmit("MonitorTask started\r\n");

    for (;;)
    {
        EventBits_t bits = xEventGroupWaitBits(
            xSystemEventGroup,
            EVTBIT_SENSOR_READY | EVTBIT_UART_CONNECTED,
            pdTRUE,
            pdTRUE,
            pdMS_TO_TICKS(10000));

        if ((bits & (EVTBIT_SENSOR_READY | EVTBIT_UART_CONNECTED)) ==
            (EVTBIT_SENSOR_READY | EVTBIT_UART_CONNECTED))
        {
            snprintf(buf, sizeof(buf),
                     "[Monitor] Sensor & UART OK\r\n");
            Debug_Transmit(buf);
        }

        /* Stack HWM of key tasks */
        UBaseType_t rx_hwm  = uxTaskGetStackHighWaterMark(xRxTaskHandle);
        UBaseType_t tx_hwm  = uxTaskGetStackHighWaterMark(xTxTaskHandle);
        UBaseType_t sen_hwm = uxTaskGetStackHighWaterMark(xSensorLoggerTaskHandle);

        snprintf(buf, sizeof(buf),
                 "[StackHWM] RX=%u TX=%u SEN=%u words\r\n",
                 (unsigned int)rx_hwm,
                 (unsigned int)tx_hwm,
                 (unsigned int)sen_hwm);
        Debug_Transmit(buf);

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

/* ===== Priority Inversion Demo Tasks ===== */
void Task_LowPriority(void *pvParameters)
{
    (void)pvParameters;
    char buf[64];

    for (;;)
    {
        if (xSemaphoreTake(xSharedBufferMutex, portMAX_DELAY) == pdPASS)
        {
            snprintf(buf, sizeof(buf),
                     "[Low] Got mutex, doing long work...\r\n");
            Debug_Transmit(buf);

            for (volatile int i = 0; i < 500000; i++)
            {
                CriticalCounter++;
            }

            snprintf(buf, sizeof(buf),
                     "[Low] Releasing mutex\r\n");
            Debug_Transmit(buf);

            xSemaphoreGive(xSharedBufferMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void Task_MediumPriority(void *pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        Debug_Transmit("[Medium] Running CPU load...\r\n");
        for (volatile int i = 0; i < 200000; i++)
        {
            __NOP();
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void Task_HighPriority(void *pvParameters)
{
    (void)pvParameters;
    char buf[64];

    vTaskDelay(pdMS_TO_TICKS(1000)); /* Let Low get the mutex first */

    for (;;)
    {
        snprintf(buf, sizeof(buf),
                 "[High] Trying to lock mutex...\r\n");
        Debug_Transmit(buf);

        if (xSemaphoreTake(xSharedBufferMutex, portMAX_DELAY) == pdPASS)
        {
            snprintf(buf, sizeof(buf),
                     "[High] Got mutex, CriticalCounter=%lu\r\n",
                     (unsigned long)CriticalCounter);
            Debug_Transmit(buf);

            xSemaphoreGive(xSharedBufferMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

/* ===== Recursive Mutex Demo ===== */
void Recursive_Level2(void)
{
    if (xSemaphoreTakeRecursive(xRecursiveMutex, pdMS_TO_TICKS(100)) == pdPASS)
    {
        Debug_Transmit("[RecUser] Level2 acquired recursive mutex\r\n");
        xSemaphoreGiveRecursive(xRecursiveMutex);
    }
}

void Recursive_Level1(void)
{
    if (xSemaphoreTakeRecursive(xRecursiveMutex, pdMS_TO_TICKS(100)) == pdPASS)
    {
        Debug_Transmit("[RecUser] Level1 acquired recursive mutex\r\n");
        Recursive_Level2();
        xSemaphoreGiveRecursive(xRecursiveMutex);
    }
}

void Task_RecursiveUser(void *pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        Recursive_Level1();
        vTaskDelay(pdMS_TO_TICKS(4000));
    }
}

/* ===== Critical Section Demo ===== */
void CriticalNested_Level2(void)
{
    taskENTER_CRITICAL();
    CriticalCounter++;
    taskEXIT_CRITICAL();
}

void CriticalNested_Level1(void)
{
    taskENTER_CRITICAL();
    CriticalCounter++;
    CriticalNested_Level2();
    taskEXIT_CRITICAL();
}

/* ===== Task: Button – wakes via NotifyFromISR ===== */
void Task_Button(void *pvParameters)
{
    (void)pvParameters;
    char buf[64];

    Debug_Transmit("ButtonTask started\r\n");

    for (;;)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        snprintf(buf, sizeof(buf),
                 "[ButtonTask] Button pressed!\r\n");
        Debug_Transmit(buf);
    }
}

/* ===== Task: BitNotify – demonstrates xTaskNotify bits/eSetBits ===== */
void Task_BitNotify(void *pvParameters)
{
    (void)pvParameters;
    uint32_t notifyValue;

    Debug_Transmit("BitNotifyTask started\r\n");

    for (;;)
    {
        /* Wait for any bits to be set by ISR (e.g., from TIM2) */
        xTaskNotifyWait(0x00, 0xFFFFFFFF, &notifyValue, portMAX_DELAY);

        notify_bit_state = notifyValue;

        char buf[64];
        snprintf(buf, sizeof(buf),
                 "[BitNotify] Received bits: 0x%08lX\r\n",
                 (unsigned long)notifyValue);
        Debug_Transmit(buf);
    }
}

/* ===== Software Timer Callbacks ===== */
void vLedTimerCallback(TimerHandle_t xTimer)
{
    (void)xTimer;
    /* Example LED pin: PA5 (adjust as needed) */
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
}

void vHeartbeatTimerCallback(TimerHandle_t xTimer)
{
    (void)xTimer;
    /* We count uptime here (1s tick) */
    system_uptime++;
}
