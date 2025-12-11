#ifndef __APP_RTOS_H
#define __APP_RTOS_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
#include "event_groups.h"

/* ---- Task Handles ---- */
extern TaskHandle_t xRxTaskHandle;
extern TaskHandle_t xTxTaskHandle;
extern TaskHandle_t xProcessTaskHandle;
extern TaskHandle_t xSensorLoggerTaskHandle;
extern TaskHandle_t xMonitorTaskHandle;

/* ---- Queues ---- */
extern QueueHandle_t xRxQueue;
extern QueueHandle_t xTxQueue;
extern QueueHandle_t xCmdQueue;

/* ---- Mutexes ---- */
extern SemaphoreHandle_t xMutex;
extern SemaphoreHandle_t xRecursiveMutex;

/* ---- Event Groups ---- */
extern EventGroupHandle_t xSystemEventGroup;

/* ---- Timers ---- */
extern TimerHandle_t xSensorTimer;
extern TimerHandle_t xHeartbeatTimer;

void MX_FREERTOS_Init(void);

#endif
