#include "msp430common.h"
#include "uart0.h"
#include "common.h"
#include "GTM900C.h"
#include "string.h"
#include "GSM.h"
#include "Console.h"
#include "common.h"
#include "store.h"
#include "rtc.h"
#include "hydrologycommand.h"
#include "message.h"
#include "uart0.h"
#include "FreeRTOS.h"

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

enum RETURN_VALUE
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
    GPRS_DATA_RECV
};

typedef enum tagTelecomOperater
{
//    ERROR_OPERATER,
    CHINA_MOBILE,
    CHINA_UNICOM,
    CHINA_TELECOM
}Operater;



char centerIP[16];// = "125.220.159.168";
char centerPort[7];// = "6666";
char *psrc = NULL;
int dataLen = 0;

char* _ReceiveData = NULL;
int _ReceiveDataLen = 0;

void GPRS_Print_Error(int errorCode)
{
    switch(errorCode)
    {
        // 1:�����������
        case 1:
        {
            Console_WriteStringln("Wrong AT Command Parameter!");
            break;
        }
        // 2:һ���ڲ��Ĵ������
        case 2:
        {
            Console_WriteStringln("An internel error in GTM900 Module!");
            break;
        }
        // 3:IP ��ַ���߶˿ں����ڱ�ʹ��
        case 3:
        {
            Console_WriteStringln("IP Address or Port is already used!");
            break;
        }
        // 4:�ڴ�����ʧ�ܻ��� BUFFER �Ѿ���ʹ����
        case 4:
        {
            Console_WriteStringln("Memory apply failed or buffer run out!");
            break;
        }
        // 5:�� socket ��֧�ִ��ֲ���
        case 5:
        {
            Console_WriteStringln("Unsupported operation of socket!");
            break;
        }
        // 6:�������ӵ�������
        case 6:
        {
            Console_WriteStringln("Connot connect to host, please check host!");
            break;
        }
        // 7:���ӱ��Է��ܾ�
        case 7:
        {
            Console_WriteStringln("Connection being refused by host, please check host fireware");
            break;
        }
        // 8:���ӳ�ʱ
        case 8:
        {
            Console_WriteStringln("Connection time out");
            break;
        }
        // 9:�������Ѿ�����
        case 9:
        {
            Console_WriteStringln("Link already exists");
            break;
        }
        // 10:�� DNSR ���������Ҳ�����������ַ
        case 10:
        {
            Console_WriteStringln("Can not find host IP in DNSR");
            break;
        }
        // 11:����һ����ʱ�� DNS �����Ժ��������ִ��
        case 11:
        {
            Console_WriteStringln("Temporary DSN Error");
            break;
        }
        // 12:����һ�����õ� DNS ����
        case 12:
        {
            Console_WriteStringln("Permanent DSN Error");
            break;
        }
        // 13:�� DNSR �������ҵ���������ַ������ IP ��ַ��Ч
        case 13:
        {
            Console_WriteStringln("Invalid IP in DNSR");
            break;
        }
        // 14:�� socket ���ӻ�û�н���
        case 14:
        {
            Console_WriteStringln("Socket not exist");
            break;
        }
        // 15:��������ݳ���̫��
        case 15:
        {
            Console_WriteStringln("Data out of length");
            break;
        }
        // 16:�����ӱ��Զ˸�λ
        case 16:
        {
            Console_WriteStringln("Host reset Link");
            break;
        }
        // 17:�������д������
        case 17:
        {
            Console_WriteStringln("Error occurs on Link");
            break;
        }
        // 18:û��ʣ�໺����������Ϣ
        case 18:
        {
            Console_WriteStringln("No Cache to save information");
            break;
        }
        // 19:û�����磬����һ������������� TCPIP �Ͽ�
        case 19:
        {
            Console_WriteStringln("No net exists or TCP/IP function failed");
            break;
        }
        // 20:PDP ������û�м���
        case 20:
        {
            Console_WriteStringln("PDP not activitied");
            break;
        }  
        // 21:GPRS attach û�гɹ�
        case 21:
        {
            Console_WriteStringln("GPRS attach failed");
            break;
        }
        // 22:���ƵĲ�������ִ��
        case 22:
        {
            Console_WriteStringln("Similar operation already being excuted");
            break;
        }
        // 23:PDP �������Ѿ�����
        case 23:
        {
            Console_WriteStringln("PDP already activitied");
            break;
        }
        // 24:��ǰΪ������ģʽ
        case 24:
        {
            Console_WriteStringln("Tcp is on multi-link mode!");
            break;
        }
        // 25:��ǰΪ������ģʽ
        case 25:
        {
            Console_WriteStringln("Tcp is on single-link mode!");
            break;
        }
        // 26:����� index ���벻����
        case 26:
        {
            Console_WriteStringln("Index number not exist !");
            break;
        }
        // 27:��Ч��ɾ��ģʽ
        case 27:
        {
            Console_WriteStringln("Invalid delete mode!");
            break;
        }
        // 28:��ǰ������ʹ�ø�����
        case 28:
        {
            Console_WriteStringln("Can not use this command at present!");
            break;
        }
        // 29:���ʹ�������
        case 29:
        {
            Console_WriteStringln("Sending window full!");
            break;
        }
        // 30:��ǰ����ͨ���У����Ժ�ִ�� PDP ����� TCPIP ��ʼ���Ĳ���
        case 30:
        {
            Console_WriteStringln("Phone on line, please excuted PDP activite and TCP Initialize later");
            break;
        }
        // 31:��������
        case 31:
        {
            Console_WriteStringln("Unexpected error");
            break;
        }
    }
}

int GPRS_AT_SetAPN_Response(char *_recv)
{
    return TRUE;
}

int GPRS_AT_ActiveGPRSNet_Response(char *_recv)
{
    return TRUE;
}


int GPRS_AT_CheckSimStat_Response(char *_recv,int *_retErrorCode)
{
    int retValue = TRUE;
    if(Utility_Strncmp(_recv, "%TSIM", 5) == 0)
    {
        if(_recv[6] == '0')
        {
            retValue = FALSE;
        }
    }
    else if(Utility_Strncmp(_recv,"ERROR",5)==0)//ERROR: 14  ð�ź�14�м��пո�
    {
        char bufErrorTmp[3] = {0};//����atoi��������Ҫ���ַ�����������2λ�����룬1λ������

        bufErrorTmp[0] = _recv[7];	
        if(_recv[8] != 0)
        {
            bufErrorTmp[1] = _recv[8];
        }
        *_retErrorCode = atoi(bufErrorTmp);	
        GPRS_Print_Error(*_retErrorCode);
        retValue = FALSE;
    }
    return retValue;    
}

Operater TelecomOperater;

int GPRS_AT_QueryOperater_Response(char *_recv,int *_retErrorCode)
{
    int retValue = TRUE;
    if(Utility_Strncmp(_recv, "+COPS: 0,0,\"CHINA  MOBILE\"", 26) == 0)
    {
        TelecomOperater = CHINA_MOBILE;
        retValue = TRUE;
    }
    else if(Utility_Strncmp(_recv,"ERROR",5)==0)//ERROR: 14  ð�ź�14�м��пո�
    {
        char bufErrorTmp[3] = {0};//����atoi��������Ҫ���ַ�����������2λ�����룬1λ������

        bufErrorTmp[0] = _recv[7];	
        if(_recv[8] != 0)
        {
            bufErrorTmp[1] = _recv[8];
        }
        *_retErrorCode = atoi(bufErrorTmp);	
        GPRS_Print_Error(*_retErrorCode);
        retValue = FALSE;
    }
    return retValue;
}

int GPRS_AT_QueryIP_Response(char *_recv,int *_retErrorCode)
{
    int retValue = TRUE;
    if(Utility_Strncmp(_recv, "%ETCPIP:", 8) == 0)
    {
        if(_recv[8] == 0)
        {
            retValue = FALSE;
        }	
    }else if(Utility_Strncmp(_recv,"ERROR",5)==0)//ERROR: 14  ð�ź�14�м��пո�
    {
        char bufErrorTmp[3] = {0};//����atoi��������Ҫ���ַ�����������2λ�����룬1λ������

        bufErrorTmp[0] = _recv[7];	
        if(_recv[8] != 0)
        {
            bufErrorTmp[1] = _recv[8];
        }
        *_retErrorCode = atoi(bufErrorTmp);	
        GPRS_Print_Error(*_retErrorCode);
        retValue = FALSE;
    }
    return retValue;
}

//�����Ӻ�����������Ӧ��һ����һ�£���Ϊֻ����һ��connect����error
int GPRS_AT_OpenTCPLink_Response(char *_recv)
{
    return TRUE;
}

int GPRS_AT_QueryTCPLink_Response(char *_recv,int *_retErrorCode)
{
    int retValue = TRUE;
    if(Utility_Strncmp(_recv, "%IPOPEN:", 8) == 0)
    {
        if(_recv[8] == 0)
        {
            retValue = FALSE;
        }			        
    }
    else if(Utility_Strncmp(_recv,"ERROR",5)==0)//ERROR: 14  ð�ź�14�м��пո�
    {
        char bufErrorTmp[3] = {0};//����atoi��������Ҫ���ַ�����������2λ�����룬1λ������

        bufErrorTmp[0] = _recv[7];	
        if(_recv[8] != 0)
        {
            bufErrorTmp[1] = _recv[8];
        }
        *_retErrorCode = atoi(bufErrorTmp);	
        GPRS_Print_Error(*_retErrorCode);
        retValue = FALSE;
    }
    return retValue;
}

//����û���ǳ���������Ҫ����
int GPRS_AT_CloseTCPLink_Response(char *_recv,int *_retErrorCode)
{
    int retValue = FALSE;
    //IPCLOSE�����IP���ӣ����ȷ���%IPCLOSE,Ȼ���ٷ���OK,���û�����ӣ�ֻ�᷵��һ��������Ӧ�ɹ� OK
    if((Utility_Strncmp(_recv,"%IPCLOSE",8) ==0) || (Utility_Strncmp(_recv,"OK",2) ==0))
    {
        retValue =  TRUE;
    }
    else if(Utility_Strncmp(_recv,"ERROR",5)==0)//ERROR: 14  ð�ź�14�м��пո�
    {
        char bufErrorTmp[3] = {0};//����atoi��������Ҫ���ַ�����������2λ�����룬1λ������

        bufErrorTmp[0] = _recv[7];	
        if(_recv[8] != 0)
        {
            bufErrorTmp[1] = _recv[8];
        }
        *_retErrorCode = atoi(bufErrorTmp);	
        GPRS_Print_Error(*_retErrorCode);
        retValue = FALSE;
    }
    return retValue;
}

//����û���ǳ���������Ҫ����
int GPRS_AT_QuitGPRSNet_Response(char *_recv,int *_retErrorCode)
{
    int retValue = FALSE;
    //IPCLOSE�����IP���ӣ����ȷ���%IPCLOSE,Ȼ���ٷ���OK,���û�����ӣ�ֻ�᷵��һ��������Ӧ�ɹ� OK
    if((Utility_Strncmp(_recv,"%IPCLOSE",8) ==0) || (Utility_Strncmp(_recv,"OK",2) ==0))
    {
        retValue =  TRUE;
    }
    else if(Utility_Strncmp(_recv,"ERROR",5)==0)//ERROR: 14  ð�ź�14�м��пո�
    {
        char bufErrorTmp[3] = {0};//����atoi��������Ҫ���ַ�����������2λ�����룬1λ������

        bufErrorTmp[0] = _recv[7];	
        if(_recv[8] != 0)
        {
            bufErrorTmp[1] = _recv[8];
        }
        *_retErrorCode = atoi(bufErrorTmp);	
        GPRS_Print_Error(*_retErrorCode);
        retValue = FALSE;
    }
    return retValue;
}

//ʹ��ASCii�뻹��Hex�뷢����Ϣ��0,1,1---ASCii�� 1,1,0---Hex
int GPRS_AT_SetIOMode_Response(char *_recv)
{
    int retValue = TRUE;
    //SetIOMode�����IP���ӣ����ȷ���OK,û��IP���ͻ᷵�� EXT: I
    if(Utility_Strncmp(_recv, "EXT:", 4) == 0)
    {
        retValue = FALSE;	
    }
    return retValue;
}

int _GTM900SendBufLeft = 16;
int GRPS_AT_Send_Response(char *_recv,int *_retErrorCode)
{
    int retValue = FALSE;
    if(Utility_Strncmp(_recv, "%IPSEND:", 8) == 0)
    {
        char bufLeftTmp[3] = {0};//����atoi��������Ҫ���ַ���������
        bufLeftTmp[0] = _recv[8];
        if(_recv[9] != 0)
        {
            bufLeftTmp[1] = _recv[9];
        }
        _GTM900SendBufLeft = atoi(bufLeftTmp);
        retValue = TRUE;
    }
    else if(Utility_Strncmp(_recv,"ERROR",5)==0)//ERROR: 14  ð�ź�14�м��пո�
    {
        char bufErrorTmp[3] = {0};//����atoi��������Ҫ���ַ�����������2λ�����룬1λ������

        bufErrorTmp[0] = _recv[7];	
        if(_recv[8] != 0)
        {
            bufErrorTmp[1] = _recv[8];
        }
        *_retErrorCode = atoi(bufErrorTmp);	
        GPRS_Print_Error(*_retErrorCode);
        retValue = FALSE;
    }

    return retValue;
}

int RemainingData = 0;
int GPRS_AT_QueryRemainData_Response(char *_recv,int *_retErrorCode)
{
    int retValue = TRUE;
    
    if(Utility_Strncmp(_recv, "%IPDQ:", 6) == 0)
    {
        RemainingData = atoi(&_recv[7]);
    }
    else if(Utility_Strncmp(_recv,"ERROR",5)==0)//ERROR: 14  ð�ź�14�м��пո�
    {
        char bufErrorTmp[3] = {0};//����atoi��������Ҫ���ַ�����������2λ�����룬1λ������

        bufErrorTmp[0] = _recv[7];	
        if(_recv[8] != 0)
        {
            bufErrorTmp[1] = _recv[8];
        }
        *_retErrorCode = atoi(bufErrorTmp);	
        GPRS_Print_Error(*_retErrorCode);
        retValue = FALSE;
    }
    return retValue;
}

int GRPS_AT_Receive_Response(char *_recv,int *_retErrorCode)
{
    int retValue = FALSE;
    char* pdata = NULL;
    char* psrc = _recv + 10;
    int _datalensize = 0;
    
    if(Utility_Strncmp(_recv, "%IPDR:", 6) == 0)
    {
        while((*psrc) != ',')
        {
            _datalensize++;
            psrc++;
        }
        psrc = _recv + 10;
        pdata = (char*)pvPortMalloc(_datalensize);
        if(pdata == NULL)
        {
            Console_WriteStringln("Malloc in GRPS_AT_Receive_Response failed");
            printf("pvPortMalloc(%d) failed \r\n",_datalensize);
            return MSG_SEND_FAILED;
        }
        Utility_Strncpy(pdata, _recv + 10, _datalensize);
        _ReceiveDataLen = Utility_atoi(pdata, _datalensize);
        vPortFree(pdata);
        pdata = NULL;//
        _ReceiveData = (char*)pvPortMalloc(_ReceiveDataLen*2);
        if(_ReceiveData == NULL)
        {
            Console_WriteStringln("Malloc in GRPS_AT_Receive_Response failed");
            printf("pvPortMalloc(%d) failed \r\n",_ReceiveDataLen*2);
            return MSG_SEND_FAILED;
        }
        
        while((*psrc) != '\"')
        {
            psrc++;
        }
        Utility_Strncpy(_ReceiveData, psrc+1, _ReceiveDataLen*2);
        // vPortFree(_ReceiveData);  //+++
        // _ReceiveData = NULL;
        retValue = TRUE;
    }
    else if(Utility_Strncmp(_recv,"ERROR",5)==0)//ERROR: 14  ð�ź�14�м��пո�
    {
        char bufErrorTmp[3] = {0};//����atoi��������Ҫ���ַ�����������2λ�����룬1λ������

        bufErrorTmp[0] = _recv[7];	
        if(_recv[8] != 0)
        {
            bufErrorTmp[1] = _recv[8];
        }
        *_retErrorCode = atoi(bufErrorTmp);	
        GPRS_Print_Error(*_retErrorCode);
        retValue = FALSE;
    }

    return retValue;
}

int GPRS_AT_OFF_CALL_Response(char *_recv)
{
    return TRUE;
}

int GPRS_SubProc_AT_ResponseOne(char *_recv, int *_retMsgNum, int cmdType, int *_retErrorCode)
{
    int retValue=FALSE;

    if(Utility_Strncmp(_recv,"RING",4)==0)//����ǵ绰���ˣ�ֱ�ӹҵ������µȴ����ܡ�
    {
        GPRS_AT_OFF_CALL();
        retValue = FALSE;//���ֵ绰
        return retValue;
    }
    
    switch(cmdType)
    {
        case SET_APN:
        {
            retValue = GPRS_AT_SetAPN_Response(_recv);
            break;
        }
        case ACTIVE_GPRS:
        {
            retValue = GPRS_AT_ActiveGPRSNet_Response(_recv);
            break;
        }
        case CHECK_SIM_STATUS:
        {
            retValue = GPRS_AT_CheckSimStat_Response(_recv, _retErrorCode);
            break;
        }
        case QUERY_OPERATER:
        {
            retValue = GPRS_AT_QueryOperater_Response(_recv, _retErrorCode);
            break;
        }
        case QUERY_IP:
        {
            retValue = GPRS_AT_QueryIP_Response(_recv, _retErrorCode);//�����һ����"%ETCPIP:0"��ʶֵΪ0��ֱ�ӷ��أ����治�ý�����
            break;
        }
        case OPEN_TCP_LINK:
        {
            retValue = GPRS_AT_OpenTCPLink_Response(_recv);
            break;
        }
        case QUERY_TCP_LINK:
        {
            retValue = GPRS_AT_QueryTCPLink_Response(_recv, _retErrorCode);//�����һ����"%ETCPIP:0"��ʶֵΪ0��ֱ�ӷ��أ����治�ý�����
            break;
        }
        case CLOSE_TCP_LINK:
        {
            retValue = GPRS_AT_CloseTCPLink_Response(_recv, _retErrorCode);
            break;
        }
        case QUIT_GPRS_NET:
        {
            retValue = GPRS_AT_QuitGPRSNet_Response(_recv, _retErrorCode);
            break;
        }
        case SETIO_MODE:
        {
            retValue = GPRS_AT_SetIOMode_Response(_recv);
            break;
        }
        case SEND_DATA:
        {
            retValue = GRPS_AT_Send_Response(_recv, _retErrorCode);
            break;
        }
        case QUERY_REMAIN_DATA:
        {
            retValue = GPRS_AT_QueryRemainData_Response(_recv, _retErrorCode);
            break;
        }
        case RECEIVE_DATA:
        {
            retValue = GRPS_AT_Receive_Response(_recv, _retErrorCode);
            break;
        }
        case OFF_CALL:
        {
            retValue = GPRS_AT_OFF_CALL_Response(_recv);
            break;
        }
        default:
        {
            retValue = TRUE;
            break;
        }
    }
    return retValue;
}


//ֻҪ����ERROR����OK,������Ӧִ�����ˣ���Ҫ�������������������⣬�����������!!!!!!!!!!! 17-08-21
int GPRS_SubProc_AT_ResponseTwo(char *_recv, int *_retErrorCode)
{
    int retValue = FALSE;
    if(Utility_Strncmp(_recv,"OK",2)==0)
    {
        retValue = TRUE;
    }
    else if(Utility_Strncmp(_recv,"CONNECT",7) ==0 )
    {
        retValue = TRUE;
    }else if(Utility_Strncmp(_recv,"ERROR",5)==0)//ERROR: 14  ð�ź�14�м��пո�
    {
        char bufErrorTmp[3] = {0};//����atoi��������Ҫ���ַ�����������2λ�����룬1λ������

        bufErrorTmp[0] = _recv[7];	
        if(_recv[8] != 0)
        {
            bufErrorTmp[1] = _recv[8];
        }
        *_retErrorCode = atoi(bufErrorTmp);	
        GPRS_Print_Error(*_retErrorCode);
        retValue = FALSE;
    }
    else
    {
        retValue = FALSE;
    }
    return retValue;
}

int GPRS_Proc_AT_Response(char *_recv, int *_retMsgNum, int cmdType, int  *errorCode)
{
    int _recvTryTimes1 = 0;
    int _retValue = FALSE;
    int _okFlag;//�趨�����ж��費��Ҫ��2����Ӧ

    //ֻ����һ��OK�ģ�okFlag��ʼΪTRUE,����2�еģ���ʼΪFALSE
    switch(cmdType)
    {
        case SET_APN:
        {
            _okFlag = TRUE;
            break;
        }
        case ACTIVE_GPRS:
        {
            _okFlag = TRUE;
            break;
        }
        case CHECK_SIM_STATUS:
        {  
            _okFlag = FALSE;
            break;
        }
        case QUERY_OPERATER:
        {  
            _okFlag = FALSE;
            break;
        }
        case QUERY_IP:
        {  
            _okFlag = FALSE;
            break;
        }
        case OPEN_TCP_LINK:
        {
            _okFlag = TRUE;
            break;
        }
        case QUERY_TCP_LINK:
        {
            _okFlag = FALSE;
            break;
        }
        case CLOSE_TCP_LINK:
        {
            _okFlag = FALSE;
            break;
        }
        case QUIT_GPRS_NET:
        {
            _okFlag = FALSE;
            break;
        }
        case SETIO_MODE:
        {
            _okFlag = TRUE;
            break;
        }
        case SEND_DATA:
        {  
            _okFlag = FALSE;
            break;
        }
        case QUERY_REMAIN_DATA:
        {  
            _okFlag = FALSE;
            break;
        }
        case RECEIVE_DATA:
        {  
            _okFlag = FALSE;
            break;
        }
        case OFF_CALL:
        {
            _okFlag = TRUE;
            break;
        }
        default:
        {
            _okFlag = TRUE;
            break;
        }
    } 

    
    while(_recvTryTimes1 < 100)
    {
        if(UART0_RecvLineTry(_recv,UART0_MAXBUFFLEN,_retMsgNum)==-1)
        {
            _recvTryTimes1 += 1;
            if(cmdType == ACTIVE_GPRS)
                System_Delayms(700);//ԭ�����������ʱ30s���������֣����������ָ��պõ绰��ǰ�������ģ��Ῠס��Ҳ������ring������ֱ�����Ž�������ͨ��45s�Զ��Ҷϣ��ƶ���65s�Զ��Ҷ�
            else if(cmdType == QUERY_REMAIN_DATA)
                System_Delayms(100);
            else 
                System_Delayms(500);//ֻ�м���GPRS�Ῠ��ʱ�䣬���������ʱ�䲻����30s
            continue;
        }
        else
        {
            if(_okFlag == FALSE)//������صĵ�һ�в���OK,����Ҫ������һ�У�����ֱ�Ӵ����ڶ���
            {
                int respRet ;
                respRet = GPRS_SubProc_AT_ResponseOne(_recv,_retMsgNum,cmdType, errorCode);
                if(respRet == TRUE)//�����һ�������з��ؽ�����ͼ�������������ȷҲ������յ�һ��OK,ֻҪ��һ�������ִ����־���յ���Ϣ������һ���ߣ�����һ������
                {
                    _okFlag = TRUE;
                }
                else//�����һ�����ؽ����ΪTRUE�����治��Ҫ�����ˡ�
                {
                    _retValue = FALSE;
                    break;
                }
            }
            else
            {
                _retValue = GPRS_SubProc_AT_ResponseTwo(_recv, errorCode);
                if(_retValue == TRUE){
                    break;
                }
                else
                {
                    if(errorCode!=0)//ֻ�г��ִ�����˳������򲻷���
                        break;
                }
            }
        }
    }
    return _retValue;
}

int GPRS_WriteBytes(char* psrc, int len)
{
    UART0_Send(psrc, len ,1);
    return 0;
}

int GPRS_AT_SetAPN()
{
    int errorCode = 0;
    int _retMsgNum = 0;
    int _cmdLen = 0;
    int _retValue = FALSE;
    char _send[30] = "at+cgdcont=1,\"ip\",\"";
    char _recv[30] = {0};
    
    _cmdLen += 19;
    
    if(TelecomOperater == CHINA_MOBILE)
    {
        Utility_Strncpy(&_send[_cmdLen], "cmnet", Utility_Strlen("cmnet"));
    }
    else
        return SET_APN_FAILED;
    
    _cmdLen += Utility_Strlen("cmnet");
    _send[_cmdLen] = '\"';
    _cmdLen += 1;
  
  //��ATָ��᷵��:
  //OK
  //ע�⣬��Ҫ�ȹرջ��ԣ���������鷳
    GPRS_PrepareForSend();
    GPRS_WriteBytes(_send,_cmdLen);
    _retValue = GPRS_Proc_AT_Response(_recv, &_retMsgNum, SET_APN, &errorCode);

    if(_retValue==TRUE)
    {
        TraceMsg("Set APN Success",1);
        return SET_APN_SUCCESS;
    }
    else
    {
        Console_WriteStringln("Set APN Failed");
    }

    return SET_APN_FAILED;
}

int GPRS_AT_ActiveGPRSNet()
{
    int errorCode=0;
    int _retMsgNum = 0;//
    int _cmdLen = 0;
    int _retValue = 0;
    char _send[10] = "at%etcpip";
    char _recv[30] = { 0 };
    _cmdLen += 9;


    //��ATָ��᷵��:
    //OK
    //ע�⣬��Ҫ�ȹرջ��ԣ���������鷳
    GPRS_PrepareForSend();
    GPRS_WriteBytes(_send,_cmdLen);
    _retValue = GPRS_Proc_AT_Response(_recv, &_retMsgNum, ACTIVE_GPRS, &errorCode);

    if(_retValue == TRUE)
    {
        TraceMsg("Active GPRS success",1);
        return ACTIVE_GPRS_SUCCESS;
    }
    else
    {
        Console_WriteStringln("Active GPRS failed");
    }
    	
    return ACTIVE_GPRS_FAILED;
}


int GPRS_AT_CheckSimStat()
{
    int * errorCode = NULL;
    int _retMsgNum = 0;//
    int _cmdLen = 0;
    int _retValue = 0;
    char _send[10] = "AT%TSIM";
    char _recv[30] = { 0 };
    _cmdLen += 7;


    //��ATָ��᷵��:
    //OK
    //ע�⣬��Ҫ�ȹرջ��ԣ���������鷳
    GPRS_PrepareForSend();
    GPRS_WriteBytes(_send,_cmdLen);
    _retValue = GPRS_Proc_AT_Response(_recv, &_retMsgNum, CHECK_SIM_STATUS, errorCode);

    if(_retValue == TRUE)
    {
        TraceMsg("CheckSimStat Success",1);
        return GPRS_CHECKSIM_SUCCESS;
    }else
    {
        Console_WriteStringln("CheckSimStat failed");
    }

    return GPRS_CHECKSIM_FAILED;
}

int GPRS_AT_QueryOperater()
{
    int errorCode=0;
    int _retMsgNum = 0;//
    int _cmdLen = 0;
    int _retValue = 0;
    char _send[8] = "AT+COPS?";
    char _recv[100] = {0};
    _cmdLen += 8;

    //��ATָ��᷵��:
    //OK
    //ע�⣬��Ҫ�ȹرջ��ԣ���������鷳
    GPRS_PrepareForSend();
    GPRS_WriteBytes(_send,_cmdLen);
    _retValue = GPRS_Proc_AT_Response(_recv, &_retMsgNum, QUERY_OPERATER, &errorCode);

    if(_retValue == TRUE)
    {
        if(TelecomOperater == CHINA_MOBILE)
        {
            TraceMsg("Operater is China Mobile",1);
            return QUERY_OPERATER_SUCCESS;
        }
    }
    else
    {
        Console_WriteStringln("QUERY Operater Failed");
    }


    return QUERY_OPERATER_FAILED;
}

int GPRS_AT_QueryIP()
{
    int errorCode=0;
    int _retMsgNum = 0;//
    int _cmdLen = 0;
    int _retValue = 0;
    char _send[10] = "at%etcpip?";
    char _recv[100] = {0};
    _cmdLen += 10;

    //��ATָ��᷵��:
    //OK
    //ע�⣬��Ҫ�ȹرջ��ԣ���������鷳
    GPRS_PrepareForSend();
    GPRS_WriteBytes(_send,_cmdLen);
    _retValue = GPRS_Proc_AT_Response(_recv, &_retMsgNum, QUERY_IP, &errorCode);
    // ��ѯָ����ȷ���һ��%ETCPIP:1,"10.101.53.141",,"114.114.114.114","211.138.156.146", 
    //ע�⣬ֻ���ر���IP�����У���Ҫ�����غ�DNS���������������ӻ������
    // �ٷ���һ�� OK

    if(_retValue == TRUE)
    {
        TraceMsg("QUERY IP Success",1);
        return QUERY_IP_SUCCESS;
    }
    else
    {
        Console_WriteStringln("QUERY IP Failed");
    }


    return QUERY_IP_FAILED;
}

void judgeintlen(int *ip,char *iplen,char len)
{
    char i;
    for(i = 0;i < len;i++)
    {
        if(ip[i] >= 100)
            iplen[i] = 3;
        else if(ip[i] >= 10)
            iplen[i] = 2;
        else
            iplen[i] = 1;
    }
}

char judgeportlen(char *port)
{
    char portlen[3] = {0,0,0};
    char i;
    
    for(i = 0;i < 3;i++)
    {
        if(port[i] >= 10)
            portlen[i] = 2;
        else if(port[i] > 0)
            portlen[i] = 1;
        else
            portlen[i] = 0;
    }
    return portlen[0]+portlen[1]+portlen[2];
}

int GPRS_AT_OpenTCPLink(int center)
{
    int errorCode=0;
    int _retMsgNum = 0;//
    int _cmdLen = 0;
    int _retValue = 0;
    int IPStrLen = 0;
    int PortStrLen = 0;
    char _recv[30] = {0};
    char centerip[6] = {0,0,0,0,0,0};
    int ip[4] = {0,0,0,0};
    char iplen[4] = {0,0,0,0};
    char i,j;
    char centerport[3] = {0,0,0};
    char centerportlen = 0;
    
    char ipvalue[10];

    char _send[50] = "AT%IPOPEN=\"TCP\",\""; 	
    _cmdLen += 17;
    
    memset(ipvalue,0,sizeof(ipvalue));
    Hydrology_ReadStoreInfo(center,ipvalue,HYDROLOGY_CENTER_IP_LEN);
    memcpy(centerip,&ipvalue[1],6);
    memcpy(centerport,&ipvalue[7],3);
    
//    Store_ReadHydrologyServerIP(centerip,center);
    centerip[0] = _BCDtoDEC(centerip[0]);
    centerip[1] = _BCDtoDEC(centerip[1]);
    centerip[2] = _BCDtoDEC(centerip[2]);
    centerip[3] = _BCDtoDEC(centerip[3]);
    centerip[4] = _BCDtoDEC(centerip[4]);
    centerip[5] = _BCDtoDEC(centerip[5]);
    
    ip[0] = centerip[0]*10 + centerip[1]/10;
    ip[1] = (centerip[1]%10)*100 + centerip[2];
    ip[2] = centerip[3]*10 + centerip[4]/10;
    ip[3] = (centerip[4]%10)*100 + centerip[5];
    
    judgeintlen(ip,iplen,4);
    
    for(i = 0,j = 0;i < 4;i++)
    {
        if(iplen[i] == 3)
        {
            centerIP[j] = ip[i]/100+'0';
            centerIP[j+1] = ((ip[i]/10)%10)+'0';
            centerIP[j+2] = ip[i]%10+'0';
            centerIP[j+3] = '.';
            j += 4;
        }
        else if(iplen[i] == 2)
        {
            centerIP[j] = ip[i]/10+'0';
            centerIP[j+1] = ip[i]%10+'0';
            centerIP[j+2] = '.';
            j += 3;
        }
        else
        {
            centerIP[j] = ip[i]+'0';
            centerIP[j+1] = '.';
            j += 2;
        }
    }
    centerIP[j-1] = '\0';
    TraceMsg(centerIP,1);
    
    IPStrLen = Utility_Strlen(centerIP);
    Utility_Strncpy(&_send[_cmdLen], centerIP, IPStrLen);
    _cmdLen += IPStrLen;

    _send[_cmdLen] = '\"';
    _cmdLen += 1;

    _send[_cmdLen] = ',';
    _cmdLen += 1;
    
//    Store_ReadHydrologyServerPort(centerport,center);
    centerport[0] = _BCDtoDEC(centerport[0]);
    centerport[1] = _BCDtoDEC(centerport[1]);
    centerport[2] = _BCDtoDEC(centerport[2]);
    
    centerportlen = judgeportlen(centerport);
    
    switch(centerportlen)
    {
        case 6:
        {
            centerPort[0] = centerport[0]/10+'0';
            centerPort[1] = centerport[0]%10+'0';
            centerPort[2] = centerport[1]/10+'0';
            centerPort[3] = centerport[1]%10+'0';
            centerPort[4] = centerport[2]/10+'0';
            centerPort[5] = centerport[2]%10+'0';
            centerPort[6] = '\0';
            break;
        }
        case 5:
        {
            centerPort[0] = centerport[0]%10+'0';
            centerPort[1] = centerport[1]/10+'0';
            centerPort[2] = centerport[1]%10+'0';
            centerPort[3] = centerport[2]/10+'0';
            centerPort[4] = centerport[2]%10+'0';
            centerPort[5] = '\0';
            break;
        }
        case 4:
        {
            centerPort[0] = centerport[1]/10+'0';
            centerPort[1] = centerport[1]%10+'0';
            centerPort[2] = centerport[2]/10+'0';
            centerPort[3] = centerport[2]%10+'0';
            centerPort[4] = '\0';
            break;
        }
        case 3:
        {
            centerPort[0] = centerport[1]%10+'0';
            centerPort[1] = centerport[2]/10+'0';
            centerPort[2] = centerport[2]%10+'0';
            centerPort[3] = '\0';
            break;
        }
        case 2:
        {
            centerPort[0] = centerport[2]/10+'0';
            centerPort[1] = centerport[2]%10+'0';
            centerPort[2] = '\0';
            break;
        }
        case 1:
        {
            centerPort[0] = centerport[2]%10+'0';
            centerPort[1] = '\0';
            break;
        }
        
    }
    if(Utility_Strncmp("59.48.98.130",centerIP,12) == 0)
        Utility_Strncpy(centerPort,"5005",4);
    PortStrLen = Utility_Strlen(centerPort);
    Utility_Strncpy(&_send[_cmdLen], centerPort, PortStrLen);
    _cmdLen += PortStrLen;
    
    TraceMsg(centerPort,1);

    //��ATָ��᷵��:
    //OK
    //ע�⣬��Ҫ�ȹرջ��ԣ���������鷳

    GPRS_PrepareForSend();
    GPRS_WriteBytes(_send,_cmdLen);
    _retValue = GPRS_Proc_AT_Response(_recv, &_retMsgNum, OPEN_TCP_LINK, &errorCode);
    //�����ɹ��᷵��"CONNECT"

    if(_retValue == TRUE)
    {
        TraceMsg("TCP Connection success",1);
        //Console_WriteStringln("TCP Connection success");
        return TCP_CONNECT_SUCCESS;
    }
    else
    {
        Console_WriteStringln("TCP Connection failed-1223");
        //++SIM_ErrorNum;
    }

    return TCP_CONNECT_FAILED;
}


//�˺�����ʱδ�õ�
int GPRS_AT_QueryTCPLink(int * errorCode)
{
    int _retMsgNum = 0;//
    int _cmdLen = 0;
    int _retValue = 0;
    char _send[10] = "AT%IPOPEN?";
    char _recv[100] = {0};
    _cmdLen += 10;

    //��ATָ��᷵��:
    //OK
    //ע�⣬��Ҫ�ȹرջ��ԣ���������鷳
    GPRS_PrepareForSend();
    GPRS_WriteBytes(_send,_cmdLen);
    _retValue = GPRS_Proc_AT_Response(_recv, &_retMsgNum, QUERY_TCP_LINK, errorCode);
    // ��ѯָ����ȷ���һ��%ETCPIP:1,"10.101.53.141",,"114.114.114.114","211.138.156.146", 
    //ע�⣬ֻ���ر���IP�����У���Ҫ�����غ�DNS���������������ӻ������
    // �ٷ���һ�� OK

    if(_retValue == FALSE)//0��֤��û�м���GPRS����
    {
        Console_WriteStringln("No Tcp Connection error");
        return NO_TCP_CONNECT;
    }
    else
    {
        TraceMsg("Tcp Connection OK",1);
        Console_WriteStringln("Tcp Connection OK");
        return TCP_CONNECT_OK;
    }

}

int GPRS_AT_CloseTCPLink(int * errorCode)
{
    int _retMsgNum = 0;//
    int _cmdLen = 0;
    int _retValue = 0;
    char _recv[30] = {0};


    char _send[50] = " AT%IPCLOSE=1";
    _cmdLen += 12;

    //��ATָ��᷵��:
    //%IPCLOSE:
    //OK
    //ע�⣬��Ҫ�ȹرջ��ԣ���������鷳
    GPRS_PrepareForSend();
    GPRS_WriteBytes(_send,_cmdLen);
    _retValue = GPRS_Proc_AT_Response(_recv, &_retMsgNum, CLOSE_TCP_LINK, errorCode);

    if(_retValue == FALSE)
    {
        Console_WriteStringln("Close TCP link failed-1286");
        return TCP_Link_CLOSE_FAILED;
    }else
    {
        TraceMsg("Close TCP link Success",1);
        return TCP_Link_CLOSE_SUCCESS;
    }

}


int GPRS_AT_QuitGPRSNet(int * errorCode)
{
    int _retMsgNum = 0;//
    int _cmdLen = 0;
    int _retValue = 0;
    char _recv[30] = {0};
    char _send[50] = "AT%IPCLOSE=5";
    _cmdLen += 12;

    //��ATָ��᷵��:
    //%IPCLOSE:
    //OK
    //ע�⣬��Ҫ�ȹرջ��ԣ���������鷳
    GPRS_PrepareForSend();
    GPRS_WriteBytes(_send,_cmdLen);
    _retValue = GPRS_Proc_AT_Response(_recv, &_retMsgNum, QUIT_GPRS_NET, errorCode);
    //�����ɹ��᷵��"CONNECT"

    if(_retValue == TRUE)
    {
        TraceMsg("Quit GPRS Net Success",1);
        //Console_WriteStringln("Quit GPRS Net Success");
        return GPRS_NET_QUIT_SUCCESS;
    }else
    {
        Console_WriteStringln("Quit GPRS Net failed");
    }

    return GPRS_NET_QUIT_FAILED;
}

int gprsConfigSuccess = FALSE;
//ʹ��ASCii�뻹��Hex�뷢����Ϣ��0,1,1---ASCii�� 1,1,0---Hex
int GPRS_AT_SetIOMode()
{
    int errorCode=0;
    int _retMsgNum = 0;//
    int _cmdLen = 0;
    int _retValue = 0;
    char _send[20] = "at%iomode=1,1,0";
    char _recv[30] = { 0 };
    _cmdLen += 15;

    //��ATָ��᷵��:
    //OK
    //ע�⣬��Ҫ�ȹرջ��ԣ���������鷳
    GPRS_PrepareForSend();
    GPRS_WriteBytes(_send,_cmdLen);
    _retValue = GPRS_Proc_AT_Response(_recv, &_retMsgNum, SETIO_MODE, &errorCode);

    if(_retValue == TRUE)
    {
        TraceMsg("Set IO Mode Success",1);
        return SET_IOMODE_SUCCESS;
    }else
    {
        Console_WriteStringln("Set IO Mode Failed");
    }

    return SET_IOMODE_FAILED;
}

//int GRPS_AT_Send(char* psrc, int dataLen, int * errorCode)
int GRPS_AT_Send()
{
#if 0
    if(_GTM900SendBufLeft == 0 )//������ͻ���=0���򲻷��͡�
    {
        return MSG_SEND_FAILED;
    }
#endif

    int errorCode=0;
    int _retMsgNum = 0;//
    int _cmdLen = 0;
    int _retValue = 0;
    char _recv[30] = {0};

    char *_send = (char *)pvPortMalloc(dataLen+12);
    if(_send == NULL)
    {
        Console_WriteStringln("Malloc in GPRS_AT_Send failed");
        printf("pvPortMalloc(%d) failed \r\n",dataLen+12);
        return MSG_SEND_FAILED;
    }
    Utility_Strncpy(_send,"AT%IPSEND=\"",11);
    _cmdLen += 11;

    Utility_Strncpy(&_send[_cmdLen], psrc, dataLen);
    _cmdLen += dataLen;

    _send[_cmdLen] = '\"';
    _cmdLen += 1;

    //��ATָ��᷵��:
    //OK
    //ע�⣬��Ҫ�ȹرջ��ԣ�������ɺ������ݽ��ո���
    GPRS_PrepareForSend();
    GPRS_WriteBytes(_send,_cmdLen);
    _retValue = GPRS_Proc_AT_Response(_recv, &_retMsgNum, SEND_DATA, &errorCode);
    // ����ָ����ȷ���һ�� %IPSEND:1,15
    // �ٷ���һ�� OK
    vPortFree(_send);
    _send = NULL;

    if(_retValue == TRUE)
    {
        TraceMsg("AT Send Data Success",1);
        return MSG_SEND_SUCCESS;
    }
    else
    {
        Console_WriteStringln("AT Send Data failed-sub1408");
    }
    
    return MSG_SEND_FAILED;
}

int GRPS_AT_QueryRemainData()
{
    int errorCode=0;
    int _retMsgNum = 0;
    int _cmdLen = 0;
    int _retValue = 0;
    char _send[8] = "AT%IPDQ";
    char _recv[200] = {0};
    
    _cmdLen += 7;
    //��ATָ��᷵��:
    //OK
    //ע�⣬��Ҫ�ȹرջ��ԣ�������ɺ������ݽ��ո���
    GPRS_PrepareForSend();
    GPRS_WriteBytes(_send,_cmdLen);
    _retValue = GPRS_Proc_AT_Response(_recv, &_retMsgNum, QUERY_REMAIN_DATA, &errorCode);
    // ����ָ����ȷ���һ�� %IPDQ:
    // �ٷ���һ�� OK

    if(_retValue == TRUE)
    {
        return QUERY_REMAIN_DATA_SUCCESS;
    }
    else
    {
        Console_WriteStringln("AT Query remaining data failed");
    }
    
    return QUERY_REMAIN_DATA_FAILED;
}

int GRPS_AT_Receive()
{
    int errorCode=0;
    int _retMsgNum = 0;
    int _cmdLen = 0;
    int _retValue = 0;
    char _send[8] = "AT%IPDR";
    char _recv[200] = {0};
    
    _cmdLen += 7;
    //��ATָ��᷵��:
    //OK
    //ע�⣬��Ҫ�ȹرջ��ԣ�������ɺ������ݽ��ո���
    GPRS_PrepareForSend();
    GPRS_WriteBytes(_send,_cmdLen);
    _retValue = GPRS_Proc_AT_Response(_recv, &_retMsgNum, RECEIVE_DATA, &errorCode);
    // ����ָ����ȷ���һ�� %IPDR:1,1,87,"7E7E"
    // �ٷ���һ�� OK

    if(_retValue == TRUE)
    {
        TraceMsg("AT Receive Data Success",1);
        return MSG_RECEIVE_SUCCESS;
    }
    else
    {
        Console_WriteStringln("AT Receive Data failed");
    }
    
    return MSG_RECEIVE_FAILED;
}

int GPRS_OFF_CALL()
{
    int _retNum;
    char _ath[]="ATH";
    char _recv[50];
    //��ָ������Ҷ��˵绰,����OK,�޵绰״̬ʱ,Ҳ����OK,OKֻ��ʾģ���յ���ָ��
    //OK  
    UART0_Send(_ath,3,1);//����ring֮��ʱ������ring��OK����ʱ����ˣ�OK�϶����ȳ���?    if(UART0_RecvLineWait(_recv,50,&_retNum)==-1)
    if(UART0_RecvLineWait(_recv,50,&_retNum)==-1)
    {
        return 0;
    }
    if(Utility_Strncmp(_recv,"OK",2)==0)
    {
        return -3; 
    }
    else
    {
        return 0;
    }
}


int GPRS_AT_OFF_CALL()
{
    int _retNum = 0;
    char _ath[]="ATH";
    char _recv[50];
    //��ָ������Ҷ��˵绰,����OK,�޵绰״̬ʱ,Ҳ����OK,OKֻ��ʾģ���յ���ָ��
    //OK  
    UART0_Send(_ath,3,1);//����ring֮��ʱ������ring��OK����ʱ����ˣ�OK�϶����ȳ���?    
    if(UART0_RecvLineWait(_recv,50,&_retNum)==-1)
    {
        return 0;
    }
    if(Utility_Strncmp(_recv,"OK",2)==0)
    {
        return -3; 
    }
    else
    {
        return 0;
    }
}

int  GPRS_ClearRecvBuf()
{
    UART0_ClearBuffer();
    return 0;
}

void GPRS_PrepareForSend()
{
    char _recvBeforeSend[100];
    int _retLenghtBeforeSend;

    if(UART0_RecvLineTry(_recvBeforeSend,UART0_MAXBUFFLEN,&_retLenghtBeforeSend)==0)
    {
        if(Utility_Strncmp(_recvBeforeSend,"RING",4)==0)//����ǵ绰���ˣ����̹رյ绰��������Ӧһ�Ų�����
        {
            GPRS_AT_OFF_CALL();
        }
    }
    GPRS_ClearRecvBuf();//��ջ���
}

int GPRS_Create_TCP_Link(int center)
{    
    int _repeats=0;
    
    if(gprsConfigSuccess == FALSE)
    {
        GSM_Open();
        
        while(GSM_AT_CloseFeedback() != 0)
        {
            if(_repeats>0)
                return -1;
            ++_repeats;
        } 
        _repeats = 0;
        
        while(GPRS_AT_CheckSimStat() != GPRS_CHECKSIM_SUCCESS)
        {
            if(_repeats>0)
                return -1;
            ++_repeats;
        } 
        _repeats = 0;
        
        while(GPRS_AT_QueryOperater() != QUERY_OPERATER_SUCCESS)
        {
            if(_repeats>0)
                return -1;
            ++_repeats;
        } 
        _repeats = 0;
        
        while(GPRS_AT_SetAPN() != SET_APN_SUCCESS)
        {
            if(_repeats>0)
                return -1;
            ++_repeats;
        } 
        _repeats = 0;
        
        while(GPRS_AT_ActiveGPRSNet() != ACTIVE_GPRS_SUCCESS)
        {
            if(_repeats>0)
                return -1;
            ++_repeats;
        } 
        _repeats = 0;
        
        while(GPRS_AT_QueryIP() != QUERY_IP_SUCCESS)
        {
            if(_repeats>0)
                return -1;
            ++_repeats;
        } 
        _repeats = 0;
        
        while(GPRS_AT_OpenTCPLink(center) != TCP_CONNECT_SUCCESS)
        {
            if(_repeats>0)
                return -1;
            ++_repeats;
        } 
        _repeats = 0;
        
        while(GPRS_AT_SetIOMode() != SET_IOMODE_SUCCESS)
        {
            if(_repeats>0)
                return -1;
            ++_repeats;
        } 
        _repeats = 0;
    
        gprsConfigSuccess = TRUE;
    }
    
    return 0;
}

int GPRS_Close_TCP_Link()
{
    int _repeats=0;
    int _retErrorCode;
    
    gprsConfigSuccess = FALSE;
    
    while(GPRS_AT_CloseTCPLink(&_retErrorCode) != TCP_Link_CLOSE_SUCCESS)
    {
        if(_repeats>0)
            return -1;
        ++_repeats;
    } 
    _repeats = 0;
    
    while(GPRS_AT_QuitGPRSNet(&_retErrorCode) != GPRS_NET_QUIT_SUCCESS)
    {
        if(_repeats>0)
            return -1;
        ++_repeats;
    } 
    _repeats = 0;
    
    return 0;
}

void GPRS_Close_GSM()
{
    gprsConfigSuccess = FALSE;
    GSM_Close(1);
    System_Delayms(1000);//�ػ���ʱ1s
}

int GPRS_Send(char* pSend, int sendDataLen, int isLastPacket, int center)
{
    int _retvalue;
    //���Ȳ鿴TCP���ӽ���û�У�û�н�������Ҫ���½�������?
    if(gprsConfigSuccess == FALSE)
    {
        GPRS_Close_GSM();
        GPRS_Create_TCP_Link(center);
    }

    if( sendDataLen > 1024)
    {
        Console_WriteStringln("Error, Packet exceed 1024, Please Decrease Sending Packet Size!");
    }

    psrc = (char *)pvPortMalloc(sendDataLen*2+1);//+1����Ϊת������hex_2_ascii����и�\0,���������Խ�����
    if(psrc == NULL)
    {
        Console_WriteStringln("Malloc in GPRS_Send Failed");
        printf(" pvPortMalloc(%d) failed \r\n",sendDataLen*2+1);
        return FALSE;
    }
    hex_2_ascii(pSend, psrc, sendDataLen);
    dataLen = sendDataLen*2;
    
    _retvalue = GRPS_AT_Send();

    vPortFree(psrc);
    psrc = NULL;
    
    if(_retvalue == MSG_SEND_SUCCESS)
    {
        TraceMsg("GPRS Send Data Success",1);
        return TRUE;
    }
    else
    {
        TraceMsg("GPRS Send Data Failed-1690",1);
    }
    
    return FALSE;
}

int GPRS_QueryRemainData()
{
    if(GRPS_AT_QueryRemainData() == QUERY_REMAIN_DATA_SUCCESS)
    {
        return RemainingData;
    }
    return -1;
}

char* GPRS_Receive()
{
    char* hexdata = NULL;
    
    if(GRPS_AT_Receive() == MSG_RECEIVE_SUCCESS)
    {
        hexdata = (char*)pvPortMalloc(_ReceiveDataLen);
        if(hexdata == 0)
        {
            printf("pvPortMalloc(%d) failed \r\n",_ReceiveDataLen);
        }
        ConvertAscIItoHex(_ReceiveData, hexdata, _ReceiveDataLen*2);
        vPortFree(_ReceiveData);
        _ReceiveData = NULL;
    }
    else if(GRPS_AT_Receive() == MSG_RECEIVE_SUCCESS)
    {
        hexdata = (char*)pvPortMalloc(_ReceiveDataLen);
        if(hexdata == 0)
        {
            printf("pvPortMalloc(%d) failed \r\n",_ReceiveDataLen);
        }
        ConvertAscIItoHex(_ReceiveData, hexdata, _ReceiveDataLen*2);
        vPortFree(_ReceiveData);
        _ReceiveData = NULL;
    }
    else if(GRPS_AT_Receive() == MSG_RECEIVE_SUCCESS)
    {
        hexdata = (char*)pvPortMalloc(_ReceiveDataLen);
        if(hexdata == 0)
        {
            printf("pvPortMalloc(%d) failed \r\n",_ReceiveDataLen);
        }
        ConvertAscIItoHex(_ReceiveData, hexdata, _ReceiveDataLen*2);
        vPortFree(_ReceiveData);
        _ReceiveData = NULL;
    }
      
    return hexdata;
}

char unprocessedGPRSdata = 0;
int GPRSDataArrived = FALSE;
void JudgeServerDataArrived(void)
{
    if(Utility_Strncmp(UART0_Rx_Buffer[0], "%IPDATA:",8) == 0)
    {
      memset(UART0_Rx_Buffer[0],0,UART0_MAXBUFFLEN);
      GPRSDataArrived = TRUE;
    }
    if(Utility_Strncmp(UART0_Rx_Buffer[1], "%IPDATA:",8) == 0)
    {
      memset(UART0_Rx_Buffer[1],0,UART0_MAXBUFFLEN);
      GPRSDataArrived = TRUE;
    }
    if(Utility_Strncmp(UART0_Rx_Buffer[2], "%IPDATA:",8) == 0)
    {
      memset(UART0_Rx_Buffer[2],0,UART0_MAXBUFFLEN);
      GPRSDataArrived = TRUE;
    }
}

int Hydrology_ProcessGPRSReceieve()
{
  char *dowmhydrologydata = NULL;
  
  if(GPRSDataArrived != TRUE)
    return -1;
//   GPRS_QueryRemainData();
   unprocessedGPRSdata = 1;
  while(unprocessedGPRSdata--)
  {
    dowmhydrologydata = GPRS_Receive();
    TraceHexMsg(dowmhydrologydata,_ReceiveDataLen);
     if(hydrologyProcessReceieve(dowmhydrologydata, _ReceiveDataLen) != 0)
        continue;
    HydrologyRecord(ERC17);
  }
  if(dowmhydrologydata != NULL)
    vPortFree(dowmhydrologydata);
  
  GPRSDataArrived = FALSE;
  return 0;
}









