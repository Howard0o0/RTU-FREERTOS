//////////////////////////////////////////////////////
//     �ļ���: rtc.h
//   �ļ��汾: 1.0.0
//   ����ʱ��: 09��11��30��
//   ��������:  
//       ����: ����
//       ��ע: ��
//
//////////////////////////////////////////////////////

#ifndef __RTC_h
#define __RTC_h

#include "msp430common.h"

 
#define RTC_RST   BIT4   //P44
 
//   RAM ����ȫ
//   0  1  2  3  4  CheckTime
//   5  6  7  8  9  SaveTime
//  10 11 12 13 14  ReportTime
//        15        StartIdx  
//        16        EndIdx
//    17 18 19      PULSE1_BYTES
//    20 21 22      PULSE2_BYTES
//    23 24 25      PULSE3_BYTES
//    26 27 28      PULSE4_BYTES
//    

#define CHECKTIME_ADDR 0
#define SAVETIME_ADDR 5
#define REPORTTIME_ADDR 10
#define STARTIDX_ADDR 15
#define ENDIDX_ADDR 16

#define PULSE1_BYTES 17
#define PULSE1_BYTE1 17
#define PULSE1_BYTE2 18
#define PULSE1_BYTE3 19

#define PULSE2_BYTES 20
#define PULSE2_BYTE1 20
#define PULSE2_BYTE2 21
#define PULSE2_BYTE3 22

#define PULSE3_BYTES 23
#define PULSE3_BYTE1 23
#define PULSE3_BYTE2 24
#define PULSE3_BYTE3 25

#define PULSE4_BYTES 26
#define PULSE4_BYTE1 26
#define PULSE4_BYTE2 27
#define PULSE4_BYTE3 28

extern char g_rtc_nowTime[5];
extern char rtc_nowTime[ 6 ];
extern char s_RTC_CheckTime[5];
extern char s_RTC_SaveTime[5];
extern char s_RTC_ReportTime[5];
extern char s_RTC_StartIdx;
extern char s_RTC_EndIdx;
//���ݱ�ǩ
int RTC_RetrieveIndex();
int RTC_ReadStartIdx(char * _dest); 
int RTC_SetStartIdx(const char _src);
int RTC_ReadEndIdx(char * _dest);
int RTC_SetEndIdx(const char _src);

//���ʱ��
int RTC_ReadCheckTimeBytes(unsigned char index, char *dest);
int RTC_ReadCheckTimeStr5_A(char * _dest); 
int RTC_SetCheckTimeBytes(unsigned char index, const char * _src);
//����ʱ��
int RTC_ReadSaveTimeBytes(unsigned char index, char *dest);
int RTC_ReadSaveTimeStr5_A(char * _dest); 
int RTC_SetSaveTimeBytes(unsigned char index, const char * _src);
//����ʱ��
int RTC_ReadReportTimeBytes(unsigned char index, char *dest);
int RTC_ReadReportTimeStr5_A(char * _dest); 
int RTC_SetReportTimeBytes(unsigned char index, const char * _src); 

//����1
int RTC_ReadPulseBytes(int _index, char * _dest);
int RTC_SetPulseBytes(int _index, char * _src);
int RTC_IncPulseBytes(int _index);

int RTC_RetrievePulseBytes();
//
//  �߼�����
// 
////////////////////////////////////
//    ����: �ж�ʱ���Ƿ��Ѿ�����
//    ����: char *   ��ʽΪ������ʱ�ֵ�char[5]����
//  ����ֵ: 1 :  Ϊ��ȥʱ��
//          0 :  Ϊ����ʱ��
//         -1 :  Ϊδ��ʱ��  
//    ��ע: >>  ��ȷ����   
////////////////////////////////////
int  RTC_IsPassed(char * time);
int  RTC_IsBadTime(char * time, int RTCTime);
////////////////////////////////////
//    ����: ������ǰʱ�䴮
//    ����: char * д�뻺��
//  ����ֵ: ��
//    ��ע: >>   д���ʽ0903062300
////////////////////////////////////
void RTC_ReadTimeStr5_A(char * dest);
void RTC_ReadTimeStr5_B(char * dest);
void RTC_SetTimeStr5_B(const char *src);

void RTC_ReadTimeStr6_A(char * dest);
void RTC_ReadTimeStr6_B(char * dest);

void RTC_SetTimeStr6_A(const char * src);
void RTC_SetTimeStr6_B(const char *src);

 void RTC_SetTimeStr6_C(const char *src);

////////////////////////////////////
//    ����: ������ǰʱ��ֵ
//    ����: char * д�뻺��,char[5]
//  ����ֵ: ��
//    ��ע: >> ����д��������ʱ�ֵ���ֵ
//          >>��:[0]=9 [1]=3 [2]=6 [3]=23 [4]=0
////////////////////////////////////
void RTC_ReadTimeBytes5(char *dest); 
void RTC_ReadTimeBytes6(char *dest);  

////////////////////////////////////
//    ����: ���õ�ǰʱ��
//    ����: char * ʱ����ֵ����
//  ����ֵ: 0 : �ɹ�
//         -1 : ��������
//    ��ע: >> ���ζ�ȡ������ʱ�ֵ���ֵ
//          >>��:[0]=9 [1]=3 [2]=6 [3]=23 [4]=0
//          >> ����δʵ�� !!!
////////////////////////////////////
int RTC_SetTimeBytes5(const char *dest);
int RTC_SetTimeBytes6(const char *dest);
////////////////////////////////////
//    ����: ����Сʱ
//    ����: char  Сʱ��ֵ
//  ����ֵ: ��
//    ��ע: >>
////////////////////////////////////
//void _RTC_SetHour(const unsigned  char hour);

////////////////////////////////////
//    ����: ���÷���
//    ����: char  ������ֵ
//  ����ֵ: ��
//    ��ע: >>
////////////////////////////////////
//void _RTC_SetMinute(const unsigned  char minute);

////////////////////////////////////
//    ����: ��������
//    ����: char  ������ֵ
//  ����ֵ: ��
//    ��ע: >>
////////////////////////////////////
//void _RTC_SetSecond(const unsigned  char second);

////////////////////////////////////
//    ����: ��������
//    ����: char  ������ֵ
//  ����ֵ: ��
//    ��ע: >>
////////////////////////////////////
//void _RTC_SetDay(const unsigned char Day);

////////////////////////////////////
//    ����: ��������
//    ����: char  ������ֵ
//  ����ֵ: ��
//    ��ע: >>
////////////////////////////////////
//void _RTC_SetDate(const  unsigned char Date);

////////////////////////////////////
//    ����: �����·�
//    ����: char  �·���ֵ
//  ����ֵ: ��
//    ��ע: >>
////////////////////////////////////
//void _RTC_SetMonth(const unsigned  char month);

////////////////////////////////////
//    ����: �������
//    ����: char  �����ֵ
//  ����ֵ: ��
//    ��ע: >>
////////////////////////////////////
//void _RTC_SetYear(const unsigned  char year);

////////////////////////////////////
//    ����: һ��������ʱ��
//    ����: char ���εĸ���ʱ��Ĵ�����ֵ
//  ����ֵ: 0 : �ɹ�
//         -1 : ��������
//    ��ע: >>
////////////////////////////////////
int _RTC_SetTime(const char second,const char minute,
                 const char hour,const char date,const char month,
                 const char day,const char year,const char control);

////////////////////////////////////
//    ����: һ��������ʱ��
//    ����: char* ���ΰ�������ʱ��Ĵ���ֵ������
//  ����ֵ: 0 : �ɹ�
//         -1 : ��������
//    ��ע: >>
////////////////////////////////////
//int _RTC_SetRTCTimeBytes(const char * src);

////////////////////////////////////
//    ����: ��ȡСʱ
//    ����: ��
//  ����ֵ: Сʱֵ
//    ��ע: >>
////////////////////////////////////
//char _RTC_ReadHour();

////////////////////////////////////
//    ����: ��ȡ����
//    ����: ��
//  ����ֵ: ����ֵ
//    ��ע: >>
////////////////////////////////////
//char _RTC_ReadMinute();

////////////////////////////////////
//    ����: ��ȡ����
//    ����: ��
//  ����ֵ: ����ֵ
//    ��ע: >>
////////////////////////////////////
 char _RTC_ReadSecond();

////////////////////////////////////
//    ����: ��ȡ����
//    ����: ��
//  ����ֵ: ����ֵ
//    ��ע: >>
////////////////////////////////////
//char _RTC_ReadDay();

////////////////////////////////////
//    ����: ��ȡ����
//    ����: ��
//  ����ֵ: ����ֵ
//    ��ע: >>
////////////////////////////////////
//char _RTC_ReadDate();


////////////////////////////////////
//    ����: ��ȡ�·�
//    ����: ��
//  ����ֵ: �·�ֵ
//    ��ע: >>
////////////////////////////////////
//char _RTC_ReadMonth();

////////////////////////////////////
//    ����: ��ȡ���
//    ����: ��
//  ����ֵ: ���ֵ
//    ��ע: >>
////////////////////////////////////
//char _RTC_ReadYear();

////////////////////////////////////
//    ����: һ���Զ�ȡʱ��
//    ����: ����ʢװ�����Ĵ���ֵ�Ļ���
//  ����ֵ: ��
//    ��ע: >>
////////////////////////////////////
void _RTC_ReadTime(char *second,char *minute,
                 char *hour,char *date,char *month,
                 char *day, char *year,char *control);

////////////////////////////////////
//    ����: һ���Զ�ȡʱ��
//    ����: ����ʢװ�����Ĵ���ֵ�����黺��
//  ����ֵ: ��
//    ��ע: >>
////////////////////////////////////
//void  _RTC_ReadRTCTimeBytes(char * dest);


/*
 *  
 */
void _RTC_Init(void);

////////////////////////////////////
//    ����: ����RTC�ĵײ㺯��Ⱥ
//    ����: *****
//  ����ֵ: *****
//    ��ע: >> �����DS1302оƬ�ֲ�
////////////////////////////////////
//�м�����
int  _RTC_SendByte(char data);
char _RTC_RecvByte(void);
char _RTC_Read_OneByte(char regAddr);
void _RTC_Write_OneByte(char regAddr,char data);
int  _RTC_MultiRead(char cmd, char * dest, int length);
int  _RTC_MultiWrite(char cmd ,const char * src, int length);
void _RTC_DisableWrite();
void _RTC_EnableWrite();
void _RTC_Go();
void _RTC_Pause();
int  _RTC_Check();
// void _RTC_EnableCharge();
// void _RTC_DisableCharge(); 
//RAM
void _RTC_WriteRAM(unsigned char index,const char data);
char _RTC_ReadRAM(unsigned char index); 



#define CPU_CLOCK       8000000
#define Delay_us(us)    __delay_cycles(CPU_CLOCK/1000000*(us))
#define u8 unsigned char

//IO��������
#define SDA_IN()  {P9DIR &= ~(1<<1);}	//P9.1����ģʽ
#define SDA_OUT() {P9DIR |= (1<<1);} //P9.1���ģʽ

#define RTC_SCLK  BIT2  // p9.2
#define RTC_IO    BIT1  // p9.1

/* cmd */
#define WRITE_CMD          0xD0
#define READ_CMD           0xD1
/* base address in ROM */
#define  RTC_RAM_BASE       (long)524256


/* reg addr */
typedef enum{
  RegAddr_Sec,    // Seconds              00-59
  RegAddr_Min,    // Minutes              00-59
  RegAddr_Hour,   // Hours                1�C12 + AM/PM 00�C23
  RegAddr_Day,    // Day                  1 - 7
  RegAddr_Date,   // Data                 01-31
  RegAddr_CMon,   // Century and month    Century + 01�C12
  RegAddr_Year,   // Year                 00 - 99
  RegAddr_Sec_A1, // Alarm 1 Seconds      00-59
  RegAddr_Min_A1, // Alarm 1 Minutes      00-59
  RegAddr_Hour_A1,// Alarm 1 Hours        1�C12 + AM/PM 00�C23
  RegAddr_Da_A1,  // Alarm 1 Day/Date     1 - 7 / 1 �C 31
  RegAddr_Min_A2, // Alarm 2 Minutes      00-59
  RegAddr_Hour_A2,// Alarm 2 Hours        1�C12 + AM/PM 00�C23
  RegAddr_Da_A2,  // Alarm 2 Day/Date     1 - 7 / 1 �C 31
  RegAddr_Control,// Control
  RegAddr_CtlStat,// Control/Status
  RegAddr_AgOfset,// Aging offset
  RegAddr_TempMSB,// MSB of Temp
  RegAddr_TempLSB,// LSB of Temp
}DS3231REG_ADDR;

 
//��������
char _BCDtoDEC(char bcd);
char _DECtoBCD(char dec); 
//�ͼ�����
int  RTC_Open(); 
int  RTC_Close();
void _RTC_Set_RST_low();
void _RTC_Set_RST_high();
void _RTC_Set_SCLK_low();
void _RTC_Set_SCLK_high();
void _RTC_Set_IO_low();
void _RTC_Set_IO_high();
void _RTC_Set_IO_OUT();
void _RTC_Set_IO_IN(); 
void _RTC_delay();

//IIC���в�������
void IIC_SCL_HIGH(void);
void IIC_SCL_LOW(void);
void IIC_SDA_HIGH(void);
void IIC_SDA_LOW(void);
int READ_SDA(void);
void IIC_Init(void);                //��ʼ��IIC��IO��				 
void IIC_Start(void);				//����IIC��ʼ�ź�
void IIC_Stop(void);	  			//����IICֹͣ�ź�
void IIC_Send_Byte(u8 txd);			//IIC����һ���ֽ�
u8 IIC_Read_Byte(unsigned char ack);//IIC��ȡһ���ֽ�
u8 IIC_Wait_Ack(void); 				//IIC�ȴ�ACK�ź�
void IIC_Ack(void);					//IIC����ACK�ź�
void IIC_NAck(void);				//IIC������ACK�ź� 

  
#endif