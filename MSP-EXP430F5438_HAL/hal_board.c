/**
 * @file  hal_board.c
 *
 * Copyright 2010 Texas Instruments, Inc.
******************************************************************************/
#include "msp430.h"
#include "hal_MSP-EXP430F5438.h"

/**********************************************************************//**
 * @brief  Initializes ACLK, MCLK, SMCLK outputs on P11.0, P11.1,
 *         and P11.2, respectively.
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
void halBoardOutputSystemClock(void) //outputs clock to testpoints
{
  CLK_PORT_DIR |= 0x07;
  CLK_PORT_SEL |= 0x07;
}

/**********************************************************************//**
 * @brief  Stops the output of ACLK, MCLK, SMCLK on P11.0, P11.1, and P11.2.
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
void halBoardStopOutputSystemClock(void)
{
  CLK_PORT_OUT &= ~0x07;
  CLK_PORT_DIR |= 0x07;	
  CLK_PORT_SEL &= ~0x07;
}

/**********************************************************************//**
 * @brief  Initializes all GPIO configurations.
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
void halBoardInit(void)
{
  //Tie unused ports
  PAOUT  = 0;
  PADIR  = 0xFFFF;
  PASEL  = 0;
  PBOUT  = 0;
  PBDIR  = 0xFFFF;
  PBSEL  = 0;
  PCOUT  = 0;
  PCDIR  = 0xFFFF;
  PCSEL  = 0;
  PDOUT  = 0;
  PDDIR  = 0xFFFF;
  PDSEL  = 0x0003;
  PEOUT  = 0;
  PEDIR  = 0xFEFF;                          // P10.0 to USB RST pin,
                                            // ...if enabled with J5
  PESEL  = 0;
  P11OUT = 0;
  P11DIR = 0xFF;
  PJOUT  = 0;
  PJDIR  = 0xFF;
  P11SEL = 0;
}

/**********************************************************************//**
 * @brief  Set function for MCLK frequency.
 *
 *
 * @return none
 *************************************************************************/
void hal430SetSystemClock(unsigned long req_clock_rate, unsigned long ref_clock_rate)
{
  /* Convert a Hz value to a KHz value, as required
   *  by the Init_FLL_Settle() function. */
  unsigned long ulCPU_Clock_KHz = req_clock_rate / 1000UL;

  //Make sure we aren't overclocking
  if(ulCPU_Clock_KHz > 25000L)
  {
    ulCPU_Clock_KHz = 25000L;
  }

  //Set VCore to a level sufficient for the requested clock speed.
  if(ulCPU_Clock_KHz <= 8000L)
  {
    SetVCore(PMMCOREV_0);
  }
  else if(ulCPU_Clock_KHz <= 12000L)
  {
    SetVCore(PMMCOREV_1);
  }
  else if(ulCPU_Clock_KHz <= 20000L)
  {
    SetVCore(PMMCOREV_2);
  }
  else
  {
    SetVCore(PMMCOREV_3);
  }

  //Set the DCO
  Init_FLL_Settle( ( unsigned short )ulCPU_Clock_KHz, req_clock_rate / ref_clock_rate );
}

// void Set_Vcore(unsigned int level)
// {
//   PMMCTL0_H = PMMPW_H;                                             // Open PMM registers for write
//   SVSMHCTL = SVSHE + SVSHRVL0 * level + SVMHE + SVSMHRRL0 * level; // Set SVS/SVM high side new level
//   SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * level;                    // Set SVM low side to new level
//   while ((PMMIFG & SVSMLDLYIFG) == 0)
//     ;                                // Wait till SVM is settled
//   PMMIFG &= ~(SVMLVLRIFG + SVMLIFG); // Clear already set flags
//   PMMCTL0_L = PMMCOREV0 * level;     // Set VCore to new level
//   if ((PMMIFG & SVMLIFG))            // Wait till new level reached
//     while ((PMMIFG & SVMLVLRIFG) == 0)
//       ;
//   SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level; // Set SVS/SVM low side to new level
//   PMMCTL0_H = 0x00;                                                // Lock PMM registers for write access
// }
// void System_Delayms(unsigned int nValue)
// { //函数参数是寄存器,操作速度过快,所以我们不用nValue
//   unsigned long nCount = 1150;
//   unsigned long i;
//   unsigned long j;
//   static int s_clock = 0;       //用来指示当前频率

//   if (s_clock == 32)
//   {
//     nCount = 3;
//   }
//   for (i = nValue; i > 0; --i)
//   {
//     for (j = nCount; j > 0; --j)
//       ;
//   }
//   return;
// }
// void Clock_SMCLK_DIV(int i)
// {
//   switch (i)
//   {
//   case 1:
//     UCSCTL5 &= ~DIVS1;
//     UCSCTL5 &= ~DIVS0; //ACLK 不分频
//     break;
//   case 2:
//     UCSCTL5 &= ~DIVS1; //ACLK 2分频
//     UCSCTL5 |= DIVS0;
//     break;
//   case 4:
//     UCSCTL5 |= DIVS1; //ACLK 4分频
//     UCSCTL5 &= ~DIVS0;
//     break;
//   case 8:
//     UCSCTL5 |= DIVS1 + DIVS0; //ACLK 8分频
//     break;
//   default:
//     UCSCTL5 &= ~DIVS1;
//     UCSCTL5 &= ~DIVS0; //ACLK 不分频
//   }
//   //等待时钟正常
//   do
//   {
//     //等待时钟稳定
//     SFRIFG1 &= ~OFIFG;
//     for (int i = 0x20; i > 0; i--)
//       ;
//   } while ((SFRIFG1 & OFIFG) == OFIFG);
// }
// void Clock_ACLK_DIV(int i)
// {
//   switch (i)
//   {
//   case 1:
//     UCSCTL5 &= ~DIVA1;
//     UCSCTL5 &= ~DIVA0; //ACLK 不分频
//     break;
//   case 2:
//     UCSCTL5 &= ~DIVA1; //ACLK 2分频
//     UCSCTL5 |= DIVA0;
//     break;
//   case 4:
//     UCSCTL5 |= DIVA1; //ACLK 4分频
//     UCSCTL5 &= ~DIVA0;
//     break;
//   case 8:
//     UCSCTL5 |= DIVA1 + DIVA0; //ACLK 8分频
//     break;
//   default:
//     UCSCTL5 &= ~DIVA1;
//     UCSCTL5 &= ~DIVA0; //ACLK 不分频
//   }
//   //等待时钟正常
//   do
//   {
//     //等待时钟稳定
//     SFRIFG1 &= ~OFIFG;
//     for (int i = 0x20; i > 0; i--)
//       ;
//   } while ((SFRIFG1 & OFIFG) == OFIFG);
// }

// /*
//  * Description  : 时钟初始化
//  * Details      : P5.2和P5.3选择为晶振XT2输入
//  *                P7.0和P7.1选择为晶振XT1输入
//  *                选择MCLK、SMCLK为XT2 => 8MHz  =(8分频)==>  1MHz
//  *                选择ACLK为XT1 => 32Khz  =(8分频)==>  4Khz
// */
// void Clock_Init()
// {
//   //unsigned char i;

//   WDTCTL = WDTPW + WDTHOLD; //关闭看门狗定时

//   P5SEL |= BIT2 + BIT3;          //P5.2和P5.3选择为晶振XT2输入
//   P7SEL |= BIT0 + BIT1;          //P7.0和P7.1选择为晶振XT1输入
//   Set_Vcore(PMMCOREV_3);         // Set frequency up to 25MHz
//   UCSCTL6 &= ~(XT1OFF + XT2OFF); // Set XT1 & XT2 On

//   UCSCTL6 |= XCAP_3; // Internal load XT1 cap 12pF，MSP430F5438A V4.0最小系统XT1未接外部晶振

//   UCSCTL6 |= XT2BYPASS; //选择外部激励振动
//   UCSCTL6 |= XT1BYPASS;
//   UCSCTL4 |= SELA__XT1CLK + SELS__XT2CLK + SELM__XT2CLK; //选择MCLK、SMCLK为XT2,
//   do                                                     // Loop until XT1,XT2 & DCO stabilizes
//   {
//     UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);
//     SFRIFG1 &= ~OFIFG;       // 清除振荡器失效标志
//                              //      for (i = 0xFF; i > 0; i--);           // 延时，等待XT2起振
//   } while (SFRIFG1 & OFIFG); // 判断XT2是否起振
//                              //Delay_ms(50);
//   System_Delayms(50);
//   Clock_ACLK_DIV(8);
//   Clock_SMCLK_DIV(8);
// }