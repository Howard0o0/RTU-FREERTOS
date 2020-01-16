#include "gprs.h"
#include "Console.h"
#include "FreeRTOS.h"
#include "common.h"
#include "communication_opr.h"
#include "hydrologycommand.h"
#include "memoryleakcheck.h"
#include "message.h"
#include "msp430common.h"
#include "msp430f5438a.h"
#include "portmacro.h"
#include "rtc.h"
#include "store.h"
#include "task.h"
#include "uart0.h"
#include <stdbool.h>
#include <string.h>


static int   check_gprs_module_is_normal();
static void  uart0_init();
static void  pull_up_pin_batt();
static void  pull_down_pin_igt();
static void  pull_up_pin_igt();
static void  send_at_cmd(char* at_cmd);
static char* rcv_at_response();
static int   check_gprs_module_is_normal();
static int   GSM_DealData(char* recv_, int _dataLen);
static int   GSM_AT_CloseFeedback();
static int   GSM_AT_OffCall();
static int   GSM_DealAllData();

#define BUF_LEN 50
#define ARRAY_MAX 15

char  centerIP[ 16 ];   // = "125.220.159.168";
char  centerPort[ 7 ];  // = "6666";
char* psrc	    = NULL;
int   dataLen	 = 0;
char* _ReceiveData    = NULL;
int   _ReceiveDataLen = 0;

int	g_work_mode;
int	main_time_error;
extern int s_alert_flag[ 8 ];

//  调试统计计数
int s_TimeOut = 0;
int s_RING    = 0;
int s_OOPS    = 0;
int s_MsgFail = 0;
int s_MsgOK   = 0;
int s_MsgDel  = 0;

int s_RecvIdx = 0;  //注意是指向最后索引的下一个
int s_ProcIdx = 0;  //注意是指向最后处理的下一个.
int s_MsgNum  = 0;  //收到短信的总数
int s_MsgArray[ ARRAY_MAX ];

//函数间交互的变量
int  s_MsgLeft  = 0;
char s_NetState = '0';

communication_module_t gprs_module = {
	.name			   = "gprs",
	.power_on		   = gprs_power_on,
	.power_off		   = gprs_power_off,
	.sleep			   = gprs_sleep,
	.wake_up		   = gprs_wake_up,
	.send_msg		   = GPRS_Send,
	.rcv_msg		   = GPRS_Receive,
	.get_real_time		   = GSM_AT_QueryTime,
	.check_if_module_is_normal = check_gprs_module_is_normal,
};

int gprs_module_driver_install(void) {

	return register_communication_module(&gprs_module);
}

/*
 * return : OK  ERR
 */
int gprs_power_on(void) {

	uart0_init();

	printf("gprs module is booting, waiting for 42s ...\n\n");

	taskENTER_CRITICAL();
	pull_up_pin_batt();
	pull_down_pin_igt();
	pull_up_pin_igt();
	taskEXIT_CRITICAL();

	return check_gprs_module_is_normal();
}

int gprs_power_off(void) {

	return OK;
}

int gprs_wake_up(void) {

	return OK;
}

int gprs_sleep(void) {
	return OK;
}

static void uart0_init() {
	UART0_Open(UART0_GTM900_TYPE);
}

static void pull_up_pin_batt() {
	P10DIR |= BIT7;
	P10OUT &= ~BIT7;
	P10OUT |= BIT7;
	System_Delayms(100);
}

static void pull_down_pin_igt() {
	P3DIR |= BIT0;
	P3OUT |= BIT0;
	System_Delayms(10);
	P3OUT &= ~BIT0;
	System_Delayms(50);
}

static void pull_up_pin_igt() {
	P3OUT |= BIT0;
	System_Delayms(1200);
	System_Delayms(30000);
}

static void send_at_cmd(char* at_cmd) {
	UART0_Send(at_cmd, strlen(at_cmd), 1);
}

/*
 *@ author : howard
 *@ 如果获取时间失败，则time_t.year置0
 */
rtc_time_t GSM_AT_QueryTime(void) {
	int	  rcvFlag = 0;
	int	  _repeat = 0;
	int	  _retNum;
	char	 _sendAT[ 30 ] = "AT+CCLK?";
	char*	pcAtResp      = NULL;
	char*	pcSeperate    = "/";
	static char* pcRcvAtBuff   = NULL;  //定义静态局部变量，分配在全局数据区

	rtc_time_t get_time_from_gprs_module;
	get_time_from_gprs_module.year = 0;

	pcRcvAtBuff = ( char* )os_malloc(200 * sizeof(char));
	if (pcRcvAtBuff == NULL) {
		printf("pcRcvAtBuff malloc failed! \r\n");
		return get_time_from_gprs_module;
	}

	UART0_Send(_sendAT, 8, 1);

	while (_repeat < 10) {
		if ((UART0_RecvLineTry(pcRcvAtBuff, 200, &_retNum) == 0)
		    && ((pcAtResp = strstr(pcRcvAtBuff, pcSeperate)) != NULL))  //有数据
		{
			rcvFlag = 1;
			break;
		}
		_repeat++;
		System_Delayms(50);
	}

	/* got AT response contained time */
	if (rcvFlag == 1) {
		//        printf("%s \r\n ",pcRcvAtBuff);
		//        printf("%s \r\n ",pcAtResp);
		pcAtResp -= 2;

		int  offset = 0;
		char year1, year2, month1, month2, date1, date2, hour1, hour2, min1, min2, sec1,
			sec2;
		year1 = *(pcAtResp + offset);
		offset++;
		year2 = *(pcAtResp + offset);
		offset++;
		offset++;
		month1 = *(pcAtResp + offset);
		offset++;
		month2 = *(pcAtResp + offset);
		offset++;
		offset++;
		date1 = *(pcAtResp + offset);
		offset++;
		date2 = *(pcAtResp + offset);
		offset++;
		offset++;
		hour1 = *(pcAtResp + offset);
		offset++;
		hour2 = *(pcAtResp + offset);
		offset++;
		offset++;
		min1 = *(pcAtResp + offset);
		offset++;
		min2 = *(pcAtResp + offset);
		offset++;
		offset++;
		sec1 = *(pcAtResp + offset);
		offset++;
		sec2 = *(pcAtResp + offset);

		/* char *year,char *month,char *date,char *hour,char *min,char *second */
		get_time_from_gprs_module.year  = (year1 - 48) * 10 + (year2 - 48);
		get_time_from_gprs_module.month = (month1 - 48) * 10 + (month2 - 48);
		get_time_from_gprs_module.date  = (date1 - 48) * 10 + (date2 - 48);
		get_time_from_gprs_module.hour  = ((hour1 - 48) * 10 + (hour2 - 48));
		get_time_from_gprs_module.min   = (min1 - 48) * 10 + (min2 - 48);
		get_time_from_gprs_module.sec   = (sec1 - 48) * 10 + (sec2 - 48);
	}

	os_free(pcRcvAtBuff);

	return get_time_from_gprs_module;
}

static char* rcv_at_response() {
	char buff[ BUF_LEN ];
	int  real_rcv_len = 0;

	memset(buff, 0, BUF_LEN);
	while (UART0_RecvLineWait(buff + real_rcv_len, BUF_LEN, &real_rcv_len) != -1)
		;

	if (strlen(buff) != 0) {
		return buff;
	}
	else {
		return NULL;
	}
}
static int check_gprs_module_is_normal() {

	char* at_rsp;

	send_at_cmd("AT+CGMI=?");
	at_rsp = rcv_at_response();

	if (strstr(at_rsp, "OK") != NULL) {
		return OK;
	}
	else {
		return ERR;
	}
}

void GPRS_Print_Error(int errorCode) {
	switch (errorCode) {
	// 1:输入参数错误
	case 1: {
		Console_WriteStringln("Wrong AT Command Parameter!");
		break;
	}
	// 2:一个内部的错误产生
	case 2: {
		Console_WriteStringln("An internel error in GTM900 Module!");
		break;
	}
	// 3:IP 地址或者端口号正在被使用
	case 3: {
		Console_WriteStringln("IP Address or Port is already used!");
		break;
	}
	// 4:内存申请失败或者 BUFFER 已经被使用完
	case 4: {
		Console_WriteStringln("Memory apply failed or buffer run out!");
		break;
	}
	// 5:该 socket 不支持此种操作
	case 5: {
		Console_WriteStringln("Unsupported operation of socket!");
		break;
	}
	// 6:不能链接到该主机
	case 6: {
		Console_WriteStringln("Connot connect to host, please check host!");
		break;
	}
	// 7:链接被对方拒绝
	case 7: {
		Console_WriteStringln(
			"Connection being refused by host, please check host fireware");
		break;
	}
	// 8:链接超时
	case 8: {
		Console_WriteStringln("Connection time out");
		break;
	}
	// 9:该链接已经建立
	case 9: {
		Console_WriteStringln("Link already exists");
		break;
	}
	// 10:在 DNSR 服务器中找不到该主机地址
	case 10: {
		Console_WriteStringln("Can not find host IP in DNSR");
		break;
	}
	// 11:出现一个暂时的 DNS 错误，稍后可以重新执行
	case 11: {
		Console_WriteStringln("Temporary DSN Error");
		break;
	}
	// 12:出现一个永久的 DNS 错误
	case 12: {
		Console_WriteStringln("Permanent DSN Error");
		break;
	}
	// 13:在 DNSR 服务器找到该主机地址，但是 IP 地址无效
	case 13: {
		Console_WriteStringln("Invalid IP in DNSR");
		break;
	}
	// 14:该 socket 链接还没有建立
	case 14: {
		Console_WriteStringln("Socket not exist");
		break;
	}
	// 15:传输的数据长度太大
	case 15: {
		Console_WriteStringln("Data out of length");
		break;
	}
	// 16:该链接被对端复位
	case 16: {
		Console_WriteStringln("Host reset Link");
		break;
	}
	// 17:该链接有错误产生
	case 17: {
		Console_WriteStringln("Error occurs on Link");
		break;
	}
	// 18:没有剩余缓存来保存信息
	case 18: {
		Console_WriteStringln("No Cache to save information");
		break;
	}
	// 19:没有网络，或者一个错误产生导致 TCPIP 断开
	case 19: {
		Console_WriteStringln("No net exists or TCP/IP function failed");
		break;
	}
	// 20:PDP 上下文没有激活
	case 20: {
		Console_WriteStringln("PDP not activitied");
		break;
	}
	// 21:GPRS attach 没有成功
	case 21: {
		Console_WriteStringln("GPRS attach failed");
		break;
	}
	// 22:类似的操作正在执行
	case 22: {
		Console_WriteStringln("Similar operation already being excuted");
		break;
	}
	// 23:PDP 上下文已经激活
	case 23: {
		Console_WriteStringln("PDP already activitied");
		break;
	}
	// 24:当前为多链接模式
	case 24: {
		Console_WriteStringln("Tcp is on multi-link mode!");
		break;
	}
	// 25:当前为单链接模式
	case 25: {
		Console_WriteStringln("Tcp is on single-link mode!");
		break;
	}
	// 26:输入的 index 号码不存在
	case 26: {
		Console_WriteStringln("Index number not exist !");
		break;
	}
	// 27:无效的删除模式
	case 27: {
		Console_WriteStringln("Invalid delete mode!");
		break;
	}
	// 28:当前不可以使用该命令
	case 28: {
		Console_WriteStringln("Can not use this command at present!");
		break;
	}
	// 29:发送窗口已满
	case 29: {
		Console_WriteStringln("Sending window full!");
		break;
	}
	// 30:当前正在通话中，请稍后执行 PDP 激活和 TCPIP 初始化的操作
	case 30: {
		Console_WriteStringln(
			"Phone on line, please excuted PDP activite and TCP Initialize later");
		break;
	}
	// 31:其它错误
	case 31: {
		Console_WriteStringln("Unexpected error");
		break;
	}
	}
}

int GPRS_AT_SetAPN_Response(char* recv_) {
	return TRUE;
}

int GPRS_AT_ActiveGPRSNet_Response(char* recv_) {
	return TRUE;
}

int GPRS_AT_CheckSimStat_Response(char* recv_, int* _retErrorCode) {
	int retValue = TRUE;
	if (Utility_Strncmp(recv_, "%TSIM", 5) == 0) {
		if (recv_[ 6 ] == '0') {
			retValue = FALSE;
		}
	}
	else if (Utility_Strncmp(recv_, "ERROR", 5) == 0)  // ERROR: 14  冒号和14中间有空格
	{
		char bufErrorTmp[ 3 ] = {
			0
		};  //调用atoi函数，需要有字符串结束符，2位错误码，1位结束符

		bufErrorTmp[ 0 ] = recv_[ 7 ];
		if (recv_[ 8 ] != 0) {
			bufErrorTmp[ 1 ] = recv_[ 8 ];
		}
		*_retErrorCode = atoi(bufErrorTmp);
		GPRS_Print_Error(*_retErrorCode);
		retValue = FALSE;
	}
	return retValue;
}

Operater TelecomOperater;

int GPRS_AT_QueryOperater_Response(char* recv_, int* _retErrorCode) {
	int retValue = TRUE;
	if (Utility_Strncmp(recv_, "+COPS: 0,0,\"CHINA  MOBILE\"", 26) == 0) {
		TelecomOperater = CHINA_MOBILE;
		retValue	= TRUE;
	}
	else if (Utility_Strncmp(recv_, "ERROR", 5) == 0)  // ERROR: 14  冒号和14中间有空格
	{
		char bufErrorTmp[ 3 ] = {
			0
		};  //调用atoi函数，需要有字符串结束符，2位错误码，1位结束符

		bufErrorTmp[ 0 ] = recv_[ 7 ];
		if (recv_[ 8 ] != 0) {
			bufErrorTmp[ 1 ] = recv_[ 8 ];
		}
		*_retErrorCode = atoi(bufErrorTmp);
		GPRS_Print_Error(*_retErrorCode);
		retValue = FALSE;
	}
	return retValue;
}

int GPRS_AT_QueryIP_Response(char* recv_, int* _retErrorCode) {
	int retValue = TRUE;
	if (Utility_Strncmp(recv_, "%ETCPIP:", 8) == 0) {
		if (recv_[ 8 ] == 0) {
			retValue = FALSE;
		}
	}
	else if (Utility_Strncmp(recv_, "ERROR", 5) == 0)  // ERROR: 14  冒号和14中间有空格
	{
		char bufErrorTmp[ 3 ] = {
			0
		};  //调用atoi函数，需要有字符串结束符，2位错误码，1位结束符

		bufErrorTmp[ 0 ] = recv_[ 7 ];
		if (recv_[ 8 ] != 0) {
			bufErrorTmp[ 1 ] = recv_[ 8 ];
		}
		*_retErrorCode = atoi(bufErrorTmp);
		GPRS_Print_Error(*_retErrorCode);
		retValue = FALSE;
	}
	return retValue;
}

//打开链接和其它命令响应有一定不一致，因为只返回一个connect或者error
int GPRS_AT_OpenTCPLink_Response(char* recv_) {
	return TRUE;
}

int GPRS_AT_QueryTCPLink_Response(char* recv_, int* _retErrorCode) {
	int retValue = TRUE;
	if (Utility_Strncmp(recv_, "%IPOPEN:", 8) == 0) {
		if (recv_[ 8 ] == 0) {
			retValue = FALSE;
		}
	}
	else if (Utility_Strncmp(recv_, "ERROR", 5) == 0)  // ERROR: 14  冒号和14中间有空格
	{
		char bufErrorTmp[ 3 ] = {
			0
		};  //调用atoi函数，需要有字符串结束符，2位错误码，1位结束符

		bufErrorTmp[ 0 ] = recv_[ 7 ];
		if (recv_[ 8 ] != 0) {
			bufErrorTmp[ 1 ] = recv_[ 8 ];
		}
		*_retErrorCode = atoi(bufErrorTmp);
		GPRS_Print_Error(*_retErrorCode);
		retValue = FALSE;
	}
	return retValue;
}

//这里没考虑出错，后面要处理
int GPRS_AT_CloseTCPLink_Response(char* recv_, int* _retErrorCode) {
	int retValue = FALSE;
	// IPCLOSE如果有IP链接，会先返回%IPCLOSE,然后再返回OK,如果没有链接，只会返回一个命令响应成功
	// OK
	if ((Utility_Strncmp(recv_, "%IPCLOSE", 8) == 0)
	    || (Utility_Strncmp(recv_, "OK", 2) == 0)) {
		retValue = TRUE;
	}
	else if (Utility_Strncmp(recv_, "ERROR", 5) == 0)  // ERROR: 14  冒号和14中间有空格
	{
		char bufErrorTmp[ 3 ] = {
			0
		};  //调用atoi函数，需要有字符串结束符，2位错误码，1位结束符

		bufErrorTmp[ 0 ] = recv_[ 7 ];
		if (recv_[ 8 ] != 0) {
			bufErrorTmp[ 1 ] = recv_[ 8 ];
		}
		*_retErrorCode = atoi(bufErrorTmp);
		GPRS_Print_Error(*_retErrorCode);
		retValue = FALSE;
	}
	return retValue;
}

//这里没考虑出错，后面要处理
int GPRS_AT_QuitGPRSNet_Response(char* recv_, int* _retErrorCode) {
	int retValue = FALSE;
	// IPCLOSE如果有IP链接，会先返回%IPCLOSE,然后再返回OK,如果没有链接，只会返回一个命令响应成功
	// OK
	if ((Utility_Strncmp(recv_, "%IPCLOSE", 8) == 0)
	    || (Utility_Strncmp(recv_, "OK", 2) == 0)) {
		retValue = TRUE;
	}
	else if (Utility_Strncmp(recv_, "ERROR", 5) == 0)  // ERROR: 14  冒号和14中间有空格
	{
		char bufErrorTmp[ 3 ] = {
			0
		};  //调用atoi函数，需要有字符串结束符，2位错误码，1位结束符

		bufErrorTmp[ 0 ] = recv_[ 7 ];
		if (recv_[ 8 ] != 0) {
			bufErrorTmp[ 1 ] = recv_[ 8 ];
		}
		*_retErrorCode = atoi(bufErrorTmp);
		GPRS_Print_Error(*_retErrorCode);
		retValue = FALSE;
	}
	return retValue;
}

//使用ASCii码还是Hex码发送消息，0,1,1---ASCii， 1,1,0---Hex
int GPRS_AT_SetIOMode_Response(char* recv_) {
	int retValue = TRUE;
	// SetIOMode如果有IP链接，会先返回OK,没有IP，就会返回 EXT: I
	if (Utility_Strncmp(recv_, "EXT:", 4) == 0) {
		retValue = FALSE;
	}
	return retValue;
}

int _GTM900SendBufLeft = 16;
int GRPS_AT_Send_Response(char* recv_, int* _retErrorCode) {
	int retValue = FALSE;
	if (Utility_Strncmp(recv_, "%IPSEND:", 8) == 0) {
		char bufLeftTmp[ 3 ] = { 0 };  //调用atoi函数，需要有字符串结束符
		bufLeftTmp[ 0 ]      = recv_[ 8 ];
		if (recv_[ 9 ] != 0) {
			bufLeftTmp[ 1 ] = recv_[ 9 ];
		}
		_GTM900SendBufLeft = atoi(bufLeftTmp);
		retValue	   = TRUE;
	}
	else if (Utility_Strncmp(recv_, "ERROR", 5) == 0)  // ERROR: 14  冒号和14中间有空格
	{
		char bufErrorTmp[ 3 ] = {
			0
		};  //调用atoi函数，需要有字符串结束符，2位错误码，1位结束符

		bufErrorTmp[ 0 ] = recv_[ 7 ];
		if (recv_[ 8 ] != 0) {
			bufErrorTmp[ 1 ] = recv_[ 8 ];
		}
		*_retErrorCode = atoi(bufErrorTmp);
		GPRS_Print_Error(*_retErrorCode);
		retValue = FALSE;
	}

	return retValue;
}

int RemainingData = 0;
int GPRS_AT_QueryRemainData_Response(char* recv_, int* _retErrorCode) {
	int retValue = TRUE;

	if (Utility_Strncmp(recv_, "%IPDQ:", 6) == 0) {
		RemainingData = atoi(&recv_[ 7 ]);
	}
	else if (Utility_Strncmp(recv_, "ERROR", 5) == 0)  // ERROR: 14  冒号和14中间有空格
	{
		char bufErrorTmp[ 3 ] = {
			0
		};  //调用atoi函数，需要有字符串结束符，2位错误码，1位结束符

		bufErrorTmp[ 0 ] = recv_[ 7 ];
		if (recv_[ 8 ] != 0) {
			bufErrorTmp[ 1 ] = recv_[ 8 ];
		}
		*_retErrorCode = atoi(bufErrorTmp);
		GPRS_Print_Error(*_retErrorCode);
		retValue = FALSE;
	}
	return retValue;
}

int GRPS_AT_Receive_Response(char* recv_, int* _retErrorCode) {
	int   retValue     = FALSE;
	char* pdata	= NULL;
	char* psrc	 = recv_ + 10;
	int   _datalensize = 0;

	if (Utility_Strncmp(recv_, "%IPDR:", 6) == 0) {
		while ((*psrc) != ',') {
			_datalensize++;
			psrc++;
		}
		psrc  = recv_ + 10;
		pdata = ( char* )os_malloc(_datalensize);
		Utility_Strncpy(pdata, recv_ + 10, _datalensize);
		_ReceiveDataLen = Utility_atoi(pdata, _datalensize);
		os_free(pdata);
		pdata	= NULL;  //
		_ReceiveData = ( char* )os_malloc(_ReceiveDataLen * 2);
		if (_ReceiveData == NULL) {
			Console_WriteStringln("os_malloc in GRPS_AT_Receive_Response failed");
			return MSG_SEND_FAILED;
		}

		while ((*psrc) != '\"') {
			psrc++;
		}
		Utility_Strncpy(_ReceiveData, psrc + 1, _ReceiveDataLen * 2);
		os_free(_ReceiveData);  //+++
		_ReceiveData = NULL;
		retValue     = TRUE;
	}
	else if (Utility_Strncmp(recv_, "ERROR", 5) == 0)  // ERROR: 14  冒号和14中间有空格
	{
		char bufErrorTmp[ 3 ] = {
			0
		};  //调用atoi函数，需要有字符串结束符，2位错误码，1位结束符

		bufErrorTmp[ 0 ] = recv_[ 7 ];
		if (recv_[ 8 ] != 0) {
			bufErrorTmp[ 1 ] = recv_[ 8 ];
		}
		*_retErrorCode = atoi(bufErrorTmp);
		GPRS_Print_Error(*_retErrorCode);
		retValue = FALSE;
	}

	return retValue;
}

int GPRS_AT_OFF_CALL_Response(char* recv_) {
	return TRUE;
}

int GPRS_SubProc_AT_ResponseOne(char* recv_, int* _retMsgNum, int cmdType, int* _retErrorCode) {
	int retValue = FALSE;

	if (Utility_Strncmp(recv_, "RING", 4) == 0)  //如果是电话来了，直接挂掉，重新等待接受。
	{
		GPRS_AT_OFF_CALL();
		retValue = FALSE;  //出现电话
		return retValue;
	}

	switch (cmdType) {
	case SET_APN: {
		retValue = GPRS_AT_SetAPN_Response(recv_);
		break;
	}
	case ACTIVE_GPRS: {
		retValue = GPRS_AT_ActiveGPRSNet_Response(recv_);
		break;
	}
	case CHECK_SIM_STATUS: {
		retValue = GPRS_AT_CheckSimStat_Response(recv_, _retErrorCode);
		break;
	}
	case QUERY_OPERATER: {
		retValue = GPRS_AT_QueryOperater_Response(recv_, _retErrorCode);
		break;
	}
	case QUERY_IP: {
		retValue = GPRS_AT_QueryIP_Response(
			recv_,
			_retErrorCode);  //如果第一步的"%ETCPIP:0"标识值为0，直接返回，后面不用进行了
		break;
	}
	case OPEN_TCP_LINK: {
		retValue = GPRS_AT_OpenTCPLink_Response(recv_);
		break;
	}
	case QUERY_TCP_LINK: {
		retValue = GPRS_AT_QueryTCPLink_Response(
			recv_,
			_retErrorCode);  //如果第一步的"%ETCPIP:0"标识值为0，直接返回，后面不用进行了
		break;
	}
	case CLOSE_TCP_LINK: {
		retValue = GPRS_AT_CloseTCPLink_Response(recv_, _retErrorCode);
		break;
	}
	case QUIT_GPRS_NET: {
		retValue = GPRS_AT_QuitGPRSNet_Response(recv_, _retErrorCode);
		break;
	}
	case SETIO_MODE: {
		retValue = GPRS_AT_SetIOMode_Response(recv_);
		break;
	}
	case SEND_DATA: {
		retValue = GRPS_AT_Send_Response(recv_, _retErrorCode);
		break;
	}
	case QUERY_REMAIN_DATA: {
		retValue = GPRS_AT_QueryRemainData_Response(recv_, _retErrorCode);
		break;
	}
	case RECEIVE_DATA: {
		retValue = GRPS_AT_Receive_Response(recv_, _retErrorCode);
		break;
	}
	case OFF_CALL: {
		retValue = GPRS_AT_OFF_CALL_Response(recv_);
		break;
	}
	default: {
		retValue = TRUE;
		break;
	}
	}
	return retValue;
}

//只要不是ERROR或者OK,其它响应执行完了，还要继续处理。这里有问题，明天继续处理!!!!!!!!!!! 17-08-21
int GPRS_SubProc_AT_ResponseTwo(char* recv_, int* _retErrorCode) {
	int retValue = FALSE;
	if (Utility_Strncmp(recv_, "OK", 2) == 0) {
		retValue = TRUE;
	}
	else if (Utility_Strncmp(recv_, "CONNECT", 7) == 0) {
		retValue = TRUE;
	}
	else if (Utility_Strncmp(recv_, "ERROR", 5) == 0)  // ERROR: 14  冒号和14中间有空格
	{
		char bufErrorTmp[ 3 ] = {
			0
		};  //调用atoi函数，需要有字符串结束符，2位错误码，1位结束符

		bufErrorTmp[ 0 ] = recv_[ 7 ];
		if (recv_[ 8 ] != 0) {
			bufErrorTmp[ 1 ] = recv_[ 8 ];
		}
		*_retErrorCode = atoi(bufErrorTmp);
		GPRS_Print_Error(*_retErrorCode);
		retValue = FALSE;
	}
	else {
		retValue = FALSE;
	}
	return retValue;
}

int GPRS_Proc_AT_Response(char* recv_, int* _retMsgNum, int cmdType, int* errorCode) {
	int _recvTryTimes1 = 0;
	int _retValue      = FALSE;
	int _okFlag;  //设定用来判断需不需要收2行响应

	//只返回一个OK的，okFlag初始为TRUE,返回2行的，初始为FALSE
	switch (cmdType) {
	case SET_APN: {
		_okFlag = TRUE;
		break;
	}
	case ACTIVE_GPRS: {
		_okFlag = TRUE;
		break;
	}
	case CHECK_SIM_STATUS: {
		_okFlag = FALSE;
		break;
	}
	case QUERY_OPERATER: {
		_okFlag = FALSE;
		break;
	}
	case QUERY_IP: {
		_okFlag = FALSE;
		break;
	}
	case OPEN_TCP_LINK: {
		_okFlag = TRUE;
		break;
	}
	case QUERY_TCP_LINK: {
		_okFlag = FALSE;
		break;
	}
	case CLOSE_TCP_LINK: {
		_okFlag = FALSE;
		break;
	}
	case QUIT_GPRS_NET: {
		_okFlag = FALSE;
		break;
	}
	case SETIO_MODE: {
		_okFlag = TRUE;
		break;
	}
	case SEND_DATA: {
		_okFlag = FALSE;
		break;
	}
	case QUERY_REMAIN_DATA: {
		_okFlag = FALSE;
		break;
	}
	case RECEIVE_DATA: {
		_okFlag = FALSE;
		break;
	}
	case OFF_CALL: {
		_okFlag = TRUE;
		break;
	}
	default: {
		_okFlag = TRUE;
		break;
	}
	}

	while (_recvTryTimes1 < 100) {
		if (UART0_RecvLineTry(recv_, UART0_MAXBUFFLEN, _retMsgNum) == -1) {
			_recvTryTimes1 += 1;
			if (cmdType == ACTIVE_GPRS)
				System_Delayms(
					700);  //原本打算最大延时30s，后来发现，如果发网络指令，刚好电话在前面进来，模块会卡住，也不会有ring进来，直到拨号结束，联通是45s自动挂断，移动是65s自动挂断
			else if (cmdType == QUERY_REMAIN_DATA)
				System_Delayms(100);
			else
				System_Delayms(
					500);  //只有激活GPRS会卡长时间，其它命令卡顿时间不超过30s
			continue;
		}
		else {
			if (_okFlag
			    == FALSE)  //如果返回的第一行不是OK,则需要处理第一行；否则直接处理第二行
			{
				int respRet;
				respRet = GPRS_SubProc_AT_ResponseOne(recv_, _retMsgNum, cmdType,
								      errorCode);
				if (respRet
				    == TRUE)  //如果第一步处理有返回结果，就继续处理，不正确也会继续收到一个OK,只要第一步不出现错误标志，收到消息就往下一步走，由下一步处理
				{
					_okFlag = TRUE;
				}
				else  //如果第一步返回结果不为TRUE，后面不需要处理了。
				{
					_retValue = FALSE;
					break;
				}
			}
			else {
				_retValue = GPRS_SubProc_AT_ResponseTwo(recv_, errorCode);
				if (_retValue == TRUE) {
					break;
				}
				else {
					if (errorCode != 0)  //只有出现错误才退出，否则不返回
						break;
				}
			}
		}
	}
	return _retValue;
}

int GPRS_WriteBytes(char* psrc, int len) {
	UART0_Send(psrc, len, 1);
	return 0;
}

int GPRS_AT_SetAPN() {
	int  errorCode   = 0;
	int  _retMsgNum  = 0;
	int  _cmdLen     = 0;
	int  _retValue   = FALSE;
	char _send[ 30 ] = "at+cgdcont=1,\"ip\",\"";
	char recv_[ 30 ] = { 0 };

	_cmdLen += 19;

	if (TelecomOperater == CHINA_MOBILE) {
		Utility_Strncpy(&_send[ _cmdLen ], "cmnet", Utility_Strlen("cmnet"));
	}
	else
		return SET_APN_FAILED;

	_cmdLen += Utility_Strlen("cmnet");
	_send[ _cmdLen ] = '\"';
	_cmdLen += 1;

	//该AT指令会返回:
	// OK
	//注意，需要先关闭回显，以免造成麻烦
	GPRS_PrepareForSend();
	GPRS_WriteBytes(_send, _cmdLen);
	_retValue = GPRS_Proc_AT_Response(recv_, &_retMsgNum, SET_APN, &errorCode);

	if (_retValue == TRUE) {
		debug_printf("Set APN Success\n\n");
		return SET_APN_SUCCESS;
	}
	else {
		err_printf("Set APN Failed, please check if antenna is pluged in\n\n");
	}

	return SET_APN_FAILED;
}

int GPRS_AT_ActiveGPRSNet() {
	int  errorCode   = 0;
	int  _retMsgNum  = 0;  
	int  _cmdLen     = 0;
	int  _retValue   = 0;
	char _send[ 10 ] = "at%etcpip";
	char recv_[ 30 ] = { 0 };
	_cmdLen += 9;

	//该AT指令会返回:
	// OK
	//注意，需要先关闭回显，以免造成麻烦
	GPRS_PrepareForSend();
	GPRS_WriteBytes(_send, _cmdLen);
	_retValue = GPRS_Proc_AT_Response(recv_, &_retMsgNum, ACTIVE_GPRS, &errorCode);

	if (_retValue == TRUE) {
		debug_printf("Active GPRS success\n\n");
		return ACTIVE_GPRS_SUCCESS;
	}
	else {
		err_printf("Active GPRS failed \n\n");
	}

	return ACTIVE_GPRS_FAILED;
}

int GPRS_AT_CheckSimStat() {
	int* errorCode   = NULL;
	int  _retMsgNum  = 0;  //
	int  _cmdLen     = 0;
	int  _retValue   = 0;
	char _send[ 10 ] = "AT%TSIM";
	char recv_[ 30 ] = { 0 };
	_cmdLen += 7;

	//该AT指令会返回:
	// OK
	//注意，需要先关闭回显，以免造成麻烦
	GPRS_PrepareForSend();
	GPRS_WriteBytes(_send, _cmdLen);
	_retValue = GPRS_Proc_AT_Response(recv_, &_retMsgNum, CHECK_SIM_STATUS, errorCode);

	if (_retValue == TRUE) {
		debug_printf("CheckSimStat Success \n\n");
		return GPRS_CHECKSIM_SUCCESS;
	}
	else {
		err_printf("Check Sim card Stat failed, please check if sim card pluged in\n\n");
	}

	return GPRS_CHECKSIM_FAILED;
}

int GPRS_AT_QueryOperater() {
	int  errorCode    = 0;
	int  _retMsgNum   = 0;  //
	int  _cmdLen      = 0;
	int  _retValue    = 0;
	char _send[ 8 ]   = "AT+COPS?";
	char recv_[ 100 ] = { 0 };
	_cmdLen += 8;

	//该AT指令会返回:
	// OK
	//注意，需要先关闭回显，以免造成麻烦
	GPRS_PrepareForSend();
	GPRS_WriteBytes(_send, _cmdLen);
	_retValue = GPRS_Proc_AT_Response(recv_, &_retMsgNum, QUERY_OPERATER, &errorCode);

	if (_retValue == TRUE) {
		if (TelecomOperater == CHINA_MOBILE) {
			debug_printf("Operater is China Mobile \n\n" );
			return QUERY_OPERATER_SUCCESS;
		}
	}
	else {
		err_printf("query Operater Failed \n\n");
	}

	return QUERY_OPERATER_FAILED;
}

int GPRS_AT_QueryIP() {
	int  errorCode    = 0;
	int  _retMsgNum   = 0;  //
	int  _cmdLen      = 0;
	int  _retValue    = 0;
	char _send[ 10 ]  = "at%etcpip?";
	char recv_[ 100 ] = { 0 };
	_cmdLen += 10;

	//该AT指令会返回:
	// OK
	//注意，需要先关闭回显，以免造成麻烦
	GPRS_PrepareForSend();
	GPRS_WriteBytes(_send, _cmdLen);
	_retValue = GPRS_Proc_AT_Response(recv_, &_retMsgNum, QUERY_IP, &errorCode);
	// 查询指令会先返回一行%ETCPIP:1,"10.101.53.141",,"114.114.114.114","211.138.156.146",
	//注意，只返回本地IP还不行，还要有网关和DNS，否则后面打开连链接会出问题
	// 再返回一行 OK

	if (_retValue == TRUE) {
		debug_printf("QUERY IP Success\n\n");
		return QUERY_IP_SUCCESS;
	}
	else {
		err_printf("query network ip failed \n\n");
	}

	return QUERY_IP_FAILED;
}

void judgeintlen(int* ip, char* iplen, char len) {
	char i;
	for (i = 0; i < len; i++) {
		if (ip[ i ] >= 100)
			iplen[ i ] = 3;
		else if (ip[ i ] >= 10)
			iplen[ i ] = 2;
		else
			iplen[ i ] = 1;
	}
}

char judgeportlen(char* port) {
	char portlen[ 3 ] = { 0, 0, 0 };
	char i;

	for (i = 0; i < 3; i++) {
		if (port[ i ] >= 10)
			portlen[ i ] = 2;
		else if (port[ i ] > 0)
			portlen[ i ] = 1;
		else
			portlen[ i ] = 0;
	}
	return portlen[ 0 ] + portlen[ 1 ] + portlen[ 2 ];
}

int GPRS_AT_OpenTCPLink(int center) {
	int  errorCode     = 0;
	int  _retMsgNum    = 0;  //
	int  _cmdLen       = 0;
	int  _retValue     = 0;
	int  IPStrLen      = 0;
	int  PortStrLen    = 0;
	char recv_[ 30 ]   = { 0 };
	char centerip[ 6 ] = { 0, 0, 0, 0, 0, 0 };
	int  ip[ 4 ]       = { 0, 0, 0, 0 };
	char iplen[ 4 ]    = { 0, 0, 0, 0 };
	char i, j;
	char centerport[ 3 ] = { 0, 0, 0 };
	char centerportlen   = 0;

	char ipvalue[ 10 ];

	char _send[ 50 ] = "AT%IPOPEN=\"TCP\",\"";
	_cmdLen += 17;

	memset(ipvalue, 0, sizeof(ipvalue));
	Hydrology_ReadStoreInfo(center, ipvalue, HYDROLOGY_CENTER_IP_LEN);

	memcpy(centerip, &ipvalue[ 1 ], 6);
	memcpy(centerport, &ipvalue[ 7 ], 3);

	//    Store_ReadHydrologyServerIP(centerip,center);
	centerip[ 0 ] = _BCDtoDEC(centerip[ 0 ]);
	centerip[ 1 ] = _BCDtoDEC(centerip[ 1 ]);
	centerip[ 2 ] = _BCDtoDEC(centerip[ 2 ]);
	centerip[ 3 ] = _BCDtoDEC(centerip[ 3 ]);
	centerip[ 4 ] = _BCDtoDEC(centerip[ 4 ]);
	centerip[ 5 ] = _BCDtoDEC(centerip[ 5 ]);

	ip[ 0 ] = centerip[ 0 ] * 10 + centerip[ 1 ] / 10;
	ip[ 1 ] = (centerip[ 1 ] % 10) * 100 + centerip[ 2 ];
	ip[ 2 ] = centerip[ 3 ] * 10 + centerip[ 4 ] / 10;
	ip[ 3 ] = (centerip[ 4 ] % 10) * 100 + centerip[ 5 ];

	judgeintlen(ip, iplen, 4);

	for (i = 0, j = 0; i < 4; i++) {
		if (iplen[ i ] == 3) {
			centerIP[ j ]     = ip[ i ] / 100 + '0';
			centerIP[ j + 1 ] = ((ip[ i ] / 10) % 10) + '0';
			centerIP[ j + 2 ] = ip[ i ] % 10 + '0';
			centerIP[ j + 3 ] = '.';
			j += 4;
		}
		else if (iplen[ i ] == 2) {
			centerIP[ j ]     = ip[ i ] / 10 + '0';
			centerIP[ j + 1 ] = ip[ i ] % 10 + '0';
			centerIP[ j + 2 ] = '.';
			j += 3;
		}
		else {
			centerIP[ j ]     = ip[ i ] + '0';
			centerIP[ j + 1 ] = '.';
			j += 2;
		}
	}
	centerIP[ j - 1 ] = '\0';
	printf("connecting to server %s",centerIP);

	IPStrLen = Utility_Strlen(centerIP);
	Utility_Strncpy(&_send[ _cmdLen ], centerIP, IPStrLen);
	_cmdLen += IPStrLen;

	_send[ _cmdLen ] = '\"';
	_cmdLen += 1;

	_send[ _cmdLen ] = ',';
	_cmdLen += 1;

	//    Store_ReadHydrologyServerPort(centerport,center);
	centerport[ 0 ] = _BCDtoDEC(centerport[ 0 ]);
	centerport[ 1 ] = _BCDtoDEC(centerport[ 1 ]);
	centerport[ 2 ] = _BCDtoDEC(centerport[ 2 ]);

	centerportlen = judgeportlen(centerport);

	switch (centerportlen) {
	case 6: {
		centerPort[ 0 ] = centerport[ 0 ] / 10 + '0';
		centerPort[ 1 ] = centerport[ 0 ] % 10 + '0';
		centerPort[ 2 ] = centerport[ 1 ] / 10 + '0';
		centerPort[ 3 ] = centerport[ 1 ] % 10 + '0';
		centerPort[ 4 ] = centerport[ 2 ] / 10 + '0';
		centerPort[ 5 ] = centerport[ 2 ] % 10 + '0';
		centerPort[ 6 ] = '\0';
		break;
	}
	case 5: {
		centerPort[ 0 ] = centerport[ 0 ] % 10 + '0';
		centerPort[ 1 ] = centerport[ 1 ] / 10 + '0';
		centerPort[ 2 ] = centerport[ 1 ] % 10 + '0';
		centerPort[ 3 ] = centerport[ 2 ] / 10 + '0';
		centerPort[ 4 ] = centerport[ 2 ] % 10 + '0';
		centerPort[ 5 ] = '\0';
		break;
	}
	case 4: {
		centerPort[ 0 ] = centerport[ 1 ] / 10 + '0';
		centerPort[ 1 ] = centerport[ 1 ] % 10 + '0';
		centerPort[ 2 ] = centerport[ 2 ] / 10 + '0';
		centerPort[ 3 ] = centerport[ 2 ] % 10 + '0';
		centerPort[ 4 ] = '\0';
		break;
	}
	case 3: {
		centerPort[ 0 ] = centerport[ 1 ] % 10 + '0';
		centerPort[ 1 ] = centerport[ 2 ] / 10 + '0';
		centerPort[ 2 ] = centerport[ 2 ] % 10 + '0';
		centerPort[ 3 ] = '\0';
		break;
	}
	case 2: {
		centerPort[ 0 ] = centerport[ 2 ] / 10 + '0';
		centerPort[ 1 ] = centerport[ 2 ] % 10 + '0';
		centerPort[ 2 ] = '\0';
		break;
	}
	case 1: {
		centerPort[ 0 ] = centerport[ 2 ] % 10 + '0';
		centerPort[ 1 ] = '\0';
		break;
	}
	}
	if (Utility_Strncmp("59.48.98.130", centerIP, 12) == 0)
		Utility_Strncpy(centerPort, "5005", 4);
	PortStrLen = Utility_Strlen(centerPort);
	Utility_Strncpy(&_send[ _cmdLen ], centerPort, PortStrLen);
	_cmdLen += PortStrLen;

	printf(":%s \n\n",centerPort);

	//该AT指令会返回:
	// OK
	//注意，需要先关闭回显，以免造成麻烦

	GPRS_PrepareForSend();
	GPRS_WriteBytes(_send, _cmdLen);
	_retValue = GPRS_Proc_AT_Response(recv_, &_retMsgNum, OPEN_TCP_LINK, &errorCode);
	//开启成功会返回"CONNECT"

	if (_retValue == TRUE) {
		debug_printf("TCP Connection success \n\n");
		// Console_WriteStringln("TCP Connection success");
		return TCP_CONNECT_SUCCESS;
	}
	else {
		err_printf("TCP Connection failed-1223 \n\n");
		//++SIM_ErrorNum;
	}

	return TCP_CONNECT_FAILED;
}

//此函数暂时未用到
int GPRS_AT_QueryTCPLink(int* errorCode) {
	int  _retMsgNum   = 0;  //
	int  _cmdLen      = 0;
	int  _retValue    = 0;
	char _send[ 10 ]  = "AT%IPOPEN?";
	char recv_[ 100 ] = { 0 };
	_cmdLen += 10;

	//该AT指令会返回:
	// OK
	//注意，需要先关闭回显，以免造成麻烦
	GPRS_PrepareForSend();
	GPRS_WriteBytes(_send, _cmdLen);
	_retValue = GPRS_Proc_AT_Response(recv_, &_retMsgNum, QUERY_TCP_LINK, errorCode);
	// 查询指令会先返回一行%ETCPIP:1,"10.101.53.141",,"114.114.114.114","211.138.156.146",
	//注意，只返回本地IP还不行，还要有网关和DNS，否则后面打开连链接会出问题
	// 再返回一行 OK

	if (_retValue == FALSE)  // 0，证明没有激活GPRS网络
	{
		Console_WriteStringln("No Tcp Connection error");
		return NO_TCP_CONNECT;
	}
	else {
		debug_printf("Tcp Connection OK\n\n");
		return TCP_CONNECT_OK;
	}
}

int GPRS_AT_CloseTCPLink(int* errorCode) {
	int  _retMsgNum  = 0;  //
	int  _cmdLen     = 0;
	int  _retValue   = 0;
	char recv_[ 30 ] = { 0 };

	char _send[ 50 ] = " AT%IPCLOSE=1";
	_cmdLen += 12;

	//该AT指令会返回:
	//%IPCLOSE:
	// OK
	//注意，需要先关闭回显，以免造成麻烦
	GPRS_PrepareForSend();
	GPRS_WriteBytes(_send, _cmdLen);
	_retValue = GPRS_Proc_AT_Response(recv_, &_retMsgNum, CLOSE_TCP_LINK, errorCode);

	if (_retValue == FALSE) {
		err_printf("Close TCP link failed-1286 \n\n");
		return TCP_Link_CLOSE_FAILED;
	}
	else {
		debug_printf("Close TCP link Success \n\n");
		return TCP_Link_CLOSE_SUCCESS;
	}
}

int GPRS_AT_QuitGPRSNet(int* errorCode) {
	int  _retMsgNum  = 0;  //
	int  _cmdLen     = 0;
	int  _retValue   = 0;
	char recv_[ 30 ] = { 0 };
	char _send[ 50 ] = "AT%IPCLOSE=5";
	_cmdLen += 12;

	//该AT指令会返回:
	//%IPCLOSE:
	// OK
	//注意，需要先关闭回显，以免造成麻烦
	GPRS_PrepareForSend();
	GPRS_WriteBytes(_send, _cmdLen);
	_retValue = GPRS_Proc_AT_Response(recv_, &_retMsgNum, QUIT_GPRS_NET, errorCode);
	//开启成功会返回"CONNECT"

	if (_retValue == TRUE) {
		debug_printf("Quit GPRS Net Success \n\n");
		// Console_WriteStringln("Quit GPRS Net Success");
		return GPRS_NET_QUIT_SUCCESS;
	}
	else {
		err_printf("Quit GPRS Net failed \n\n");
	}

	return GPRS_NET_QUIT_FAILED;
}

int gprsConfigSuccess = FALSE;
//使用ASCii码还是Hex码发送消息，0,1,1---ASCii， 1,1,0---Hex
int GPRS_AT_SetIOMode() {
	int  errorCode   = 0;
	int  _retMsgNum  = 0;  //
	int  _cmdLen     = 0;
	int  _retValue   = 0;
	char _send[ 20 ] = "at%iomode=1,1,0";
	char recv_[ 30 ] = { 0 };
	_cmdLen += 15;

	//该AT指令会返回:
	// OK
	//注意，需要先关闭回显，以免造成麻烦
	GPRS_PrepareForSend();
	GPRS_WriteBytes(_send, _cmdLen);
	_retValue = GPRS_Proc_AT_Response(recv_, &_retMsgNum, SETIO_MODE, &errorCode);

	if (_retValue == TRUE) {
		debug_printf("Set IO Mode Success\n\n");
		return SET_IOMODE_SUCCESS;
	}
	else {
		err_printf("Set IO Mode Failed \n\n");
	}

	return SET_IOMODE_FAILED;
}

// int GRPS_AT_Send(char* psrc, int dataLen, int * errorCode)
int GRPS_AT_Send() {
#if 0
    if(_GTM900SendBufLeft == 0 )//如果发送缓存=0，则不发送。
    {
        return MSG_SEND_FAILED;
    }
#endif

	int  errorCode   = 0;
	int  _retMsgNum  = 0;  //
	int  _cmdLen     = 0;
	int  _retValue   = 0;
	char recv_[ 30 ] = { 0 };

	char* _send = ( char* )os_malloc(dataLen + 12);
	if (_send == NULL) {
		err_printf("os_malloc in GPRS_AT_Send failed \n\n");
		return MSG_SEND_FAILED;
	}
	Utility_Strncpy(_send, "AT%IPSEND=\"", 11);
	_cmdLen += 11;

	Utility_Strncpy(&_send[ _cmdLen ], psrc, dataLen);
	_cmdLen += dataLen;

	_send[ _cmdLen ] = '\"';
	_cmdLen += 1;

	//该AT指令会返回:
	// OK
	//注意，需要先关闭回显，以免造成后续数据接收干扰
	GPRS_PrepareForSend();
	GPRS_WriteBytes(_send, _cmdLen);
	_retValue = GPRS_Proc_AT_Response(recv_, &_retMsgNum, SEND_DATA, &errorCode);
	// 发送指令会先返回一行 %IPSEND:1,15
	// 再返回一行 OK
	os_free(_send);
	_send = NULL;

	if (_retValue == TRUE) {
		debug_printf("AT Send Data Success \n\n");
		return MSG_SEND_SUCCESS;
	}
	else {
		err_printf("AT Send Data failed-sub1408 \n\n");
	}

	return MSG_SEND_FAILED;
}

int GRPS_AT_QueryRemainData() {
	int  errorCode    = 0;
	int  _retMsgNum   = 0;
	int  _cmdLen      = 0;
	int  _retValue    = 0;
	char _send[ 8 ]   = "AT%IPDQ";
	char recv_[ 200 ] = { 0 };

	_cmdLen += 7;
	//该AT指令会返回:
	// OK
	//注意，需要先关闭回显，以免造成后续数据接收干扰
	GPRS_PrepareForSend();
	GPRS_WriteBytes(_send, _cmdLen);
	_retValue = GPRS_Proc_AT_Response(recv_, &_retMsgNum, QUERY_REMAIN_DATA, &errorCode);
	// 发送指令会先返回一行 %IPDQ:
	// 再返回一行 OK

	if (_retValue == TRUE) {
		return QUERY_REMAIN_DATA_SUCCESS;
	}
	else {
		Console_WriteStringln("AT Query remaining data failed");
	}

	return QUERY_REMAIN_DATA_FAILED;
}

int GRPS_AT_Receive() {
	int  errorCode    = 0;
	int  _retMsgNum   = 0;
	int  _cmdLen      = 0;
	int  _retValue    = 0;
	char _send[ 8 ]   = "AT%IPDR";
	char recv_[ 200 ] = { 0 };

	_cmdLen += 7;
	//该AT指令会返回:
	// OK
	//注意，需要先关闭回显，以免造成后续数据接收干扰
	GPRS_PrepareForSend();
	GPRS_WriteBytes(_send, _cmdLen);
	_retValue = GPRS_Proc_AT_Response(recv_, &_retMsgNum, RECEIVE_DATA, &errorCode);
	// 发送指令会先返回一行 %IPDR:1,1,87,"7E7E"
	// 再返回一行 OK

	if (_retValue == TRUE) {
		debug_printf("AT Receive Data Success \n\n");
		return MSG_RECEIVE_SUCCESS;
	}
	else {
		err_printf("AT Receive Data failed \n\n");
	}

	return MSG_RECEIVE_FAILED;
}

int GPRS_OFF_CALL() {
	int  retNum_;
	char ath_[] = "ATH";
	char recv_[ 50 ];
	//该指令如果挂断了电话,返回OK,无电话状态时,也返回OK,OK只表示模块收到了指令
	// OK
	UART0_Send(ath_, 3, 1);  //两个ring之间时间差大于ring和OK出现时间差，因此，OK肯定会先出来?
				 // if(UART0_RecvLineWait(recv_,50,&retNum_)==-1)
	if (UART0_RecvLineWait(recv_, 50, &retNum_) == -1) {
		return 0;
	}
	if (Utility_Strncmp(recv_, "OK", 2) == 0) {
		return -3;
	}
	else {
		return 0;
	}
}

int GPRS_AT_OFF_CALL() {
	int  retNum_ = 0;
	char ath_[]  = "ATH";
	char recv_[ 50 ];
	//该指令如果挂断了电话,返回OK,无电话状态时,也返回OK,OK只表示模块收到了指令
	// OK
	UART0_Send(ath_, 3, 1);  //两个ring之间时间差大于ring和OK出现时间差，因此，OK肯定会先出来?
	if (UART0_RecvLineWait(recv_, 50, &retNum_) == -1) {
		return 0;
	}
	if (Utility_Strncmp(recv_, "OK", 2) == 0) {
		return -3;
	}
	else {
		return 0;
	}
}

int GPRS_ClearRecvBuf() {
	UART0_ClearBuffer();
	return 0;
}

void GPRS_PrepareForSend() {
	char _recvBeforeSend[ 100 ];
	int  _retLenghtBeforeSend;

	if (UART0_RecvLineTry(_recvBeforeSend, UART0_MAXBUFFLEN, &_retLenghtBeforeSend) == 0) {
		if (Utility_Strncmp(_recvBeforeSend, "RING", 4)
		    == 0)  //如果是电话来了，立刻关闭电话，其它响应一概不处理
		{
			GPRS_AT_OFF_CALL();
		}
	}
	GPRS_ClearRecvBuf();  //清空缓存
}

int GPRS_Create_TCP_Link(int center) {
	int repeats_ = 0;

	if (gprsConfigSuccess == FALSE) {
		// gprs_power_on();

		while (GSM_AT_CloseFeedback() != 0) {
			if (repeats_ > 0)
				return -1;
			++repeats_;
		}
		repeats_ = 0;

		while (GPRS_AT_CheckSimStat() != GPRS_CHECKSIM_SUCCESS) {
			if (repeats_ > 0)
				return -1;
			++repeats_;
		}
		repeats_ = 0;

		while (GPRS_AT_QueryOperater() != QUERY_OPERATER_SUCCESS) {
			if (repeats_ > 0)
				return -1;
			++repeats_;
		}
		repeats_ = 0;

		while (GPRS_AT_SetAPN() != SET_APN_SUCCESS) {
			if (repeats_ > 0)
				return -1;
			++repeats_;
		}
		repeats_ = 0;

		while (GPRS_AT_ActiveGPRSNet() != ACTIVE_GPRS_SUCCESS) {
			if (repeats_ > 0)
				return -1;
			++repeats_;
		}
		repeats_ = 0;

		while (GPRS_AT_QueryIP() != QUERY_IP_SUCCESS) {
			if (repeats_ > 0)
				return -1;
			++repeats_;
		}
		repeats_ = 0;

		while (GPRS_AT_OpenTCPLink(center) != TCP_CONNECT_SUCCESS) {
			if (repeats_ > 0)
				return -1;
			++repeats_;
		}
		repeats_ = 0;

		while (GPRS_AT_SetIOMode() != SET_IOMODE_SUCCESS) {
			if (repeats_ > 0)
				return -1;
			++repeats_;
		}
		repeats_ = 0;

		gprsConfigSuccess = TRUE;
	}

	return 0;
}

int GPRS_Close_TCP_Link() {
	int repeats_ = 0;
	int _retErrorCode;

	gprsConfigSuccess = FALSE;

	while (GPRS_AT_CloseTCPLink(&_retErrorCode) != TCP_Link_CLOSE_SUCCESS) {
		if (repeats_ > 0)
			return -1;
		++repeats_;
	}
	repeats_ = 0;

	while (GPRS_AT_QuitGPRSNet(&_retErrorCode) != GPRS_NET_QUIT_SUCCESS) {
		if (repeats_ > 0)
			return -1;
		++repeats_;
	}
	repeats_ = 0;

	return 0;
}

void GPRS_Close_GSM() {
	gprsConfigSuccess = FALSE;
	gprs_power_off();
	System_Delayms(1000);  //关机延时1s
}

int GPRS_Send(char* pSend, int sendDataLen, int isLastPacket, int center) {
	int _retvalue;
	//首先查看TCP连接建立没有，没有建立，需要重新建立连接?
	if (gprsConfigSuccess == FALSE) {
		GPRS_Close_GSM();
		GPRS_Create_TCP_Link(center);
	}

	if (sendDataLen > 1024) {
		err_printf(
			"Error, Packet exceed 1024, Please Decrease Sending Packet Size! \n\n");
	}

	psrc = ( char* )os_malloc(
		sendDataLen * 2 + 1);  //+1是因为转换函数hex_2_ascii最后有个\0,否则最后会出越界错误
	if (psrc == NULL) {
		err_printf("os_malloc in GPRS_Send Failed \n\n");
		return FALSE;
	}
	hex_2_ascii(pSend, psrc, sendDataLen);
	dataLen = sendDataLen * 2;

	_retvalue = GRPS_AT_Send();

	os_free(psrc);
	psrc = NULL;
	GPRS_Close_TCP_Link();

	if (_retvalue == MSG_SEND_SUCCESS) {
		debug_printf("GPRS Send Data Success \n\n");
		return TRUE;
	}
	else {
		err_printf("GPRS Send Data Failed-1690 \n\n");
	}

	return FALSE;
}

int GPRS_QueryRemainData() {
	if (GRPS_AT_QueryRemainData() == QUERY_REMAIN_DATA_SUCCESS) {
		return RemainingData;
	}
	return -1;
}

char  hexdata[ 100 ];
char* GPRS_Receive() {

	memset(hexdata, 0, 100);
	if (GRPS_AT_Receive() == MSG_RECEIVE_SUCCESS) {
		ConvertAscIItoHex(_ReceiveData, hexdata, _ReceiveDataLen * 2);
		os_free(_ReceiveData);
		_ReceiveData = NULL;
	}
	else if (GRPS_AT_Receive() == MSG_RECEIVE_SUCCESS) {
		ConvertAscIItoHex(_ReceiveData, hexdata, _ReceiveDataLen * 2);
		os_free(_ReceiveData);
		_ReceiveData = NULL;
	}
	else if (GRPS_AT_Receive() == MSG_RECEIVE_SUCCESS) {
		ConvertAscIItoHex(_ReceiveData, hexdata, _ReceiveDataLen * 2);
		os_free(_ReceiveData);
		_ReceiveData = NULL;
	}

	return hexdata;
}

char unprocessedGPRSdata = 0;
int  GPRSDataArrived     = FALSE;
void JudgeServerDataArrived(void) {
	if (Utility_Strncmp(UART0_Rx_Buffer[ 0 ], "%IPDATA:", 8) == 0) {
		memset(UART0_Rx_Buffer[ 0 ], 0, UART0_MAXBUFFLEN);
		GPRSDataArrived = TRUE;
	}
	if (Utility_Strncmp(UART0_Rx_Buffer[ 1 ], "%IPDATA:", 8) == 0) {
		memset(UART0_Rx_Buffer[ 1 ], 0, UART0_MAXBUFFLEN);
		GPRSDataArrived = TRUE;
	}
	if (Utility_Strncmp(UART0_Rx_Buffer[ 2 ], "%IPDATA:", 8) == 0) {
		memset(UART0_Rx_Buffer[ 2 ], 0, UART0_MAXBUFFLEN);
		GPRSDataArrived = TRUE;
	}
}

int Hydrology_ProcessGPRSReceieve() {
	char*			dowmhydrologydata = NULL;
	communication_module_t* comm_dev	  = get_communication_dev();

	if (GPRSDataArrived != TRUE)
		return -1;
	//   GPRS_QueryRemainData();
	unprocessedGPRSdata = 1;
	while (unprocessedGPRSdata--) {
		dowmhydrologydata = comm_dev->rcv_msg();
		TraceHexMsg(dowmhydrologydata, _ReceiveDataLen);
		if (hydrologyProcessReceieve(dowmhydrologydata, _ReceiveDataLen) != 0)
			continue;
		HydrologyRecord(ERC17);
	}
	if (dowmhydrologydata != NULL)
		os_free(dowmhydrologydata);

	GPRSDataArrived = FALSE;
	return 0;
}

//这条指令会收到:
// ATE0
// OK
static int GSM_AT_CloseFeedback() {
	int  retNum_;
	int  repeats_    = 0;
	char recv_[ 30 ] = "ATE0";
CloseFeedback:
	// UART0_Send(recv_,4,1); lmj20170814
	UART0_Send(recv_, 4,
		   1);  // lmj0814不用UART0发送里面的补充回车换行，直接将回车换行放在指令字符串当中
	//    System_Delayms(50);
	if (UART0_RecvLineWait(recv_, 30, &retNum_) == -1) {
		++s_TimeOut;
		return -1;
	}
	//收到ERROR ,
	if (Utility_Strncmp(recv_, "ERROR", 5) == 0) {
		if (repeats_ > 2) {
			err_printf("gprs module Close feedback failed\n\n");
			return -2;
		}
		//++SIM_ErrorNum;
		++repeats_;
		goto CloseFeedback;
	}
	//如果是ATE0,则再获得一个OK,就可以返回了
	if (Utility_Strncmp(recv_, "ATE0", 4) == 0) {
		if (UART0_RecvLineWait(recv_, 30, &retNum_) == -1) {
			++s_TimeOut;
			return -1;
		}
		if (Utility_Strncmp(recv_, "OK", 2) == 0) {
			debug_printf("Close feedback success \n\n");
			return 0;
		}
		//++SIM_ErrorNum;
		return -2;
	}
	//如果是OK,说明已经关闭
	if (Utility_Strncmp(recv_, "OK", 2) == 0)
		return 0;
	GSM_DealData(recv_, retNum_);
	//++SIM_BadNum;
	return -2;
}

static int GSM_AT_OffCall() {
	GSM_DealAllData();
	int  retNum_;
	char ath_[] = "ATH";
	char recv_[ 50 ];
	//该指令如果挂断了电话,那么就没有返回,无电话状态时,返回OK
	// OK
	UART0_Send(ath_, 3, 1);
	if (UART0_RecvLineWait(recv_, 50, &retNum_) == -1)
		return 0;
	if (Utility_Strncmp(recv_, "OK", 2) == 0)
		return -3;
	else {
		GSM_DealData(recv_, retNum_);
		return 0;
	}
}

// 专门用来清空缓冲区;
// 对每行数据调用SIM_Deal
static int GSM_DealAllData() {
	int  retNum_;
	char recv_[ UART0_MAXBUFFLEN ];
	while (UART0_RecvLineTry(recv_, UART0_MAXBUFFLEN, &retNum_) == 0) {
		if (GSM_DealData(recv_, retNum_) == 2) {
			return 2;
		}
	}
	return 0;
}

static int GSM_DealData(char* recv_, int _dataLen) {
	//
	//  主动型数据
	//
	if (Utility_Strncmp(recv_, "RING", 4) == 0) {  //电话
		++s_RING;
		debug_printf(" RING ! \n\n");
		// Console_WriteStringln("SIM_Deal:RING !");
		GSM_AT_OffCall();
		return 2;
	}
	if (Utility_Strncmp(recv_, "+CMTI: ", 7) == 0) {
		Led1_WARN();
		debug_printf("Got A Msg ! \n\n");
		++s_MsgNum;		  //存放
		if (recv_[ 13 ] == '\0')  // recvLine函数将末尾处理为'\0'
			s_MsgArray[ s_RecvIdx ] = (recv_[ 12 ] - '0');
		else
			s_MsgArray[ s_RecvIdx ] = (recv_[ 12 ] - '0') * 10 + recv_[ 13 ] - '0';

		if (s_RecvIdx < ARRAY_MAX - 1)
			++s_RecvIdx;  //增加索引,ArrayIdx指向的是第一个无效位
		else
			s_RecvIdx = 0;
		//++Deal_Msg;
		return 2;
	}
	if (Utility_Strncmp(recv_, "+CMGS: ", 7) == 0) {
		return 1;
	}
	//
	//  响应型数据
	//
	//按最可能收到 以及 处理优先级的 顺序排列
	if (Utility_Strncmp(recv_, "OK", 2) == 0) {
		return 0;
	}
	//其他的都不管了.
	return -1;
}