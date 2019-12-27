
#ifndef	_BLE_
#define _BLE_

#pragma once

#define BLESTARTSTRING "AT:SBM14580-Start\r\n"
#define BLESTARTSTRLEN  19
#define BLECONNECTSTRING "AT:BLE-CONN-OK\r\n"
#define BLECONNECTSTRINGLEN 16


#define BLEBUFOF  "AT:BUF-OF\r\n"
#define BLEBUFON  "AT:BUF-ON\r\n"


typedef enum tagBLEResult
{
	BLE_SUCCESS = 0,
	BLE_ERROR

} BLERet;

typedef enum tagBLEErrorCode
{
	BLE_OK = 0,
	BLE_VALIDCHECKError = 1,
	BLE_UsrTimeError,
	BLE_DeviceErrror,
	BLE_LockIdError,
	BLE_FunCodeError,
	BLE_UserIdError,
	BLE_DataValidateError,
	BLE_DataLenCheckError,
	BLE_LockPwdError,
	BLE_READKEYError,
	BLE_CRCCHECKError
} BLEErrorCode;

typedef enum tagBLE_STATE
{
	// Connectable state
	BLE_CONNECTABLE,

	// Connected state
	BLE_CONNECTED,

	// Disabled State
	BLE_DISABLED
} BLE_STATE;

///////////接口
/*  BLERet:BLE_SUCCESS 0  BLE_ERROR 1 */
void BleDriverInstall();			//注册蓝牙驱动
BLERet BLE_ADVSTART();                  //开启广播  
BLERet BLE_ADVSTOP();                   //关闭广播                                         
BLERet BLE_CONNECT();             //检测蓝牙是否连接
int BLE_MAIN();                 //初始化蓝牙，开启广播，等待连接 if 0 connectd , -1 not connected
BLERet BLE_SLEEP();                     //休眠模式                         
BLERet BLE_RST();                       //重启蓝牙
BLERet BLE_BLESPP();                    //进入透传模式
BLERet BLE_BLESPPEND();                   //退出透传模式

void SPPRX(char * result,int len);           //透传模式下RTU发送信息给蓝牙模块
void SPPTX(char * result,int * len);           //透传模式下RTU接收蓝牙模块信息
int BLE_isexist();      //1:exist 0:no bluetooth

////////////内部

void BLE_buffer_Clear();                                         //清除BUFF                

void BLE_SendMsg(char *atCmd,int cmdLen);
BLERet BLE_BLESP();
BLERet ATTEST();
void BLE_RecAt(char *result,int *num);      					////RTU接收蓝牙模块的消息                                   
void BLE_SendAtCmd(char *atCmd,int cmdLen);                  //RTU发送指令给蓝牙模块
BLERet BLE_SetName ( void );                                    //                              
BLERet BLE_SERVER();
BLERet BLE_GATTSSRVCRE();
BLERet BLE_GATTSSRVSTART();
BLERet BLE_BLESPPCFG();                                  //                              
BLERet BLE_INIT();
BLERet BLE_ATE();
BLERet BLE_Open();
void BLE_Close();
int BLE_RecvLineTry ( char* _dest,const int _max, int* _pNum );




#endif
