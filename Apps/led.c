#include "msp430f5438a.h"

void led_init(void){
    
    P5DIR |= BIT3;
    P7DIR |= BIT1;
    P7DIR |= BIT2;
    P7DIR |= BIT3;
    P9DIR |= BIT0;
    P10DIR |= BIT6; 

    P5SEL &= ~BIT3;
    P7SEL &= ~BIT1;

    P5OUT |= BIT3;
    P7OUT |= BIT1;
    P7OUT |= BIT2;
    P7OUT |= BIT3;
    P7OUT |= BIT3;
    P10OUT |= BIT6;
}



void Led6_On()//D5
{
   P10OUT |=BIT6;
}

void Led6_Off()
{ 
   P10OUT &= ~ BIT6;
}