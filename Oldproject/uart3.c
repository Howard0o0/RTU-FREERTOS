  //////////////////////////////////////////////////////
<<<<<<< HEAD
//     ï¿½Ä¼ï¿½ï¿½ï¿½: uart3.c
//   ï¿½Ä¼ï¿½ï¿½æ±¾: 1.0.0
//   ï¿½ï¿½ï¿½ï¿½Ê±ï¿½ï¿½: 09ï¿½ï¿½11ï¿½ï¿½30ï¿½ï¿½
//   ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½:  
//       ï¿½ï¿½ï¿½ï¿½: ï¿½ï¿½ï¿½ï¿½
//       ï¿½ï¿½×¢: ï¿½ï¿½
=======
//     ÎÄ¼þÃû: uart3.c
//   ÎÄ¼þ°æ±¾: 1.0.0
//   ´´½¨Ê±¼ä: 09Äê11ÔÂ30ÈÕ
//   ¸üÐÂÄÚÈÝ:  
//       ×÷Õß: ÁÖÖÇ
//       ¸½×¢: ÎÞ
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
//
//////////////////////////////////////////////////////

#include "msp430common.h"
#include "common.h"
#include "uart3.h"
#include "uart0.h"
#include "led.h"
#include "stdio.h"
<<<<<<< HEAD
#include "ioDev.h"
=======
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
/************A1********/
#define TXD3 BIT4
#define RXD3 BIT5

char testbuf[2];
int testcount=0;

char * UART3_Tx_Buf=NULL; 
<<<<<<< HEAD
char UART3_Rx_Buffer[UART3_MAXIndex][UART3_MAXBUFFLEN]; //  ï¿½ï¿½ï¿½Ý´æ´¢ï¿½ï¿½ 
int  UART3_Rx_BufLen[UART3_MAXIndex];                   //  Ã¿ï¿½Ð½ï¿½ï¿½Õµï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ý³ï¿½ï¿½ï¿½  
int  UART3_Rx_INTIndex=0;                               //  ï¿½Ð¶Ï¸ï¿½Ð´ï¿½ï¿½ï¿½ï¿½Î»ï¿½ï¿½
int  UART3_Rx_INTLen=0;                                 //  ï¿½Ð¶Ï¸ï¿½Ð´ï¿½ï¿½ï¿½ÐµÄµÚ¼ï¿½ï¿½ï¿½ï¿½Ö·ï¿½ 
int  UART3_Rx_RecvIndex=0;                              //  ï¿½ï¿½Ç°ï¿½Ã¶ï¿½ï¿½ï¿½ï¿½ï¿½Î»ï¿½ï¿½ 
=======
char UART3_Rx_Buffer[UART3_MAXIndex][UART3_MAXBUFFLEN]; //  Êý¾Ý´æ´¢Çø 
int  UART3_Rx_BufLen[UART3_MAXIndex];                   //  Ã¿ÐÐ½ÓÊÕµ½µÄÊý¾Ý³¤¶È  
int  UART3_Rx_INTIndex=0;                               //  ÖÐ¶Ï¸ÃÐ´µÄÐÐÎ»ÖÃ
int  UART3_Rx_INTLen=0;                                 //  ÖÐ¶Ï¸ÃÐ´¸ÃÐÐµÄµÚ¼¸¸ö×Ö·û 
int  UART3_Rx_RecvIndex=0;                              //  µ±Ç°¸Ã¶ÁµÄÐÐÎ»ÖÃ 
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad

unsigned int UART3_Tx_Flag=0;
unsigned int UART3_Tx_Len=0;
extern int WIFI_Inited_Flag;


<<<<<<< HEAD
//Ö¸Ê¾ï¿½ï¿½Ç°ï¿½ï¿½ï¿½Íµï¿½
=======
//Ö¸Ê¾µ±Ç°ÀàÐÍµÄ
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
static int s_uart3_type=0;  

int UART3_Open(int  _type)
{
    s_uart3_type = _type;
    
<<<<<<< HEAD
    //ï¿½ï¿½ï¿½ï¿½rs232  ï¿½ï¿½Æ½×ªï¿½ï¿½ï¿½ï¿½Â·
=======
    //¿ªÆôrs232  µçÆ½×ª»»µçÂ·
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
 //   P4DIR |= BIT0;
 //   P4OUT |= BIT0;
    
 //   UCTL1 = 0x00;
 //   UCTL1 &=~ SWRST; 
 //   UCTL1 |= CHAR;
 //   UTCTL1 = 0X00;	
<<<<<<< HEAD
    //115200ï¿½ï¿½  XT2=8000000   SMCLK
=======
    //115200£¬  XT2=8000000   SMCLK
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
    //UTCTL1=SSEL1; UBR0_1 = 0x45; UBR1_1 = 0x00; UMCTL1 = 0x4A;
    
    //  9600,    XT2=8000000   SMCLK
 //   UTCTL1=SSEL1;UBR0_1 = 0x41;UBR1_1 = 0x03;UMCTL1 = 0x00;
     
 //   UART3_ClearBuffer();
        
<<<<<<< HEAD
 //   ME2 |= UTXE1+URXE1;   //Ê¹ï¿½ï¿½UART3ï¿½ï¿½TXDï¿½ï¿½RXD  
 //   IE2 |= URXIE1+UTXIE1; //Ê¹ï¿½ï¿½UART3ï¿½ï¿½RXï¿½ï¿½TXï¿½Ð¶ï¿½  
    
 //   P3SEL |= BIT6;//ï¿½ï¿½ï¿½ï¿½P3.6ÎªUART3ï¿½ï¿½TXD 
 //   P3SEL |= BIT7;//ï¿½ï¿½ï¿½ï¿½P3.7ÎªUART3ï¿½ï¿½RXD
 //   P3DIR |= BIT6;//P3.6Îªï¿½ï¿½ï¿½ï¿½Ü½ï¿½   
//     if(s_uart3_type==1)
////    {
//        P10DIR |= BIT0;             //ly P100ï¿½ï¿½ï¿½ß£ï¿½uart3ï¿½ï¿½ï¿½Úµï¿½ï¿½Ô£ï¿½ï¿½ÍµÄ»ï¿½P104ï¿½ï¿½,105ï¿½ï¿½ï¿½ï¿½485ï¿½ï¿½
=======
 //   ME2 |= UTXE1+URXE1;   //Ê¹ÄÜUART3µÄTXDºÍRXD  
 //   IE2 |= URXIE1+UTXIE1; //Ê¹ÄÜUART3µÄRXºÍTXÖÐ¶Ï  
    
 //   P3SEL |= BIT6;//ÉèÖÃP3.6ÎªUART3µÄTXD 
 //   P3SEL |= BIT7;//ÉèÖÃP3.7ÎªUART3µÄRXD
 //   P3DIR |= BIT6;//P3.6ÎªÊä³ö¹Ü½Å   
//     if(s_uart3_type==1)
////    {
//        P10DIR |= BIT0;             //ly P100À­¸ß£¬uart3ÓÃÓÚµ÷ÊÔ£¬µÍµÄ»°P104£¬,105¾ÍÊÇ485¿Ú
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
//        P10OUT |= BIT0;  
////    }
////    else
////    {
<<<<<<< HEAD
//        P10DIR |= BIT0;             //ly P100ï¿½ï¿½ï¿½ß£ï¿½uart3ï¿½ï¿½ï¿½Úµï¿½ï¿½Ô£ï¿½ï¿½ÍµÄ»ï¿½P104ï¿½ï¿½,105ï¿½ï¿½ï¿½ï¿½485ï¿½ï¿½
=======
//        P10DIR |= BIT0;             //ly P100À­¸ß£¬uart3ÓÃÓÚµ÷ÊÔ£¬µÍµÄ»°P104£¬,105¾ÍÊÇ485¿Ú
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
//        P10OUT &=~ BIT0;
////    }
   
 
  UCA3CTL1 |= UCSWRST;
  UCA3CTL1 |= UCSSEL1;   //smclk 1M 
  
  UCA3BR0 = 8;
  UCA3BR1 = 0;
  UCA3MCTL |= UCBRF_0+UCBRS_6;
  UCA3CTL1 &= ~UCSWRST;
  
  UART3_ClearBuffer();
  /********************/
  P10DIR |= TXD3;
  P10SEL |= TXD3 + RXD3;
  
  /*2418 UC1IE UCA1RXIE 5438 UCA1IE UCRXIE*/
<<<<<<< HEAD
  UCA3IE |= UCRXIE;//ï¿½ï¿½ï¿½Ü´ï¿½ï¿½Ú½ï¿½ï¿½ï¿½ï¿½Ð¶ï¿½          
=======
  UCA3IE |= UCRXIE;//ÊÇÄÜ´®¿Ú½ÓÊÕÖÐ¶Ï          
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad

  return 0;
}
void UART3_Open_9600(int _type)
{
  s_uart3_type = _type;
    
<<<<<<< HEAD
    //ï¿½ï¿½ï¿½ï¿½rs232  ï¿½ï¿½Æ½×ªï¿½ï¿½ï¿½ï¿½Â·
=======
    //¿ªÆôrs232  µçÆ½×ª»»µçÂ·
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
 //   P4DIR |= BIT0;
 //   P4OUT |= BIT0;
    
 //   UCTL1 = 0x00;
 //   UCTL1 &=~ SWRST; 
 //   UCTL1 |= CHAR;
 //   UTCTL1 = 0X00;	
<<<<<<< HEAD
    //115200ï¿½ï¿½  XT2=8000000   SMCLK
=======
    //115200£¬  XT2=8000000   SMCLK
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
    //UTCTL1=SSEL1; UBR0_1 = 0x45; UBR1_1 = 0x00; UMCTL1 = 0x4A;
    
    //  9600,    XT2=8000000   SMCLK
 //   UTCTL1=SSEL1;UBR0_1 = 0x41;UBR1_1 = 0x03;UMCTL1 = 0x00;
     
  //  UART3_ClearBuffer();
        
<<<<<<< HEAD
  //  ME2 |= UTXE1+URXE1;   //Ê¹ï¿½ï¿½UART3ï¿½ï¿½TXDï¿½ï¿½RXD  
 //   IE2 |= URXIE1+UTXIE1; //Ê¹ï¿½ï¿½UART3ï¿½ï¿½RXï¿½ï¿½TXï¿½Ð¶ï¿½  
    
 //   P3SEL |= BIT6;//ï¿½ï¿½ï¿½ï¿½P3.6ÎªUART3ï¿½ï¿½TXD 
 //   P3SEL |= BIT7;//ï¿½ï¿½ï¿½ï¿½P3.7ÎªUART3ï¿½ï¿½RXD
 //   P3DIR |= BIT6;//P3.6Îªï¿½ï¿½ï¿½ï¿½Ü½ï¿½    
  UCA3CTL1 |= UCSWRST;
  UCA3CTL1 |= UCSSEL1;   //smclk 1M 
  
  //104ï¿½ï¿½Ó¦9600ï¿½ï¿½ï¿½ï¿½ï¿½Ê£ï¿½52ï¿½ï¿½Ó¦19200? 8ï¿½ï¿½Ó¦115200
=======
  //  ME2 |= UTXE1+URXE1;   //Ê¹ÄÜUART3µÄTXDºÍRXD  
 //   IE2 |= URXIE1+UTXIE1; //Ê¹ÄÜUART3µÄRXºÍTXÖÐ¶Ï  
    
 //   P3SEL |= BIT6;//ÉèÖÃP3.6ÎªUART3µÄTXD 
 //   P3SEL |= BIT7;//ÉèÖÃP3.7ÎªUART3µÄRXD
 //   P3DIR |= BIT6;//P3.6ÎªÊä³ö¹Ü½Å    
  UCA3CTL1 |= UCSWRST;
  UCA3CTL1 |= UCSSEL1;   //smclk 1M 
  
  //104¶ÔÓ¦9600²¨ÌØÂÊ£¬52¶ÔÓ¦19200? 8¶ÔÓ¦115200
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
  UCA3BR0 = 104;
  UCA3BR1 = 0;
  UCA3MCTL |= UCBRF_0+UCBRS_6;//
  //UCA1MCTL = 10;
  UCA3CTL1 &= ~UCSWRST;
  
  UART3_ClearBuffer();
  /********************/
  P10DIR |= TXD3;
  P10SEL |= TXD3 + RXD3;
  
  /*2418 UC1IE UCA1RXIE 5438 UCA1IE UCRXIE*/
  UCA3IE |= UCRXIE;
}
void UART3_Close()
{ 
<<<<<<< HEAD
   //ï¿½Ø±ï¿½RS232ï¿½ï¿½Æ½×ªï¿½ï¿½ï¿½ï¿½Â·
=======
   //¹Ø±ÕRS232µçÆ½×ª»»µçÂ·
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
  //P4DIR |= BIT0;
  //P4OUT &= ~BIT0; 
   
   UART3_ClearBuffer(); 
<<<<<<< HEAD
   //ï¿½Ø±Õ´ï¿½ï¿½ï¿½1
=======
   //¹Ø±Õ´®¿Ú1
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
   
/*2418 UC1IE UCA1RXIE 5438 UCA1IE UCRXIE*/
   UCA3IE &= ~UCRXIE;	 

}

void UART3_ClearBuffer()
{
<<<<<<< HEAD
    DownInt();//ï¿½ï¿½ï¿½Ð¶ï¿½
=======
    DownInt();//¹ØÖÐ¶Ï
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
    
    UART3_Tx_Buf=0;
    UART3_Rx_INTIndex=0;
    UART3_Rx_INTLen=0;
    UART3_Rx_RecvIndex=0;
  
    UART3_Tx_Flag=0;
    UART3_Tx_Len=0; 
    
    for(int i=0;i<UART3_MAXIndex;++i){
      UART3_Rx_BufLen[i]=0;
    }
    
<<<<<<< HEAD
    UpInt();//ï¿½ï¿½ï¿½Ð¶ï¿½
=======
    UpInt();//¿ªÖÐ¶Ï
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
}
int  UART3_Send(char * _data ,int _len, int _CR)
{

<<<<<<< HEAD
    if(UART3_Tx_Flag!=0)//ï¿½È´ï¿½ï¿½ï¿½Ò»ï¿½Î·ï¿½ï¿½Í½ï¿½ï¿½ï¿½
    {//ï¿½Íµï¿½500ms 
        System_Delayms(500);
   
        UART3_Tx_Flag=0;//Ç¿ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Îª0;
    }
    if(_len>1)
    {
        //ï¿½ï¿½È«ï¿½Ö±ï¿½ï¿½ï¿½ï¿½ï¿½Öµ
        UART3_Tx_Buf=_data; //ï¿½ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ð¶ï¿½ ï¿½ï¿½ï¿½ï¿½Îª0
        UART3_Tx_Len=_len; //ï¿½ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ð¶ï¿½ ï¿½ï¿½ï¿½ï¿½Îª0,ï¿½à·¢ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
        UART3_Tx_Flag=1; //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ï¿½ï¿½ï¿½Ýµï¿½ï¿½Ð¶ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Îª0ï¿½ï¿½  
=======
    if(UART3_Tx_Flag!=0)//µÈ´ýÉÏÒ»´Î·¢ËÍ½áÊø
    {//¾ÍµÈ500ms 
        System_Delayms(500);
   
        UART3_Tx_Flag=0;//Ç¿ÖÆÉèÖÃÎª0;
    }
    if(_len>1)
    {
        //¸øÈ«¾Ö±äÁ¿¸³Öµ
        UART3_Tx_Buf=_data; //ÓÉ×îºóÒ»´ÎÊý¾ÝÖÐ¶Ï ÉèÖÃÎª0
        UART3_Tx_Len=_len; //ÓÉ×îºóÒ»´ÎÊý¾ÝÖÐ¶Ï ÉèÖÃÎª0,¶à·¢×îºóÒ»¸ö½áÊø·ûºÅ
        UART3_Tx_Flag=1; //Õâ¸ö±äÁ¿×îºóÓÉ×îºóÒ»¸öÊý¾ÝµÄÖÐ¶ÏÖØÐÂÉèÖÃÎª0£»  
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
       for(int i=0;i<UART3_Tx_Len;i++)
       {
         
         /*2418 UC1IFG UCA1TXIFG 5438 UCA1IFG UCTXIFG*/
        while (!(UCA3IFG&UCTXIFG));
            UCA3TXBUF =_data[i];
       }
        UART3_Tx_Flag=0;
    }
<<<<<<< HEAD
    if(_len==1)//1ï¿½ï¿½ï¿½Ö·ï¿½ï¿½ï¿½Ê±ï¿½ï¿½ ï¿½ï¿½ï¿½Ð¶Ï·ï¿½ï¿½ï¿½ ï¿½Þ·ï¿½ï¿½É¹ï¿½.ï¿½ï¿½Ê±ï¿½È»ï¿½ï¿½É²ï¿½Ñ¯ï¿½ï¿½ï¿½ï¿½.ï¿½Ôºï¿½ï¿½ï¿½Ð¾ï¿½
=======
    if(_len==1)//1¸ö×Ö·ûµÄÊ±ºò ÓÃÖÐ¶Ï·¢ËÍ ÎÞ·¨³É¹¦.ÔÝÊ±ÏÈ»»³É²éÑ¯·¢ËÍ.ÒÔºó´ýÑÐ¾¿
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
    {
      /*2418 UC1IFG UCA1TXIFG 5438 UCA1IFG UCTXIFG*/
        while (!(UCA3IFG&UCTXIFG));
        UCA3TXBUF = _data[0];
        
    }
    if(_CR)
<<<<<<< HEAD
    {//ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
=======
    {//²¹·¢Ò»¸ö»»ÐÐ
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
      /*2418 UC1IFG UCA1TXIFG 5438 UCA1IFG UCTXIFG*/
        while (!(UCA3IFG&UCTXIFG));
        UCA3TXBUF=13;
        /*2418 UC1IFG UCA1TXIFG 5438 UCA1IFG UCTXIFG*/
        while (!(UCA3IFG&UCTXIFG));
        UCA3TXBUF=10;
        
    }
    if(_CR)
<<<<<<< HEAD
    {//ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
=======
    {//²¹·¢Ò»¸ö»»ÐÐ
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
      /*2418 UC1IFG UCA1TXIFG 5438 UCA1IFG UCTXIFG*/
        while (!(UCA3IFG&UCTXIFG));
        UCA3TXBUF=13;
        /*2418 UC1IFG UCA1TXIFG 5438 UCA1IFG UCTXIFG*/
        while (!(UCA3IFG&UCTXIFG));
        UCA3TXBUF=10;
        
    }
<<<<<<< HEAD


    PT_IODev  ptDevBle =  getIODev();
    if( ptDevBle->isinit() ){
        if( ptDevBle->isspp() ){
            ptDevBle->read(_data,_len);
            if(_CR)//ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
            {
                System_Delayms(1000);    //ptDevBle->readï¿½Ä¼ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ì«ï¿½Ì£ï¿½esp32ï¿½á·´Ó¦ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
                ptDevBle->read("\r\n",2);
            }
        }
    }
    
    return 0;
}

//ï¿½ï¿½ï¿½ï¿½
=======
    return 0;
}

//µ÷ÊÔ
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
int  UART3_SendtoInt(int num)
{
/*2418 UC1IFG UCA1TXIFG 5438 UCA1IFG UCTXIFG*/
        while (!(UCA3IFG&UCTXIFG));
            UCA3TXBUF =num;
          System_Delayms(2000);
    return 0;
}

int  UART3_RecvLine(char * _dest ,int _max, int * _pNum)
{
    int i=0;
<<<<<<< HEAD
    //ï¿½Ã¶ï¿½ï¿½ï¿½Î»ï¿½Ã³ï¿½ï¿½ï¿½Îª0, ï¿½ï¿½Ñ­ï¿½ï¿½ï¿½È´ï¿½ 
    while(UART3_Rx_BufLen[UART3_Rx_RecvIndex]==0);
    //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ë£ï¿½ï¿½Í°ï¿½ï¿½ï¿½ï¿½Ý¸ï¿½ï¿½Æ³ï¿½ï¿½ï¿½, 
=======
    //¸Ã¶ÁµÄÎ»ÖÃ³¤¶ÈÎª0, ÔòÑ­»·µÈ´ý 
    while(UART3_Rx_BufLen[UART3_Rx_RecvIndex]==0);
    //ÓÐÊý¾ÝÁË£¬¾Í°ÑÊý¾Ý¸´ÖÆ³öÀ´, 
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
    for(i=0; ( i< _max) && ( i<UART3_Rx_BufLen[UART3_Rx_RecvIndex]); ++i)
    {
        _dest[i]=UART3_Rx_Buffer[UART3_Rx_RecvIndex][i];
    }
    *_pNum = UART3_Rx_BufLen[UART3_Rx_RecvIndex];
<<<<<<< HEAD
    //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ïºï¿½,ï¿½Í°Ñ¸ï¿½Î»ï¿½ÃµÄ³ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Îª0,ï¿½ï¿½ï¿½ï¿½ï¿½Ð¶Ï¿ï¿½ï¿½Ô¸ï¿½ï¿½ï¿½ï¿½ï¿½.
    UART3_Rx_BufLen[UART3_Rx_RecvIndex]=0;
    //ï¿½ï¿½Î»ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½
    // ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½9 ï¿½Í¼ï¿½È¥9 ,ï¿½Óµï¿½Ò»ï¿½Ð¿ï¿½Ê¼,ï¿½ï¿½ï¿½ï¿½Í¼ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½.
=======
    //¸´ÖÆÍê±Ïºó,¾Í°Ñ¸ÃÎ»ÖÃµÄ³¤¶ÈÉèÖÃÎª0,¸æËßÖÐ¶Ï¿ÉÒÔ¸²¸ÇÁË.
    UART3_Rx_BufLen[UART3_Rx_RecvIndex]=0;
    //¶¨Î»µ½ÏÂÒ»ÐÐ
    // Èç¹û µÈÓÚ9 ¾Í¼õÈ¥9 ,´ÓµÚÒ»ÐÐ¿ªÊ¼,·ñÔò¾Í¼ÌÐø×ÔÔö.
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
    if( UART3_Rx_RecvIndex >= UART3_MAXIndex -1)
        UART3_Rx_RecvIndex=0;
    else
        ++UART3_Rx_RecvIndex;
    return 0;
}

int  UART3_RecvLineTry(char * _dest,const int _max, int * _pNum)
{
    int i=0;
<<<<<<< HEAD
    //ï¿½Ã¶ï¿½ï¿½ï¿½Î»ï¿½Ã³ï¿½ï¿½ï¿½Îª0, ï¿½ï¿½Ñ­ï¿½ï¿½ï¿½È´ï¿½ 
=======
    //¸Ã¶ÁµÄÎ»ÖÃ³¤¶ÈÎª0, ÔòÑ­»·µÈ´ý 
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad

    if(UART3_Rx_BufLen[UART3_Rx_RecvIndex]==0)
    {
        return -1;
    }

    TraceInt4(UART3_Rx_RecvIndex,1);
    TraceInt4(UART3_Rx_BufLen[UART3_Rx_RecvIndex],1);
<<<<<<< HEAD
    //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ë£ï¿½ï¿½Í°ï¿½ï¿½ï¿½ï¿½Ý¸ï¿½ï¿½Æ³ï¿½ï¿½ï¿½, 
=======
    //ÓÐÊý¾ÝÁË£¬¾Í°ÑÊý¾Ý¸´ÖÆ³öÀ´, 
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
    for(i=0; ( i< _max) && ( i<UART3_Rx_BufLen[UART3_Rx_RecvIndex]); ++i)
    {
        _dest[i]=UART3_Rx_Buffer[UART3_Rx_RecvIndex][i];
    }
    *_pNum =UART3_Rx_BufLen[UART3_Rx_RecvIndex];
<<<<<<< HEAD
    //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ïºï¿½,ï¿½Í°Ñ¸ï¿½Î»ï¿½ÃµÄ³ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Îª0,ï¿½ï¿½ï¿½ï¿½ï¿½Ð¶Ï¿ï¿½ï¿½Ô¸ï¿½ï¿½ï¿½ï¿½ï¿½.
    UART3_Rx_BufLen[UART3_Rx_RecvIndex]=0;
    //ï¿½ï¿½Î»ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½
    // ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½9 ï¿½Í¼ï¿½È¥9 ,ï¿½Óµï¿½Ò»ï¿½Ð¿ï¿½Ê¼,ï¿½ï¿½ï¿½ï¿½Í¼ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½.
=======
    //¸´ÖÆÍê±Ïºó,¾Í°Ñ¸ÃÎ»ÖÃµÄ³¤¶ÈÉèÖÃÎª0,¸æËßÖÐ¶Ï¿ÉÒÔ¸²¸ÇÁË.
    UART3_Rx_BufLen[UART3_Rx_RecvIndex]=0;
    //¶¨Î»µ½ÏÂÒ»ÐÐ
    // Èç¹û µÈÓÚ9 ¾Í¼õÈ¥9 ,´ÓµÚÒ»ÐÐ¿ªÊ¼,·ñÔò¾Í¼ÌÐø×ÔÔö.
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
    if( UART3_Rx_RecvIndex >= UART3_MAXIndex -1)
        UART3_Rx_RecvIndex=0;
    else
        ++UART3_Rx_RecvIndex;
    return 0;
}
int  UART3_RecvLineWait(char *_dest ,const int _max, int * _pNum)
{
    int i=0; 
<<<<<<< HEAD
    //ï¿½Ã¶ï¿½ï¿½ï¿½Î»ï¿½Ã³ï¿½ï¿½ï¿½Îª0, ï¿½ï¿½Ñ­ï¿½ï¿½ï¿½È´ï¿½ 
=======
    //¸Ã¶ÁµÄÎ»ÖÃ³¤¶ÈÎª0, ÔòÑ­»·µÈ´ý 
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
    while(UART3_Rx_BufLen[UART3_Rx_RecvIndex]==0)
    {
        System_Delayms(30);
        ++i;
        if(i>10) 
            return -1;
    }
<<<<<<< HEAD
    //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ë£ï¿½ï¿½Í°ï¿½ï¿½ï¿½ï¿½Ý¸ï¿½ï¿½Æ³ï¿½ï¿½ï¿½, 
=======
    //ÓÐÊý¾ÝÁË£¬¾Í°ÑÊý¾Ý¸´ÖÆ³öÀ´, 
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
    for(i=0; ( i< _max) && ( i<UART3_Rx_BufLen[UART3_Rx_RecvIndex]); ++i)
    {
        _dest[i]=UART3_Rx_Buffer[UART3_Rx_RecvIndex][i];
    }
    *_pNum = UART3_Rx_BufLen[UART3_Rx_RecvIndex];
<<<<<<< HEAD
    //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ïºï¿½,ï¿½Í°Ñ¸ï¿½Î»ï¿½ÃµÄ³ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Îª0,ï¿½ï¿½ï¿½ï¿½ï¿½Ð¶Ï¿ï¿½ï¿½Ô¸ï¿½ï¿½ï¿½ï¿½ï¿½.
    UART3_Rx_BufLen[UART3_Rx_RecvIndex]=0;
    //ï¿½ï¿½Î»ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½
    // ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½9 ï¿½Í¼ï¿½È¥9 ,ï¿½Óµï¿½Ò»ï¿½Ð¿ï¿½Ê¼,ï¿½ï¿½ï¿½ï¿½Í¼ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½.
=======
    //¸´ÖÆÍê±Ïºó,¾Í°Ñ¸ÃÎ»ÖÃµÄ³¤¶ÈÉèÖÃÎª0,¸æËßÖÐ¶Ï¿ÉÒÔ¸²¸ÇÁË.
    UART3_Rx_BufLen[UART3_Rx_RecvIndex]=0;
    //¶¨Î»µ½ÏÂÒ»ÐÐ
    // Èç¹û µÈÓÚ9 ¾Í¼õÈ¥9 ,´ÓµÚÒ»ÐÐ¿ªÊ¼,·ñÔò¾Í¼ÌÐø×ÔÔö.
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
    if( UART3_Rx_RecvIndex >= UART3_MAXIndex -1)
        UART3_Rx_RecvIndex=0;
    else
        ++UART3_Rx_RecvIndex;
    return 0;
}
int  UART3_RecvLineLongWait(char *_dest,int _max, int * _pNum)
{
    int i=0;
<<<<<<< HEAD
    //ï¿½Ã¶ï¿½ï¿½ï¿½Î»ï¿½Ã³ï¿½ï¿½ï¿½Îª0, ï¿½ï¿½Ñ­ï¿½ï¿½ï¿½È´ï¿½ 
    while(UART3_Rx_BufLen[UART3_Rx_RecvIndex]==0)
    {//ï¿½È´ï¿½5ï¿½ï¿½.
=======
    //¸Ã¶ÁµÄÎ»ÖÃ³¤¶ÈÎª0, ÔòÑ­»·µÈ´ý 
    while(UART3_Rx_BufLen[UART3_Rx_RecvIndex]==0)
    {//µÈ´ý5Ãë.
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
        System_Delayms(50);
        ++i;
        if(i>100)
            return -1;        
    }
<<<<<<< HEAD
    //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ë£ï¿½ï¿½Í°ï¿½ï¿½ï¿½ï¿½Ý¸ï¿½ï¿½Æ³ï¿½ï¿½ï¿½, 
=======
    //ÓÐÊý¾ÝÁË£¬¾Í°ÑÊý¾Ý¸´ÖÆ³öÀ´, 
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
    for(i=0; ( i< _max) && ( i<UART3_Rx_BufLen[UART3_Rx_RecvIndex]); ++i)
    {
        _dest[i]=UART3_Rx_Buffer[UART3_Rx_RecvIndex][i];
    }
    *_pNum = UART3_Rx_BufLen[UART3_Rx_RecvIndex];
<<<<<<< HEAD
    //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ïºï¿½,ï¿½Í°Ñ¸ï¿½Î»ï¿½ÃµÄ³ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Îª0,ï¿½ï¿½ï¿½ï¿½ï¿½Ð¶Ï¿ï¿½ï¿½Ô¸ï¿½ï¿½ï¿½ï¿½ï¿½.
    UART3_Rx_BufLen[UART3_Rx_RecvIndex]=0;
    //ï¿½ï¿½Î»ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½
    // ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½9 ï¿½Í¼ï¿½È¥9 ,ï¿½Óµï¿½Ò»ï¿½Ð¿ï¿½Ê¼,ï¿½ï¿½ï¿½ï¿½Í¼ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½.
=======
    //¸´ÖÆÍê±Ïºó,¾Í°Ñ¸ÃÎ»ÖÃµÄ³¤¶ÈÉèÖÃÎª0,¸æËßÖÐ¶Ï¿ÉÒÔ¸²¸ÇÁË.
    UART3_Rx_BufLen[UART3_Rx_RecvIndex]=0;
    //¶¨Î»µ½ÏÂÒ»ÐÐ
    // Èç¹û µÈÓÚ9 ¾Í¼õÈ¥9 ,´ÓµÚÒ»ÐÐ¿ªÊ¼,·ñÔò¾Í¼ÌÐø×ÔÔö.
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
    if( UART3_Rx_RecvIndex >= UART3_MAXIndex -1)
        UART3_Rx_RecvIndex=0;
    else
        ++UART3_Rx_RecvIndex;
    return 0;
}

//void Judge_Watermeter()
//{
//    if(UART3_Rx_INTLen == 9)
//    {
//        UART3_Rx_BufLen[UART3_Rx_INTIndex] = UART3_Rx_INTLen;
//        UART3_Rx_INTLen=0;
//        if(UART3_Rx_INTIndex >= UART3_MAXIndex-1)
//            UART3_Rx_INTIndex=0;
//        else
//            ++UART3_Rx_INTIndex;
//    }
//}


//
<<<<<<< HEAD
//    ï¿½ï¿½ï¿½Üµï¿½Ò»ï¿½ï¿½ï¿½Ö·ï¿½.
//    
//    ×°ï¿½Øµï¿½UART3_Rx_Buffer[UART3_Rx_INTIndex][UART3_Rx_INTLen]ï¿½ï¿½.
//    ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½UART3_Rx_INTLen ;ï¿½ï¿½ï¿½UART3_Rx_INTLenï¿½Ñ¾ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ö·ï¿½Îªï¿½ï¿½ï¿½ï¿½
//    ï¿½ï¿½Ð´ï¿½ï¿½ï¿½Ð»ï¿½ï¿½ï¿½Ä³ï¿½ï¿½ï¿½Îª UART3_Rx_INTLen+1;
//    ï¿½ï¿½ï¿½ï¿½UART3_Rx_INTIndex,Ö¸ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½. ï¿½ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Î´ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½.
//    ï¿½ï¿½Ã´ï¿½Í²ï¿½ï¿½ï¿½ï¿½ï¿½,ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ð´ï¿½ï¿½Ç°ï¿½Ä»ï¿½ï¿½ï¿½ï¿½ï¿½.
=======
//    ½ÓÊÜµ½Ò»¸ö×Ö·û.
//    
//    ×°ÔØµ½UART3_Rx_Buffer[UART3_Rx_INTIndex][UART3_Rx_INTLen]ÖÐ.
//    ²¢µÝÔöUART3_Rx_INTLen ;Èç¹ûUART3_Rx_INTLenÒÑ¾­ÊÇ×îºóÒ»¸ö»òÕß×Ö·ûÎª»»ÐÐ
//    ÌîÐ´±¾ÐÐ»º´æµÄ³¤¶ÈÎª UART3_Rx_INTLen+1;
//    µÝÔöUART3_Rx_INTIndex,Ö¸ÏòÏÂÒ»¸ö»º³åÇø. Èç¹ûÏÂÒ»¸ö»º´æÇøµÄÊý¾ÝÈÔÎ´±»´¦Àí.
//    ÄÇÃ´¾Í²»µÝÔö,¸²¸ÇÌîÐ´µ±Ç°µÄ»º³åÇø.
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
//    
//    
//    
/*************VECTOR*/
#pragma vector=USCI_A3_VECTOR 
<<<<<<< HEAD
__interrupt void UART3_RX_ISR(void)   //ï¿½ï¿½ï¿½ï¿½ï¿½Õµï¿½ï¿½ï¿½ï¿½Ö·ï¿½ï¿½ï¿½Ê¾ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
=======
__interrupt void UART3_RX_ISR(void)   //½«½ÓÊÕµ½µÄ×Ö·ûÏÔÊ¾µ½´®¿ÚÊä³ö
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
{
   //_DINT();
   char _temp; 
      //char *tbuffer;  
   _temp = UCA3RXBUF;
#if 1

    UART3_Rx_Buffer[UART3_Rx_INTIndex][UART3_Rx_INTLen]=_temp;
    ++UART3_Rx_INTLen;
    
    if(s_uart3_type == UART3_CONSOLE_TYPE)
    {   
      
        if(((_temp==0x0A) && (UART3_Rx_INTLen!=0) && (UART3_Rx_Buffer[UART3_Rx_INTIndex][UART3_Rx_INTLen-2]==0x0D)) || (_temp == ')'))
        {
<<<<<<< HEAD
            //ï¿½ï¿½ï¿½ï¿½ï¿½Í·ï¿½ï¿½ï¿½Õµï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ð·ï¿½ï¿½ï¿½,Ö±ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
            if(UART3_Rx_INTLen==1)
            {
                UART3_Rx_INTLen=0; //ï¿½ï¿½ï¿½Â¿ï¿½Ê¼ï¿½ï¿½ï¿½ï¿½ 
=======
            //Èç¹ûÊÇÍ·²¿ÊÕµ½µÄÕâ¸ö»»ÐÐ·ûºÅ,Ö±½ÓÅ×Æú
            if(UART3_Rx_INTLen==1)
            {
                UART3_Rx_INTLen=0; //ÖØÐÂ¿ªÊ¼½ÓÊÕ 
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
                return ;
            }
            else
            {
<<<<<<< HEAD
                //   ï¿½ï¿½Î»ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ 
                //UART3_Rx_Buffer[UART3_Rx_INTIndex][UART3_Rx_INTLen-1]=13; //ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ã¸ï¿½13
                UART3_Rx_BufLen[UART3_Rx_INTIndex] = UART3_Rx_INTLen - 2;//ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ð·ï¿½
=======
                //   ¶¨Î»µ½ÏÂÒ»ÐÐ 
                //UART3_Rx_Buffer[UART3_Rx_INTIndex][UART3_Rx_INTLen-1]=13; //Ëæ±ãÉèÖÃ¸ö13
                UART3_Rx_BufLen[UART3_Rx_INTIndex] = UART3_Rx_INTLen - 2;//²»°üÀ¨»»ÐÐ·û
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
                UART3_Rx_INTLen=0;
                if(UART3_Rx_INTIndex >= UART3_MAXIndex-1)
                    UART3_Rx_INTIndex=0;
                else
                    ++UART3_Rx_INTIndex;
                //UART3_Rx_INTIndex += UART3_Rx_INTIndex < (UART3_MAXIndex - 1) ? 1 : 1-UART3_MAXIndex;  
                return ;
            }
        }
    }
    else
    {
        Judge_Watermeter();
        return ;
    }
    
//    if(_temp==0x0A)
//        return ;
//
//    if((_temp==0x0D) && (UART3_Rx_INTLen!=0) && UART3_Rx_Buffer[UART3_Rx_INTIndex][UART3_Rx_INTLen-1]==0x0D)
//    {
//        return;
//    }
    
    if(UART3_Rx_INTLen >= UART3_MAXBUFFLEN-1)
<<<<<<< HEAD
    {//ï¿½Ð³ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½, ï¿½ï¿½ï¿½ï¿½Ö±ï¿½Ó½Ø¶Ï³ï¿½Ò»ï¿½ï¿½.
=======
    {//ÐÐ³¤¶ÈÂú³öÁË, ÎÒÃÇÖ±½Ó½Ø¶Ï³ÉÒ»ÐÐ.
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
        UART3_Rx_BufLen[UART3_Rx_INTIndex] = UART3_Rx_INTLen + 1;
        UART3_Rx_INTLen=0;
        if(UART3_Rx_INTIndex >= UART3_MAXIndex-1)
            UART3_Rx_INTIndex=0;
        else
            ++UART3_Rx_INTIndex;
    }
    
<<<<<<< HEAD
    //ï¿½Ð¶Ï»ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ç·ï¿½ï¿½ï¿½ï¿½ï¿½:UART3_Rx_INTIndexï¿½ï¿½Â¼ï¿½Â´Î±ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Â´ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ñ¾ï¿½ï¿½Ð´æ´¢ï¿½ï¿½Ëµï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
    if( UART3_Rx_BufLen[UART3_Rx_INTIndex]!=0)
    {
        //ï¿½ï¿½Ò»ï¿½Ð»ï¿½Î´ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ç¾Í¸ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ò»ï¿½ï¿½
=======
    //ÅÐ¶Ï»º³åÇøÊÇ·ñÒÑÂú:UART3_Rx_INTIndex¼ÇÂ¼ÏÂ´Î±£´æµÄÐÐÊý£¬ÏÂ´ÎÐÐÊýÒÑ¾­ÓÐ´æ´¢£¬ËµÃ÷»º³åÆ÷ÂúÁË
    if( UART3_Rx_BufLen[UART3_Rx_INTIndex]!=0)
    {
        //ÏÂÒ»ÐÐ»¹Î´±»´¦Àí£¬ÄÇ¾Í¸²¸Ç×îºó´¦ÀíµÄÕâÒ»ÐÐ
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
        if(UART3_Rx_INTIndex <= 0)
            UART3_Rx_INTIndex = UART3_MAXIndex-1;
        else
            --UART3_Rx_INTIndex;
<<<<<<< HEAD
        //ï¿½Ñ¸ï¿½ï¿½Ð³ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Îª0,ï¿½ï¿½ï¿½Ð¶ï¿½Õ¼ï¿½ï¿½
=======
        //°Ñ¸ÃÐÐ³¤¶ÈÉèÖÃÎª0,ÓÉÖÐ¶ÏÕ¼ÓÃ
>>>>>>> c6add1162a097a95545c7c072c4a341bef9737ad
        UART3_Rx_BufLen[UART3_Rx_INTIndex]=0;
    }

 #endif
}


int putchar(int c)
{
    while (!(UCA3IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
    UCA3TXBUF = (unsigned char)c; 
  
    return c;
}