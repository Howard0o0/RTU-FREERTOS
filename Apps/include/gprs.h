

#ifndef __GPRS_H
#define __GPRS_H

enum AtCmdType
{
    SET_APN,
    ACTIVE_GPRS,
    CHECK_SIM_STATUS,
    QUERY_OPERATER,
    QUERY_IP,
    OPEN_TCP_LINK,
    QUERY_TCP_LINK,
    CLOSE_TCP_LINK,
    QUIT_GPRS_NET,
    SETIO_MODE,
    SEND_DATA,
    QUERY_REMAIN_DATA,
    RECEIVE_DATA,
    OFF_CALL
};

typedef enum RETURN_VALUE
{
    OK_MESSAGE_RECV,
    NO_MESSAGE_RECV_ERROR,
    FAIL_MESSAGE_RECV_ERROR,
    GSM_OPEN_ERROR,
    GSM_OPEN_SUCCESS,
    GPRS_NOT_OPEN,
    GPRS_OPEN_ERROR,
    GPRS_OPEN_SUCCESS,
    GPRS_REBOOT_ERROR,
    GPRS_REBOOT_SUCCESS,
    SET_APN_FAILED,
    SET_APN_SUCCESS,
    QUERY_OPERATER_FAILED,
    QUERY_OPERATER_SUCCESS,
    QUERY_IP_FAILED,
    QUERY_IP_SUCCESS,
    ACTIVE_GPRS_FAILED,
    ACTIVE_GPRS_SUCCESS,
    TCP_CONNECT_SUCCESS,
    TCP_CONNECT_FAILED,
    NO_TCP_CONNECT,
    TCP_CONNECT_OK,
    TCP_CLOSE_FAILED,
    TCP_CLOSE_SUCCESS,
    TCP_Link_CLOSE_FAILED,
    TCP_Link_CLOSE_SUCCESS,
    TCP_Link_ALREADY_EXIST,
    TCP_Link_NOT_EXIST,
    TCP_Link_ERROR,
    GPRS_NET_QUIT_FAILED,
    GPRS_NET_QUIT_SUCCESS,
    TCP_QUERY_FAILED,
    TCP_QUERY_SUCCESS,
    SET_IOMODE_FAILED,
    SET_IOMODE_SUCCESS,
    MSG_SEND_FAILED,
    MSG_SEND_SUCCESS,
    QUERY_REMAIN_DATA_FAILED,
    QUERY_REMAIN_DATA_SUCCESS,
    MSG_RECEIVE_FAILED,
    MSG_RECEIVE_SUCCESS,    
    GPRS_WRITE_SUCCESS,
    GPRS_WRITE_FAILED,
    GPRS_CHECKSIM_SUCCESS,
    GPRS_CHECKSIM_FAILED,
    PHONE_CALL_COME,
    NEW_SMS_COME,
    GPRS_NO_DATA_RECV,
    GPRS_DATA_RECV,
    OK,
    ERR
}gprs_state;

typedef enum tagTelecomOperater
{
//    ERROR_OPERATER,
    CHINA_MOBILE,
    CHINA_UNICOM,
    CHINA_TELECOM
}Operater;

/*
 * GPRS_Send -> GPRS_Close_TCP_Link
*/

/*
 * return : OK  ERR
 */
gprs_state gprs_power_on(void);  

gprs_state gprs_power_off(void);

gprs_state gprs_sleep(void);

gprs_state gprs_wake_up(void);

int GPRS_Send(char* pSend, int sendDataLen, int isLastPacket, int center);

char* GPRS_Receive();

int gprs_module_driver_install(void);

void JudgeServerDataArrived(void);

int Hydrology_ProcessGPRSReceieve();

/* 以下接口都是服务于上面的接口的，上层应用应该调用上面的接口 */
void GPRS_PrepareForSend();

int GPRS_AT_OFF_CALL();

int GRPS_AT_Receive();

int GPRS_QueryRemainData();

int GPRS_Create_TCP_Link(int center);

int GPRS_Close_TCP_Link();

void GPRS_Close_GSM();


extern char* _ReceiveData;
extern int _ReceiveDataLen;
extern int gprsConfigSuccess;


#endif