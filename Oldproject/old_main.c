#include "BC95.h"
#include "DTU.h"
#include "Sampler.h"
#include "common.h"
#include "console.h"
#include "hydrologycommand.h"
#include "hydrologytask.h"
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

int IsDebug = 0;

int main(void) {
    Restart_Init();

    Select_Debug_Mode(0);

    TraceOpen();

    TraceMsg("Device Open !", 1);

    Main_Init();

    Sampler_Open();

    Hydrology_InitWaitConfig();

    HydrologyUpdateElementTable();   //����Ҫ�ر�

    HydrologyDataPacketInit();   //����Ҫ�ر����DP�Ĵ�С�ͳ�ʼ��

    while (1) {
        HydrologyTask();
    }
}

void Restart_Init() {
    P1SEL = 0x00;   //�ر�����˿��ж�
    P1DIR = 0x00;
    Clock_Init();   // CPUʱ�ӳ�ʼ��

    _EINT();   //���ж�
    // IE1|=OFIE+NMIIE+ACCVIE;
    // //���������ж�������NMI�ж�������FLASH�洢���Ƿ������ж�����
    SFRIE1 |= OFIE + NMIIE + ACCVIE;
    WatchDog_Init();
    Led_Init();   // ָʾ�� ��ʼ��
                  //    Led1_WARN();

    //    P9DIR |= BIT7;              //ly ����232оƬ��������
    //    P9OUT |= BIT7;

    TimerA_Init(30720);   // ϵͳ��ʱ����ʼ��
    TimerB_Init(61440);

    Store_Init();   // ��ʼ��ROM
    RTC_Open();
    Sampler_Init();   //�˿ڳ�ʼ��,��Ҫ�ȳ�ʼ��Store

    P10DIR |= BIT0;   // ly P100���ߣ�uart3���ڵ��ԣ��͵Ļ�P104��,105����485��
    P10OUT |= BIT0;

    P10DIR |=
        BIT1;   // ly P101���ߣ�uart1 P104��,105���͵Ļ�����485��,�ߵĻ���������
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
            if (GSM_CheckOK() <
                0) {   //�޷������ͷ�����.
                GSM_Close(1);
                return -1;
            }
        }
    }

    char _phone[12];
    char _data[30];   // 30�㹻��

    _data[0] = '$';
    if (Store_ReadDeviceNO(&_data[1]) < 0) {
        _data[1]  = '0';
        _data[2]  = '0';
        _data[3]  = '0';
        _data[4]  = '0';
        _data[5]  = '0';
        _data[6]  = '0';
        _data[7]  = '0';
        _data[8]  = '0';
        _data[9]  = '0';
        _data[10] = '0';
        _data[11] = '0';
    }

    Utility_Strncpy(&_data[12], "<restart#", 9);
    if (Store_GSM_ReadCenterPhone(5, _phone) == 0) {
        if (_phone[0] != '0' && _phone[1] != '0' && _phone[2] != '0') {
            GSM_SendMsgTxt(_phone, _data, 21);
        }
    }
    GSM_Process(1, 0);   // GSM ������
    GSM_Close(0);        //�ر� GSM
    System_Delayms(2000);   //����ػ�����쿪������ʧ��.
    return 0;
}

int Restart_DTUInit() {
    char _data[UART3_MAXBUFFLEN];
    int _dataLen = 0;
    int _repeats = 0;
    int _out     = 0;
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
        if (Main_ProcCommand(_data, _dataLen, NULL) ==
            3) {   //������һ��SYN,�ͽ�������״̬
            while (1) {
                _repeats = 0;
                while (UART3_RecvLineLongWait(_data, UART3_MAXBUFFLEN,
                                              &_dataLen) < 0) {
                    ++_repeats;
                    if (_repeats > 4) {
                        _out = 1;
                        break;
                    }
                }
                if (_out != 0)
                    break;
                if (Main_ProcCommand(_data, _dataLen, NULL) ==
                    2) {   //��ʾ�˳�����״̬
                    break;
                }
            }
        }
    }
    //�����ڵ���״̬��ʱ��Ҫ�رյ�
    if (trace_open == 0) {   //��������ǹرյ�,���ڵ���Ҫ�ر�
                             // Console_Close();
    }

    return 0;
}

int WorkMode_Init(char *ptype) {
    char _curType, _selType;
    int _ret;

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
    _selType = 'S';   // GPRSģʽ GTM900
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
    if (Store_ReadSystemType(&_curType) < 0) {   //����޷�����
                                                 //�����ģʽ,�Ͳ��ж��Ƿ� ����������.
        _ret = 0;
    } else {
        _ret = _selType != _curType ? 1 : 0;
    }

    *ptype = _selType;

    return _ret;
}

void Main_Init() {   //ʱ��ʹ��8M
    //���ж�,�Է�֮ǰ�����д������жϱ��ر�
    EnInt();
    TraceOpen();
    RTC_Open();   // ��RTC
    TimerB_Clear();
    WatchDog_Clear();   // �����λ������
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
