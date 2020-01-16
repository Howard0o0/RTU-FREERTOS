#include "BC95.h"
#include "FreeRTOS.h"
#include "common.h"
#include "console.h"
#include "memoryleakcheck.h"
#include "uart0.h"
#include "FreeRTOS.h"
#include "memoryleakcheck.h"
#include "hydrologycommand.h"
#include "uart1.h"
#include <msp430x54x.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//相关AT指令

#define AT_BC95_NCONFIG "AT+NCONFIG?"
#define AT_BC95_SCRAMB_1 "AT+NCONFIG=CR_0354_0338_SCRAMBLING,TRUE"  //开启扰码功能
#define AT_BC95_SCRAMB_2 "AT+NCONFIG=CR_0859_SI_AVOID,TURE"
#define AT_BC95_AUTOCONFIG "AT+NCONFIG=AUTOCONNECT,TRUE"  //设置成自动模式
#define AT_BC95_SIM "AT+CIMI"				  //询问sim卡
#define AT_BC95_CFUN "AT+CFUN?"				  //询问是否是全功能模式
#define AT_BC95_SFUN "AT+CFUN=1"			  //设置全功能模式
#define AT_BC95_CSQ "AT+CSQ"				  //查询信号
#define AT_BC95_CGATT "AT+CGATT?"			  ///查询网络状态
#define AT_BC95_NNMI "AT+NNMI=1"			  //将接收到的数据打印到串口
#define AT_BC95_CPSMS "AT+CPSMS?"			  //查询是否是睡眠模式
#define AT_BC95_SPSMS "AT+CPSMS=0"  //关闭睡眠模式，才能显示接收到的数据
#define AT_BC95_ATE0 "ATE0"	 //关闭回显
#define AT_BC95_NSMI "AT+NSMI=1"    //设置发送消息后返回是否发送成功

#define BC95_WAITTIME 300  // 30S
#define BC95_DELAY_MS 1000
#define BC95_REPEAT_TIMES 10
#define BC95_RETRY_TIMES 3

#define Cloud_IP "221.229.214.202"
#define Cloud_IP_Port "5683"

int  g_autoconfigState = 0;  //扰码功能
int  g_scrambState     = 0;  ////自动模式
int  g_cfunstate       = 0;  //全功能模式


int BC95_Char_to_Hex(char chr)
{
	if ((chr >= 0x00) && (chr <= 0x09))
	{
		chr = chr + 0x30;
	}
	else {
		chr = chr + 87;
	}

	return chr;
}



int BC95_UintToStr4(unsigned int _src, char* _dest) {
	_dest[ 0 ] = (_src & 0xF000) >> 12;
	_dest[ 0 ] = BC95_Char_to_Hex(_dest[ 0 ]);

	_dest[ 1 ] = (_src & 0x0F00) >> 8;
	_dest[ 1 ] = BC95_Char_to_Hex(_dest[ 1 ]);

	_dest[ 2 ] = (_src & 0x00F0) >> 4;
	_dest[ 2 ] = BC95_Char_to_Hex(_dest[ 2 ]);

	_dest[ 3 ] = _src & 0x000F;
	_dest[ 3 ] = BC95_Char_to_Hex(_dest[ 3 ]);

	return 0;
}


/********
Function：给BC95模块上电
Description： VBAT:P10.7  RST:P10.3  ( in msp2418: VBAT[P4.2]  IGT[P1.0] )
********/
int BC95_Open() {
	int  second_time_MAX	   = 10;  //等待10s
	char * _data = NULL;

	/* BC95 connect cpu via uart0 */
	UART0_Open_9600(UART0_GSM_TYPE);

	// 为模块上电
	P10DIR |= BIT7;  // P10.7 为 BATT,给高电平
	P10OUT &= ~BIT7;
	P10OUT |= BIT7;

	System_Delayms(100);  // 需要延时

	P10DIR |= BIT3;  //  p3.0 Ϊ /IGT
	P10OUT &= ~BIT3;
	P10OUT |= BIT3;

	System_Delayms(10000);

        _data = BC95_Receive(second_time_MAX);
        if (strstr(_data, "OK")) {
                printf("BC95 Open \n\n");
                return 0;
        }


	return 1;
}

/********
Function：给BC95模块断电
********/
void BC95_Close() {
	P10OUT &= ~BIT7;  // P4.2输出低电平
	P10OUT &= ~BIT3;
	UART0_Close();
}

/********
Function：BC95发送数据
********/

int BC95_Send(char* data) {
	UART0_Send(data, Utility_Strlen(data), 1);
	debug_printf("AT Send Cmd : %s \n\n",data);
	return 0;
}

/********
Function：查询模块状态
********/
void BC95_QueryState() {
        char * _data = NULL;

	BC95_Send(AT_BC95_NCONFIG);
        debug_printf("%s\n\n",AT_BC95_NCONFIG);

        _data = BC95_Receive(BC95_REPEAT_TIMES);
        if (strstr(_data, "+NCONFIG:CR_0354_0338_SCRAMBLING,TRUE")) {
                g_scrambState = 1;
        }
        if (strstr(_data, "+NCONFIG:AUTOCONNECT,TRUE")) {
                g_autoconfigState = 1;
        }
        if ((g_autoconfigState == 1) && (g_scrambState == 1)) {
                return;
        }

}

/********
Function：查询时间
********/
void gettimefromdata(char* data,char* year, char* month, char* date, char* hour, char* min, char* second)
{
        char* substr   = "+CCLK:";
        char* pcAtResp = strstr(data, substr);
        int   offset   = 6;
        char  year1, year2, month1, month2, date1, date2, hour1, hour2, min1, min2, sec1,
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
        *year   = (year1 - 48) * 10 + (year2 - 48);
        *month  = (month1 - 48) * 10 + (month2 - 48);
        *date   = (date1 - 48) * 10 + (date2 - 48);
        *hour   = ((hour1 - 48) * 10 + (hour2 - 48)) + 8;
        *min    = (min1 - 48) * 10 + (min2 - 48);
        *second = (sec1 - 48) * 10 + (sec2 - 48);
}

int BC95_QueryTime(char* year, char* month, char* date, char* hour, char* min, char* second) {
	/* char *year,char *month,char *date,char *hour,char *min,char *second */
        char * _data = NULL;
	int  rcvFlag		       = 0;

	BC95_Send("AT+CCLK?");
        _data = BC95_Receive(BC95_REPEAT_TIMES);
        if (strstr(_data, "+CCLK:") != NULL) {
                rcvFlag = 1;
        }

	/* +CCLK: */
	if (rcvFlag) {
                gettimefromdata(_data,year,month,date,hour,min,second);
		return 0;
	}

	return -1;
}

/********
Function：设置扰码功能
********/

void BC95_SetScramb() {
	BC95_Send(AT_BC95_SCRAMB_1);
        debug_printf("%s\n\n",AT_BC95_SCRAMB_1);
	System_Delayms(BC95_DELAY_MS);

	BC95_Send(AT_BC95_SCRAMB_2);
        debug_printf("%s\n\n",AT_BC95_SCRAMB_2);
	System_Delayms(BC95_DELAY_MS);
}

/********
Function：设置自动模式
********/
void BC95_SetAutoConfig() {
	BC95_Send(AT_BC95_AUTOCONFIG);
        debug_printf("%s\n\n",AT_BC95_AUTOCONFIG);
	System_Delayms(BC95_DELAY_MS);
}

/********
Function：询问SIM卡
********/
BC95State BC95_QuerySimState() {

        char * _data = NULL;

	BC95_Send(AT_BC95_SIM);
        debug_printf("%s\n\n",AT_BC95_SIM);

        _data = BC95_Receive(BC95_REPEAT_TIMES);
        if (strstr(_data, "OK")) {
                return BC95StateOK;
        }

	return BC95ErrorSIM;
}


/********
Function：询问是否是全功能模式
********/

BC95State BC95_QueryFunState() {

        char * _data = NULL;

	BC95_Send(AT_BC95_CFUN);
        debug_printf("%s\n\n",AT_BC95_CFUN);

        _data = BC95_Receive(BC95_REPEAT_TIMES);
        if (strstr(_data, "+CFUN:1")) {
                g_cfunstate = 1;
        }
        if ((strstr(_data, "OK")) && (g_cfunstate == 1)) {
                return BC95StateOK;
        }


	return BC95ErrorCFUN;
}

/********
Function：设置成全功能模式
********/
void BC95_SetCFun() {
	BC95_Send(AT_BC95_SFUN);
        debug_printf("%s\n\n",AT_BC95_SFUN);
	System_Delayms(BC95_DELAY_MS);
}

/********
Function：查询信号
********/
BC95State BC95_QueryCSQState() {

        char * _data = NULL;

	BC95_Send(AT_BC95_CSQ);
        debug_printf("%s\n\n",AT_BC95_CSQ);

        _data = BC95_Receive(BC95_REPEAT_TIMES);
        if (strstr(_data, "+CSQ:99,99")) {
                return BC95ErrorCSQ;
        }
        if (strstr(_data, "+CSQ:")) {
                return BC95StateOK;
        }


	return BC95ErrorCSQ;
}

BC95State BC95_QueryCGATTState() {

        char * _data = NULL;

	BC95_Send(AT_BC95_CGATT);
        debug_printf("%s\n\n",AT_BC95_CGATT);

        _data = BC95_Receive(BC95_REPEAT_TIMES);
        if (strstr(_data, "+CGATT:1")) {
                return BC95StateOK;
        }
        if (strstr(_data, "+CGATT:0")) {
                return BC95ErrorCFUN;
        }

	return BC95ErrorCFUN;
}

//查询是否立即向串口显示接收到的消息
BC95State BC95_SetNNMI() {

         char * _data = NULL;

	BC95_Send(AT_BC95_NNMI);
        debug_printf("%s\n\n",AT_BC95_NNMI);

        _data = BC95_Receive(BC95_REPEAT_TIMES);
        if (strstr(_data, "OK"))  //设置成功
        {
                return BC95StateOK;
        }
        else {
                return BC95ErrorNNMI;
        }

	return BC95ErrorNNMI;
}

//查询是否是低功耗模式
BC95State BC95_QueryIsPSMS() {

        char * _data = NULL;

	BC95_Send(AT_BC95_CPSMS);
        debug_printf("%s\n\n",AT_BC95_CPSMS);

        _data = BC95_Receive(BC95_REPEAT_TIMES);
        if (strstr(_data, "+CPSMS:0"))  //不是睡眠模式
        {
                return BC95StateOK;
        }
        if (strstr(_data, "+CGATT:1"))  //睡眠模式
        {
                return BC95ErrorCPSMS;
        }

	return BC95ErrorCPSMS;
}

/********
Function：关闭低功耗模式，串口才能显示接收到的下行数据
********/
void BC95_SetCpsms() {
	BC95_Send(AT_BC95_SPSMS);
        debug_printf("%s\n\n",AT_BC95_SPSMS);
	System_Delayms(BC95_DELAY_MS);
}

//关闭回显
BC95State BC95_CloseATE() {

        char * _data = NULL;
        
	BC95_Send(AT_BC95_ATE0);
        debug_printf("%s\n\n",AT_BC95_ATE0);

        _data = BC95_Receive(BC95_REPEAT_TIMES); 
        if (strstr(_data, "OK"))  //设置成功
        {
                return BC95StateOK;
        }
        else {
                return BC95ErrorATE;
        }

	return BC95ErrorNNMI;
}

BC95State BC95_SetSendMsgIndications(void) {

        char * _data = NULL;

	BC95_Send(AT_BC95_NSMI);
        debug_printf("%s\n\n",AT_BC95_NSMI);

        _data = BC95_Receive(BC95_REPEAT_TIMES);
        if (strstr(_data, "OK"))  //设置成功
        {
                return BC95StateOK;
        }
        else {
                return BC95ErrorNSMI;
        }

	return BC95ErrorNNMI;
}

BC95State BC95_ConfigProcess()  //如果查询失败，需要处理
{
	BC95State state      = BC95StateOK;
	int       retrytimes = 0;

	BC95_QueryState();

	if (g_scrambState == 0)  //扰码功能没有开启，开启扰码功能
	{
		BC95_SetScramb();
	}

	if (g_autoconfigState == 0)  //自动模式没有开启，开启自动模式
	{
		BC95_SetAutoConfig();
	}

	//查询是否检测到SIM卡
	while (retrytimes < BC95_REPEAT_TIMES) {
		state = BC95_QuerySimState();
		if (state == BC95StateOK) {
			break;
		}

		retrytimes++;
		System_Delayms(50);
	}
	if (retrytimes == BC95_REPEAT_TIMES) {
		return state;
	}

	//查询是否全功能模式
	retrytimes = 0;
	while (retrytimes < BC95_REPEAT_TIMES) {
		state = BC95_QueryFunState();
		if (state == BC95StateOK) {
			break;
		}
		else {
			BC95_SetCFun();
		}

		retrytimes++;
		System_Delayms(50);
	}
	if (retrytimes == BC95_REPEAT_TIMES) {
		return state;
	}

	//查询信号
	retrytimes = 0;
	while (retrytimes < BC95_REPEAT_TIMES) {
		state = BC95_QueryCSQState();
		if (state == BC95StateOK) {
			break;
		}

		retrytimes++;
		System_Delayms(50);
	}
	if (retrytimes == BC95_REPEAT_TIMES) {
		return state;
	}

	//查询附着网络状况
	retrytimes = 0;
	while (retrytimes < BC95_REPEAT_TIMES * 2) {
		state = BC95_QueryCGATTState();
		if (state == BC95StateOK) {
			break;
		}

		retrytimes++;
		System_Delayms(1500);
	}
	if (retrytimes == BC95_REPEAT_TIMES * 2)  //询问CGATT State失败
	{
		return state;
	}

	//设置立即向串口打印数据的功能
	retrytimes = 0;
	while (retrytimes < BC95_REPEAT_TIMES) {
		state = BC95_SetNNMI();
		if (state == BC95StateOK) {
			break;
		}
		else {
			retrytimes++;
			System_Delayms(50);
		}
	}
	if (retrytimes == BC95_REPEAT_TIMES) {
		return state;
	}

	//是否是睡眠模式
	retrytimes = 0;
	while (retrytimes < BC95_REPEAT_TIMES) {
		state = BC95_QueryIsPSMS();
		if (state == BC95StateOK) {
			break;
		}
		else {
			BC95_SetCpsms();
		}

		retrytimes++;
		System_Delayms(50);
	}
	if (retrytimes == BC95_REPEAT_TIMES) {
		return state;
	}

	//关闭回显
	retrytimes = 0;
	while (retrytimes < BC95_REPEAT_TIMES) {
		state = BC95_CloseATE();
		if (state == BC95StateOK) {
			break;
		}

		retrytimes++;
		System_Delayms(50);
	}
	if (retrytimes == BC95_REPEAT_TIMES) {
		return state;
	}

	//设置设备对接的IoT平台的IP地址和端口号
	retrytimes = 0;
	while (retrytimes < BC95_REPEAT_TIMES)
	{
		state = BC95_ConnectToIotCloud(Cloud_IP, Cloud_IP_Port);
		if (state == BC95StateOK)
		{
			break;
		}

		retrytimes++;
		System_Delayms(50);
	}
	if (retrytimes >= BC95_REPEAT_TIMES) {
		return state;
	}

	return state;
}

BC95State BC95_ConnectToIotCloud(char* serverIp, char* serverPort) {

        char * _data = NULL;
	char* at_head			= "AT+NCDP=";
	char* comma			= ",";
	

	char head_ip_comma_port_end[40];
	memset(head_ip_comma_port_end, 0, 40);

	strcat(head_ip_comma_port_end, at_head);
	strcat(head_ip_comma_port_end, serverIp);
	strcat(head_ip_comma_port_end, comma);
	strcat(head_ip_comma_port_end, serverPort);

	BC95_Send(head_ip_comma_port_end);

        _data = BC95_Receive(BC95_REPEAT_TIMES);
        if (strstr(_data, "OK"))
        {
                printf("BC95 Connect to Cloud Success \n\n");
                return BC95StateOK;
        }
        if (strstr(_data, "+CGATT:0"))
        {
                printf("BC95 Connect to Cloud fail !! \n\n");
                return BC95ErrorNCDP;
        }


	return BC95ErrorNCDP;
}

// BC95发送数据
/*
 *BC95模块先把数据上传到云平台（可能是华为云或者电信云等），再由云平台转到我们自己的服务器上
 *这里是上传可变长度的数据，因此需要根据云平台的编解码插件来增加一些头，这里在发送的数据中需要加一个4个字节的16位无符号整型作为上传数据包长度
 */
BC95State BC95_SendSocketData(char* send_data, int send_data_len) {
	int i	  = 0;
	int j	  = 0;
	int data_len   = 0;

        char * _data = NULL;

        char * up_data_len_str = (char *)os_malloc(BC95_SOCKET_UP_DATA_LENGTH_LINK * sizeof(char));
        char * send_data_ascii_mode = (char *)os_malloc(BC95_SOCKET_DATA_LEN * sizeof(char));
        char * socket_data = (char *)os_malloc(BC95_SOCKET_DATA_LEN * sizeof(char));

        memset(up_data_len_str,0,BC95_SOCKET_UP_DATA_LENGTH_LINK);
        memset(send_data_ascii_mode,0,BC95_SOCKET_DATA_LEN);
        memset(socket_data,0,BC95_SOCKET_DATA_LEN);

	//将上传数据的长度转化为字符串形式
	BC95_UintToStr4(send_data_len, up_data_len_str);

	//将上传数据转化为ASCII字符串形式
	ASCII_to_AsciiStr(send_data, send_data_len, send_data_ascii_mode);  //转成ASCII字符串

	//组包
	data_len = send_data_len + BC95_SOCKET_UP_DATA_LENGTH_LINK / 2;
	sprintf(socket_data, "AT+NMGS=%d,", data_len);  //加入逗号之前的数据，包括逗号

	//加入数据关联长度的字符串
	// data_len = send_data_len + BC95_SOCKET_UP_DATA_LENGTH_LINK+1;//加1跳过逗号
	for (i = Utility_Strlen(socket_data), j = 0; j < BC95_SOCKET_UP_DATA_LENGTH_LINK; j++) {
		socket_data[ i ] = up_data_len_str[ j ];
		i++;
	}

	data_len = Utility_Strlen(socket_data) + Utility_Strlen(send_data_ascii_mode);
	//加入上行数据ASCII字符串
	for (i = Utility_Strlen(socket_data), j = 0; i < data_len; i++) {
		socket_data[ i ] = send_data_ascii_mode[ j ];
		j++;
	}

	//发送
	BC95_Send(socket_data);

        printf("BC95 send data:\n\n");
        printf("%s\n\n",socket_data);

        os_free(up_data_len_str);
        up_data_len_str = NULL;
        os_free(send_data_ascii_mode);
        send_data_ascii_mode = NULL;
        os_free(socket_data);
        socket_data = NULL;

        _data = BC95_Receive(BC95_REPEAT_TIMES);
        if (strstr(_data, "OK"))  //发送数据成功
        {
                return BC95StateOK;
        }
        err_printf("BC95 send data falied!\n\n");


	return BC95ErrorSEND;
}

/* when needCloudConfirm=1, need check the send indication from iot */
BC95State BC95_SendDataToCloud(char* send_data, int send_data_len, int needCloudConfirm) {
	BC95State ret;
	char*     bc95Response;

	ret = BC95_SendSocketData(send_data, send_data_len);
	if (ret != BC95StateOK)
		return BC95ErrorSEND;

	if (needCloudConfirm) {
		bc95Response = BC95_Receive(BC95_REPEAT_TIMES);
		if (bc95Response && strstr(bc95Response, "+NNMI"))
			return BC95StateOK;
		else {
			return BC95ErrorSEND;
		}
	}

	return BC95StateOK;
}

void BC95_RecvSocketData(char* recv_data, int* recv_data_len) {
	int   _repeat  = 0;
	int   _dataLen = 0;
	char* _data    = recv_data;

	while (_repeat < BC95_WAITTIME) {
		while (UART0_RecvLineTry(_data, UART1_MAXBUFFLEN, &_dataLen) == 0)  //有数据
		{
                        printf("BC95 recv data:\n\n");
                        printf("%s\n\n",_data);

			if (strstr(_data, "+NNMI:"))  //判断是否为接收到的数据
			{
				BC95_AnalysisRecvSocketData(_data, _dataLen, recv_data,
							    recv_data_len);

				return;
			}
		}
		_repeat++;
		System_Delayms(100);
	}
	if (_repeat == BC95_WAITTIME) {
                err_printf("not recv data!\n\n");
		return;
	}
}

void BC95_RecvDataFromCloud(char* recv_data, int* recv_data_len) {
	BC95_RecvSocketData(recv_data, recv_data_len);
	recv_data++;
	recv_data++;
	recv_data++;
	if (*recv_data_len > 0)
	// 下行报文经过云平台的时候总会多出三个字节的报头(云平台自动添加的)，应该和云平台的插件部署有关
	{
		*recv_data_len = *recv_data_len - 3;
	}
}


//解析接收的数据
void BC95_AnalysisRecvSocketData(char* revc_data, int revc_data_len, char* down_Stream_data,
				 int* down_Stream_data_len) {
	int  i						    = 0;
	int  comma_count				    = 0;  //逗号计数
	int  down_Stream_data_ascii_len			    = 0;
	char down_Stream_data_ascii[ BC95_SOCKET_DATA_LEN ] = { 0 };

	while (i < revc_data_len) {
		if (revc_data[ i ] == ',') {
			comma_count++;
			if (comma_count == 1)  //返回的数据中有1个逗号，逗号后面的数据为下行数据
			{
				i++;
				break;
			}
		}
		i++;
	}

	while (revc_data[ i ] != '\0')  //获取数据
	{
		down_Stream_data_ascii[ down_Stream_data_ascii_len ] = revc_data[ i ];
		down_Stream_data_ascii_len++;
		i++;
	}

	AsciiStr_to_ASCII(down_Stream_data_ascii, down_Stream_data_ascii_len, down_Stream_data);
	*down_Stream_data_len = down_Stream_data_ascii_len / 2;
}



/* ret: NULL(fail)  otherwise(success) */
char* BC95_Receive(int waitingtime) {
        int  _dataLen = 0;
	int  _repeat = 0;
	char _data[ UART1_MAXBUFFLEN ];

	/* wait till uart0 buffered filled and return the buffer */
        while(_repeat < waitingtime){
                while (UART0_RecvLineWait(_data + _dataLen, UART1_MAXBUFFLEN, &_dataLen) != -1)
		        ;
                if(_dataLen != 0){
                        break;
                }
                _repeat++;
                System_Delayms(50);
        }

        if(_dataLen != 0){
                debug_printf("%s\n\n",_data);
                return _data;
        }

	/* wait time out , return NULL */
	return NULL;
}

//个人测试函数用，嘻嘻嘻
void BC95_AtTest(void) {
	while (1) {
		BC95_Send("AT+CEREG?");
		char* bc95Response = BC95_Receive(BC95_REPEAT_TIMES);
		printf("%s \n\n", bc95Response);
	}
}



communication_module_t bc95_module = {
	.name	  = "bc95",
	.power_on      = bc95_open,
	.power_off     = bc95_close,
	.sleep	 = bc95_sleep,
	.wake_up       = bc95_wake_up,
	.send_msg      = bc95_send,
	.rcv_msg       = bc95_receive,
	.get_real_time = bc95_AT_gettime,
	.check_if_module_is_normal = check_bc95_module_is_normal,
};

static int bc95_open()
{
	printf("bc95 module is booting, waiting for 5s ...\n\n");
        if(BC95_Open() != 0){
                err_printf("BC95 Open failed!\n\n");
                return ERROR;
        }

        return check_bc95_module_is_normal();
}

static int bc95_close()
{
        BC95_Close();
        return OK;
}

static int bc95_sleep()
{
        return OK;
}

static int bc95_wake_up()
{
        return OK;
}


static int bc95_send(char* pSend, int sendDataLen, int isLastPacket, int center)
{

        if(BC95_ConfigProcess() != BC95StateOK){
                return ERROR;
        }

        while(BC95_query_cdp_server_setting() != BC95StateOK){
                BC95_ConnectToIotCloud(Cloud_IP, Cloud_IP_Port);
        }

        if(BC95_SendDataToCloud(pSend,sendDataLen,0) == BC95StateOK){
                return OK;     
        }else{
                return ERROR;
        }
        
}

static char* bc95_receive()
{
        char * recv_data = NULL;
        int recv_data_len = 0;
        BC95_RecvDataFromCloud(recv_data, &recv_data_len);
        return recv_data;
}

static rtc_time_t bc95_AT_gettime()
{
        rtc_time_t bc95_time;
        bc95_time.year = 0;
        BC95_QueryTime(&bc95_time.year, &bc95_time.month, &bc95_time.date, &bc95_time.hour,
                        &bc95_time.min, &bc95_time.sec);
        return bc95_time;
}

static int check_bc95_module_is_normal()
{
        char* at_rsp;

	BC95_Send("AT+CGMI=?");
	at_rsp = BC95_Receive(BC95_REPEAT_TIMES);

	if (strstr(at_rsp, "OK") != NULL) {
		return OK;
	}
	else {
		return ERROR;
	}
}

BC95State BC95_query_cdp_server_setting(void)
{
	int _repeat = 0;
	int _dataLen = 0;
	// char _data[UART1_MAXBUFFLEN] = {0};
        char * _data = NULL;
	char *at_cmd = "AT+NCDP?";


	BC95_Send(at_cmd);
        debug_printf("%s\n\n",at_cmd);

        _data = BC95_Receive(BC95_REPEAT_TIMES);
        if(strstr(_data,Cloud_IP)){
                return BC95StateOK;
        }
	
	return -1;
}

int bc95_module_driver_install()
{
        return register_communication_module(&bc95_module);
}


void BC95_Unit_test() {
	int   len				= 0;
	char  msgLen				= 0xA;
	char* msgData				= "hahamama12";
	char  recv_data[ BC95_SOCKET_DATA_LEN ] = { 0 };

	bc95_open();

	while (1) {


                bc95_send(msgData,Utility_Strlen(msgData),0,0);

		Console_WriteStringln("BC95 send data:");

		Console_WriteStringln(msgData);

		// BC95_RecvSocketData(recv_data, &len);

		// Console_WriteStringln("BC95 recv  down data:");

		// Console_WriteStringln(recv_data);

		System_Delayms(5000);
	}
}
