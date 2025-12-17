tasks.c 분석 (Scheduler 로직 핵심)

경로:

FreeRTOS/Source/tasks.c


이 파일의 정체:

“누가 언제 CPU를 가질지” 결정하는 순수 논리

2-1. pxCurrentTCB
TCB_t * volatile pxCurrentTCB = NULL;


현재 실행 중인 태스크

PendSV에서 이거 바꾼다

2-2. Ready List 구조
List_t pxReadyTasksLists[ configMAX_PRIORITIES ];


priority별 ready list

우선순위 = 인덱스

2-3. vTaskSwitchContext()
void vTaskSwitchContext( void )
{
    if( uxSchedulerSuspended != ( UBaseType_t ) pdFALSE )
    {
        xYieldPending = pdTRUE;
    }
    else
    {
        taskSELECT_HIGHEST_PRIORITY_TASK();
    }
}

핵심

스케줄러 suspend 상태면 → 나중에

아니면 → 즉시 최고 우선순위 task 선택

2-4. taskSELECT_HIGHEST_PRIORITY_TASK()
#define taskSELECT_HIGHEST_PRIORITY_TASK()        \
{                                                 \
    uxTopPriority = uxTopReadyPriority;           \
    pxCurrentTCB = listGET_OWNER_OF_NEXT_ENTRY(   \
                        &( pxReadyTasksLists[ uxTopPriority ] ) ); \
}

의미

priority 높은 것부터

같은 priority면 round-robin

2-5. xTaskIncrementTick() (🔥 tick의 핵심)
BaseType_t xTaskIncrementTick( void )
{
    ++xTickCount;

delay task 처리
if( xTickCount >= xNextTaskUnblockTime )
{
    prvCheckDelayedTasks();
}


Delay list 확인

unblock time 도달 → Ready List 이동

2-6. vTaskDelay()
void vTaskDelay( TickType_t xTicksToDelay )
{
    prvAddCurrentTaskToDelayedList( xTicksToDelay, pdFALSE );
}


➡️ 현재 태스크를 Ready List에서 제거
➡️ Delayed List로 이동

2-7. Stack High Water Mark
UBaseType_t uxTaskGetStackHighWaterMark( TaskHandle_t xTask )


스택에 남아 있는 최소 여유 공간 측정

스택 오버플로우 사전 탐지

3️⃣ 전체 흐름 요약 (진짜 중요)
SysTick IRQ
 └─ xPortSysTickHandler
     └─ xTaskIncrementTick
         └─ Ready Task 발생?
             └─ PendSV Set
                 └─ xPortPendSVHandler
                     ├─ Save old context
                     ├─ vTaskSwitchContext (tasks.c)
                     └─ Restore new context


👉 이 구조는 Zephyr / Linux RT도 동일
