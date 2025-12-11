# FreeRTOS Advanced Practice Project

This repository provides a fully structured FreeRTOS learning project for STM32.
It integrates real world concepts such as ISR-to-task communication, queues,
mutexes (including recursive mutex), event groups, task notifications, stack 
monitoring, and priority inversion.

The project follows a clean embedded firmware structure used in industry.

---

## 📁 Project Structure

Core/
Inc/
main.h
app_rtos.h // RTOS object declarations (queues, mutexes, event groups, timers, task handles)
app_tasks.h // Task function prototypes + helper utilities

Src/
main.c // HAL initialization + RTOS startup
app_rtos.c // Creation of queues/mutexes/timers/event groups + task creation
app_tasks.c // Implementation of all application tasks
isr_user.c // Interrupt handlers + FreeRTOS FromISR API examples

FreeRTOSConfig.h // RTOS configuration: stack checks, interrupt priorities, hooks, etc.



---

## 🧩 File Descriptions

### **Core/Inc/main.h**
- Basic definitions
- Peripheral handles (UART, Timer, GPIO)
- Exported function prototypes

### **Core/Inc/app_rtos.h**
Declares all RTOS objects used across the project:
- QueueHandle_t
- SemaphoreHandle_t / recursive mutex
- EventGroupHandle_t
- TimerHandle_t
- TaskHandle_t

### **Core/Inc/app_tasks.h**
Contains prototypes for:
- All task implementations
- Helper functions such as:
  - `Debug_Transmit()`
  - `ProcessCommand()`
  - Priority inversion demo functions
  - Recursive mutex demo functions

---

### **Core/Src/main.c**
- Initializes HAL, system clock, peripherals
- Calls `MX_FREERTOS_Init()` (inside `app_rtos.c`)
- Starts the scheduler (`vTaskStartScheduler()`)

---

### **Core/Src/app_rtos.c**
Creates all RTOS objects:

- Task creation  
- Queue creation  
- Binary & recursive mutex creation  
- Event groups  
- Software timers  
- Priority settings  

This file acts as the project's **RTOS factory**.

---

### **Core/Src/app_tasks.c**
Implements all tasks:

- UART RX Task (ISR → Queue → Task)
- UART TX Task (thread-safe output)
- Command Processing Task
- Sensor Logger (periodic)
- Priority Inversion demo
- Recursive Mutex demo
- Monitor Task (stack & heap reporting)

---

### **Core/Src/isr_user.c**
Interrupt handlers and FreeRTOS FromISR usage:

- UART RX interrupt → xQueueSendFromISR  
- Timer interrupts → EventGroups / Notify  
- Critical section handling in ISR  

---

### **FreeRTOSConfig.h**
Contains all FreeRTOS settings:

- Stack overflow detection  
- Interrupt priority limits  
- Timer task configuration  
- Memory allocation scheme  
- Idle/trace hooks  

---

## 🚀 Features Covered

- Queues, mutexes, event groups  
- Task notification (ISR & task)  
- Priority inversion & inheritance  
- Recursive mutex  
- Stack monitoring  
- Critical sections  
- ISR-safe FreeRTOS API  
- Real command parser architecture  

---

## 📚 Best For

This project helps you build the foundation required for:

- FreeRTOS kernel-level understanding  
- Transition to Zephyr RTOS  
- Writing real embedded firmware (BLE, sensors, networking)  
- Firmware architecture design (RTOS-based systems)

---

## 🛠 Build Environment

- STM32CubeIDE or Makefile ARM GCC  
- FreeRTOS 10.x  
- Any STM32 MCU (tested on F1/F4)

---

Enjoy learning FreeRTOS the way it is used in real products!
