#ifndef __APP_TASKS_H
#define __APP_TASKS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_rtos.h"

/* ===== Task functions ===== */
void Task_UART_RX(void *pvParameters);
void Task_UART_TX(void *pvParameters);
void Task_CommandProcessor(void *pvParameters);
void Task_SensorLogger(void *pvParameters);
void Task_Monitor(void *pvParameters);
void Task_LowPriority(void *pvParameters);
void Task_MediumPriority(void *pvParameters);
void Task_HighPriority(void *pvParameters);
void Task_RecursiveUser(void *pvParameters);
void Task_Button(void *pvParameters);
void Task_BitNotify(void *pvParameters);

/* ===== Timer callback functions ===== */
void vLedTimerCallback(TimerHandle_t xTimer);
void vHeartbeatTimerCallback(TimerHandle_t xTimer);

/* ===== Helper functions ===== */
void Debug_Transmit(const char *msg);
void ProcessCommand(const char *cmd);

/* Recursive mutex demo helpers */
void Recursive_Level1(void);
void Recursive_Level2(void);

/* Critical section demo */
void CriticalNested_Level1(void);
void CriticalNested_Level2(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_TASKS_H */
