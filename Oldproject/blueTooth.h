/******************************************/
//     文件名: bootloader.h
//      时间：2018.2.1
//      作者：张衡
//      版本：1.0
/******************************************/
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
/*  BLERet:BLE_SUCCESS   BLE_ERROR */
void BleDriverInstall();
BLERet BLE_ADVSTART();                  //开启广播  
BLERet BLE_ADVSTOP();                   //结束广播                                          
BLERet BLE_CONNECT();             //判断蓝牙是否连接
int BLE_MAIN();                 //初始化蓝牙-》广播-》等待连接-》 if 0 connectd , -1 not connected
BLERet BLE_SLEEP();                     //蓝牙休眠  可直接用BLE_MAIN初始化                         
BLERet BLE_RST();                       //重启蓝牙
BLERet BLE_BLESPP();                    //透传
BLERet BLE_BLESPPEND();                   //结束透传

void SPPRX(char * result,int len);           //手机收
void SPPTX(char * result,int * len);           //手机发


////////////底层

void BLE_buffer_Clear();                                         //清BUFF                
void SPPRX_test();           //手机收
void SPPTX_test();           //手机发


void BLE_SendMsg(char *atCmd,int cmdLen);
BLERet BLE_BLESP();
BLERet ATTEST();
void BLE_RecAt(char *result);      //RTU接受蓝牙                                    
void BLE_SendAtCmd(char *atCmd,int cmdLen);                        //RTU发送给蓝牙
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




