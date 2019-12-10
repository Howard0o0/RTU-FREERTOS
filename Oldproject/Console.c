//////////////////////////////////////////////////////
//     �ļ���: Console.c
//   �ļ��汾: 1.0.0  
//   ����ʱ��: 09�� 11��3��
//   ��������:  
//       ����: ����
//       ��ע: 
// 
//
//////////////////////////////////////////////////////
#include "msp430common.h"
#include "Console.h"
#include "Common.h"
#include "uart3.h"
#include "uart0.h"
#include "main.h"
#include "wifi_config.h"
#include <string.h>
#include "stdio.h"
#include "FreeRTOS.h"

static int Opened=0;//�Ƿ��

extern int g_wifi_connetflag;
extern int WIFI_Inited_Flag;

//���豸
int Console_Open() 
{    
    UART3_Open(UART3_CONSOLE_TYPE);
    Opened=1;
    return 0;
}

int Console_Close()
{   
    UART3_Close();
    return 0;
}

int Console_WriteHexCharln(char * _str,int len)
{
    int i = 0;
    char* dst = NULL;

    dst = (char*)pvPortMalloc(len*2);

    if(dst == 0)
    {
        Console_WriteStringln("Console_WriteHexCharln pvPortMalloc failed");
        printf("pvPortMalloc(%d) failed \r\n",len*2);
    }

    memset(dst,0,len*2);

    for ( i = 0; i < len; i++)
    {
        Utility_CharToHex(_str[i],&(dst[i*2]));
    }

    Console_WriteBytesln(dst,len*2);

    vPortFree(dst);
    dst = NULL; //ly ����Ұָ��  ++++
    
    return 0;
}

int Console_WriteString(char * _str)
{
    if(Opened!=1)
        return -1;

#if 1
    if (WIFI_Inited_Flag ==1)
    {
        if (g_wifi_connetflag == 1)
        {
            WIFI_SendData(_str,Utility_Strlen(_str));
        }
    }
    
    else
#endif

    {
        UART3_Send(_str,Utility_Strlen(_str),0);
    }
    return 0;
}

int Console_WriteStringln(char * _str)
{
    if(Opened!=1)
       return -1;

#if 1
    if (WIFI_Inited_Flag ==1)
    {
        if ( g_wifi_connetflag == 1)
        {
            WIFI_SendData(_str,Utility_Strlen(_str));
        }  
    }
    else
#endif
    {
        UART3_Send(_str,Utility_Strlen(_str),1);
    }
    return 0;
}
int Console_WriteBytes(char * _bytes, int _len)
{
    if(Opened!=1)
        return -1;

#if 1
    if (WIFI_Inited_Flag ==1)
    {
        if ( g_wifi_connetflag == 1)
        {
            WIFI_SendData(_bytes,_len);
        }
    }
    else
#endif
    {
       UART3_Send(_bytes,_len,0);
    }
    return 0;
}
int Console_WriteBytesln(char * _bytes, int _len)
{
    if(Opened!=1)
        return -1;

#if 1
    if (WIFI_Inited_Flag ==1)
    {
        if ( g_wifi_connetflag == 1)
        {
            WIFI_SendData(_bytes,_len);
        }
    }
    else
#endif
    {
       UART3_Send(_bytes,_len,1);   
    }
    return 0;
}

int Console_WriteInt(int val)
{
    char _temp[4];
    
    Utility_UintToStr4(val,_temp);

    Console_WriteBytesln(_temp,4);
 
    return 0;
}

int Console_WriteErrorStringlnFuncLine(char* str, char const* _funcname,int _linename)
{
    Console_WriteString((char*)_funcname);
    Console_WriteString(",");
    Console_WriteInt(_linename);
    Console_WriteStringln(str);    
    return 0;
}

