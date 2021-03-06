//////////////////////////////////////////////////////
//     �ļ���: timer.c
//   �ļ��汾: 1.0.0  
//   ����ʱ��: 09��11��30��
//   ��������: �ޡ� 
//       ����: ����
//       ��ע:  
// 
//
//////////////////////////////////////////////////////

#include "msp430common.h"
#include "timer.h"
#include "common.h"
#include "led.h"
#include "hydrologytask.h"

static unsigned int s_ClockTicks=0;   
static unsigned int s_reset_count=0;

unsigned int sys_errorcode_runcode=100; // 定义 sys_errorcode_runcode为全局变量，取值1-69， 初值定为100，防止误点灯   LSHB 20200506

void Timer_Zero()
{
    s_ClockTicks=0;
} 
int Timer_Passed_Mins(int _min)
{
    if(_min <= s_ClockTicks)
        return 1;
    else
        return 0;
}
/*time1 a3*/
//ע�� ACLK ʹ���� 8��Ƶ,
void TimerA_Init(unsigned int ccr0)
{
  /*2418 TACTL 5438 TA1CTL*/
    TA1CTL =  TACLR;      //�������
    TA1CTL |= TASSEL0;    //ѡ�� ACLKʱ��
    TA1CTL |= ID1+ID0;    // 1/8��Ƶ 
    /*2418 TACCTL0 5438 TA1CCTL0*/
    TA1CCTL0 = CCIE;	 // CCR0 �ж�����
    //1���� �ж�һ��
    /*2418 TACCR0 5438 TA1CCR0*/
    TA1CCR0 = ccr0;  //  (32768HZ / (8 * 8) ) * 60s =30720
    /*2418 TACTL 5438 TA1CTL*/
    TA1CTL |= MC0 ;// ������ ģʽ   
} 
 
void WatchDog_Init()
{ 
    //������������.
    //ʹ��ACLK 32K/8 ��Ƶ
  /*2418 WDTSSEL 5438 WDTSSEL0*/
    WDTCTL = WDTPW + WDTSSEL0; 
}  
void WatchDog_Clear()
{
  /*2418 WDTSSEL 5438 WDTSSEL0*/
    WDTCTL = WDTPW  + WDTSSEL0 + WDTCNTCL;
} 
/*time0 b7*/
void TimerB_Init(unsigned int ccr0)
{
  /*2418 TBCTL 5438 TB0CTL*/
    TB0CTL = TBCLR;
    TB0CTL |= TBSSEL0;    //ѡ��ACLKʱ��
    TB0CTL |= ID0;  //1/8��Ƶ
    TB0CTL |= MC1  ;   //   ��������ģʽ          
    //TBCTL |= TBIE;       //ʹ��TBIFG�ж�
    //4���� �ж�һ�� ,(��2��)
    //TBCCR0=ccr0; // (32768HZ /(8*8) ) * 240 / 2 = 61440
    TBCCR1=61440; 
//    TBCCR2=10240;
//    TBCCR2=4096;//�����幷�� �ж�. 2s�ж�1��
//    TBCCR2=2048;
    TBCCR2=1024;//�����幷�� �ж�.
    /*2418 TBCCTL1 TBCCTL2 5438 TB0CCTL1 TB0CCTL2*/
    TB0CCTL1=CCIE;
    TB0CCTL2=CCIE;
    /*2418 P41  5438 P96 systemdebug*/
    P11DIR |= BIT1;
    P11OUT |= BIT1;     //P9.6=>P11.1
}

void TimerB_Clear()
{
    s_reset_count=0;
}

// void Timer0_A0_Enable()  // 启动Timer0_A0中断 LSHB 20200506
// {
//     TA0CCTL0 |= CCIE;
// }

// void Timer0_A0_Clear()  // 清除Timer0_A0中断 LSHB 20200506
// {
//      TA0CCTL0 &= ~CCIE;
// }

// void TimerB0_Enable()  // 启动TIMERB0中断 LSHB 20200506
// {
//     TB0CCTL0=CCIE;
// }

// void TimerB0_Clear()  // 清除TIMERB0中断 LSHB 20200506
// {
//     TB0CCTL0=~CCIE;
// }

// #pragma vector = TIMER0_A0_VECTOR
// __interrupt void TIMER0_A0_ISR (void)
// {    
  
//   sys_error_status(sys_errorcode_runcode);  //用于系统错误和工作状态指示灯控制 LSHB   20200506
// }

/*   TIMERA0_VECTOR   *   TIMER1_A0_VECTOR*/
volatile unsigned long ulStatsOverflowCount = 0;
#pragma vector=TIMER1_A0_VECTOR 
__interrupt void TIMERA0_ISR(void)
{
    //����CPU,���CPU���������ѵ�,���ᵼ�»���
    //LPM3_EXIT;
   // Led1_On();
 //   System_Delayms(75);
   // Led1_Off();
    LPM2_EXIT;
    ++s_ClockTicks;
    ulStatsOverflowCount++;
    HydrologyTimeBase();
    TimerB_Clear();
    WatchDog_Clear();
}

//12���ӵĿ��Ź�
// #pragma vector = TIMERB0_VECTOR
// __interrupt void TIMERB0_ISR (void)
// {
// //    System_Delayms(1000);
// }

#pragma vector=TIMERB1_VECTOR
__interrupt void TIMERB1_ISR(void)
{ 
    //����Ҫ��һ��TBIV�����ж�
    unsigned int _tbiv = TBIV ; 
    switch(_tbiv)
    {
      case 2:
        //�������һ��
        //�����������1000
        ++s_reset_count;
        if(s_reset_count>60)
        {//����12���Ӿ�����
            System_Reset();
        }
        TBCCR1 += 61440;
        break;
      case 4:
        WatchDog_Clear(); 
        TBCCR2 += 1024;
        Clear_ExternWatchdog();
        break;
      case 10:
        break;
      default:
        break;
    } 
}

