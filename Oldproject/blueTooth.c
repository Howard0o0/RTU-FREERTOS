/******************************************/
//     �ļ���: bootloader.c
//      ʱ�䣺2018.2.1
//      ���ߣ�����
//      �汾��1.0
/******************************************/

#include "msp430common.h"
#include "blueTooth.h"
#include "uart2.h"
#include "uart3.h"
#include "console.h"
#include "common.h"
#include <string.h>
#include <stdio.h>
#include "ioDev.h"
#include <stdint.h>

#define BLE_MAX_PROTOCOL_DATA_LEN 500
#define BLE_REPEAT_TIMES 10

void BLE_buffer_Clear()
{
	UART2_ClearBuffer();
}

void SPPRX_test()
{
      char result[100]={0};
     // int time=0;
      int len=0;
      while(UART3_RecvLineWait(result,100,&len)<0)
      {
//          if(time>40)//12s
//            return;
//          time++;
      }
      
      printf("cmd:");
      for(int i=0;i<100;i++)
        printf("%c",result[i]);
      printf("\r\n");
      
      
      BLE_SendAtCmd(result,len);
      
}

void SPPTX_test()
{
      char result[100]={0};
      int time=0;
      int len=0;
      while(UART2_RecvLineWait(result,100,&len)<0)
      {
         if(time>100)//12s
           return;
         time++;
      }
      
      printf("cmd:");
      for(int i=0;i<100;i++)
        printf("%c",result[i]);
      printf("\r\n");
      
      
      //printf(result);
      
}


void SPPRX(char * result,int len)
{
//      char result[100]={0};
//      int time=0;
//      int len=0;
//      while(UART3_RecvLineWait(result,100,&len)<0)
//      {
////          if(time>40)//12s
////            return;
////          time++;
//      }
//      
      printf("cmd:");
      for(int i=0;i<len;i++)
        printf("%c",result[i]);
      printf("\r\n");
      
      
      BLE_SendMsg(result,len);
      
}

void SPPTX(char * result,int * len)
{
//      char result[100]={0};
     int time=0;
//      int len=0;
      printf("3s\r\n");


      while(UART2_RecvLineWait(result,100,len)<0)
      {
         if(time>10)//12s
           return;
         time++;
      }
      
      printf("cmd:");
      for(int i=0;i<*len;i++)
        printf("%c",result[i]);
      printf("\r\n");
      
      
      //printf(result);
      
}

//void BLE_SendAtCmd(char *atCmd,int cmdLen)
//{
//  atCmd[cmdLen] = 0x0D;
//  atCmd[cmdLen+1] = 0x0A;
////  atCmd[cmdLen+2] = 0;
//  UART2_Send(atCmd,cmdLen+2,0);  
//}


void BLE_SendAtCmd(char *atCmd,int cmdLen)
{
  char end[]={0x0D,0x0A};
  //atCmd[cmdLen] = 0;
  UART2_Send(atCmd,cmdLen,0); 
  UART2_Send(end,sizeof(end),0);
}


void BLE_SendMsg(char *atCmd,int cmdLen)
{
  //char end[]={0x0D,0x0A};
  //atCmd[cmdLen] = 0;
  UART2_Send(atCmd,cmdLen,0); 
  // UART2_Send(end,sizeof(end),0);
}

void BLE_RecAt(char *result)
{
          int _repeat = 0;
          int len;
          while ( _repeat < BLE_REPEAT_TIMES )
          {
            if(BLE_RecvLineTry ( result,100,&len ) == 0) // rcv AT response
            {
//              printf("AT RCV: %s \r\n",result);
//              printf("%s \r\n",result);
              break;
            }
              
            _repeat++;
          }
}

BLERet BLE_ATE()        //����
{
          char cmd[] = {0x41,0x54,0x45,0x30 };
          char result[100];
          BLE_SendAtCmd(cmd,sizeof(cmd));
          //printf("AT SEND: %s \r\n",cmd);
          BLE_RecAt(result);
          //printf("REC:%s\r\n",result);
          if(strstr(result,"K") !=0)
          {
            return BLE_SUCCESS;
          }
          else 
            return BLE_ERROR;
          // System_Delayms(500);
}

BLERet ATTEST()  // AT
{
      
      char cmd[] = {0x41 ,0x54  };
      char result[100];    
    
      /* test at:AT */
      for(int i=0;i<3;i++)
      {
          BLE_SendAtCmd(cmd,sizeof(cmd));
          //printf("AT SEND: %s \r\n",cmd);
          BLE_RecAt(result);
          //printf("REC:%s\r\n",result);          
          System_Delayms(500);
      }
      if(strstr(result,"K") != 0)
      {
        return BLE_SUCCESS;
      }
      else
      {
        return BLE_ERROR;
      }
}


BLERet BLE_SetName ( void )//    AT+BLENAME : ESPRESSIF
{
  
        char cmd[]={0x41,0x54,0x2B,0x42,0x4C,0x45,0x41,0x44,0x56,0x44,0x41,0x54,0x41,0x3D,0x22,0x30,0x32,0x30,0x31,0x30,0x36,0x30,0x34,0x30,0x39,0x35,0x32,0x35,0x34,0x35,0x35,0x30,0x33,0x30,0x33,0x30,0x32,0x41,0x30,0x22};
       // char end[]={0x0D,0x0A};
//        char cmd[100];
//        memset(cmd,0,100);
//        memcpy(cmd,"AT+BLEADVDATA=\"0201060A09457370726573736966030302A0\"",sizeof("AT+BLEADVDATA=\"0201060A09457370726573736966030302A0\""));
        
	char result[100];
        
//        UART2_Send(cmd,sizeof(cmd),0);
//        UART2_Send(end,sizeof(end),0);
         
	BLE_SendAtCmd(cmd,sizeof(cmd));
        
        //printf("AT SEND: %s \r\n",cmd);
	BLE_RecAt(result);
        //printf("REC:%s\r\n",result);
        if(strstr(result,"K") != 0)
        {
          return BLE_SUCCESS;
        }
        else
        {
          return BLE_ERROR;
        }
          
}

BLERet BLE_SERVER()  // //AT+BLEINIT=2  ��ʼ��Ϊ�����
{
        char cmd[]={0x41,0x54,0x2B,0x42,0x4C,0x45,0x49,0x4E,0x49,0x54,0x3D,0x32 };
        
//        char cmd[num];
//        memset(cmd,0,num);
//        memcpy(cmd,"AT+BLEINIT=2",sizeof("AT+BLEINIT=2"));
               
               
	char result[100];
	BLE_SendAtCmd(cmd,sizeof(cmd));
        //printf("AT SEND: %s \r\n",cmd);
	BLE_RecAt(result);
        //printf("REC:%s\r\n",result);
        if(strstr(result,"K") != 0)
        {
          return BLE_SUCCESS;
        }
        else
          return BLE_ERROR;
}



BLERet BLE_GATTSSRVCRE()// AT+BLEGATTSSRVCRE GATTS ��������
{
        char cmd[]={0x41,0x54,0x2B,0x42,0x4C,0x45,0x47,0x41,0x54,0x54,0x53,0x53,0x52,0x56,0x43,0x52,0x45 };
        
//        char cmd[100];
//        memset(cmd,0,100);
//        memcpy(cmd,"AT+BLEGATTSSRVCRE",sizeof("AT+BLEGATTSSRVCRE"));
        
	char result[100];
	BLE_SendAtCmd(cmd,sizeof(cmd));
        //printf("AT SEND: %s \r\n",cmd);
	BLE_RecAt(result);
        //printf("REC:%s\r\n",result);
        if(strstr(result,"K") != 0)
        {
          return BLE_SUCCESS;
        }
        else
          return BLE_ERROR;
}


BLERet BLE_GATTSSRVSTART() //AT+BLEGATTSSRVSTART��GATTS ��������
{
	char cmd[]={0x41,0x54,0x2B,0x42,0x4C,0x45,0x47,0x41,0x54,0x54,0x53,0x53,0x52,0x56,0x53,0x54,0x41,0x52,0x54  };
//        char cmd[100];
//        memset(cmd,0,100);
//        memcpy(cmd,"AT+BLEGATTSSRVSTART",sizeof("AT+BLEGATTSSRVSTART"));
  
	char result[100];
	BLE_SendAtCmd(cmd,sizeof(cmd));
        //printf("AT SEND: %s \r\n",cmd);
	BLE_RecAt(result);
        //printf("REC:%s\r\n",result);
        if(strstr(result,"K") != 0)
        {
          return BLE_SUCCESS;
        }
        else
          return BLE_ERROR;
}


BLERet BLE_ADVSTART()  //AT+BLEADVSTART ��ʼ BLE �㲥
{
	char cmd[]={0x41,0x54,0x2B,0x42,0x4C,0x45,0x41,0x44,0x56,0x53,0x54,0x41,0x52,0x54  };
//        char cmd[100];
//        memset(cmd,0,100);
//        memcpy(cmd,"AT+BLEADVSTART",sizeof("AT+BLEADVSTART"));
        
	char result[100];
	BLE_SendAtCmd(cmd,sizeof(cmd));
        //printf("AT SEND: %s \r\n",cmd);
	BLE_RecAt(result);
        //printf("REC:%s\r\n",result);
        if(strstr(result,"K") != 0)
        {
          return BLE_SUCCESS;
        }
        else
          return BLE_ERROR;
}


BLERet BLE_BLESPPCFG()//AT+BLESPPCFG=1,1,7,1,5  ����͸��
{
	char cmd[]={0x41,0x54,0x2B,0x42,0x4C,0x45,0x53,0x50,0x50,0x43,0x46,0x47,0x3D,0x31,0x2C,0x31,0x2C,0x37,0x2C,0x31,0x2C,0x35  };
	
//        char cmd[100];
//        memset(cmd,0,100);
//        memcpy(cmd,"AT+BLESPPCFG=1,1,7,1,5",sizeof("AT+BLESPPCFG=1,1,7,1,5"));
        char result[100];
	BLE_SendAtCmd(cmd,sizeof(cmd));
        //printf("AT SEND: %s \r\n",cmd);
	BLE_RecAt(result);
        //printf("REC:%s\r\n",result);
        if(strstr(result,"K") != 0)
        {
          return BLE_SUCCESS;
        }
        else
          return BLE_ERROR;
}

BLERet BLE_BLESP()//AT+BLESPP  ����͸��
{


	char cmd[]={0x41,0x54,0x2B,0x42,0x4C,0x45,0x53,0x50,0x50  };
//        char cmd[100];
//        memset(cmd,0,100);
//        memcpy(cmd,"AT+BLESPP",sizeof("AT+BLESPP"));
	char result[100];
	BLE_SendAtCmd(cmd,sizeof(cmd));
  //printf("AT SEND: %s \r\n",cmd);
	BLE_RecAt(result);
  //printf("REC:%s\r\n",result);
  if(strstr(result,"K") != 0)
  {
    return BLE_SUCCESS;
  }
  else
  {
    BLE_SendAtCmd(" ",1);
    BLE_RecAt(result);
    if(strstr(result,"RR") != 0)
      return BLE_ERROR;
    else 
      return BLE_SUCCESS;
  }

}

BLERet BLE_BLESPP()
{
  System_Delayms ( 1000 );
  int time=0;
  while(BLE_BLESP()!=BLE_SUCCESS)
  {
    time++;
    printf( "SPP...\r\n" );
    System_Delayms ( 500 );
    if(time>5)
    {
      printf("failed,please check system\r\n");
      return BLE_ERROR;
    }
  }
  printf( "SPP!\r\n" );
  System_Delayms ( 1000 );

      char re[100];
      //char cmd[]={0x41,0x54};
      //BLE_SendAtCmd(" ",1);
      BLE_RecAt(re);

  return BLE_SUCCESS;
}

BLERet BLE_BLESPPEND()//+++ �س�  �˳�͸��
{
	char cmd[]={0x2B,0x2B,0x2B};
        char end[]={0x41 ,0x54,0x0D ,0x0A};
//        char end1[]={};
        char cmd1[]={0x31,0x32,0x33};
        BLE_buffer_Clear();
//        char cmd[100];
//        memset(cmd,0,100);
//        memcpy(cmd,"+++ \r\n",sizeof("+++ \r\n"));
	char result[100];
       System_Delayms(2000);
        UART2_Send(cmd,sizeof(cmd),0);
        BLE_RecAt(result);
        //printf("REC:%s\r\n",result);
        System_Delayms(1000);
        BLERet ret;
        ret=ATTEST();
        return ret;
        //UART2_Send(cmd1,sizeof(cmd1),0);
        //UART2_Send(end1,sizeof(end1),0);
}

BLERet BLE_INIT()       //��ʼ������
{
    P10DIR |= BIT1;         //MCU-P101=0
    P10OUT &= ~BIT1;
    P10DIR |= BIT2;         //MCU-P102=0
    P10OUT &= ~BIT2;
    P9DIR |= BIT6;
    P9OUT |= BIT6;
    UART2_Open(1);
    UART3_Send("uart open",9,1);
    
    printf("wait for 10s ...\r\n");
    System_Delayms(1000);
    
    BLERet ret = BLE_ERROR;
      
    BLE_BLESPPEND();
    // System_Delayms ( 1000 );
    
    
    for(int i=0;i<3;i++)
    {
      ret=BLE_RST(); 
      System_Delayms ( 1000 );
      ret=BLE_ATE();
      System_Delayms ( 1000 );
      ret=ATTEST();
      System_Delayms ( 1000 );
      if(ret==BLE_SUCCESS)
        return BLE_SUCCESS;
    }
    return BLE_ERROR;
}



BLERet BLE_Open()
{
      BLERet ret = BLE_ERROR;
      System_Delayms ( 2000 );
      
      ret = BLE_SERVER();
      System_Delayms ( 1000 );
      if(ret == BLE_ERROR)
        return ret;
      printf("BLE server\r\n");
      
      ret = BLE_SetName(); 
      System_Delayms ( 1000 );
      if(ret == BLE_ERROR)
        return ret;
      printf("BLE set name\r\n");

      ret = BLE_GATTSSRVCRE();
      System_Delayms ( 1000 );
      if(ret == BLE_ERROR)
        return ret;
      printf("BLE service\r\n");
      
      ret = BLE_GATTSSRVSTART();
      System_Delayms ( 1000 );
      //if(ret == BLE_ERROR)
        //return ret;
      printf("BLE service start\r\n");

      ret = BLE_ADVSTART();
      System_Delayms ( 1000 );
      if(ret == BLE_ERROR)
        return ret;
      printf("BLE adv start\r\n");

      return ret;
}



BLERet BLE_CONNECT()    //�ж��Ƿ�����
{
	char cmd[]={0x41,0x54,0x2B,0x42,0x4C,0x45,0x43,0x4F,0x4E,0x4E,0x3F };
//        char cmd[100];
//        memset(cmd,0,100);
//        memcpy(cmd,"AT+BLECONN?",sizeof("AT+BLECONN?"));
	char result[100];
        for(int i=0;i<3;i++)
        {
          BLE_SendAtCmd(cmd,sizeof(cmd));
          //printf("AT SEND: %s \r\n",cmd);
          BLE_RecAt(result);
          //printf("REC:%s\r\n",result);
          if(strstr(result,"N:0") != 0)
          {
            return BLE_SUCCESS;
          }
        }
	return BLE_ERROR;
}

BLERet BLE_ADVSTOP()    //�����㲥
{
	char cmd[]={0x41,0x54,0x2B,0x42,0x4C,0x45,0x41,0x44,0x56,0x53,0x54,0x4F,0x50 };
//        char cmd[100];
//        memset(cmd,0,100);
//        memcpy(cmd,"AT+BLEADVSTOP",sizeof("AT+BLEADVSTOP"));
	char result[100];
	BLE_SendAtCmd(cmd,sizeof(cmd));
        //printf("AT SEND: %s \r\n",cmd);
	BLE_RecAt(result);
        //printf("REC:%s\r\n",result);
        if(strstr(result,"K") != 0)
        {
          return BLE_SUCCESS;
        }
        else
          return BLE_ERROR;
}


BLERet BLE_RST()    //��������
{
        char cmd[]={0x41,0x54,0x2B,0x52,0x53,0x54  };
//        char cmd[10];
//        memset(cmd,0,10);
//        memcpy(cmd,"AT+RST",6);
        
      
	
	char result[100];
	BLE_SendAtCmd(cmd,sizeof(cmd));
        //printf("AT SEND: %s \r\n",cmd);
	BLE_RecAt(result);
        //printf("REC:%s\r\n",result);
        if(strstr(result,"ea") != 0)
        {
          return BLE_SUCCESS;
        }
        else
          return BLE_ERROR;

}

BLERet BLE_SLEEP()      //����
{
  
          char cmd[]={0x41,0x54,0x2B,0x43,0x57,0x4D,0x4F,0x44,0x45,0x3D,0x30  };
//        char cmd[20];
//        memset(cmd,0,20);
//        memcpy(cmd,"AT+CWMODE=0",11);
        
      
	
	char result[100];
	BLE_SendAtCmd(cmd,sizeof(cmd));
        //printf("AT SEND: %s \r\n",cmd);
	BLE_RecAt(result);
        //printf("REC:%s\r\n",result);
        if(strstr(result,"K") != 0)
        {
        }
        else
          return BLE_ERROR;

	char cmd2[]={0x41,0x54,0x2B,0x42,0x4C,0x45,0x49,0x4E,0x49,0x54,0x3D,0x30 };
//        memset(cmd,0,20);
//        memcpy(cmd,"AT+BLEINIT=0",12);
        
        BLE_SendAtCmd(cmd2,sizeof(cmd2));
        //printf("AT SEND: %s \r\n",cmd2);
	BLE_RecAt(result);
        //printf("REC:%s\r\n",result);
        if(strstr(result,"OK") != 0)
        {
          return BLE_SUCCESS;
        }
        else
          return BLE_ERROR;
	
}


void BLE_Close()
{
	UART2_Close();
}


int BLE_RecvLineTry ( char* _dest,const int _max, int* _pNum )
{
	if ( 0 == UART2_RecvLineWait ( _dest, _max,  _pNum ) )
	{
                _dest[*_pNum] = 0;  // end symbol
		return 0;
	}
	else
	{
		return -1;
	}
}


int BLE_MAIN()
{
        if(BLE_INIT()!=BLE_SUCCESS)
          return -1;
        printf( "BLE Init success!\r\n\r\n" );
        
        
        int time=0;
        while(BLE_Open() != BLE_SUCCESS)
        {
          printf( "open ble failed ,reset system\r\n" );
          time++;
          System_Delayms ( 500 );
          if(time>5)
          {
            printf("failed,please check system\r\n");
            return -1;
          }
        }         
        printf( "BLE open success!\r\n\r\n" );
        
        
        
        time=0;
        while(BLE_CONNECT() != BLE_SUCCESS)
        {
          time++;
          printf( "CONNECT...\r\n" );
          System_Delayms ( 1000 );
          if(time>30)
          {
            printf("failed to connect\r\n");
            return -1;
          }
        }
        printf( "CONNECTED!\r\n\r\n" );
        
        
        
        
        time=0;
        while(BLE_BLESPPCFG() != BLE_SUCCESS)
        {
          time++;
          printf( "CONFIG...\r\n" );
          System_Delayms ( 500 );
          if(time>5)
          {
            printf("failed,please check system\r\n");
            return -1;
          }
        }
        printf( "CONFIG!\r\n\r\n" );
        
        
        printf("waiting CCCD\r\n");
        char result[100];
        int flag1=0,flag2=0;
        for(int i=0;i<2;i++)
        {
          time=0;
          while(1)
          {
            BLE_RecAt(result);
            if(flag1==0 && strstr(result,"6") != 0)
            {
              printf("success  ...\r\n");
              flag1=1;
              break;
            }
            else if(flag2==0 && strstr(result,"7") != 0)
            {
              printf("success  ...\r\n");
              flag2=1;
              break;
            }
            else if(time>20)
            {
              printf("failed to enable CCCD\r\n");
              return -1;
            }
            time++;
            printf("waiting CCCD\r\n");
            System_Delayms(500);
          }
        }
        printf("CCCD!\r\n\r\n");


        // printf("waiting CCCD\r\n");
        // char result[100];
        // BLE_RecAt(result);
        // time=0;
        // while(strstr(result,"+W") == 0)
        // {
        //   time++;
        //   //printf("REC:%s\r\n",result);
        //   printf("waiting CCCD\r\n");
        //   BLE_RecAt(result);
        //   if(time>25)
        //   {
        //     printf("failed to enable CCCD\r\n");
        //     return -1;
        //   }
        // }
        // //printf("REC:%s\r\n",result);
        // printf("CCCD!\r\n\r\n");
        
        
        // time=0;
        // while(BLE_BLESPP()!=BLE_SUCCESS)
        // {
        //   time++;
        //   printf( "SPP...\r\n" );
        //   System_Delayms ( 1000 );
        //   if(time>5)
        //   {
        //     printf("failed,please check system\r\n");
        //     return -1;
        //   }
        // }
        // printf( "SPP!\r\n" );
        
        // System_Delayms(2000);
        return 0;
	//System_Reset();
}

/*
 * author   :   Howard
 * date     :   2019/10/09
 * Desc     :   BLE driver
*/
int ble_init();
int ble_isCanUse();
int ble_open();
void ble_getMsg(char *msgRecv,int *len);
int ble_sendMsg(char *msgRecv,int len);
int ble_close();

T_IODev T_CommuteDevBLE = 
{
    .name = "BLE",
    .isCanUseFlag = 0,
    .isCanUse = ble_isCanUse,
    .open = ble_open,
    .getMsg = ble_getMsg,
    .sendMsg = ble_sendMsg,
    .close = ble_close,
    .init = ble_init,
};


// 0 success, -1 failed
int ble_init()
{
  int iRet = BLE_MAIN();
  if(iRet == 0)
  {
    T_CommuteDevBLE.isCanUseFlag = 1;
  }
  else
  {
    T_CommuteDevBLE.isCanUseFlag = 0;
  }
  

  return iRet;
}

// 1 available, -1 not available
int ble_isCanUse()
{
  if( BLE_CONNECT() == BLE_SUCCESS )
  {
    T_CommuteDevBLE.isCanUseFlag = 1;
    return 1;
  }
  else
  {
    T_CommuteDevBLE.isCanUseFlag = 0;
    return 0;
  }

}

// 0 success , otherwise fail
int ble_open()
{
  int iRet;
  iRet = BLE_BLESPP();


  return iRet;
}

void ble_getMsg(char *msgRecv,int *len)
{

  SPPTX(msgRecv,len);
}

//0 success
int ble_sendMsg(char *msgRecv,int len)
{
  SPPRX(msgRecv,len);

  return 0;
}

int ble_close()
{
  return BLE_BLESPPEND();
}





void BleDriverInstall()
{
  RegisterIODev(&T_CommuteDevBLE);
}