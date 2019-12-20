#include "hydrologytask.h"
#include "GSM.h"
#include "Sampler.h"
#include "adc.h"
#include "common.h"
#include "main.h"
#include "math.h"
#include "msp430common.h"
#include "packet.h"
#include "rtc.h"
#include "stdint.h"
#include "store.h"
#include <stdio.h>


//#include "convertsampledata.h"
//#include "hydrology.h"
//#include "hydrologmakebody.h"
#include "FreeRTOS.h"
#include "GTM900C.h"
#include "hydrologycommand.h"
#include "message.h"
#include "string.h"
#include "task.h"
#include "timer.h"
#include "uart1.h"
#include "uart3.h"
#include "uart_config.h"

extern int		UserElementCount;
extern int		RS485RegisterCount;
extern int		IsDebug;
extern int		DataPacketLen;
extern hydrologyElement inputPara[ MAX_ELEMENT ];
extern hydrologyElement outputPara[ MAX_ELEMENT ];
uint16_t		time_10min = 0, time_5min = 0, time_1min = 1, time_1s = 0;

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
	TraceInt4(UserElementCount, 1);
	TraceInt4(RS485RegisterCount, 1);
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

void HydrologyDataPacketInit() {
	char packet_len = 0;
	int  i		= 0;
	packet_len += HYDROLOGY_DATA_SEND_FLAG_LEN;
	packet_len += HYDROLOGY_DATA_TIME_LEN;
	while (Element_table[ i ].ID != 0) {
		pvPortMallocElement(Element_table[ i ].ID, Element_table[ i ].D,
				    Element_table[ i ].d, &inputPara[ i ]);
		packet_len += inputPara[ i ].num;
		i++;
	}
	DataPacketLen = packet_len;
	Hydrology_WriteStoreInfo(HYDROLOGY_DATA_PACKET_LEN, &packet_len,
				 HYDROLOGY_DATA_PACKET_LEN_LEN);

	/*初始化的时候更新下时间*/
	char newtime[ 6 ] = { 0 };
	GSM_Open();
	GSM_AT_QueryTime(&newtime[ 0 ], &newtime[ 1 ], &newtime[ 2 ], &newtime[ 3 ], &newtime[ 4 ],
			 &newtime[ 5 ]);
	System_Delayms(50);
	TraceHexMsg(newtime, 6);
	_RTC_SetTime(newtime[ 5 ], newtime[ 4 ], newtime[ 3 ], newtime[ 2 ], newtime[ 1 ], 1,
		     newtime[ 0 ], 0);
}

int HydrologySample(char* _saveTime) {

	int	  i     = 0;
	int	  adc_i = 0, isr_i = 0;
	int	  interval = 0;
	int	  a = 0, b = 0, c = 0;
	volatile int rs485_i		 = 0;
	long	 isr_count		 = 0;
	char	 io_i		 = 0;
	char	 isr_count_temp[ 5 ] = { 0 };
	char	 sampleinterval[ 2 ];
	char	 _temp_sampertime[ 6 ] = { 0 };
	static char  sampletime[ 6 ]       = { 0 };

	Hydrology_ReadStoreInfo(HYDROLOGY_SAMPLE_INTERVAL, sampleinterval,
				HYDROLOGY_SAMPLE_INTERVAL_LEN);  //??????????
	sampleinterval[ 0 ] = _BCDtoDEC(sampleinterval[ 0 ]);
	sampleinterval[ 1 ] = _BCDtoDEC(sampleinterval[ 1 ]);
	interval	    = sampleinterval[ 1 ] + sampleinterval[ 0 ] * 100;

	Utility_Strncpy(sampletime, _saveTime, 6);

	int tmp = sampletime[ 4 ] % (interval / 60);
	if (tmp != 0) {
		TraceMsg("Not Sample Time!", 1);
		return -1;
	}

	TraceMsg(" Start Sample:   ", 0);
	UART1_Open_9600(UART1_U485_TYPE);

	while (Element_table[ i ].ID != 0) {
		memset(value, 0, sizeof(value));
		switch (Element_table[ i ].Mode) {
		case ADC: {
			ADC_Sample();
			adc_i = ( int )Element_table[ i ].Channel;
			ADC_Element(value, adc_i);
			// adc_i++;
			break;
		}
		case ISR_COUNT: {
			isr_i = ( int )Element_table[ i ].Channel;
			Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT1
							+ (isr_i - 1) * HYDROLOGY_ISR_COUNT_LEN,
						isr_count_temp, HYDROLOGY_ISR_COUNT_LEN);
			isr_count = (isr_count_temp[ 4 ] * 0x100000000)
				    + (isr_count_temp[ 3 ] * 0x1000000)
				    + (isr_count_temp[ 2 ] * 0x10000)
				    + (isr_count_temp[ 1 ] * 0x100) + (isr_count_temp[ 0 ]);
			memcpy(value, ( char* )(&isr_count), 4);
			// isr_i++;
			break;
		}
		case IO_STATUS: {
			io_i = ( int )Element_table[ i ].Channel;
			Hydrology_ReadIO_STATUS(value, io_i);
			// io_i++;
			break;
		}
		case RS485: {
			// rs485_i = (int)Element_table[i].Channel;
			// //??????????????????485?????index???????channel??index
			Hydrology_ReadRS485(value, rs485_i);
			rs485_i++;
			break;
		}
		}

		switch (Element_table[ i ].type) {
		case ANALOG: {
			convertSampleTimetoHydrology(g_rtc_nowTime, _temp_sampertime);
			Hydrology_SetObservationTime(Element_table[ i ].ID, _temp_sampertime, i);
			Hydrology_WriteStoreInfo(HYDROLOGY_ANALOG1 + a * HYDROLOGY_ANALOG_LEN,
						 value, HYDROLOGY_ANALOG_LEN);
			a++;
			break;
		}
		case PULSE: {
			convertSampleTimetoHydrology(g_rtc_nowTime, _temp_sampertime);
			Hydrology_SetObservationTime(Element_table[ i ].ID, _temp_sampertime, i);
			Hydrology_WriteStoreInfo(HYDROLOGY_PULSE1 + b * HYDROLOGY_PULSE_LEN, value,
						 HYDROLOGY_PULSE_LEN);
			b++;
			break;
		}
		case SWITCH: {
			convertSampleTimetoHydrology(g_rtc_nowTime, _temp_sampertime);
			Hydrology_SetObservationTime(Element_table[ i ].ID, _temp_sampertime, i);
			Hydrology_WriteStoreInfo(HYDROLOGY_SWITCH1 + c * HYDROLOGY_SWITCH_LEN,
						 value, HYDROLOGY_SWITCH_LEN);
			c++;
			break;
		}
		}
		i++;
	}
	UART3_Open(UART3_CONSOLE_TYPE);
	UART1_Open(UART1_BT_TYPE);
	TraceMsg("Sample Done!  ", 0);
	return 0;
}

int HydrologyOnline() {
	// if (time_10min >= 1)
	// hydrologyProcessSend(LinkMaintenance);

	return 0;
}

int HydrologyOffline() {
	GPRS_Close_TCP_Link();
	GPRS_Close_GSM();

	return 0;
}

int HydrologySaveData(char* _saveTime, char funcode)  // char *_saveTime
{
	int   i = 0, acount = 0, pocunt = 0;
	float floatvalue    = 0;
	long  intvalue1     = 0;
	int   intvalue2     = 0;
	int   type	  = 0;
	int   cnt	   = 0;
	int   _effect_count = 0;
	Hydrology_ReadDataPacketCount(&_effect_count);  //????????????��?????????????

	type = hydrologyJudgeType(funcode);
	char	storeinterval;
	static char storetime[ 6 ] = { 0, 0, 0, 0, 0, 0 };
	Hydrology_ReadStoreInfo(HYDROLOGY_WATERLEVEL_STORE_INTERVAL, &storeinterval,
				HYDROLOGY_WATERLEVEL_STORE_INTERVAL_LEN);  // ly
	storeinterval = _BCDtoDEC(storeinterval);
	Utility_Strncpy(storetime, _saveTime, 6);
	int tmp = storetime[ 4 ] % (storeinterval);
	if (tmp != 0) {
		TraceMsg("Not Save Time!", 1);
		return -1;
	}

	TraceMsg("Start Store:   ", 0);
	if (type == 1) {
		while (Element_table[ i ].ID != 0) {
			switch (Element_table[ i ].type) {
			case ANALOG: {
				Hydrology_ReadAnalog(&floatvalue, acount++);
				pvPortMallocElement(Element_table[ i ].ID, Element_table[ i ].D,
						    Element_table[ i ].d,
						    &inputPara[ i ]);  //???id ??num??????value????
				converToHexElement(( double )floatvalue, Element_table[ i ].D,
						   Element_table[ i ].d, inputPara[ i ].value);
				break;
			}
			case PULSE: {
				Hydrology_ReadPulse(&intvalue1, pocunt++);
				pvPortMallocElement(Element_table[ i ].ID, Element_table[ i ].D,
						    Element_table[ i ].d, &inputPara[ i ]);
				converToHexElement(( double )intvalue1, Element_table[ i ].D,
						   Element_table[ i ].d, inputPara[ i ].value);
				break;
			}
			case SWITCH: {
				Hydrology_ReadSwitch(&intvalue2);
				pvPortMallocElement(Element_table[ i ].ID, Element_table[ i ].D,
						    Element_table[ i ].d, &inputPara[ i ]);
				converToHexElement(( double )intvalue2, Element_table[ i ].D,
						   Element_table[ i ].d, inputPara[ i ].value);
				break;
			}
			case STORE: {
				inputPara[ i ].guide[ 0 ] = Element_table[ i ].ID;
				inputPara[ i ].guide[ 1 ] = Element_table[ i ].ID;

				if(Debug)
		    			TraceMsg("hydrologytask.c  HydrologySaveData malloc ", 1);
				inputPara[ i ].value      = ( char* )pvPortMalloc(SinglePacketSize);
				if (NULL != inputPara[ i ].value) {
					inputPara[ i ].num = SinglePacketSize;
					Console_WriteStringln("hydrologytask.c HydrologySaveData Malloc Failed");
					// Hydrology_ReadRom(RomElementBeginAddr,inputPara[i].value,SinglePacketSendCount++);
				}
				break;
			}
			default:
				break;
			}
			i++;
			cnt++;
		}
	}

#if 0
	 g_HydrologyDataPacket.send_flag = 0x00;
	 Hydrology_ReadObservationTime(Element_table[0].ID,g_HydrologyDataPacket.observationtime,0);
	for(i = 0;i < cnt;i++)
	{
		memcpy((g_HydrologyDataPacket.element)[i].guide, inputPara[i].guide,2);
		(g_HydrologyDataPacket.element)[i].value = (char*) pvPortMalloc(inputPara[i].num);
		if(NULL == (g_HydrologyDataPacket.element)[i].value)
			 return -1;
		memcpy((g_HydrologyDataPacket.element)[i].value,inputPara[i].value, inputPara[i].num);
		(g_HydrologyDataPacket.element)[i].num = inputPara[i].num; 
	}
#else
	char _data[ 40 ] = { 0 };  //???????40?????
	char observationtime[ 5 ];
	int  len   = 0;
	_data[ 0 ] = 0x00;  // ��?????? ???0x00
	Hydrology_ReadObservationTime(Element_table[ 0 ].ID, observationtime,
				      0);  // or save_time
	memcpy(&_data[ 1 ], observationtime, HYDROLOGY_OBSERVATION_TIME_LEN);
	len += 6;
	for (i = 0; i < cnt; i++) {
		memcpy(&_data[ len ], inputPara[ i ].value, inputPara[ i ].num);
		len += inputPara[ i ].num;
		if (inputPara[ i ].value != NULL) {
			vPortFree(inputPara[ i ].value);
			inputPara[ i ].value = NULL;
		}
	}
	++_effect_count;  //????????1
	Hydrology_SetDataPacketCount(_effect_count);
#endif
	if (Store_WriteDataItemAuto(_data) < 0) {
		return -1;
	}
	TraceMsg("Save Data Success", 1);
	return 0;
}
int HydrologyInstantWaterLevel(char* _saveTime)  //??��??????��????��??�l?????
{

	static char endtime[ 6 ] = { 0, 0, 0, 0, 0, 0 };
	Utility_Strncpy(endtime, _saveTime, 6);
	int ret = 0;
	ret     = Utility_Is_A_ReportTime(endtime);  //?????��??????????

	if (!ret) {
		printf("Not Send Time, now time is: %d/%d/%d  %d:%d:%d \r\n", endtime[ 0 ],
		       endtime[ 1 ], endtime[ 2 ], endtime[ 3 ], endtime[ 4 ], endtime[ 5 ]);
		return -1;
	}
	/*
	 int tmp = endtime[4]%(timerinterval*5);
	 if(tmp!= 0 && !IsDebug)
	 {
	//if((endtime[4]-'0')%((timerinterval-'0')*5)!= 0 )
	 TraceMsg(" Time is:   ",0);
	 TraceHexMsg(endtime,5);
	     return -1;
	 }
    */
	int _effect_count = 0;				//?��??flash????����??????????
	Hydrology_ReadDataPacketCount(&_effect_count);  //???????????��?????????????
	TraceInt4(_effect_count, 1);
	//???????��?????��?
	int _startIdx = 0;
	int _endIdx   = 0;

	char _send[ 40 ] = { 0 };
	int  _ret	= 0;
	int  _seek_num   = 0;  //????????
	int  sendlen     = 0;

	_ret = FlowCheckSampleData(&_startIdx, &_endIdx);  //???startidx endidx
	if (_ret != 0) {
		return -1;
	}
	while (_effect_count != 0) {
		TraceMsg("read data in :", 0);
		TraceInt4(_startIdx, 1);
		TraceInt4(_effect_count, 1);
		if (_seek_num > HYDROLOGY_DATA_MAX_IDX)  //??????????????????????????????????????
		{
			TraceMsg("seek num out of range", 1);
			// hydrologHEXvPortFree();
			System_Delayms(2000);
			System_Reset();
		}

		_ret = Store_ReadDataItem(_startIdx, _send,
					  0);  //????????ret??????????????

		if (_ret < 0) {
			TraceMsg("can't read data ! very bad .", 1);
			return -1;  //??????????
				    //?????????.
		}
		else if (_ret == 1) {
			TraceMsg("It's sended data", 1);
			if (_startIdx
			    >= HYDROLOGY_DATA_MAX_IDX) {  //????????startidx???????????index????????????
				_startIdx = HYDROLOGY_DATA_MIN_IDX;
			}
			else {
				++_startIdx;
			}  //???????
			++_seek_num;
			Hydrology_SetStartIdx(_startIdx);  //?????_startIdx.
			TraceInt4(_startIdx, 1);
			TraceInt4(_endIdx, 1);
			// hydrologyExitSend();
		}
		else  //��?????????
		{
			sendlen = _ret;
			hydrologyProcessSend(_send, TimerReport);
			Store_MarkDataItemSended(_startIdx);  //??????????????
			--_effect_count;
			Hydrology_SetDataPacketCount(_effect_count);  //??????????????��????cnt
			if (_startIdx >= HYDROLOGY_DATA_MAX_IDX) {
				_startIdx = HYDROLOGY_DATA_MIN_IDX;
			}
			else {
				++_startIdx;  //???????
			}
			++_seek_num;

			TraceMsg(_send, 1);
			Hydrology_SetStartIdx(_startIdx);  //????_startIdx.

			// TraceInt4(_startIdx, 1);
			// TraceInt4(_endIdx, 1);
			//  hydrologyExitSend();
		}
	}

	TraceMsg("Report done", 1);
	System_Delayms(2000);
	/**************************************************************************************/
	// hydrologyProcessSend(TimerReport);

	if (!IsDebug) {
		System_Delayms(5000);
		JudgeServerDataArrived();
		Hydrology_ProcessGPRSReceieve();
		JudgeServerDataArrived();
		Hydrology_ProcessGPRSReceieve();
		JudgeServerDataArrived();
		Hydrology_ProcessGPRSReceieve();
		if (GPRS_Close_TCP_Link() != 0)
			GPRS_Close_GSM();
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
	//    char _temp_voltage[4];
	//
	//    _temp_voltage[0] = A[0] >> 8;
	//    _temp_voltage[1] = A[0] & 0x00FF;
	//
	//    Store_SetHydrologyVoltage(_temp_voltage);
	//
	return 0;
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
		GSM_Open();
		GSM_AT_QueryTime(&newtime[ 0 ], &newtime[ 1 ], &newtime[ 2 ], &newtime[ 3 ],
				 &newtime[ 4 ], &newtime[ 5 ]);
		System_Delayms(50);
		TraceHexMsg(newtime, 6);
		_RTC_SetTime(newtime[ 5 ], newtime[ 4 ], newtime[ 3 ], newtime[ 2 ], newtime[ 1 ],
			     1, newtime[ 0 ], 0);  //????????????????RTC???
	}
	if (RTC_IsBadTime(g_rtc_nowTime, 1) < 0) {  //????????????????,?????????????
		TraceMsg("Still bad time!", 1);
		System_Reset();
		return -2;
	}
	return 0;
}
int HydrologyTask() {
	char rtc_nowTime[ 6 ];
	// TimerB_Clear();
	// WatchDog_Clear();
	// Hydrology_ProcessUARTReceieve();

	// Hydrology_TimeCheck();

	// if (!IsDebug) {
	//     if (time_1min)
	//         time_1min = 0;
	//     else
	//         return -1;
	// }
	while (RTC_IsBadTime(g_rtc_nowTime, 1) != 0) {
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}

	RTC_ReadTimeBytes5(g_rtc_nowTime);
	RTC_ReadTimeBytes6(rtc_nowTime);

	HydrologySample(rtc_nowTime);
	HydrologySaveData(rtc_nowTime, TimerReport);
	HydrologyInstantWaterLevel(rtc_nowTime);

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

void task_hydrology_init(void* pvParameters) {

	char rtc_nowTime[ 6 ];

	while (1) {

		printf("time check ! \r\n");
		TimerB_Clear();
		WatchDog_Clear();
		Hydrology_ProcessUARTReceieve();

		Hydrology_TimeCheck();

		printf("stack remained : %d \r\n", ( int )uxTaskGetStackHighWaterMark(NULL));
		vTaskDelay(100 / portTICK_PERIOD_MS);

		RTC_ReadTimeBytes5(g_rtc_nowTime);
		RTC_ReadTimeBytes6(rtc_nowTime);
	}
}

// void task_hydrology_sample(void *pvParameters) {
// 	while (1){
// 		if (RTC_IsBadTime(g_rtc_nowTime, 1) != 0){
// 			vTaskDelay(1000 / portTICK_PERIOD_MS);
// 			continue;
// 		}
// 		HydrologySample(rtc_nowTime);
// 		vTaskDelay(100 / portTICK_PERIOD_MS);
// 	}
// }

// void task_hydrology_save_data(void *pvParameters) {

// 	while (1){
// 		if (RTC_IsBadTime(g_rtc_nowTime, 1) != 0){
// 			vTaskDelay(1000 / portTICK_PERIOD_MS);
// 			continue;
// 		}
// 		HydrologySaveData(rtc_nowTime, TimerReport);
// 		vTaskDelay(100 / portTICK_PERIOD_MS);
// 	}
// }

// void task_hydrology_instant_waterlevel(void *pvParameters) {
// 	while (1){
// 		if (RTC_IsBadTime(g_rtc_nowTime, 1) != 0){
// 			vTaskDelay(1000 / portTICK_PERIOD_MS);
// 			continue;
// 		}
// 		HydrologyInstantWaterLevel(rtc_nowTime);
// 		vTaskDelay(100 / portTICK_PERIOD_MS);
// 	}
// }