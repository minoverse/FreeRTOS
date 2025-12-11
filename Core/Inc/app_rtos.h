#ifndef __APP_RTOS_H
#define __APP_RTOS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
#include "event_groups.h"

/* ====== General constants ====== */
#define CMD_MAX_LEN          64
#define SHARED_BUFFER_SIZE   128

/* ====== Event Group bits ====== */
#define EVTBIT_SENSOR_READY       (1U << 0)
#define EVTBIT_UART_CONNECTED     (1U << 1)
#define EVTBIT_LOG_FLUSHED        (1U << 2)

/* ====== Data types ====== */
typedef struct
{
    char text[CMD_MAX_LEN];
} TxMessage_t;

typedef struct
{
    char cmd[CMD_MAX_LEN];
} CmdMessage_t;

/* ====== Task handles ====== */
extern TaskHandle_t xRxTaskHandle;
extern TaskHandle_t xTxTaskHandle;
extern TaskHandle_t xProcessTaskHandle;
extern TaskHandle_t xSensorLoggerTaskHandle;
extern TaskHandle_t xMonitorTaskHandle;
extern TaskHandle_t xLowTaskHandle;
extern TaskHandle_t xMediumTaskHandle;
extern TaskHandle_t xHighTaskHandle;
extern TaskHandle_t xRecursiveTaskHandle;
extern TaskHandle_t xButtonTaskHandle;
extern TaskHandle_t xBitNotifyTaskHandle;

/* ====== Queues ====== */
extern QueueHandle_t xRxByteQueue;   // ISR -> RX Task (1 byte)
extern QueueHandle_t xTxMsgQueue;    // Any Task -> TX Task
extern QueueHandle_t xCmdMsgQueue;   // RX Task -> Processor Task

/* ====== Mutexes ====== */
extern SemaphoreHandle_t xSharedBufferMutex;    // normal mutex
extern SemaphoreHandle_t xRecursiveMutex;       // recursive mutex

/* ====== Event Group ====== */
extern EventGroupHandle_t xSystemEventGroup;

/* ====== Software Timers ====== */
extern TimerHandle_t xLedTimerHandle;
extern TimerHandle_t xHeartbeatTimerHandle;

/* ====== Shared data ====== */
extern uint8_t  Shared_Buffer[SHARED_BUFFER_SIZE];
extern uint32_t Sensor_Reading_Count;
extern uint32_t CriticalCounter;
extern volatile uint32_t system_uptime;
extern volatile uint32_t error_count;
extern volatile uint32_t notify_bit_state;

/* Initialize RTOS objects and tasks */
void MX_FREERTOS_Init(void);

/* Hooks (mapped in FreeRTOSConfig.h) */
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);
void vApplicationMallocFailedHook(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_RTOS_H */
