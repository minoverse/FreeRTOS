#include "app_rtos.h"
#include "app_tasks.h"

/* Task handles */
TaskHandle_t xRxTaskHandle;
TaskHandle_t xTxTaskHandle;
TaskHandle_t xProcessTaskHandle;
TaskHandle_t xSensorLoggerTaskHandle;
TaskHandle_t xMonitorTaskHandle;

/* RTOS objects */
QueueHandle_t xRxQueue;
QueueHandle_t xTxQueue;
QueueHandle_t xCmdQueue;

SemaphoreHandle_t xMutex;
SemaphoreHandle_t xRecursiveMutex;

EventGroupHandle_t xSystemEventGroup;

TimerHandle_t xSensorTimer;
TimerHandle_t xHeartbeatTimer;

void MX_FREERTOS_Init(void)
{
    /* Queues */
    xRxQueue  = xQueueCreate(32, sizeof(uint8_t));
    xTxQueue  = xQueueCreate(16, sizeof(char[64]));
    xCmdQueue = xQueueCreate(8,  sizeof(char[64]));

    /* Mutexes */
    xMutex = xSemaphoreCreateMutex();
    xRecursiveMutex = xSemaphoreCreateRecursiveMutex();

    /* Event Group */
    xSystemEventGroup = xEventGroupCreate();

    /* Tasks */
    xTaskCreate(Task_UART_RX, "RX", 256, NULL, 4, &xRxTaskHandle);
    xTaskCreate(Task_UART_TX, "TX", 256, NULL, 3, &xTxTaskHandle);
    xTaskCreate(Task_CommandProcessor, "PROC", 512, NULL, 3, &xProcessTaskHandle);
    xTaskCreate(Task_SensorLogger, "SENSOR", 256, NULL, 2, &xSensorLoggerTaskHandle);
    xTaskCreate(Task_Monitor, "MON", 256, NULL, 1, &xMonitorTaskHandle);
}
