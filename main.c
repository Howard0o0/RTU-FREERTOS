/* Standard includes. */
#include <stdio.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

/* Hardware includes. */
#include "hal_MSP-EXP430F5438.h"
#include "msp430.h"
#include "msp430f5438a.h"

/* Standard demo includes. */
#include "GenQTest.h"
#include "ParTest.h"
#include "comtest2.h"
#include "dynamic.h"

#include "driverlib.h"
// #include "led.h"
// #include "uart3.h"

/* Old project */
#include "BC95.h"
#include "DTU.h"
#include "Sampler.h"
#include "common.h"
#include "console.h"
#include "hydrologycommand.h"
#include "hydrologytask.h"
#include "ioDev.h"
#include "led.h"
#include "main.h"
#include "msp430common.h"
#include "reportbinding.h"
#include "rom.h"
#include "rtc.h"
#include "store.h"
#include "timer.h"
#include "uart0.h"
#include "uart1.h"
#include "uart3.h"
#include "uart_config.h"
#include "wifi_config.h"
#include <string.h>

/* APP */
#include "ble_task.h"
#include "hydrolody_task.h"
#include "rtc_task.h"
#include "boot.h"
#include "communication_opr.h"
#include "gprs.h"
#include "memoryleakcheck.h"
#include "camera.h"


int IsDebug = 0;
// extern SemaphoreHandle_t xSemaphore_BLE;

/*
 * halboard necessary init coperation here.
 */
static void setup_board_hardware(void);

/*
 * !!!   应用程序的入口在这里    !!!!!
 */
void app_main(void* pvParameters) {

	install_io_devs_driver();

	bios_check();

	// communication_module_unit_test();

	create_hydrology_tasks();

	vTaskDelete(NULL);
}
/*-----------------------------------------------------------*/

void main(void) {
	setup_board_hardware();

	xTaskCreate(app_main, "app_main", configMINIMAL_STACK_SIZE *4 , NULL, tskIDLE_PRIORITY + 2,
		    NULL);

	vTaskStartScheduler();

	/* If all is well then this line will never be reached.  If it is
	reached then it is likely that there was insufficient (FreeRTOS) heap
	memory space to create the idle task.  This may have been trapped by the
	malloc() failed hook function, if one is configured. */
	printf("hoops you shouldn't have seen this, see line 102 in main() \r\n");
	while (1)
		;
}
/*-----------------------------------------------------------*/

void vApplicationTickHook(void) {
}
/*-----------------------------------------------------------*/

static void setup_board_hardware(void) {

	Restart_Init();

	Select_Debug_Mode(0);

	TraceOpen();

	Main_Init();

	WDT_A_hold(WDT_A_BASE);

	printf("device power on ! \n\n");

	Sampler_Open();
}
/*-----------------------------------------------------------*/


/* The MSP430X port uses this callback function to configure its tick interrupt.
This allows the application to choose the tick interrupt source.
configTICK_VECTOR must also be set in FreeRTOSConfig.h to the correct
interrupt vector for the chosen tick interrupt source.  This implementation of
vApplicationSetupTimerInterrupt() generates the tick from timer A0, so in this
case configTICK_VECTOR is set to TIMER0_A0_VECTOR. */
void vApplicationSetupTimerInterrupt(void) {
	const unsigned short usACLK_Frequency_Hz = 32768 / 8;

	/* Ensure the timer is stopped. */
	TA0CTL = 0;

	/* Run the timer from the ACLK. */
	TA0CTL = TBSSEL_1;

	/* Clear everything to start with. */
	TA0CTL |= TBCLR;

	/* Set the compare match value according to the tick rate we want. */
	TA0CCR0 = usACLK_Frequency_Hz / configTICK_RATE_HZ;

	/* Enable the interrupts. */
	TA0CCTL0 = CCIE;

	/* Start up clean. */
	TA0CTL |= TBCLR;

	/* Up mode. */
	TA0CTL |= MC_1;
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook(void) {
	/* Called on each iteration of the idle task.  In this case the idle
	task just enters a low power mode. */
	// printf("enter vApplicationIdleHook \r\n");
	__bis_SR_register(LPM3_bits + GIE);
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook(void) {
	/* Called if a call to pvPortMalloc() fails because there is
	insufficient free memory available in the FreeRTOS heap.  pvPortMalloc()
	is called internally by FreeRTOS API functions that create tasks, queues
	or semaphores. */
	taskDISABLE_INTERRUPTS();
	printf("vApplicationMallocFailedHook triggerd ! \r\n");
	printf("xPortGetFreeHeapSize : %d \r\n", xPortGetFreeHeapSize());
	for (;;)
		;
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char* pcTaskName) {
	( void )pxTask;
	( void )pcTaskName;

	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	printf("vApplicationStackOverflowHook triggerd ! \r\n");
	printf("Task:%s\r\n", ( char* )pcTaskName);
	for (;;)
		;
}
/*-----------------------------------------------------------*/

void Restart_Init() {

	P1SEL = 0x00;  //关闭脉冲端口中断
	P1DIR = 0x00;
	Clock_Init();  // CPU时钟初始化

	_EINT();  //打开中断
	// IE1|=OFIE+NMIIE+ACCVIE; //振荡器故障中断允许、NMI中断允许、FLASH存储器非法访问中断允许
	SFRIE1 |= OFIE + NMIIE + ACCVIE;
	WatchDog_Init();
	Led_Init();  // 指示灯 初始化
		     //    Led1_WARN();

	//    P9DIR |= BIT7;              //ly 拉高232芯片控制引脚
	//    P9OUT |= BIT7;

	TimerA_Init(30720);  // 系统定时器初始化
	TimerB_Init(61440);

	Store_Init();  // 初始化ROM
	RTC_Open();
	Sampler_Init();  //端口初始化,需要先初始化Store

	P10DIR |= BIT0;  // ly P100拉高，uart3用于调试，低的话P104，,105就是485口
	P10OUT |= BIT0;

	P10DIR |= BIT1;  // ly P101拉高，uart1 P104，,105，低的话就是485口,高的话就是蓝牙
	P10OUT &= ~BIT1;

	Console_Open();

	return;
}

void Main_Init() {  //时钟使用8M
	//开中断,以防之前部分有错误导致中断被关闭
	EnInt();
	TraceOpen();
	RTC_Open();  // 打开RTC
	TimerB_Clear();
	WatchDog_Clear();  // 清除复位计数器
	//在里再次运行这个函数  增加可靠性.
	Sampler_Init();
}

void Main_GotoSleep() {
	//开中断,以防之前部分有错误导致中断被关闭
	EnInt();

	TraceMsg("Device is going to sleep !", 1);
	//关闭外围,进入休眠
	RTC_Close();
	Console_Close();
	//    DTU_Close();
	//    GSM_Close(1);
	//    call gprs close
	Sampler_Close();
	TimerB_Clear();
	WatchDog_Clear();
	// TraceMsg("Device sleep !",1); //此处串口已经关闭
	LPM2;
	//    LPM3;
}
