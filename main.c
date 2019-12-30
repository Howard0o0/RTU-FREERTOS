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
#include "gprs.h"
#include "memoryleakcheck.h"
#include "communication_opr.h"

int IsDebug = 0;
// extern SemaphoreHandle_t xSemaphore_BLE;

/*
 * Every init coperation here.
 */
static void prvSetupHardware(void);

static void       task_print_1(void* pvParameters);
static void       task_print_2(void* pvParameters);
static void       task_print_3(void* pvParameters);
SemaphoreHandle_t lock;

void main(void) {
	/* Configure the peripherals used by this demo application.  This
	includes configuring the joystick input select button to generate
	interrupts. */
	prvSetupHardware();

	register_communication_module();
	lock_communication_dev();
	communication_dev_t* comm_dev = get_communication_dev();
	if(comm_dev->power_on() != OK){
		printf("gprs boot err \r\n");
	}
	else{
		printf("gprs boot ok \r\n");
	}
	unlock_communication_dev();


	while (1)
		;

	/* end of gprs test */

	/**********TEST************/
	// vSemaphoreCreateBinary(lock);
	// xTaskCreate(task_print_1, "TEST1", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY +
	// 	1, 	    NULL);
	// xTaskCreate(task_print_2, "TEST2", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY +
	// 	1, 	    NULL);
	// xTaskCreate(task_print_3, "TEST3", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY +
	// 	1, 	    NULL);

	// while(1)
	// {
	//         char *p;
	//         p=(char *)mypvPortMalloc(sizeof(char)*10);
	//         p=(char *)mypvPortMalloc(sizeof(char)*10);
	//         p=(char *)mypvPortMalloc(sizeof(char)*30);
	//         myvPortFree(p);
	//         show_block();
	// }

	/**********TEST************/

	/*****BLE*************/

	// pBLE_Dev  ptDevBle =  getIODev();
	// ptDevBle->init();
	// xTaskCreate(BLE_RE, "BLE", configMINIMAL_STACK_SIZE * 3, NULL, tskIDLE_PRIORITY + 1,
	// NULL); xSemaphoreGive(xSemaphore_BLE);

	/*****BLE*************/

	hydrology_run();

	// hydrology_init();
	// xTaskCreate(task_hydrology_run, "task_hydrology_run", configMINIMAL_STACK_SIZE * 25,
	// NULL, 	    configMAX_PRIORITIES - 2, NULL);

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

void vApplicationTickHook(void) {}
/*-----------------------------------------------------------*/

static void prvSetupHardware(void) {
	halBoardInit();

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

static void task_print_1(void* pvParameters) {
	static int cnt    = 0;
	char*      buffer = NULL;

	while (1) {
		xSemaphoreTake(lock, portMAX_DELAY);
		printf("P1 START\r\n");
		// buffer = ( char* )pvPortMalloc(100);
		// for(int i=0;i<10;i++)
		//         printf("haha p1: %d \r\n", cnt++);
		// vPortFree(buffer);
		// buffer = NULL;
		// printf("P1 HWM left:%d\r\n", uxTaskGetStackHighWaterMark(NULL));
		printf("P1 END\r\n");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

static void task_print_2(void* pvParameters) {
	static int cnt    = 0;
	char*      buffer = NULL;
	while (1) {
		// printf("P2 START\r\n");
		// buffer = ( char* )pvPortMalloc(100);
		// for(int i=0;i<10;i++)
		//         printf("haha p2: %d \r\n", cnt++);
		// vPortFree(buffer);
		// buffer = NULL;
		// // printf("P2 HWM left:%d\r\n", uxTaskGetStackHighWaterMark(NULL));
		// printf("P2 END\r\n");
		printf("%ds\r\n", cnt++);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

static void task_print_3(void* pvParameters) {
	static int cnt    = 0;
	char*      buffer = NULL;
	while (1) {
		buffer = ( char* )mypvPortMalloc(sizeof(char) * 10);
		myvPortFree(buffer);
		buffer = ( char* )mypvPortMalloc(sizeof(char) * 20);
		show_block();
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
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

int Restart_GSMInit() {
	GSM_Open();
	if (GSM_CheckOK() < 0) {
		System_Delayms(1000);
		GSM_Open();
		if (GSM_CheckOK() < 0) {
			System_Delayms(1000);
			GSM_Open();
			if (GSM_CheckOK() < 0) {  //�޷������ͷ�����.
				GSM_Close(1);
				return -1;
			}
		}
	}

	char _phone[ 12 ];
	char _data[ 30 ];  // 30�㹻��

	_data[ 0 ] = '$';
	if (Store_ReadDeviceNO(&_data[ 1 ]) < 0) {
		_data[ 1 ]  = '0';
		_data[ 2 ]  = '0';
		_data[ 3 ]  = '0';
		_data[ 4 ]  = '0';
		_data[ 5 ]  = '0';
		_data[ 6 ]  = '0';
		_data[ 7 ]  = '0';
		_data[ 8 ]  = '0';
		_data[ 9 ]  = '0';
		_data[ 10 ] = '0';
		_data[ 11 ] = '0';
	}

	Utility_Strncpy(&_data[ 12 ], "<restart#", 9);
	if (Store_GSM_ReadCenterPhone(5, _phone) == 0) {
		if (_phone[ 0 ] != '0' && _phone[ 1 ] != '0' && _phone[ 2 ] != '0') {
			GSM_SendMsgTxt(_phone, _data, 21);
		}
	}
	GSM_Process(1, 0);     // GSM ������
	GSM_Close(0);	  //�ر� GSM
	System_Delayms(2000);  //����ػ�����쿪������ʧ��.
	return 0;
}

int Restart_DTUInit() {
	char _data[ UART3_MAXBUFFLEN ];
	int  _dataLen = 0;
	int  _repeats = 0;
	int  _out     = 0;
	//
	//��������һ��DTU�ȴ��������õĹ���.
	//
	if (trace_open == 0) {
		//���û�򿪵��ԵĻ�,������Ҫ����򿪵�
		// Console_Open();
	}
	UART3_ClearBuffer();
	// Console_WriteStringln("ACK");

	if (UART3_RecvLineLongWait(_data, UART3_MAXBUFFLEN, &_dataLen) == 0) {
		//����ȵ���������, �ͽ�������״̬.�ȴ�15����
		// Console_WriteStringln("waiting for 15 seconds .");
		if (Main_ProcCommand(_data, _dataLen, NULL) == 3) {  //������һ��SYN,�ͽ�������״̬
			while (1) {
				_repeats = 0;
				while (UART3_RecvLineLongWait(_data, UART3_MAXBUFFLEN, &_dataLen)
				       < 0) {
					++_repeats;
					if (_repeats > 4) {
						_out = 1;
						break;
					}
				}
				if (_out != 0)
					break;
				if (Main_ProcCommand(_data, _dataLen, NULL) == 2) {  //��ʾ�˳�����״̬
					break;
				}
			}
		}
	}
	//�����ڵ���״̬��ʱ��Ҫ�رյ�
	if (trace_open == 0) {  //��������ǹرյ�,���ڵ���Ҫ�ر�
				// Console_Close();
	}

	return 0;
}

int WorkMode_Init(char* ptype) {
	char _curType, _selType;
	int  _ret;

#if 0
    //��ȡѡ����״̬    DTU/GSM   232 
    P3DIR &= ~BIT3;   //P33Ϊ����
    P3DIR &= ~BIT2;   //P32Ϊ����
    if(P3IN & BIT2)
    {//P32Ϊ��
        _selType='D';
    }
    else
    {
        _selType='G';
    }
#endif
	_selType = 'S';  // GPRSģʽ GTM900
	switch (_selType) {
	case 'G':
		g_main_type = MAIN_TYPE_GSM;
		TraceMsg("Device is GSM Mode !", 1);
		break;
	case 'D':
		g_main_type = MAIN_TYPE_DTU;
		TraceMsg("Device is DTU Mode !", 1);
		break;

	case 'S':
		g_main_type = MAIN_TYPE_GTM900;
		TraceMsg("Device is GPRS Mode !", 1);
		break;

	default:
		//�����������.������

		TraceMsg("Bad Mode !", 1);
		System_Reset();
		break;
	}

	//�жϵ�ǰ����״̬,   DTU/GSM
	if (Store_ReadSystemType(&_curType) < 0) {  //����޷�����
						    //�����ģʽ,�Ͳ��ж��Ƿ� ����������.
		_ret = 0;
	}
	else {
		_ret = _selType != _curType ? 1 : 0;
	}

	*ptype = _selType;

	return _ret;
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
