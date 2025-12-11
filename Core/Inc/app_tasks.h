#ifndef __APP_TASKS_H
#define __APP_TASKS_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

void Task_UART_RX(void *argument);
void Task_UART_TX(void *argument);
void Task_CommandProcessor(void *argument);
void Task_SensorLogger(void *argument);
void Task_Monitor(void *argument);

/* Demo helpers */
void Debug_Transmit(const char *msg);
void ProcessCommand(const char *cmd);

void Recursive_Test_Level1(void);
void Recursive_Test_Level2(void);

#endif
