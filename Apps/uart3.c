#include "uart3.h"
#include "driverlib.h"
#include "msp430f5438a.h"
#include "stdio.h"

#define BAUD_RATE                               115200
#define USCI_A_UART_MULTIPROCESSOR_MODE_ADDRESS        0xAA

void uart3_init(void){
    //Stop WDT
    WDT_A_hold(WDT_A_BASE);

    //P3.4,5 = USCI_A3 TXD/RXD
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

}

int putchar(int c)
{
    
    //Wait till ,module is ready to transmit again
    while (!USCI_A_UART_getInterruptStatus(USCI_A3_BASE,
               USCI_A_UART_TRANSMIT_INTERRUPT_FLAG)) ;
    
  
    USCI_A_UART_transmitData(USCI_A3_BASE,(uint8_t)c);

    USCI_A_UART_transmitBreak(USCI_A3_BASE);
    return c;
}