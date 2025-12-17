#include "main.h"
#include "app_rtos.h"
#include "app_tasks.h"
#include <stdio.h>

/* ===== RTOS Handles Definitions ===== */

/* Tasks */
TaskHandle_t xRxTaskHandle          = NULL;
TaskHandle_t xTxTaskHandle          = NULL;
TaskHandle_t xProcessTaskHandle     = NULL;
TaskHandle_t xSensorLoggerTaskHandle= NULL;
TaskHandle_t xMonitorTaskHandle     = NULL;
TaskHandle_t xLowTaskHandle         = NULL;
TaskHandle_t xMediumTaskHandle      = NULL;
TaskHandle_t xHighTaskHandle        = NULL;
TaskHandle_t xRecursiveTaskHandle   = NULL;
TaskHandle_t xButtonTaskHandle      = NULL;
TaskHandle_t xBitNotifyTaskHandle   = NULL;

/* Queues */
QueueHandle_t xRxByteQueue          = NULL;
QueueHandle_t xTxMsgQueue           = NULL;
QueueHandle_t xCmdMsgQueue          = NULL;

/* Mutexes */
SemaphoreHandle_t xSharedBufferMutex= NULL;
SemaphoreHandle_t xRecursiveMutex   = NULL;

/* Event Group */
EventGroupHandle_t xSystemEventGroup= NULL;

/* Software Timers */
TimerHandle_t xLedTimerHandle       = NULL;
TimerHandle_t xHeartbeatTimerHandle = NULL;

/* Shared data */
uint8_t  Shared_Buffer[SHARED_BUFFER_SIZE];
uint32_t Sensor_Reading_Count       = 0;
uint32_t CriticalCounter            = 0;
volatile uint32_t system_uptime     = 0;
volatile uint32_t error_count       = 0;
volatile uint32_t notify_bit_state  = 0;

/* UART RX byte buffer (used by HAL + ISR) */
uint8_t uartRxByte;

/* External HAL handles */
extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef  htim2; //extern why? 이 파일에는 메모리를 할당하지 말고, 다른 곳에 있는 htim2를 참조

/* ===== FreeRTOS Objects & Tasks Initialization ===== */
void MX_FREERTOS_Init(void)
{
    /* --- Mutexes --- */
    xSharedBufferMutex = xSemaphoreCreateMutex();
    xRecursiveMutex    = xSemaphoreCreateRecursiveMutex();

    /* --- Queues --- */
    xRxByteQueue  = xQueueCreate(32, sizeof(uint8_t));
    xTxMsgQueue   = xQueueCreate(16, sizeof(TxMessage_t));
    xCmdMsgQueue  = xQueueCreate(8,  sizeof(CmdMessage_t));

    /* --- Event Group --- */
    xSystemEventGroup = xEventGroupCreate();

    /* --- Software Timers --- */
    xLedTimerHandle = xTimerCreate(
        "LED",
        pdMS_TO_TICKS(500),
        pdTRUE,
        NULL,
        vLedTimerCallback);

    xHeartbeatTimerHandle = xTimerCreate(
        "HB",
        pdMS_TO_TICKS(1000),
        pdTRUE,
        NULL,
        vHeartbeatTimerCallback);

    xTimerStart(xLedTimerHandle, 0);
    xTimerStart(xHeartbeatTimerHandle, 0);

    /* --- Tasks --- */

    xTaskCreate(Task_UART_RX,
                "RxTask",
                256,
                NULL,
                tskIDLE_PRIORITY + 4,
                &xRxTaskHandle);

    xTaskCreate(Task_UART_TX,
                "TxTask",
                256,
                NULL,
                tskIDLE_PRIORITY + 3,
                &xTxTaskHandle);

    xTaskCreate(Task_CommandProcessor,
                "CmdTask",
                512,
                NULL,
                tskIDLE_PRIORITY + 3,
                &xProcessTaskHandle);

    xTaskCreate(Task_SensorLogger,
                "SensorLog",
                256,
                NULL,
                tskIDLE_PRIORITY + 2,
                &xSensorLoggerTaskHandle);

    xTaskCreate(Task_Monitor,
                "Monitor",
                512,
                NULL,
                tskIDLE_PRIORITY + 1,
                &xMonitorTaskHandle);

    /* Priority inversion demo tasks */
    xTaskCreate(Task_LowPriority,
                "Low",
                256,
                NULL,
                tskIDLE_PRIORITY + 1,
                &xLowTaskHandle);

    xTaskCreate(Task_MediumPriority,
                "Medium",
                256,
                NULL,
                tskIDLE_PRIORITY + 2,
                &xMediumTaskHandle);

    xTaskCreate(Task_HighPriority,
                "High",
                256,
                NULL,
                tskIDLE_PRIORITY + 4,
                &xHighTaskHandle);

    /* Recursive Mutex demo task */
    xTaskCreate(Task_RecursiveUser,
                "RecUser",
                256,
                NULL,
                tskIDLE_PRIORITY + 2,
                &xRecursiveTaskHandle);

    /* Button notify task */
    xTaskCreate(Task_Button,
                "Button",
                256,
                NULL,
                tskIDLE_PRIORITY + 2,
                &xButtonTaskHandle);

    /* Task Notification bitwise demo task */
    xTaskCreate(Task_BitNotify,
                "BitNotify",
                256,
                NULL,
                tskIDLE_PRIORITY + 2,
                &xBitNotifyTaskHandle);

    /* Start UART RX interrupt (first byte) */
    HAL_UART_Receive_IT(&huart2, &uartRxByte, 1);

    /* Start TIM2 base interrupt for periodic uptime tick */
    HAL_TIM_Base_Start_IT(&htim2);
}

/* ===== Idle Hook: Periodic Diagnostics ===== */
void vApplicationIdleHook(void)
{
    static TickType_t last = 0;
    TickType_t now = xTaskGetTickCount();

    if ((now - last) > pdMS_TO_TICKS(10000))
    {
        last = now;
        char buf[64];
        snprintf(buf, sizeof(buf),
                 "[Idle] FreeHeap=%u MinEver=%u\r\n",
                 (unsigned int)xPortGetFreeHeapSize(),
                 (unsigned int)xPortGetMinimumEverFreeHeapSize());
        Debug_Transmit(buf);
    }
}

/* ===== Stack Overflow Hook ===== */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    Debug_Transmit("[StackOverflow] in task: ");
    Debug_Transmit(pcTaskName);
    Debug_Transmit("\r\n");

    taskDISABLE_INTERRUPTS();
    for (;;)
    {
    }
}

/* ===== Malloc Failed Hook ===== */
void vApplicationMallocFailedHook(void)
{
    Debug_Transmit("[MallocFailed] Out of heap memory!\r\n");
    taskDISABLE_INTERRUPTS();
    for (;;)
    {
    }
}
