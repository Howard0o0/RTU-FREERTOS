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
static void task_print( void *pvParameters );

void main( void )
{
	/* Configure the peripherals used by this demo application.  This includes
	configuring the joystick input select button to generate interrupts. */
	prvSetupHardware();
        
	// xTaskCreate( task_print, "LCD", 1024 * 2, NULL, tskIDLE_PRIORITY + 1, NULL );
	// vTaskStartScheduler();

	//Stop WDT
    WDT_A_hold(WDT_A_BASE);
	
    //P10.4,5 = USCI_A3 TXD/RXD
    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_P10,
        GPIO_PIN4 + GPIO_PIN5
        );

    //Initialize USCI UART module
    //Baudrate = 9600, clock freq = 1.048MHz
    //UCBRx = 6, UCBRFx = 13, UCBRSx = 0, UCOS16 = 1
    USCI_A_UART_initParam param = {0};
    param.selectClockSource = USCI_A_UART_CLOCKSOURCE_SMCLK;
    param.clockPrescalar = 6;
    param.firstModReg = 13;
    param.secondModReg = 0;
    param.parity = USCI_A_UART_NO_PARITY;
    param.msborLsbFirst = USCI_A_UART_LSB_FIRST;
    param.numberofStopBits = USCI_A_UART_ONE_STOP_BIT;
    param.uartMode = USCI_A_UART_AUTOMATIC_BAUDRATE_DETECTION_MODE;
    param.overSampling = USCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;
    if (STATUS_FAIL == USCI_A_UART_init(USCI_A3_BASE, &param)){
        return;
    }

    //Enable UART module for operation
    USCI_A_UART_enable(USCI_A3_BASE);

    //Transmit Break
    USCI_A_UART_transmitBreak(USCI_A3_BASE);

    //Wait till ,module is ready to transmit again
    while (!USCI_A_UART_getInterruptStatus(USCI_A3_BASE,
               USCI_A_UART_TRANSMIT_INTERRUPT_FLAG)) ;

    
    while(1){
        USCI_A_UART_transmitData(USCI_A3_BASE,0x55);
    }
        

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
	LFXT_Start( XT1DRIVE_0 );
	hal430SetSystemClock( configCPU_CLOCK_HZ, configLFXT_CLOCK_HZ );

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

static void task_print( void *pvParameters ){

	uart3_init();
	while (1){
		// printf("haha \r\n");
		USCI_A_UART_transmitData(USCI_A3_BASE,0x55);
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
void vApplicationSetupTimerInterrupt( void )
{
const unsigned short usACLK_Frequency_Hz = 32768;

	/* Ensure the timer is stopped. */
	TA0CTL = 0;

	/* Run the timer from the ACLK. */
	TA0CTL = TASSEL_1;

	/* Clear everything to start with. */
	TA0CTL |= TACLR;

	/* Set the compare match value according to the tick rate we want. */
	TA0CCR0 = usACLK_Frequency_Hz / configTICK_RATE_HZ;

	/* Enable the interrupts. */
	TA0CCTL0 = CCIE;

	/* Start up clean. */
	TA0CTL |= TACLR;

	/* Up mode. */
	TA0CTL |= MC_1;
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

