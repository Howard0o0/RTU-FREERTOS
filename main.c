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
#include "BLE_Task.h"
#include "Hydrolody_Task.h"
#include "RTC_Task.h"
#include "communication_opr.h"
#include "gprs.h"
#include "memoryleakcheck.h"


int IsDebug = 0;
// extern SemaphoreHandle_t xSemaphore_BLE;

/*
 * Every init coperation here.
 */
static void prvSetupHardware(void);

static void install_io_devs_driver(void);

static void bios_check(void);

void app_main(void* pvParameters);



void main(void) {

	xTaskCreate(app_main, "app_main", configMINIMAL_STACK_SIZE * 4, NULL,
		    tskIDLE_PRIORITY + 2, NULL);

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


void app_main(void* pvParameters){
	prvSetupHardware();

	install_io_devs_driver();
	bios_check();

	hydrology_init();

	create_hydrology_tasks();
	
	
	vTaskDelete(NULL);
}
/*-----------------------------------------------------------*/


void vApplicationTickHook(void) {
}
/*-----------------------------------------------------------*/

static void prvSetupHardware(void) {
	// halBoardInit();

	BleDriverInstall();
	UART1_Open(1);

	WDT_A_hold(WDT_A_BASE);

	Restart_Init();

	Select_Debug_Mode(0);

	TraceOpen();

	TraceMsg("Device Open !", 1);

	Main_Init();

	Sampler_Open();
}
/*-----------------------------------------------------------*/


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

/*-----------------------------------------------------------*/
/*-----------------------------------------------------------*/
/*-----------------------------------------------------------*/
void Restart_Init() {
	P1SEL = 0x00;  //�ر�����˿��ж�
	P1DIR = 0x00;
	Clock_Init();  // CPUʱ�ӳ�ʼ��

	_EINT();  //���ж�
	// IE1|=OFIE+NMIIE+ACCVIE;
	// //���������ж�������NMI�ж�������FLASH�洢���Ƿ������ж�����
	SFRIE1 |= OFIE + NMIIE + ACCVIE;
	WatchDog_Init();
	Led_Init();  // ָʾ�� ��ʼ��
		     //    Led1_WARN();

	//    P9DIR |= BIT7;              //ly ����232оƬ��������
	//    P9OUT |= BIT7;

	TimerA_Init(30720);  // ϵͳ��ʱ����ʼ��
	TimerB_Init(61440);

	Store_Init();  // ��ʼ��ROM
	RTC_Open();
	Sampler_Init();  //�˿ڳ�ʼ��,��Ҫ�ȳ�ʼ��Store

	P10DIR |= BIT0;  // ly P100���ߣ�uart3���ڵ��ԣ��͵Ļ�P104��,105����485��
	P10OUT |= BIT0;

	P10DIR |= BIT1;  // ly P101���ߣ�uart1 P104��,105���͵Ļ�����485��,�ߵĻ���������
	P10OUT &= ~BIT1;

	Console_Open();

	/*wyq  ����485ȥ�������Ĳ���*/
	//     P3DIR &= ~BIT2;
	//    if(P3IN & BIT2)
	//    {
	//        IsDebug = 1;
	//    }
	//    else
	//    {
	//        IsDebug = 0;
	//    }

	return;
}

static void install_io_devs_driver(void) {
	install_communication_module_driver();
	get_communication_dev()->power_on();
}

static void bios_check(void) {
	communication_module_t* comm_module = get_communication_dev();
	if (comm_module->check_if_module_is_normal() == OK) {
		printf("[OK] %s module\r\n", comm_module->name);
	}
	else {
		printf("[ERROR] %s module not respond\r\n", comm_module->name);
	}
}

void Main_Init() {  //ʱ��ʹ��8M
	//���ж�,�Է�֮ǰ�����д������жϱ��ر�
	EnInt();
	TraceOpen();
	RTC_Open();  // ��RTC
	TimerB_Clear();
	WatchDog_Clear();  // �����λ������
	//�����ٴ������������  ���ӿɿ���.
	Sampler_Init();
}

void Main_GotoSleep() {
	//���ж�,�Է�֮ǰ�����д������жϱ��ر�
	EnInt();

	TraceMsg("Device is going to sleep !", 1);
	//�ر���Χ,��������
	RTC_Close();
	Console_Close();
	//    DTU_Close();
	//    GSM_Close(1);
	//    call gprs close
	Sampler_Close();
	TimerB_Clear();
	WatchDog_Clear();
	// TraceMsg("Device sleep !",1); //�˴������Ѿ��ر�
	LPM2;
	//    LPM3;
}
