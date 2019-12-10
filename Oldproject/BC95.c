#include <msp430x54x.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "BC95.h"
#include "uart1.h"
#include "uart0.h"
#include "common.h"
#include "console.h"
#include <stdlib.h>
#include "uart0.h"
#include "FreeRTOS.h"

//���ATָ��

#define AT_BC95_NCONFIG "AT+NCONFIG?"
#define AT_BC95_SCRAMB_1 "AT+NCONFIG=CR_0354_0338_SCRAMBLING,TRUE" //�������빦��
#define AT_BC95_SCRAMB_2 "AT+NCONFIG=CR_0859_SI_AVOID,TURE"
#define AT_BC95_AUTOCONFIG "AT+NCONFIG=AUTOCONNECT,TRUE" //���ó��Զ�ģʽ
#define AT_BC95_SIM "AT+CIMI"							 //ѯ��sim��
#define AT_BC95_CFUN "AT+CFUN?"							 //ѯ���Ƿ���ȫ����ģʽ
#define AT_BC95_SFUN "AT+CFUN=1"						 //����ȫ����ģʽ
#define AT_BC95_CSQ "AT+CSQ"							 //��ѯ�ź�
#define AT_BC95_CGATT "AT+CGATT?"						 //��ѯ����״̬
#define AT_BC95_NNMI "AT+NNMI=1"						 //�����յ������ݴ�ӡ������
#define AT_BC95_CPSMS "AT+CPSMS?"						 //��ѯ�Ƿ���˯��ģʽ
#define AT_BC95_SPSMS "AT+CPSMS=0"						 //�ر�˯��ģʽ��������ʾ���յ�������
#define AT_BC95_ATE0 "ATE0"								 //�رջ���
#define AT_BC95_NSMI "AT+NSMI=1"						 //���÷�����Ϣ�󷵻��Ƿ��ͳɹ�

#define BC95_WAITTIME 300 //30S
#define BC95_DELAY_MS 1000
#define BC95_REPEAT_TIMES 10
#define BC95_RETRY_TIMES 3

int g_autoconfigState = 0; //���빦��
int g_scrambState = 0;	 ////�Զ�ģʽ
int g_cfunstate = 0;	   //ȫ����ģʽ
char g_BC95RcvBuf[400];

int BC95_Char_to_Hex(char chr)
{
	if ((chr >= 0x00) && (chr <= 0x09))
	{
		chr = chr + 0x30;
	}
	else
	{
		chr = chr + 87;
	}

	return chr;
}

int BC95_Extract_DownData(char *inputData, int inputDataLen, char *outputData)
{
	int i = 0, j = 0, k = 0;
	int data_len = 0;
	char *temp;

	while (inputData[i] != ',')
	{
		i++;
	}

	temp = (char *)pvPortMalloc(sizeof(char) * (inputDataLen - i));

	for (j = i + 1, k = 0; j < inputDataLen; j++)
	{
		temp[k] = inputData[j];
		k++;
	}

	AsciiStr_to_ASCII(inputData, inputDataLen, outputData);

	data_len = inputDataLen / 2;

	vPortFree(temp);
        temp = NULL; //++++

	return data_len;
}

int BC95_UintToStr4(unsigned int _src, char *_dest)
{
	_dest[0] = (_src & 0xF000) >> 12;
	_dest[0] = BC95_Char_to_Hex(_dest[0]);

	_dest[1] = (_src & 0x0F00) >> 8;
	_dest[1] = BC95_Char_to_Hex(_dest[1]);

	_dest[2] = (_src & 0x00F0) >> 4;
	_dest[2] = BC95_Char_to_Hex(_dest[2]);

	_dest[3] = _src & 0x000F;
	_dest[3] = BC95_Char_to_Hex(_dest[3]);

	return 0;
}

/********
Function����BC95ģ���ϵ�
Description�� VBAT:P10.7  RST:P10.3  ( in msp2418: VBAT[P4.2]  IGT[P1.0] )
********/
int BC95_Open()
{
	int _repeat = 0;
	int _dataLen = 0;
	int second_time_MAX = 10; //�ȴ�10s
	char _data[UART1_MAXBUFFLEN] = {0};

	/* BC95 connect cpu via uart0 */
	UART0_Open_9600(UART0_GSM_TYPE);

	// Ϊģ���ϵ�
	P10DIR |= BIT7; // P10.7 Ϊ BATT,���ߵ�ƽ
	P10OUT &= ~BIT7;
	P10OUT |= BIT7;

	System_Delayms(100); // ��Ҫ��ʱ

	P10DIR |= BIT3; //  p3.0 Ϊ /IGT
	P10OUT &= ~BIT3;
	P10OUT |= BIT3;

	System_Delayms(10000);

	while (_repeat < second_time_MAX)
	{
		while (UART0_RecvLineTry(_data, UART1_MAXBUFFLEN, &_dataLen) == 0) //������
		{
			printf("BC95 response: %s \r\n", _data);
			if (strstr(_data, "OK"))
			{
				printf("BC95 Open \r\n");
				return 0;
			}
		}
		printf("[%d] waiting BC95 Ready ...\r\n", _repeat);
		_repeat++;
		System_Delayms(5000);
	}

	return 1;
}

/********
Function����BC95ģ��ϵ�
********/
void BC95_Close()
{
	P10OUT &= ~BIT7; //P4.2����͵�ƽ
	P10OUT &= ~BIT3;
	UART0_Close();
}

/********
Function��BC95��������
********/

int BC95_Send(char *data)
{
	UART0_Send(data, Utility_Strlen(data), 1);
	return 0;
}

char *BC95_QueryMsg(void)
{
	int _repeat = 0;
	int _dataLen = 0;
	char _data[UART1_MAXBUFFLEN] = {0};

	BC95_Send("AT+NQMGR");
	Console_WriteStringln("AT+NQMGR");

	while (_repeat < BC95_REPEAT_TIMES)
	{
		if (UART0_RecvLineTry(_data, UART1_MAXBUFFLEN, &_dataLen) == 0) //������
		{
			// printf("[BC95 Response] %s \r\n",_data);
			Console_WriteStringln(_data);
			return _data;
		}
		_repeat++;
		System_Delayms(50);
	}

	return NULL;
}

/********
Function����ѯģ��״̬
********/
void BC95_QueryState()
{

	int _repeat = 0;
	int _dataLen = 0;
	char _data[UART1_MAXBUFFLEN] = {0};

	BC95_Send(AT_BC95_NCONFIG);
	Console_WriteStringln(AT_BC95_NCONFIG);

	while (_repeat < BC95_REPEAT_TIMES)
	{
		while (UART0_RecvLineTry(_data, UART1_MAXBUFFLEN, &_dataLen) == 0) //������
		{
			Console_WriteStringln(_data);

			if (strstr(_data, "+NCONFIG:CR_0354_0338_SCRAMBLING,TRUE"))
			{
				g_scrambState = 1;
			}
			if (strstr(_data, "+NCONFIG:AUTOCONNECT,TRUE"))
			{
				g_autoconfigState = 1;
			}
			if ((g_autoconfigState == 1) && (g_scrambState == 1))
			{
				return;
			}
		}
		_repeat++;
		System_Delayms(50);
	}
}

/********
Function����ѯʱ��
********/
//int BC95_QueryTime(char *newtime)
//{
///* char *year,char *month,char *date,char *hour,char *min,char *second */
//	int _repeat = 0;
//	int _dataLen = 0;
//	char _data[UART1_MAXBUFFLEN] = {0};
//	int rcvFlag = 0;
//
//	BC95_Send("AT+CCLK?");
//	Console_WriteStringln("AT+CCLK?");
//
//	while (_repeat < BC95_REPEAT_TIMES)
//	{
//		if (UART0_RecvLineTry(_data, UART1_MAXBUFFLEN, &_dataLen) == 0) //������
//		{
//			Console_WriteStringln(_data);
//			if(strstr(_data,"+CCLK:") != NULL)
//			{
//				rcvFlag = 1;
//			}
//			break ;
//		}
//		_repeat++;
//		System_Delayms(50);
//	}
//
//	/* +CCLK: */
//	if(rcvFlag)
//	{
//		char *substr = "+CCLK:";
//		char *pcAtResp = strstr(_data, substr);
//		int offset = 6;
//		char year1,year2,month1,month2,date1,date2,hour1,hour2,min1,min2,sec1,sec2;
//		year1 = *(pcAtResp+offset);
//		offset++;
//		year2 = *(pcAtResp+offset);
//		offset++;
//		offset++;
//		month1 = *(pcAtResp+offset);
//		offset++;
//		month2 = *(pcAtResp+offset);
//		offset++;
//		offset++;
//		date1 = *(pcAtResp+offset);
//		offset++;
//		date2 = *(pcAtResp+offset);
//		offset++;
//		offset++;
//		hour1 = *(pcAtResp+offset);
//		offset++;
//		hour2 = *(pcAtResp+offset);
//		offset++;
//		offset++;
//		min1 = *(pcAtResp+offset);
//		offset++;
//		min2 = *(pcAtResp+offset);
//		offset++;
//		offset++;
//		sec1 = *(pcAtResp+offset);
//		offset++;
//		sec2 = *(pcAtResp+offset);
//
//	/* char *year,char *month,char *date,char *hour,char *min,char *second */
//		*newtime= (year1-48)*10 + (year2-48);
//                newtime++;
//		*newtime = (month1-48)*10 + (month2-48);
//                newtime++;
//		*newtime = (date1-48)*10 + (date2-48);
//                newtime++;
//		*newtime=( (hour1-48)*10 + (hour2-48)) + 8;
//                newtime++;
//		*newtime = (min1-48)*10 + (min2-48);
//                newtime++;
//		*newtime = (sec1-48)*10 + (sec2-48);
//                newtime++;
//
//		return 0;
//	}
//
//	return -1;
//}

int BC95_QueryTime(char *year,char *month,char *date,char *hour,char *min,char *second)
{
/* char *year,char *month,char *date,char *hour,char *min,char *second */
	int _repeat = 0;
	int _dataLen = 0;
	char _data[UART1_MAXBUFFLEN] = {0};
	int rcvFlag = 0;

	BC95_Send("AT+CCLK?");
	//Console_WriteStringln("AT+CCLK?");

	while (_repeat < BC95_REPEAT_TIMES)
	{
		if (UART0_RecvLineTry(_data, UART1_MAXBUFFLEN, &_dataLen) == 0) //������
		{
			Console_WriteStringln(_data);
			if(strstr(_data,"+CCLK:") != NULL)
			{
				rcvFlag = 1;
                                 break ;
			}
                       
			
		}
		_repeat++;
		System_Delayms(50);
	}

	/* +CCLK: */
	if(rcvFlag)
	{
		char *substr = "+CCLK:";
		char *pcAtResp = strstr(_data, substr);
		int offset = 6;
		char year1,year2,month1,month2,date1,date2,hour1,hour2,min1,min2,sec1,sec2;
		year1 = *(pcAtResp+offset);
		offset++;
		year2 = *(pcAtResp+offset);
		offset++;
		offset++;
		month1 = *(pcAtResp+offset);
		offset++;
		month2 = *(pcAtResp+offset);
		offset++;
		offset++;
		date1 = *(pcAtResp+offset);
		offset++;
		date2 = *(pcAtResp+offset);
		offset++;
		offset++;
		hour1 = *(pcAtResp+offset);
		offset++;
		hour2 = *(pcAtResp+offset);
		offset++;
		offset++;
		min1 = *(pcAtResp+offset);
		offset++;
		min2 = *(pcAtResp+offset);
		offset++;
		offset++;
		sec1 = *(pcAtResp+offset);
		offset++;
		sec2 = *(pcAtResp+offset);

	/* char *year,char *month,char *date,char *hour,char *min,char *second */
		*year = (year1-48)*10 + (year2-48);
		*month = (month1-48)*10 + (month2-48);
		*date = (date1-48)*10 + (date2-48);
		*hour =( (hour1-48)*10 + (hour2-48)) + 8;
		*min = (min1-48)*10 + (min2-48);
		*second = (sec1-48)*10 + (sec2-48);

		return 0;
	}

	return -1;
}

/********
Function���������빦��
********/

void BC95_SetScramb()
{
	BC95_Send(AT_BC95_SCRAMB_1);
	Console_WriteStringln(AT_BC95_SCRAMB_1);
	System_Delayms(BC95_DELAY_MS);

	BC95_Send(AT_BC95_SCRAMB_2);
	Console_WriteStringln(AT_BC95_SCRAMB_1);
	System_Delayms(BC95_DELAY_MS);
}

/********
Function�������Զ�ģʽ
********/
void BC95_SetAutoConfig()
{
	BC95_Send(AT_BC95_AUTOCONFIG);
	Console_WriteStringln(AT_BC95_AUTOCONFIG);
	System_Delayms(BC95_DELAY_MS);
}

/********
Function��ѯ��SIM��
********/
BC95State BC95_QuerySimState()
{

	int _repeat = 0;
	int _dataLen = 0;
	char _data[UART1_MAXBUFFLEN] = {0};

	BC95_Send(AT_BC95_SIM);
	Console_WriteStringln(AT_BC95_SIM);

	while (_repeat < BC95_REPEAT_TIMES)
	{
		while (UART0_RecvLineTry(_data, UART1_MAXBUFFLEN, &_dataLen) == 0) //������
		{
			Console_WriteStringln(_data);
			if (strstr(_data, "OK"))
			{
				return BC95StateOK;
			}
		}
		_repeat++;
		System_Delayms(50);
	}

	return BC95ErrorSIM;
}

char pcUDPBody[1048];
char *makeUDPBody(char *socket, char *ipAddr, char *port, char *len, char *data)
{
	strcat(pcUDPBody, "AT+NSOST=");
	strcat(pcUDPBody, socket);
	strcat(pcUDPBody, ",");
	strcat(pcUDPBody, ipAddr);
	strcat(pcUDPBody, ",");
	strcat(pcUDPBody, len);
	strcat(pcUDPBody, ",");
	strcat(pcUDPBody, data);

	return pcUDPBody;
}

/*
AT+NSOCR=DGRAM,17,4587,1 //Create a socket
0
OK
AT+NSOST=0,192.53.100.53,5683,25,400241C7B17401724D0265703D323031363038323331363438   //Send a message
0,25
OK
*/
BC95State BC95_SendUdpMsg(char *ipAddr, char *port, char *msg)
{
	//int _repeat = 0;
	//int _dataLen = 0;
	//char _data[UART1_MAXBUFFLEN] = {0};
	char *pcBC95Response = NULL;

	/* creat a socket */
	BC95_Send("AT+NSOCR=DGRAM,17,4587,1");
	printf("AT+NSOCR=DGRAM,17,4587,1 \r\n");
	pcBC95Response = BC95_Receive();
	// printf("%s \r\n",pcBC95Response);
	Console_WriteStringln(pcBC95Response);
	if (strstr(pcBC95Response, "OK"))
	{
		printf("UDP Socket Created \r\n");
	}
	else
	{
		printf("UDP Socket Creat failed! \r\n");
		return BC95ErrorCreatSocket;
	}

	/* send a message */
	makeUDPBody("0", ipAddr, port, "3", msg);
	BC95_Send(pcUDPBody);
	printf("%s \r\n", pcUDPBody);
	pcBC95Response = BC95_Receive();
	printf("%s \r\n", pcBC95Response);
	if (strstr(pcBC95Response, "OK"))
	{
		printf("UDP msg sent \r\n");
		return BC95StateOK;
	}
	else
	{
		printf("UDP msg send failed! \r\n");
		return BC95ErrorCreatSocket;
	}
}

/********
Function��ѯ���Ƿ���ȫ����ģʽ
********/

BC95State BC95_QueryFunState()
{

	int _repeat = 0;
	int _dataLen = 0;
	char _data[UART1_MAXBUFFLEN] = {0};

	BC95_Send(AT_BC95_CFUN);
	Console_WriteStringln(AT_BC95_CFUN);

	while (_repeat < BC95_REPEAT_TIMES)
	{
		while (UART0_RecvLineTry(_data, UART1_MAXBUFFLEN, &_dataLen) == 0)
		{
			Console_WriteStringln(_data);

			if (strstr(_data, "+CFUN:1"))
			{
				g_cfunstate = 1;
			}

			if ((strstr(_data, "OK")) && (g_cfunstate == 1))
			{
				return BC95StateOK;
			}
		}
		_repeat++;
		System_Delayms(50);
	}

	return BC95ErrorCFUN;
}

/********
Function�����ó�ȫ����ģʽ
********/
void BC95_SetCFun()
{
	BC95_Send(AT_BC95_SFUN);
	Console_WriteStringln(AT_BC95_SFUN);
	System_Delayms(BC95_DELAY_MS);
}

/********
Function����ѯ�ź�
********/
BC95State BC95_QueryCSQState()
{

	int _repeat = 0;
	int _dataLen = 0;
	char _data[UART0_MAXBUFFLEN] = {0};

	BC95_Send(AT_BC95_CSQ);
	Console_WriteStringln(AT_BC95_CSQ);

	while (_repeat < BC95_REPEAT_TIMES)
	{
		while (UART0_RecvLineTry(_data, UART1_MAXBUFFLEN, &_dataLen) == 0) //������
		{
			Console_WriteStringln(_data);

			if (strstr(_data, "+CSQ:99,99"))
			{
				return BC95ErrorCSQ;
			}

			if (strstr(_data, "+CSQ:"))
			{
				return BC95StateOK;
			}
		}
		_repeat++;
		System_Delayms(50);
	}

	return BC95ErrorCSQ;
}

BC95State BC95_QueryCGATTState()
{

	int _repeat = 0;
	int _dataLen = 0;
	char _data[UART1_MAXBUFFLEN] = {0};

	BC95_Send(AT_BC95_CGATT);
	Console_WriteStringln(AT_BC95_CGATT);

	while (_repeat < BC95_REPEAT_TIMES)
	{
		while (UART0_RecvLineTry(_data, UART1_MAXBUFFLEN, &_dataLen) == 0) //������
		{
			Console_WriteStringln(_data);

			if (strstr(_data, "+CGATT:1"))
			{
				return BC95StateOK;
			}

			if (strstr(_data, "+CGATT:0"))
			{
				return BC95ErrorCFUN;
			}
		}
		_repeat++;
		System_Delayms(1000);
	}

	return BC95ErrorCFUN;
}

//��ѯ�Ƿ������򴮿���ʾ���յ�����Ϣ
BC95State BC95_SetNNMI()
{
	int _repeat = 0;
	int _dataLen = 0;
	char _data[UART1_MAXBUFFLEN] = {0};

	BC95_Send(AT_BC95_NNMI);
	Console_WriteStringln(AT_BC95_NNMI);

	while (_repeat < BC95_REPEAT_TIMES)
	{
		while (UART0_RecvLineTry(_data, UART1_MAXBUFFLEN, &_dataLen) == 0) //������
		{
			Console_WriteStringln(_data);

			if (strstr(_data, "OK")) //���óɹ�
			{
				return BC95StateOK;
			}

			else
			{
				return BC95ErrorNNMI;
			}
		}
		_repeat++;
		System_Delayms(1000);
	}

	return BC95ErrorNNMI;
}

//��ѯ�Ƿ��ǵ͹���ģʽ
BC95State BC95_QueryIsPSMS()
{
	int _repeat = 0;
	int _dataLen = 0;
	char _data[UART1_MAXBUFFLEN] = {0};

	BC95_Send(AT_BC95_CPSMS);
	Console_WriteStringln(AT_BC95_CPSMS);

	while (_repeat < BC95_REPEAT_TIMES)
	{
		while (UART0_RecvLineTry(_data, UART1_MAXBUFFLEN, &_dataLen) == 0) //������
		{
			Console_WriteStringln(_data);

			if (strstr(_data, "+CPSMS:0")) //����˯��ģʽ
			{
				return BC95StateOK;
			}

			if (strstr(_data, "+CGATT:1")) //˯��ģʽ
			{
				return BC95ErrorCPSMS;
			}
		}
		_repeat++;
		System_Delayms(1000);
	}

	return BC95ErrorCPSMS;
}

/********
Function���رյ͹���ģʽ�����ڲ�����ʾ���յ�����������
********/
void BC95_SetCpsms()
{
	BC95_Send(AT_BC95_SPSMS);
	Console_WriteStringln(AT_BC95_SPSMS);
	System_Delayms(BC95_DELAY_MS);
}

//�رջ���
BC95State BC95_CloseATE()
{
	int _repeat = 0;
	int _dataLen = 0;
	char _data[UART1_MAXBUFFLEN] = {0};

	BC95_Send(AT_BC95_ATE0);
	Console_WriteStringln(AT_BC95_ATE0);

	while (_repeat < BC95_REPEAT_TIMES)
	{
		while (UART0_RecvLineTry(_data, UART1_MAXBUFFLEN, &_dataLen) == 0) //������
		{
			Console_WriteStringln(_data);

			if (strstr(_data, "OK")) //���óɹ�
			{
				return BC95StateOK;
			}

			else
			{
				return BC95ErrorATE;
			}
		}
		_repeat++;
		System_Delayms(1000);
	}

	return BC95ErrorNNMI;
}

BC95State BC95_SetSendMsgIndications(void)
{
	int _repeat = 0;
	int _dataLen = 0;
	char _data[UART1_MAXBUFFLEN] = {0};

	BC95_Send(AT_BC95_NSMI);
	Console_WriteStringln(AT_BC95_NSMI);

	while (_repeat < BC95_REPEAT_TIMES)
	{
		while (UART0_RecvLineTry(_data, UART1_MAXBUFFLEN, &_dataLen) == 0) //������
		{
			Console_WriteStringln(_data);

			if (strstr(_data, "OK")) //���óɹ�
			{
				return BC95StateOK;
			}

			else
			{
				return BC95ErrorNSMI;
			}
		}
		_repeat++;
		System_Delayms(1000);
	}

	return BC95ErrorNNMI;
}

BC95State BC95_ConfigProcess() //�����ѯʧ�ܣ���Ҫ����
{
	BC95State state = BC95StateOK;
	int retrytimes = 0;

	BC95_QueryState();

	if (g_scrambState == 0) //���빦��û�п������������빦��
	{
		BC95_SetScramb();
	}

	if (g_autoconfigState == 0) //�Զ�ģʽû�п����������Զ�ģʽ
	{
		BC95_SetAutoConfig();
	}

	//��ѯ�Ƿ��⵽SIM��
	while (retrytimes < BC95_REPEAT_TIMES)
	{
		state = BC95_QuerySimState();
		if (state == BC95StateOK)
		{
			break;
		}

		retrytimes++;
		System_Delayms(50);
	}
	if (retrytimes == BC95_REPEAT_TIMES)
	{
		return state;
	}

	//��ѯ�Ƿ�ȫ����ģʽ
	retrytimes = 0;
	while (retrytimes < BC95_REPEAT_TIMES)
	{
		state = BC95_QueryFunState();
		if (state == BC95StateOK)
		{
			break;
		}
		else
		{
			BC95_SetCFun();
		}

		retrytimes++;
		System_Delayms(50);
	}
	if (retrytimes == BC95_REPEAT_TIMES)
	{
		return state;
	}

	//��ѯ�ź�
	retrytimes = 0;
	while (retrytimes < BC95_REPEAT_TIMES)
	{
		state = BC95_QueryCSQState();
		if (state == BC95StateOK)
		{
			break;
		}

		retrytimes++;
		System_Delayms(50);
	}
	if (retrytimes == BC95_REPEAT_TIMES)
	{
		return state;
	}

	//��ѯ��������״��
	retrytimes = 0;
	while (retrytimes < BC95_REPEAT_TIMES * 2)
	{
		state = BC95_QueryCGATTState();
		if (state == BC95StateOK)
		{
			break;
		}

		retrytimes++;
		System_Delayms(1500);
	}
	if (retrytimes == BC95_REPEAT_TIMES * 2) //ѯ��CGATT Stateʧ��
	{
		return state;
	}

	//���������򴮿ڴ�ӡ���ݵĹ���
	retrytimes = 0;
	while (retrytimes < BC95_REPEAT_TIMES)
	{
		state = BC95_SetNNMI();
		if (state == BC95StateOK)
		{
			break;
		}
		else
		{
			retrytimes++;
			System_Delayms(50);
		}
	}
	if (retrytimes == BC95_REPEAT_TIMES)
	{
		return state;
	}

	//�Ƿ���˯��ģʽ
	retrytimes = 0;
	while (retrytimes < BC95_REPEAT_TIMES)
	{
		state = BC95_QueryIsPSMS();
		if (state == BC95StateOK)
		{
			break;
		}
		else
		{
			BC95_SetCpsms();
		}

		retrytimes++;
		System_Delayms(50);
	}
	if (retrytimes == BC95_REPEAT_TIMES)
	{
		return state;
	}

	//�رջ���
	retrytimes = 0;
	while (retrytimes < BC95_REPEAT_TIMES)
	{
		state = BC95_CloseATE();
		if (state == BC95StateOK)
		{
			break;
		}

		retrytimes++;
		System_Delayms(50);
	}
	if (retrytimes == BC95_REPEAT_TIMES)
	{
		return state;
	}

	/* Sent message indications is enabled */
	// retrytimes = 0;
	// while (retrytimes < BC95_REPEAT_TIMES)
	// {
	// 	state = BC95_SetSendMsgIndications();
	// 	if (state == BC95StateOK)
	// 	{
	// 		break;
	// 	}

	// 	retrytimes++;
	// 	System_Delayms(50);
	// }
	// if (retrytimes == BC95_REPEAT_TIMES)
	// {
	// 	return state;
	// }

	//�����豸�Խӵ�IoTƽ̨(��Ϊ��)��IP��ַ�Ͷ˿ں�
	retrytimes = 0;
	while (retrytimes < BC95_REPEAT_TIMES)
	{
		state = BC95_ConnectToIotCloud("49.4.85.232", "5683");
		if (state == BC95StateOK)
		{
			break;
		}

		retrytimes++;
		System_Delayms(50);
	}
	if (retrytimes >= BC95_REPEAT_TIMES)
	{
		return state;
	}

	return state;
}

BC95State BC95_ConnectToIotCloud(char *serverIp, char *serverPort)
{
	int _repeat = 0;

	// head_ip_comma_port_end = " AT+NCDP="49.4.85.232","5683" "
	char *at_head = "AT+NCDP=";
	char *comma = ",";
	int _dataLen = 0;
	char _data[UART1_MAXBUFFLEN] = {0};
	char *head_ip_comma_port_end = (char *)pvPortMalloc(40);

	strcat(head_ip_comma_port_end, at_head);
	strcat(head_ip_comma_port_end, serverIp);
	strcat(head_ip_comma_port_end, comma);
	strcat(head_ip_comma_port_end, serverPort);

	BC95_Send(head_ip_comma_port_end);
	Console_WriteStringln(head_ip_comma_port_end);

	while (_repeat < BC95_REPEAT_TIMES)
	{
		while (UART0_RecvLineTry(_data, UART1_MAXBUFFLEN, &_dataLen) == 0) //������
		{
			Console_WriteStringln(_data);

			if (strstr(_data, "OK"))
			{
				printf("BC95 Connect to HUAWEI Cloud Success \r\n");
				return BC95StateOK;
			}

			if (strstr(_data, "+CGATT:0"))
			{
				printf("BC95 Connect to HUAWEI Cloud fail !! \r\n");
				return BC95ErrorNCDP;
			}
		}
		_repeat++;
		System_Delayms(1000);
	}

	return BC95ErrorNCDP;
}

//BC95��������
/*
*BC95ģ���Ȱ������ϴ�����ƽ̨�������ǻ�Ϊ�ƻ��ߵ����Ƶȣ���������ƽ̨ת�������Լ��ķ�������
*�������ϴ��ɱ䳤�ȵ����ݣ������Ҫ������ƽ̨�ı������������һЩͷ�������ڷ��͵���������Ҫ��һ��4���ֽڵ�16λ�޷���������Ϊ�ϴ����ݰ�����
*/
char up_data_len_str[BC95_SOCKET_UP_DATA_LENGTH_LINK] = {0}; //�ϴ����ݵĹ�������,����4��16λ�޷�������ת��λ�ַ�����4���ַ�����2���ֽ�
char _data[UART1_MAXBUFFLEN] = {0};
char send_data_ascii_mode[BC95_SOCKET_DATA_LEN] = {0};
char socket_data[BC95_SOCKET_DATA_LEN] = {0};

BC95State BC95_SendSocketData(char *send_data, int send_data_len)
{
	int i = 0;
	int j = 0;
	int data_len = 0;
	int retrytimes = 0;
	int _dataLen = 0;

	
	//���ϴ����ݵĳ���ת��Ϊ�ַ�����ʽ
	BC95_UintToStr4(send_data_len, up_data_len_str);

	//���ϴ�����ת��ΪASCII�ַ�����ʽ
	ASCII_to_AsciiStr(send_data, send_data_len, send_data_ascii_mode); //ת��ASCII�ַ���

	//���
	data_len = send_data_len + BC95_SOCKET_UP_DATA_LENGTH_LINK / 2;
	sprintf(socket_data, "AT+NMGS=%d,", data_len); //���붺��֮ǰ�����ݣ���������

	//�������ݹ������ȵ��ַ���
	// data_len = send_data_len + BC95_SOCKET_UP_DATA_LENGTH_LINK+1;//��1��������
	for (i = Utility_Strlen(socket_data), j = 0; j < BC95_SOCKET_UP_DATA_LENGTH_LINK; j++)
	{
		socket_data[i] = up_data_len_str[j];
		i++;
	}

	//i = Utility_Strlen(socket_data);//����ڶ�������
	//socket_data[i] = ',';

	data_len = Utility_Strlen(socket_data) + Utility_Strlen(send_data_ascii_mode); //������������ASCII�ַ���
	for (i = Utility_Strlen(socket_data), j = 0; i < data_len; i++)
	{
		socket_data[i] = send_data_ascii_mode[j];
		j++;
	}

	//����
	BC95_Send(socket_data);

	Console_WriteStringln("BC95 send data:");

	Console_WriteStringln(socket_data);

	retrytimes = 0;

	while (retrytimes < BC95_REPEAT_TIMES)
	{
		while (UART0_RecvLineTry(_data, UART1_MAXBUFFLEN, &_dataLen) == 0) //������
		{
			Console_WriteStringln(_data);

			if (strstr(_data, "OK")) //�������ݳɹ�
			{
				return BC95StateOK;
			}
		}
		retrytimes++;
		System_Delayms(500);
	}
	if (retrytimes == BC95_REPEAT_TIMES)
	{
		Console_WriteStringln("BC95 send data falied!");
	}

	return BC95ErrorSEND;
}

/* when needCloudConfirm=1, need check the send indication from iot */
BC95State BC95_SendDataToCloud(char *send_data, int send_data_len, int needCloudConfirm)
{
	BC95State ret;
	char *bc95Response;

	ret = BC95_SendSocketData(send_data, send_data_len);
	if (ret != BC95StateOK)
		return BC95ErrorSEND;

	if (needCloudConfirm)
	{
		bc95Response = BC95_Receive();
		if (bc95Response && strstr(bc95Response, "+NNMI"))
			return BC95StateOK;
		else
		{
			return BC95ErrorSEND;
		}
	}

	return BC95StateOK;
}

void BC95_RecvSocketData(char *recv_data, int *recv_data_len)
{
	int _repeat = 0;
	int _dataLen = 0;
	char *_data = recv_data;

	while (_repeat < BC95_WAITTIME)
	{
		while (UART0_RecvLineTry(_data, UART1_MAXBUFFLEN, &_dataLen) == 0) //������
		{
			Console_WriteStringln("BC95 recv data:");

			Console_WriteStringln(_data);

			if (strstr(_data, "+NNMI:")) //�ж��Ƿ�Ϊ���յ�������
			{
				BC95_AnalysisRecvSocketData(_data, _dataLen, recv_data, recv_data_len);

				return;
			}
		}
		_repeat++;
		System_Delayms(100);
	}
	if (_repeat == BC95_WAITTIME)
	{
		Console_WriteStringln("not recv data!");
		return;
	}
}


void BC95_RecvDataFromCloud(char *recv_data, int *recv_data_len)
{
	BC95_RecvSocketData(recv_data, recv_data_len);
	recv_data++;
	recv_data++;
	recv_data++;
	if (*recv_data_len > 0)  // ���б��ľ�����ƽ̨��ʱ���ܻ��������ֽڵı�ͷ(��ƽ̨�Զ����ӵ�)��Ӧ�ú���ƽ̨�Ĳ�������й�
	{
		*recv_data_len = *recv_data_len - 3;
	}
	
}

//BC95�������������
/*
*BC95ģ���Ȱ������ϴ�����ƽ̨�������ǻ�Ϊ�ƻ��ߵ����Ƶȣ���������ƽ̨ת�������Լ��ķ�������
*�������ϴ��ɱ䳤�ȵ����ݣ������Ҫ������ƽ̨�ı������������һЩͷ�������ڷ��͵���������Ҫ��һ��4���ֽڵ�16λ�޷���������Ϊ�ϴ����ݰ�����
*/
BC95State BC95_SendAndRecvData(char *send_data, int send_data_len, char *recv_data, int *recv_data_len)
{
	int i = 0;
	int j = 0;
	int data_len = 0;
	int retrytimes = 0;
	int _dataLen = 0;

	char up_data_len_str[BC95_SOCKET_UP_DATA_LENGTH_LINK] = {0}; //�ϴ����ݵĹ�������,����4��16λ�޷�������ת��λ�ַ�����4���ַ�����2���ֽ�
	char _data[UART1_MAXBUFFLEN] = {0};
	char send_data_ascii_mode[BC95_SOCKET_DATA_LEN] = {0};
	char socket_data[BC95_SOCKET_DATA_LEN] = {0};

	//���ϴ����ݵĳ���ת��Ϊ�ַ�����ʽ
	BC95_UintToStr4(send_data_len, up_data_len_str);

	//���ϴ�����ת��ΪASCII�ַ�����ʽ
	Console_WriteStringln(send_data);
	ASCII_to_AsciiStr(send_data, send_data_len, send_data_ascii_mode); //ת��ASCII�ַ���

	//���
	data_len = send_data_len + BC95_SOCKET_UP_DATA_LENGTH_LINK / 2;
	sprintf(socket_data, "AT+NMGS=%d,", data_len); //���붺��֮ǰ�����ݣ���������

	//�������ݹ������ȵ��ַ���
	// data_len = send_data_len + BC95_SOCKET_UP_DATA_LENGTH_LINK+1;//��1��������
	for (i = Utility_Strlen(socket_data), j = 0; j < BC95_SOCKET_UP_DATA_LENGTH_LINK; j++)
	{
		socket_data[i] = up_data_len_str[j];
		i++;
	}

	//i = Utility_Strlen(socket_data);//����ڶ�������
	//socket_data[i] = ',';

	data_len = Utility_Strlen(socket_data) + Utility_Strlen(send_data_ascii_mode); //������������ASCII�ַ���
	for (i = Utility_Strlen(socket_data), j = 0; i < data_len; i++)
	{
		socket_data[i] = send_data_ascii_mode[j];
		j++;
	}

	//����
	BC95_Send(socket_data);

	Console_WriteStringln("BC95 send data:");

	Console_WriteStringln(socket_data);

	//��������
	retrytimes = 0;

	while (retrytimes < BC95_REPEAT_TIMES)
	{
		while (UART0_RecvLineTry(_data, UART1_MAXBUFFLEN, &_dataLen) == 0) //������
		{
			Console_WriteStringln(_data);

			if (strstr(_data, "+NNMI:")) //�ж��Ƿ�Ϊ���յ�������
			{

				*recv_data_len = BC95_Extract_DownData(_data, _dataLen, recv_data);
				return BC95StateOK;
			}
		}
		retrytimes++;
		System_Delayms(1000);
	}
	if (retrytimes == BC95_REPEAT_TIMES)
	{
		Console_WriteStringln("BC95 send data falied!");
	}

	return BC95ErrorSEND;
}

//�������յ�����
void BC95_AnalysisRecvSocketData(char *revc_data, int revc_data_len, char *down_Stream_data, int *down_Stream_data_len)
{
	int i = 0;
	int comma_count = 0; //���ż���
	int down_Stream_data_ascii_len = 0;
	char down_Stream_data_ascii[BC95_SOCKET_DATA_LEN] = {0};

	while (i < revc_data_len)
	{
		if (revc_data[i] == ',')
		{
			comma_count++;
			if (comma_count == 1) //���ص���������1�����ţ����ź��������Ϊ��������
			{
				i++;
				break;
			}
		}
		i++;
	}

	while (revc_data[i] != '\0') //��ȡ����
	{
		down_Stream_data_ascii[down_Stream_data_ascii_len] = revc_data[i];
		down_Stream_data_ascii_len++;
		i++;
	}

	AsciiStr_to_ASCII(down_Stream_data_ascii, down_Stream_data_ascii_len, down_Stream_data);
	*down_Stream_data_len = down_Stream_data_ascii_len / 2;
}

void BC95_Communication_test()
{
	int len = 0;
	char msgLen = 0xA;
	char *msgData = "hahamama12";
	char msgIdAndData[100];
	strcat(msgIdAndData, &msgLen);
	strcat(msgIdAndData, msgData);
	char recv_data[BC95_SOCKET_DATA_LEN] = {0};

	while (1)
	{
		BC95_SendSocketData(msgIdAndData, Utility_Strlen(msgIdAndData));

		Console_WriteStringln("BC95 send data:");

		Console_WriteStringln(msgIdAndData);

		BC95_RecvSocketData(recv_data, &len);

		Console_WriteStringln("BC95 recv  down data:");

		Console_WriteStringln(recv_data);

		System_Delayms(5000);
	}
}

/* ret: NULL(fail)  otherwise(success) */
char *BC95_Receive(void)
{
	int _repeat = 0;
	char _data[500];
	int _dataLen;

	/* wait till uart0 buffered filled and return the buffer */
	while (_repeat < BC95_REPEAT_TIMES)
	{
		if (UART0_RecvLineTry(_data, UART1_MAXBUFFLEN, &_dataLen) == 0)
		{
			return _data;
		}
		_repeat++;
		System_Delayms(50);
	}

	/* wait time out , return NULL */
	return NULL;
}

//���˲��Ժ����ã�������
void BC95_AtTest(void)
{
	while (1)
	{
		BC95_Send("AT+CEREG?");
		char *bc95Response = BC95_Receive();
		printf("%s \r\n", bc95Response);
	}
}

void BC95_Test(void)
{
	while (1)
	{
		//char recv_data[500] = {0};
		//int recv_data_len = 0;
		//BC95_SendDataToCloud("101AFC6", 7, 1);
                BC95_SendSocketData("101AFC6", 7);
		/*BC95_RecvDataFromCloud(recv_data, &recv_data_len);
		if(recv_data_len != 0)
		{
			printf("recv_data: %s,  len: %d \r\n", recv_data + 3, recv_data_len);
		}
*/

		System_Delayms(1000);
	}
	
}



