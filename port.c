port.c 분석 (Context Switch & Interrupt 핵심)

경로:

FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c


이 파일의 정체 한 줄 요약:

CPU 레벨에서 “태스크를 갈아끼우는 법”을 구현한 파일

1-1. 전역 핵심 변수
volatile uint32_t ulCriticalNesting = 9999UL;

의미

Critical Section 중첩 깊이

0이 되기 전까지는 인터럽트 재허용 ❌

왜 필요?
taskENTER_CRITICAL();
  taskENTER_CRITICAL();
    ...
  taskEXIT_CRITICAL();
taskEXIT_CRITICAL();


→ 중첩 지원 없으면 한 번 EXIT 시 인터럽트 풀려버림 (버그)

1-2. xPortStartScheduler()
BaseType_t xPortStartScheduler( void )
{
    /* FPU 사용 여부 설정 */
    prvSetupFPU();

    /* PendSV, SysTick 우선순위 설정 */
    portNVIC_SHPR3_REG |= portNVIC_PENDSV_PRI;
    portNVIC_SHPR3_REG |= portNVIC_SYSTICK_PRI;

    /* 첫 태스크 시작 */
    vPortStartFirstTask();

    return 0;
}

핵심 포인트
🔹 PendSV / SysTick priority를 가장 낮게 설정

왜?

PendSV = context switch 전용 인터럽트

다른 ISR 다 끝난 뒤에만 실행돼야 함

👉 이게 없으면:

ISR 도중에 context switch 발생

레지스터 상태 깨짐

RTOS 즉사

1-3. vPortStartFirstTask()
ldr r0, =0xE000ED08   // VTOR
ldr r0, [r0]
ldr r0, [r0]
msr msp, r0

이 코드의 의미

MSP(Main Stack Pointer)를

Reset Vector의 초기 값으로 되돌림

왜?

지금까지는 main() 실행하면서
MSP를 써왔는데
RTOS 시작 시점부터는 MSP = ISR 전용
Task는 PSP(Process Stack Pointer)

👉 이 줄이 없으면:

첫 태스크 스택부터 이미 꼬여 있음

1-4. xPortPendSVHandler() (🔥 가장 중요)
void xPortPendSVHandler( void )
{
    __asm volatile
    (
        " mrs r0, psp                \n"
        " isb                        \n"

① 현재 태스크 컨텍스트 저장
stmdb r0!, {r4-r11}


r0~r3, r12, lr, pc, xPSR
→ 자동 저장 (하드웨어)

r4~r11
→ 수동 저장 (여기서!)

👉 그래서 PendSV에서만 context switch 가능

② 현재 TCB에 스택 포인터 저장
ldr r1, =pxCurrentTCB
ldr r1, [r1]
str r0, [r1]


“이 태스크는 여기까지 실행했다” 기록

③ 다음 태스크 선택
bl vTaskSwitchContext


➡️ tasks.c로 넘어감
➡️ Ready List 기반으로 다음 태스크 결정

④ 다음 태스크 컨텍스트 복원
ldr r1, =pxCurrentTCB
ldr r1, [r1]
ldr r0, [r1]

ldmia r0!, {r4-r11}
msr psp, r0


➡️ 완전히 다른 태스크의 CPU 상태로 복귀

1-5. xPortSysTickHandler()
void xPortSysTickHandler( void )
{
    vPortRaiseBASEPRI();
    {
        if( xTaskIncrementTick() != pdFALSE )
        {
            portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET;
        }
    }
    vPortClearBASEPRI();
}

이게 RTOS의 심장이다
🔹 xTaskIncrementTick()

delay 중인 task 검사

timeout 만료 task → Ready List 이동

time slicing 처리

🔹 PendSV 강제 발생
portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET;


👉 “이 tick 끝나면 태스크 바꿔라”
