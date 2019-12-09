/* Standard includes. */
#include <stdio.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Hardware includes. */
#include "msp430.h"
#include "hal_MSP-EXP430F5438.h"

/* Standard demo includes. */
#include "ParTest.h"
#include "dynamic.h"
#include "comtest2.h"
#include "GenQTest.h"

#include "led.h"
#include "driverlib.h"
#include "uart3.h"


/*
 * Every init coperation here.
 */
static void prvSetupHardware( void );

static void task_lcd( void *pvParameters );
static void task_print_1( void *pvParameters );
static void task_print_2( void *pvParameters );

void main( void )
{
	/* Configure the peripherals used by this demo application.  This includes
	configuring the joystick input select button to generate interrupts. */
	prvSetupHardware();
        
	xTaskCreate( task_print_1, "LCD", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, NULL );
	xTaskCreate( task_print_2, "LCD", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, NULL );
	vTaskStartScheduler();

	/* If all is well then this line will never be reached.  If it is reached
	then it is likely that there was insufficient (FreeRTOS) heap memory space
	to create the idle task.  This may have been trapped by the malloc() failed
	hook function, if one is configured. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void ){

}
/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
	halBoardInit();
	// LFXT_Start( XT2DRIVE_0 );
	// hal430SetSystemClock( configCPU_CLOCK_HZ, configLFXT_CLOCK_HZ );
	Clock_Init();

	WDT_A_hold(WDT_A_BASE);
	
}
/*-----------------------------------------------------------*/

static void task_lcd( void *pvParameters ){

	led_init();
	while (1){
		Led6_On();
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		Led6_Off();
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	
        
}
/*-----------------------------------------------------------*/

static void task_print_1( void *pvParameters ){

	uart3_init();
	static int cnt = 0;
	while (1){
		printf("p1: %d \r\n",cnt++);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	
        
}

static void task_print_2( void *pvParameters ){

	uart3_init();
	static int cnt = 0;
	while (1){
		printf("p2: %d \r\n",cnt++);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	
        
}
/*-----------------------------------------------------------*/

/* The MSP430X port uses this callback function to configure its tick interrupt.
This allows the application to choose the tick interrupt source.
configTICK_VECTOR must also be set in FreeRTOSConfig.h to the correct
interrupt vector for the chosen tick interrupt source.  This implementation of
vApplicationSetupTimerInterrupt() generates the tick from timer A0, so in this
case configTICK_VECTOR is set to TIMER0_B0_VECTOR. */
void vApplicationSetupTimerInterrupt( void )
{
	const unsigned short usACLK_Frequency_Hz = 32768/8;

	/* Ensure the timer is stopped. */
	TB0CTL = 0;

	/* Run the timer from the ACLK. */
	TB0CTL = TBSSEL_1;

	/* Clear everything to start with. */
	TB0CTL |= TBCLR;

	/* Set the compare match value according to the tick rate we want. */
	TB0CCR0 = usACLK_Frequency_Hz / configTICK_RATE_HZ;

	/* Enable the interrupts. */
	TB0CCTL0 = CCIE;

	/* Start up clean. */
	TB0CTL |= TBCLR;

	/* Up mode. */
	TB0CTL |= MC_1;
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	/* Called on each iteration of the idle task.  In this case the idle task
	just enters a low power mode. */
	__bis_SR_register( LPM3_bits + GIE );
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues or
	semaphores. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pxTask;
	( void ) pcTaskName;

	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

