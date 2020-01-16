#include "hydrologytask.h"
#include "Sampler.h"
#include "adc.h"
#include "common.h"
#include "main.h"
#include "math.h"
#include "memoryleakcheck.h"
#include "msp430common.h"
#include "packet.h"
#include "rtc.h"
#include "stdint.h"
#include "store.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "communication_opr.h"
#include "gprs.h"
#include "hydrologycommand.h"
#include "message.h"
#include "semphr.h"
#include "string.h"
#include "task.h"
#include "timer.h"
#include "uart1.h"
#include "uart3.h"
#include "uart_config.h"
#include <stdbool.h>

extern int		 UserElementCount;
extern int		 RS485RegisterCount;
extern int		 IsDebug;
extern int		 DataPacketLen;
extern hydrologyElement  inputPara[ MAX_ELEMENT ];
extern hydrologyElement  outputPara[ MAX_ELEMENT ];
uint16_t		 time_10min = 0, time_5min = 0, time_1min = 1, time_1s = 0;
extern SemaphoreHandle_t GPRS_Lock;

void HydrologyTimeBase() {
	time_1min++;
	time_5min++;
	time_10min++;
}

void convertSampleTimetoHydrology(char* src, char* dst) {
	dst[ 0 ] = _DECtoBCD(src[ 0 ]);
	dst[ 1 ] = _DECtoBCD(src[ 1 ]);
	dst[ 2 ] = _DECtoBCD(src[ 2 ]);
	dst[ 3 ] = _DECtoBCD(src[ 3 ]);
	dst[ 4 ] = _DECtoBCD(src[ 4 ]);
}

void convertSendTimetoHydrology(char* src, char* dst) {
	dst[ 0 ] = _DECtoBCD(src[ 0 ]);
	dst[ 1 ] = _DECtoBCD(src[ 1 ]);
	dst[ 2 ] = _DECtoBCD(src[ 2 ]);
	dst[ 3 ] = _DECtoBCD(src[ 3 ]);
	dst[ 4 ] = _DECtoBCD(src[ 4 ]);
	dst[ 5 ] = _DECtoBCD(src[ 5 ]);
}

float ConvertAnalog(int v, int range) {
	float tmp;

	// tmp = v / (4096.0) * range;
	tmp = (v - 4096.0) / (4096.0) * range;
	return tmp;
}

void ADC_Element(char* value, int index) {
	// int range[5] = {1,20,100,5000,4000};     //模拟量范围
	int   range[ 16 ] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	float floatvalue  = 0;

	// floatvalue = ConvertAnalog(A[index+1],range[index]);
	floatvalue = ConvertAnalog(A[ index ], range[ index ]); /*++++++++++++++++*/
	memcpy(value, ( char* )(&floatvalue), 4);
}

char value[ 4 ] = { 0, 0, 0, 0 };

void HydrologyUpdateElementTable() {
	int  i     = 0;
	char user  = 0;
	char rs485 = 0;

	Hydrology_ReadStoreInfo(HYDROLOGY_USER_ElEMENT_COUNT, &user,
				HYDROLOGY_USER_ElEMENT_COUNT_LEN);
	Hydrology_ReadStoreInfo(HYDROLOGY_RS485_REGISTER_COUNT, &rs485,
				HYDROLOGY_RS485_REGISTER_COUNT_LEN);
	UserElementCount   = ( int )user;
	RS485RegisterCount = ( int )rs485;
	// TraceInt4(UserElementCount, 1);
        printf("total element num:%d\n\n",UserElementCount);
	// TraceInt4(RS485RegisterCount, 1);
        printf("rs485 element num:%d\n\n",RS485RegisterCount);
	for (i = 0; i < UserElementCount; i++) {

		Hydrology_ReadStoreInfo(HYDROLOGY_ELEMENT1_ID + i * HYDROLOGY_ELEMENT_ID_LEN,
					&Element_table[ i ].ID, HYDROLOGY_ELEMENT_ID_LEN);
		Hydrology_ReadStoreInfo(HYDROLOGY_ELEMENT1_TYPE + i * HYDROLOGY_ELEMENT_TYPE_LEN,
					&Element_table[ i ].type, HYDROLOGY_ELEMENT_TYPE_LEN);
		Hydrology_ReadStoreInfo(HYDROLOGY_ELEMENT1_MODE + i * HYDROLOGY_ELEMENT_MODE_LEN,
					&Element_table[ i ].Mode, HYDROLOGY_ELEMENT_MODE_LEN);
		Hydrology_ReadStoreInfo(HYDROLOGY_ELEMENT1_CHANNEL
						+ i * HYDROLOGY_ELEMENT_CHANNEL_LEN,
					&Element_table[ i ].Channel, HYDROLOGY_ELEMENT_CHANNEL_LEN);
		// Hydrology_ReadStoreInfo(HYDROLOGY_SWITCH1,temp_value,HYDROLOGY_SWITCH_LEN);
		getElementDd(
			Element_table[ i ].ID, &Element_table[ i ].D,
			&Element_table[ i ].d);  // D,d存了取还是直接取，可以先测直接取的，看可行否
	}
	Element_table[ i ].ID      = NULL;
	Element_table[ i ].type    = NULL;
	Element_table[ i ].D       = NULL;
	Element_table[ i ].d       = NULL;
	Element_table[ i ].Mode    = NULL;
	Element_table[ i ].Channel = NULL;
}
char s_isr_count_flag = 0;
void HydrologyDataPacketInit() {
	char packet_len = 0;
	int  i		= 0;
	packet_len += HYDROLOGY_DATA_SEND_FLAG_LEN;
	packet_len += HYDROLOGY_DATA_TIME_LEN;
	while (Element_table[ i ].ID != 0) {
		os_mallocElement(Element_table[ i ].ID, Element_table[ i ].D,
				      Element_table[ i ].d, &inputPara[ i ]);
		packet_len += inputPara[ i ].num;
		i++;
	}
	DataPacketLen = packet_len;
	Hydrology_WriteStoreInfo(HYDROLOGY_DATA_PACKET_LEN, &packet_len,
				 HYDROLOGY_DATA_PACKET_LEN_LEN);
	Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT_FLAG, &s_isr_count_flag,
				HYDROLOGY_ISR_COUNT_FLAG_LEN);
	/*初始化的时候更新下时间*/
	lock_communication_dev();
	communication_module_t* comm_dev = get_communication_dev();
	rtc_time_t		rtc_time = comm_dev->get_real_time();
	unlock_communication_dev();
	System_Delayms(50);
	if (rtc_time.year == 0) {
		printf("update rtc through gprs module failed \n\n");
		return;
	}
	printf("update rtc time, %d/%d/%d %d:%d:%d \n\n", rtc_time.year, rtc_time.month,
	       rtc_time.date, rtc_time.hour, rtc_time.min, rtc_time.sec);
	_RTC_SetTime(( char )rtc_time.sec, ( char )rtc_time.min, ( char )rtc_time.hour,
		     ( char )rtc_time.date, ( char )rtc_time.month, 1, ( char )rtc_time.year, 0);
}

//////////////sample//////////

static int get_sample_interval_form_flash()
{
        char	 sampleinterval[ 2 ];

        Hydrology_ReadStoreInfo(HYDROLOGY_SAMPLE_INTERVAL, sampleinterval,
				HYDROLOGY_SAMPLE_INTERVAL_LEN);  //??????????
	sampleinterval[ 0 ] = _BCDtoDEC(sampleinterval[ 0 ]);
	sampleinterval[ 1 ] = _BCDtoDEC(sampleinterval[ 1 ]);
	return sampleinterval[ 1 ] + sampleinterval[ 0 ] * 100; 
}

static void take_adc_sample_only_once()
{
        int j = 0;

        while (Element_table[ j ].ID != 0) {
		if (Element_table[ j ].Mode == ADC) {
			ADC_Sample();
			break;
		}
		j++;
	}
}

static struct{
        int adc_i;
        int isr_i;
        long isr_count;
        char isr_count_temp[ 5 ];
        char io_i;
        volatile int rs485_i;
}sample_mode_num;

static struct{
        int analog_num;
        int pulse_num;
        int switch_num;
}sample_type_num;

static void select_mode_to_sample(int i)
{

        switch (Element_table[ i ].Mode) {
		case ADC: {
			sample_mode_num.adc_i = ( int )Element_table[ i ].Channel;
			ADC_Element(value, sample_mode_num.adc_i);
			break;
		}
		case ISR_COUNT: {
			sample_mode_num.isr_i = ( int )Element_table[ i ].Channel;
			Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT1 
                                                + (sample_mode_num.isr_i - 1) 
                                                * HYDROLOGY_ISR_COUNT_LEN,
						sample_mode_num.isr_count_temp,
                                                HYDROLOGY_ISR_COUNT_LEN);
			sample_mode_num.isr_count = 
                                        (sample_mode_num.isr_count_temp[ 4 ] * 0x100000000)
				        + (sample_mode_num.isr_count_temp[ 3 ] * 0x1000000)
				        + (sample_mode_num.isr_count_temp[ 2 ] * 0x10000)
				        + (sample_mode_num.isr_count_temp[ 1 ] * 0x100) 
                                        + (sample_mode_num.isr_count_temp[ 0 ]);
			memcpy(value, ( char* )(&sample_mode_num.isr_count), 4);
			break;
		}
		case IO_STATUS: {
			sample_mode_num.io_i = ( int )Element_table[ i ].Channel;
			Hydrology_ReadIO_STATUS(value, sample_mode_num.io_i);
			break;
		}
		case RS485: {
			Hydrology_ReadRS485(value, sample_mode_num.rs485_i);
			sample_mode_num.rs485_i++;
			break;
		}
	}
}

static void select_type_to_save_sample_data_temp(int i)
{
        char	 _temp_sampertime[ 6 ] = { 0 };
 
        switch (Element_table[ i ].type) {
		case ANALOG: {
			convertSampleTimetoHydrology(g_rtc_nowTime, _temp_sampertime);
			Hydrology_SetObservationTime(Element_table[ i ].ID, _temp_sampertime, i);
			Hydrology_WriteStoreInfo(HYDROLOGY_ANALOG1 + sample_type_num.analog_num * HYDROLOGY_ANALOG_LEN,
						 value, HYDROLOGY_ANALOG_LEN);
			sample_type_num.analog_num++;
			break;
		}
		case PULSE: {
			convertSampleTimetoHydrology(g_rtc_nowTime, _temp_sampertime);
			Hydrology_SetObservationTime(Element_table[ i ].ID, _temp_sampertime, i);
			Hydrology_WriteStoreInfo(HYDROLOGY_PULSE1 + sample_type_num.pulse_num * HYDROLOGY_PULSE_LEN, value,
						 HYDROLOGY_PULSE_LEN);
			sample_type_num.pulse_num++;
			break;
		}
		case SWITCH: {
			convertSampleTimetoHydrology(g_rtc_nowTime, _temp_sampertime);
			Hydrology_SetObservationTime(Element_table[ i ].ID, _temp_sampertime, i);
			Hydrology_WriteStoreInfo(HYDROLOGY_SWITCH1 + sample_type_num.switch_num * HYDROLOGY_SWITCH_LEN,
						 value, HYDROLOGY_SWITCH_LEN);
			sample_type_num.switch_num++;
			break;
		}
	}
}

static void sample_count_init()
{
        sample_mode_num.adc_i = 0;
        sample_mode_num.io_i = 0;
        sample_mode_num.isr_count = 0;
        memset(sample_mode_num.isr_count_temp,0,5);
        sample_mode_num.isr_i = 0;
        sample_mode_num.rs485_i = 0;

        sample_type_num.analog_num = 0;
        sample_type_num.pulse_num = 0;
        sample_type_num.switch_num = 0;

}

static void take_sample_and_save_sample_data_temporary()
{
        int i = 0;

        take_adc_sample_only_once();
	while (Element_table[ i ].ID != 0) {
		memset(value, 0, sizeof(value));
                select_mode_to_sample(i);
                select_type_to_save_sample_data_temp(i);
		i++;
	}
}

int HydrologySample() {

        sample_count_init();

        if(now_time_reach_interval( (get_sample_interval_form_flash() / 60) )){
                printf("Not Sample Time! now time is: %d/%d/%d  %d:%d:%d \n\n", rtc_nowTime[ 0 ],
		       rtc_nowTime[ 1 ], rtc_nowTime[ 2 ], rtc_nowTime[ 3 ], rtc_nowTime[ 4 ],
		       rtc_nowTime[ 5 ]);
		return -1;
        }

	printf("\n\n Start Sample:   \n\n");
	UART1_Open_9600(UART1_U485_TYPE);

        take_sample_and_save_sample_data_temporary();

	UART3_Open(UART3_CONSOLE_TYPE);
	UART1_Open(UART1_BT_TYPE);
	printf("\n\n Sample Done!   \n\n");

	return 0;
}


static uint8_t get_store_interval(void) {
	char storeinterval;
	Hydrology_ReadStoreInfo(HYDROLOGY_WATERLEVEL_STORE_INTERVAL, &storeinterval,
				HYDROLOGY_WATERLEVEL_STORE_INTERVAL_LEN);  // ly
	storeinterval = _BCDtoDEC(storeinterval);

	return ( uint8_t )storeinterval;
}

static int arrived_store_time() {

	uint8_t store_interval = get_store_interval();
	if (now_time_reach_interval(store_interval) == TRUE) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

static void fetch_element_value_from_buffer_in_rom() {
	int   i = 0, acount = 0, pocunt = 0;
	float floatvalue	= 0;
	long  intvalue1		= 0;
	int   intvalue2		= 0;
	int   type		= 0;
	int   cnt		= 0;
	char  switch_value[ 4 ] = { 0 };

	while (Element_table[ i ].ID != 0) {
		switch (Element_table[ i ].type) {
		case ANALOG: {
			Hydrology_ReadAnalog(&floatvalue, acount++);
			os_mallocElement(Element_table[ i ].ID, Element_table[ i ].D,
					      Element_table[ i ].d,
					      &inputPara[ i ]);  //获得id ，num，开辟value的空间
			converToHexElement(( double )floatvalue, Element_table[ i ].D,
					   Element_table[ i ].d, inputPara[ i ].value);
			break;
		}
		case PULSE: {
			Hydrology_ReadPulse(&intvalue1, pocunt++);
			os_mallocElement(Element_table[ i ].ID, Element_table[ i ].D,
					      Element_table[ i ].d, &inputPara[ i ]);
			converToHexElement(( double )intvalue1, Element_table[ i ].D,
					   Element_table[ i ].d, inputPara[ i ].value);
			break;
		}
		case SWITCH: {
			Hydrology_ReadSwitch(switch_value);
			os_mallocElement(Element_table[ i ].ID, Element_table[ i ].D,
					      Element_table[ i ].d, &inputPara[ i ]);
			inputPara[ i ].value[ 0 ] = switch_value[ 3 ];
			inputPara[ i ].value[ 1 ] = switch_value[ 2 ];
			inputPara[ i ].value[ 2 ] = switch_value[ 1 ];
			inputPara[ i ].value[ 3 ] = switch_value[ 0 ];

			break;
		}
		case STORE: {
			inputPara[ i ].guide[ 0 ] = Element_table[ i ].ID;
			inputPara[ i ].guide[ 1 ] = Element_table[ i ].ID;
			inputPara[ i ].num	= SinglePacketSize;
			break;
		}
		default:
			break;
		}
		i++;
	}
}

static void get_elements_sample_time(char sample_time[ 5 ]) {
	Hydrology_ReadObservationTime(Element_table[ 0 ].ID, sample_time, 0);
}

static int get_element_count(void) {
	int i = 0;
	while (Element_table[ i ].ID != 0) {
		i++;
	}

	return i;
}
static void put_elements_value_into_package(char package[]) {

	char observationtime[ 5 ];
	int  len     = 0;
	package[ 0 ] = 0x00;  // 未发送标记 记为0x00

	get_elements_sample_time(observationtime);
	memcpy(&package[ 1 ], observationtime, HYDROLOGY_OBSERVATION_TIME_LEN);
	len += 6;

	int element_count = get_element_count();

	for (int i = 0; i < element_count; i++) {
		memcpy(&package[ len ], inputPara[ i ].value, inputPara[ i ].num);
		len += inputPara[ i ].num;
	}
}

static int store_package_into_rom(char package[]) {
	if (Store_WriteDataItemAuto(package) < 0) {
		return FAILED;
	}
	else {
		return SUCCESS;
	}
}

static void update_package_count(void) {
	int _effect_count = 0;

	Hydrology_ReadDataPacketCount(&_effect_count);  //初始读取内存剩余未发送数据包数量
	++_effect_count;				//存一条就加1
	Hydrology_SetDataPacketCount(_effect_count);
}

int HydrologySaveData(char funcode)  // char *_saveTime
{
	if (arrived_store_time() == FALSE) {
		printf("not store time, %d/%d/%d %d:%d:%d\n\n", rtc_nowTime[ 0 ], rtc_nowTime[ 1 ],
		       rtc_nowTime[ 2 ], rtc_nowTime[ 3 ], rtc_nowTime[ 4 ], rtc_nowTime[ 5 ]);
		return FAILED;
	}

	char package[ HYDROLOGY_DATA_ITEM_LEN ] = { 0 };  //数据条为130个字节

	printf("start store element's value into rom \n\n");

	fetch_element_value_from_buffer_in_rom();

	put_elements_value_into_package(package);

	if (store_package_into_rom(package) != SUCCESS) {
		err_printf("store elements value into rom failed ! \n\n");
		return FAILED;
	}

	update_package_count();

	printf("store %d elements' value into rom success \n\n", get_element_count());

	return 0;
}

static int arrived_report_time(char now_time[ 6 ]) {
	if (Utility_Is_A_ReportTime(now_time) == 0) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}

static int get_packet_index(int* packet_to_send_start_index, int* packet_to_send_end_index) {
	if (FlowCheckSampleData(packet_to_send_start_index, packet_to_send_end_index) != 0) {
		return FAILED;
	}
	else {
		return SUCCESS;
	}
}

static int report_packet(int count_of_packet_to_send, int packet_to_send_start_index,
			 int packet_to_send_end_index) {

	int  _seek_num    = 0;  //防止死循环
	char _send[ 200 ] = { 0 };
	int  _ret;

	while (count_of_packet_to_send != 0) {
		// TraceMsg("read data in :", 0);
		// TraceInt4(packet_to_send_start_index, 1);
		// TraceInt4(count_of_packet_to_send, 1);
		if (_seek_num > HYDROLOGY_DATA_MAX_IDX)
		//寻找的数据条数已经超过最大值就退出，防止死循环
		{
			err_printf("seek num out of range, panic occured, rebooting... \n\n");
			System_Delayms(2000);
			System_Reset();
		}

		_ret = Store_ReadDataItem(packet_to_send_start_index, _send,
					  0);  //读取数据，ret为读出的数据长度

		if (_ret < 0) {
			err_printf("can't read packet from rom ! \n\n");
			return FAILED;  //无法读取数据 就直接退了
		}
		else if (_ret == 1) {
			debug_printf("It's a invalid packet, which has been report early \n\n");
			if (packet_to_send_start_index
			    >= HYDROLOGY_DATA_MAX_IDX) {  //如果读取的startidx超过可存的最大index，则重新置零
				packet_to_send_start_index = HYDROLOGY_DATA_MIN_IDX;
			}
			else {
				++packet_to_send_start_index;
			}  //下一数据
			++_seek_num;
			Hydrology_SetStartIdx(
				packet_to_send_start_index);  //要更新packet_to_send_start_index.
			debug_printf("packet_to_send_start_index: %d \n\n",
				     packet_to_send_start_index);
			debug_printf("packet_to_send_end_index: %d \n\n", packet_to_send_end_index);
			// hydrologyExitSend();
		}
		else  //未发送的数据
		{
			lock_communication_dev();
			hydrologyProcessSend(_send, TimerReport);
			unlock_communication_dev();

			Store_MarkDataItemSended(packet_to_send_start_index);  //设置该数据已发送
			--count_of_packet_to_send;
			Hydrology_SetDataPacketCount(
				count_of_packet_to_send);  //发送完后要更新有效数据cnt
			if (packet_to_send_start_index >= HYDROLOGY_DATA_MAX_IDX) {
				packet_to_send_start_index = HYDROLOGY_DATA_MIN_IDX;
			}
			else {
				++packet_to_send_start_index;  //下一数据
			}
			++_seek_num;

			Hydrology_SetStartIdx(
				packet_to_send_start_index);  //更新packet_to_send_start_index.
		}
	}

	return SUCCESS;
}

int hydrologyReport(char now_time[ 6 ]) {
	if (arrived_report_time(now_time) == FALSE) {
		printf("not report time, %d/%d/%d %d:%d:%d\n\n", now_time[ 0 ], now_time[ 1 ],
		       now_time[ 2 ], now_time[ 3 ], now_time[ 4 ], now_time[ 5 ]);
		return FAILED;
	}

	printf("start report \n\n");

	int packet_to_send_start_index = 0, packet_to_send_end_index = 0;
	int count_of_packet_to_send = 0;

	if (get_packet_index(&packet_to_send_start_index, &packet_to_send_end_index) == FAILED) {
		return FAILED;
	}

	Hydrology_ReadDataPacketCount(&count_of_packet_to_send);
	printf("%d valid packet to report \n\n", count_of_packet_to_send);

	if (report_packet(count_of_packet_to_send, packet_to_send_start_index,
			  packet_to_send_end_index)
	    == FAILED) {
		err_printf("error happend when reporting packet \n\n");
		return FAILED;
	}

	printf("packet report done \n\n");

	return SUCCESS;
}

int HydrologyInstantWaterLevel(char* _saveTime)  //检查发送时间，判断上下标，组报文发送
{

	static char endtime[ 6 ] = { 0, 0, 0, 0, 0, 0 };
	Utility_Strncpy(endtime, _saveTime, 6);
	int ret = 0;
	ret     = Utility_Is_A_ReportTime(endtime);  //用于判断是否到发送时间

	if (!ret) {
		printf("Not Send Time, %d/%d/%d  %d:%d:%d \r\n", endtime[ 0 ], endtime[ 1 ],
		       endtime[ 2 ], endtime[ 3 ], endtime[ 4 ], endtime[ 5 ]);
		return -1;
	}

	int _effect_count = 0;  //存储在flash的有效未发送的数据包
	Hydrology_ReadDataPacketCount(&_effect_count);  //读取内存里剩余未发送数据包数量
	TraceInt4(_effect_count, 1);
	int _startIdx = 0;
	int _endIdx   = 0;

	char _send[ 200 ] = { 0 };
	int  _ret	 = 0;
	int  _seek_num    = 0;  //防止死循环
	int  sendlen      = 0;

	_ret = FlowCheckSampleData(&_startIdx, &_endIdx);  //获得startidx endidx
	if (_ret != 0) {
		return -1;
	}
	while (_effect_count != 0) {
		TraceMsg("read data in :", 0);
		TraceInt4(_startIdx, 1);
		TraceInt4(_effect_count, 1);
		if (_seek_num > HYDROLOGY_DATA_MAX_IDX)
		//寻找的数据条数已经超过最大值就退出，防止死循环
		{
			TraceMsg("seek num out of range", 1);
			// hydrologHEXos_free();
			System_Delayms(2000);
			System_Reset();
		}

		_ret = Store_ReadDataItem(_startIdx, _send,
					  0);  //读取数据，ret为读出的数据长度

		if (_ret < 0) {
			TraceMsg("can't read data ! very bad .", 1);
			return -1;  //无法读取数据 就直接退了
		}
		else if (_ret == 1) {
			TraceMsg("It's sended data", 1);
			if (_startIdx
			    >= HYDROLOGY_DATA_MAX_IDX) {  //如果读取的startidx超过可存的最大index，则重新置零
				_startIdx = HYDROLOGY_DATA_MIN_IDX;
			}
			else {
				++_startIdx;
			}  //下一数据
			++_seek_num;
			Hydrology_SetStartIdx(_startIdx);  //要更新_startIdx.
			TraceInt4(_startIdx, 1);
			TraceInt4(_endIdx, 1);
			// hydrologyExitSend();
		}
		else  //未发送的数据
		{
			sendlen = _ret;

			lock_communication_dev();
			hydrologyProcessSend(_send, TimerReport);
			unlock_communication_dev();

			Store_MarkDataItemSended(_startIdx);  //设置该数据已发送
			--_effect_count;
			Hydrology_SetDataPacketCount(_effect_count);  //发送完后要更新有效数据cnt
			if (_startIdx >= HYDROLOGY_DATA_MAX_IDX) {
				_startIdx = HYDROLOGY_DATA_MIN_IDX;
			}
			else {
				++_startIdx;  //下一数据
			}
			++_seek_num;

			TraceMsg(_send, 1);
			Hydrology_SetStartIdx(_startIdx);  //更新_startIdx.
		}
	}

	TraceMsg("Report done", 1);
	System_Delayms(2000);
	/**************************************************************************************/
	// hydrologyProcessSend(TimerReport);

	if (!IsDebug) {
		lock_communication_dev();
		System_Delayms(5000);
		JudgeServerDataArrived();
		Hydrology_ProcessGPRSReceieve();
		JudgeServerDataArrived();
		Hydrology_ProcessGPRSReceieve();
		JudgeServerDataArrived();
		Hydrology_ProcessGPRSReceieve();
		unlock_communication_dev();
	}

	time_10min = 0;

	endtime[ 0 ] = 0;
	endtime[ 1 ] = 0;
	endtime[ 2 ] = 0;
	endtime[ 3 ] = 0;
	endtime[ 4 ] = 0;
	endtime[ 5 ] = 0;

	return 0;
}

int HydrologyVoltage() {
	return 0;
}

static bool check_if_rtc_time_format_correct(void) {

	for (int i = 0; i < 11; ++i) {
		RTC_ReadTimeBytes5(g_rtc_nowTime);
		if (RTC_IsBadTime(g_rtc_nowTime, 1) == 0) {
			return true;
		}
	}

	return false;
}

static void try_to_correct_rtc_time_via_network(void) {

	lock_communication_dev();
	communication_module_t* comm_dev = get_communication_dev();
	rtc_time_t		rtc_time = comm_dev->get_real_time();
	unlock_communication_dev();

	if (rtc_time.year == 0) {
		return;
	}
	printf("sync rtc time via network success : %d/%d/%d %d:%d:%d \r\n", rtc_time.year,
	       rtc_time.month, rtc_time.date, rtc_time.hour, rtc_time.min, rtc_time.sec);
	_RTC_SetTime(( char )rtc_time.sec, ( char )rtc_time.min, ( char )rtc_time.hour,
		     ( char )rtc_time.date, ( char )rtc_time.month, 1, ( char )rtc_time.year, 0);

	vTaskDelay(50 / portTICK_RATE_MS);
}

void check_rtc_time(void) {

	if (check_if_rtc_time_format_correct() == true) {
		return;
	}

	err_printf("rtc time format error, trying to correct rtc via network ... \r\n");
	try_to_correct_rtc_time_via_network();

	if (check_if_rtc_time_format_correct() == false) {
		err_printf("correct rtc failed, rebooting ... \r\n ");
		System_Reset();
	}
}

/*?????��???????????????????????????????*/
int Hydrology_TimeCheck() {
	RTC_ReadTimeBytes5(g_rtc_nowTime);  //??rtc???????g_rtc_nowtime

	int _time_error = 1;
	for (int i = 0; i < 11; ++i) {
		if (RTC_IsBadTime(g_rtc_nowTime, 1) == 0) {
			// TraceMsg("Time is OK .",1);
			_time_error = 0;
			break;  //????????
		}
		TraceMsg("Time is bad once .", 1);
		//????????,???
		RTC_ReadTimeBytes5(g_rtc_nowTime);
	}
	if (_time_error > 0) {  //??��????? ,?????????????????
		TraceMsg("Device time is bad !", 1);
		TraceMsg("Waiting for config !", 1);
		char newtime[ 6 ] = { 0 };

		lock_communication_dev();
		communication_module_t* comm_dev = get_communication_dev();

		rtc_time_t rtc_time = comm_dev->get_real_time();
		unlock_communication_dev();
		if (rtc_time.year == 0) {
			return -1;
		}
		printf("update rtc time, %d/%d/%d %d:%d:%d \r\n", rtc_time.year, rtc_time.month,
		       rtc_time.date, rtc_time.hour, rtc_time.min, rtc_time.sec);
		_RTC_SetTime(( char )rtc_time.sec, ( char )rtc_time.min, ( char )rtc_time.hour,
			     ( char )rtc_time.date, ( char )rtc_time.month, 1,
			     ( char )rtc_time.year, 0);

		System_Delayms(50);
	}
	if (RTC_IsBadTime(g_rtc_nowTime, 1) < 0) {  //????????????????,?????????????
		TraceMsg("Still bad time!", 1);
		System_Reset();
		return -2;
	}
	return 0;
}

void task_hydrology_run(void* pvParameters) {
	while (1) {
		TimerB_Clear();
		WatchDog_Clear();
		Hydrology_ProcessUARTReceieve();
		Hydrology_TimeCheck();

		if (RTC_IsBadTime(g_rtc_nowTime, 1) != 0) {
			vTaskDelay(100 / portTICK_PERIOD_MS);
			continue;
		}

		if (!IsDebug) {
			if (time_1min)
				time_1min = 0;
			else
				continue;
		}

		HydrologyTask();
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}
