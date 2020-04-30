#include "GTM900C.h"
#include "Sampler.h"
#include "adc.h"
#include "common.h"
#include "hydrologycommand.h"
#include "hydrologytask.h"
#include "main.h"
#include "math.h"
#include "message.h"
#include "msp430common.h"
#include "packet.h"
#include "rtc.h"
#include "stdint.h"
#include "store.h"
#include "string.h"
#include "timer.h"
#include "uart1.h"
#include "uart3.h"
#include "uart_config.h"

extern int		UserElementCount;    //++++++++++++++++++++++++++++++++
extern int		RS485RegisterCount;  //++++++++++
extern int		IsDebug;
extern int		DataPacketLen;
extern hydrologyElement inputPara[ MAX_ELEMENT ];
// extern hydrologyElement outputPara[MAX_ELEMENT];
uint16_t time_10min = 0, time_5min = 0, time_1min = 1, time_1s = 0;

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
void convert_array_bcd2dec(char* array, int len) {
	for (int i = 0; i < len; i++) {
		array[ i ] = _BCDtoDEC(array[ i ]);
	}
}
float ConvertAnalog(int v, int index) {
	float tmp;
	char  range[ HYDROLOGY_ADC_RANGE_LEN ]		   = { 0, 1 };
	char  coefficient[ HYDROLOGY_ADC_COEFFICIENT_LEN ] = { 1, 0 };
	char  offset[ HYDROLOGY_ADC_OFFSET_LEN ]	   = { 0 };
	char  base[ HYDROLOGY_ADC_BASE_LEN ]		   = { 0 };
	char  upper, lower, warnning1, warnning2;
	float Range, Coefficient, Offset;
	float Base;

	Hydrology_ReadStoreInfo(HYDROLOGY_ADC_RANGE1 + index * HYDROLOGY_ADC_RANGE_LEN, range,
				HYDROLOGY_ADC_RANGE_LEN);
	Hydrology_ReadStoreInfo(HYDROLOGY_ADC_COEFFICIENT1 + index * HYDROLOGY_ADC_COEFFICIENT_LEN,
				coefficient, HYDROLOGY_ADC_COEFFICIENT_LEN);
	Hydrology_ReadStoreInfo(HYDROLOGY_ADC_OFFSET1 + index * HYDROLOGY_ADC_OFFSET_LEN, offset,
				HYDROLOGY_ADC_OFFSET_LEN);
	Hydrology_ReadStoreInfo(HYDROLOGY_ADC_BASE1 + index * HYDROLOGY_ADC_BASE_LEN, base,
				HYDROLOGY_ADC_BASE_LEN);
	Hydrology_ReadStoreInfo(HYDROLOGY_ADC_UPPER1 + index * HYDROLOGY_ADC_UPPER_LEN, &upper,
				HYDROLOGY_ADC_UPPER_LEN);
	Hydrology_ReadStoreInfo(HYDROLOGY_ADC_LOWER1 + index * HYDROLOGY_ADC_LOWER_LEN, &lower,
				HYDROLOGY_ADC_LOWER_LEN);
	Hydrology_ReadStoreInfo(HYDROLOGY_ADC_WARNNING1_1 + index * HYDROLOGY_ADC_WARNNING_LEN,
				&warnning1, HYDROLOGY_ADC_WARNNING_LEN);
	Hydrology_ReadStoreInfo(HYDROLOGY_ADC_WARNNING2_1 + index * HYDROLOGY_ADC_WARNNING_LEN,
				&warnning2, HYDROLOGY_ADC_WARNNING_LEN);

	convert_array_bcd2dec(range, HYDROLOGY_ADC_RANGE_LEN);
	convert_array_bcd2dec(coefficient, HYDROLOGY_ADC_COEFFICIENT_LEN);
	convert_array_bcd2dec(offset, HYDROLOGY_ADC_OFFSET_LEN);
	convert_array_bcd2dec(base, HYDROLOGY_ADC_BASE_LEN);
	convert_array_bcd2dec(&upper, HYDROLOGY_ADC_UPPER_LEN);
	convert_array_bcd2dec(&lower, HYDROLOGY_ADC_LOWER_LEN);
	convert_array_bcd2dec(&warnning1, HYDROLOGY_ADC_WARNNING_LEN);
	convert_array_bcd2dec(&warnning2, HYDROLOGY_ADC_WARNNING_LEN);

	Range	    = range[ 0 ] * 100 + range[ 1 ];
	Coefficient = coefficient[ 0 ] + (( float )coefficient[ 1 ]) / 100;
	Offset	    = offset[ 0 ] + (( float )offset[ 1 ]) / 100;
	//   Base = (double)((((base[0] * 100) + base[1] * 100) + base[2] * 100) + base[3]) +
	//                                         (double)((double)base[4] +
	//                                         ((double)base[5]/100))/100;
	Base = ( float )base[ 0 ];
	Base = Base * 100 + ( float )base[ 1 ];
	Base = Base * 100 + ( float )base[ 2 ];
	Base += (( float )base[ 3 ] / 100);
	//   Base = Base + Base1 + Base2;

	tmp = ( float )v / (4096.0) * 2 * Range * Coefficient + Offset + Base;
	return tmp;
}

void ADC_Element(char* value, int index, int adc_i) { // int range[5] = {1,20,100,5000,4000};     //ģ������Χ
	// int range[16] = {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2};
	float floatvalue = 0;

	// floatvalue = ConvertAnalog(A[index+1],range[index]);
	floatvalue = ConvertAnalog(A[ index - 1 ], adc_i); /*++++++++++++++++*/
	memcpy(value, ( char* )(&floatvalue), 4);
}
char value[ 4 ] = { 0, 0, 0, 0 };

void HydrologyUpdateElementTable() /*++++++++++++++++++++++++++*/
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
		getElementDd(Element_table[ i ].ID, &Element_table[ i ].D,
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
char s_picture_flag   = 0;
void HydrologyDataPacketInit() {
	char packet_len = 0;
	int  i		= 0;
	packet_len += HYDROLOGY_DATA_SEND_FLAG_LEN;
	packet_len += HYDROLOGY_DATA_TIME_LEN;
	while (Element_table[ i ].ID != 0) {
		mallocElement(Element_table[ i ].ID, Element_table[ i ].D, Element_table[ i ].d,
			      &inputPara[ i ]);
		packet_len += inputPara[ i ].num;
		i++;
	}
	DataPacketLen = packet_len;
	Hydrology_WriteStoreInfo(HYDROLOGY_DATA_PACKET_LEN, &packet_len,
				 HYDROLOGY_DATA_PACKET_LEN_LEN);
	Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT_FLAG, &s_isr_count_flag,
				HYDROLOGY_ISR_COUNT_FLAG_LEN);
	Hydrology_ReadStoreInfo(PICTURE_FLAG, &s_picture_flag, PICTURE_FLAG_LEN);  //++
	/*��ʼ����ʱ�������ʱ��?*/
	char newtime[ 6 ] = { 0 };
	if (gprsConfigSuccess == FALSE) {
		GPRS_Close_GSM();
		GPRS_Create_TCP_Link(
			0);  //���Ȳ鿴TCP���ӽ���û�У�û�н�������Ҫ���½�������?
	}
	System_Delayms(5000);  //����ʱ���ѯATָ��ǰ������ʱ�ŷ���ʱ��
	BC95_QueryTime(&newtime[ 0 ], &newtime[ 1 ], &newtime[ 2 ], &newtime[ 3 ], &newtime[ 4 ],
		       &newtime[ 5 ]);
	System_Delayms(300);
	TraceHexMsg(newtime, 6);
	_RTC_SetTime(newtime[ 5 ], newtime[ 4 ], newtime[ 3 ], newtime[ 2 ], newtime[ 1 ], 1,
		     newtime[ 0 ], 0);	//��ѯ��Ϊ��ƽ̨ʱ�����RTCʱ��
}

int HydrologySample(char* _saveTime) {
	char	     _temp_sampertime[ 6 ] = { 0, 0, 0, 0, 0, 0 };
	volatile int rs485_i		   = 0;	 // int
	volatile int i = 0, j = 0;		 // int
	int	     adc_i = 0, isr_i = 0;	 // int
	char	     io_i = 0;
	volatile int a = 0, b = 0, c = 0;
	long	     isr_count		 = 0;
	char	     isr_count_temp[ 5 ] = { 0, 0, 0, 0, 0 };

	// HydrologyUpdateElementTable();  /*++++++++++++++++++++++����Ҫ�ر�++++++++++++++*/

	// HydrologyDataPacketInit();      /*++++++����Ҫ�ر����DP�Ĵ�С�ͳ�ʼ��++++++++++*/

	/*************************/

	char	    sampleinterval[ 2 ];
	int	    interval	    = 0;
	static char sampletime[ 6 ] = { 0, 0, 0, 0, 0, 0 };
	Hydrology_ReadStoreInfo(HYDROLOGY_SAMPLE_INTERVAL, sampleinterval,
				HYDROLOGY_SAMPLE_INTERVAL_LEN);	 // ly
	sampleinterval[ 0 ] = _BCDtoDEC(sampleinterval[ 0 ]);
	sampleinterval[ 1 ] = _BCDtoDEC(sampleinterval[ 1 ]);
	interval	    = sampleinterval[ 1 ] + sampleinterval[ 0 ] * 100;

	Utility_Strncpy(sampletime, _saveTime, 6);

	// ly Ϊ�˷�����Թرռ�飬��ʽ����ʱ��ָ��ж�?

	int tmp = sampletime[ 4 ] % (interval / 60);

	if (tmp != 0 && IsDebug == 0)
	// if(tmp!= 0 )

	{
		TraceMsg("Not Sample Time!", 1);
		// TraceMsg(" Time is:   ",0);
		// TraceHexMsg(sampletime,4);
		return -1;
	}

	/***********************************************/
	TraceMsg(" Start Sample:   ", 0);
	UART1_Open_9600(UART3_U485_TYPE);
	while (Element_table[ j ].ID != 0) {
		if (Element_table[ j ].Mode == ADC) {
			ADC_Sample();
			break;
		}
		j++;
	}
	while (Element_table[ i ].ID != 0) {
		memset(value, 0, sizeof(value));
		switch (Element_table[ i ].Mode) {
		case ADC: {
			// ADC_Sample();
			int index = 1;
			index	  = _BCDtoDEC(Element_table[ i ].Channel);  // 0x10->10
			ADC_Element(value, index, adc_i);
			adc_i++;
			break;
		}
		case ISR_COUNT: {
			isr_i = _BCDtoDEC(Element_table[ i ].Channel);
			Hydrology_ReadStoreInfo(HYDROLOGY_ISR_COUNT1
							+ (isr_i - 1) * HYDROLOGY_ISR_COUNT_LEN,
						isr_count_temp, HYDROLOGY_ISR_COUNT_LEN);
			isr_count = (isr_count_temp[ 4 ] * 0x100000000)
				    + (isr_count_temp[ 3 ] * 0x1000000)
				    + (isr_count_temp[ 2 ] * 0x10000)
				    + (isr_count_temp[ 1 ] * 0x100) + (isr_count_temp[ 0 ]);
			memcpy(value, ( char* )(&isr_count), 4);
			// TraceMsg("ISR_COUNT:",1);
			// TraceInt4(value,1);  //compile error
			// isr_i++;
			break;
		}
		case IO_STATUS: {
			io_i = _BCDtoDEC(Element_table[ i ].Channel);
			Hydrology_ReadIO_STATUS(value, io_i);
			// TraceMsg("IO_STATUS:",1);
			// TraceInt4(value,1);//compile error
			// io_i++;
			break;
		}
		case RS485: {
			// rs485_i = (int)Element_table[i].Channel;
			// //������������һ����ȡ485�����index��һ����channel��index
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
	UART1_Open(UART1_BT_TYPE);  // zh  bt_spp mode
	UART3_Open(UART3_CONSOLE_TYPE);
	TraceMsg("Sample Done!  ", 0);
	return 0;
}

int HydrologyOnline() {
	if (time_10min >= 1)
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
	float floatvalue = 0;
	long  intvalue1	 = 0;
	// long intvalue2 = 0;
	char switch_value[ 4 ] = { 0 };
	int  type	       = 0;
	int  cnt	       = 0;
	// hydrologyDownBody* downpbody = (hydrologyDownBody*) (g_HydrologyMsg.downbody);
	int _effect_count = 0;
	Hydrology_ReadDataPacketCount(&_effect_count);	//��ʼ��ȡ�ڴ�ʣ��δ�������ݰ�����

	type = hydrologyJudgeType(funcode);

	char	    storeinterval;
	static char storetime[ 6 ] = { 0, 0, 0, 0, 0, 0 };
	Hydrology_ReadStoreInfo(HYDROLOGY_WATERLEVEL_STORE_INTERVAL, &storeinterval,
				HYDROLOGY_WATERLEVEL_STORE_INTERVAL_LEN);  // ly
	storeinterval = _BCDtoDEC(storeinterval);

	Utility_Strncpy(storetime, _saveTime, 6);

	// ly Ϊ�˷�����Թرռ�飬��ʽ����ʱ��ָ��ж�?

	int tmp = storetime[ 4 ] % (storeinterval);

	// if(tmp!= 0)
	if (tmp != 0 && IsDebug == 0) {
		TraceMsg("Not Save Time!", 1);
		// TraceMsg(" Time is:   ",0);
		// TraceHexMsg(storetime,4);
		return -1;
	}

	/***********************************************/
	TraceMsg("Start Store:   ", 0);
	if (type == 1) {
		while (Element_table[ i ].ID != 0) {
			switch (Element_table[ i ].type) {
			case ANALOG: {
				Hydrology_ReadAnalog(&floatvalue, acount++);
				mallocElement(Element_table[ i ].ID, Element_table[ i ].D,
					      Element_table[ i ].d,
					      &inputPara[ i ]);	 //���id ��num������value�Ŀռ�
				converToHexElement(( double )floatvalue, Element_table[ i ].D,
						   Element_table[ i ].d, inputPara[ i ].value);
				break;
			}
			case PULSE: {
				Hydrology_ReadPulse(&intvalue1, pocunt++);
				mallocElement(Element_table[ i ].ID, Element_table[ i ].D,
					      Element_table[ i ].d, &inputPara[ i ]);
				converToHexElement(( double )intvalue1, Element_table[ i ].D,
						   Element_table[ i ].d, inputPara[ i ].value);
				break;
			}
			case SWITCH: {
				// Hydrology_ReadSwitch(&intvalue2);
				Hydrology_ReadSwitch(switch_value);
				mallocElement(Element_table[ i ].ID, Element_table[ i ].D,
					      Element_table[ i ].d, &inputPara[ i ]);
				// converToHexElement((double)intvalue2,Element_table[i].D,Element_table[i].d,inputPara[i].value);
				// memcpy(inputPara[i].value,switch_value,4);
				inputPara[ i ].value[ 0 ] = switch_value[ 3 ];
				inputPara[ i ].value[ 1 ] = switch_value[ 2 ];
				inputPara[ i ].value[ 2 ] = switch_value[ 1 ];
				inputPara[ i ].value[ 3 ] = switch_value[ 0 ];

				break;
			}
			case STORE: {
				inputPara[ i ].guide[ 0 ] = Element_table[ i ].ID;
				inputPara[ i ].guide[ 1 ] = Element_table[ i ].ID;
				inputPara[ i ].value	  = ( char* )malloc(SinglePacketSize);
				if (NULL != inputPara[ i ].value) {
					inputPara[ i ].num = SinglePacketSize;
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
    (g_HydrologyDataPacket.element)[i].value = (char*) malloc(inputPara[i].num);
    if(NULL == (g_HydrologyDataPacket.element)[i].value)
       return -1;
    memcpy((g_HydrologyDataPacket.element)[i].value,inputPara[i].value, inputPara[i].num);
    (g_HydrologyDataPacket.element)[i].num = inputPara[i].num; 
  }
#else
	char _data[ HYDROLOGY_DATA_ITEM_LEN ] = { 0 };	//������Ϊ40
	char observationtime[ 5 ];
	int  len   = 0;
	_data[ 0 ] = 0x00;  // δ���ͱ��? ��Ϊ0x00
	Hydrology_ReadObservationTime(Element_table[ 0 ].ID, observationtime, 0);  // or save_time
	memcpy(&_data[ 1 ], observationtime, HYDROLOGY_OBSERVATION_TIME_LEN);
	len += 6;
	for (i = 0; i < cnt; i++) {
		memcpy(&_data[ len ], inputPara[ i ].value, inputPara[ i ].num);
		len += inputPara[ i ].num;

		if (inputPara[ i ].value != NULL)  //++++++
		{
			free(inputPara[ i ].value);
			inputPara[ i ].value = NULL;
		}
	}

	++_effect_count;  //��һ���ͼ�1
	Hydrology_SetDataPacketCount(_effect_count);
#endif
	if (Store_WriteDataItemAuto(_data) < 0) {
		return -1;
	}
	TraceMsg("Save Data Success", 1);
	return 0;
}
int HydrologyInstantWaterLevel(char* _saveTime)	 //���?��ʱ�䣬�ж����±꣬�鱨�ķ���
{

	char	    timerinterval;
	static char endtime[ 6 ] = { 0, 0, 0, 0, 0, 0 };
	static char time[ 6 ]	 = { 0 };
	// Hydrology_ReadStoreInfo(HYDROLOGY_TIMER_INTERVAL,&timerinterval,HYDROLOGY_WATERLEVEL_STORE_INTERVAL_LEN);
	// //ly timerinterval = _BCDtoDEC(timerinterval);
	Utility_Strncpy(endtime, _saveTime, 6);

	int ret = 0;
	ret	= Utility_Is_A_ReportTime(endtime);

	if (!ret) {
		TraceMsg(" Not Send Time!", 1);
		TraceMsg(" Time is:   ", 1);
		convertSendTimetoHydrology(endtime, time);
		TraceHexMsg(time, 6);
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
      /************************************************************************************** */
	int _effect_count = 0;
	Hydrology_ReadDataPacketCount(&_effect_count);	//��ȡ�ڴ���ʣ��δ�������ݰ�����
	TraceInt4(_effect_count, 1);
	//������ݴ�?�ϱ���±�?
	int _startIdx = 0;
	int _endIdx   = 0;

	char _send[ 200 ] = { 0 };

	int _ret      = 0;
	int _seek_num = 0;  //��ֹ��ѭ��
	int sendlen   = 0;

	_ret = FlowCheckSampleData(&_startIdx, &_endIdx);  //���startidx endidx
	if (_ret != 0) {
		return -1;
	}

	while (_effect_count != 0) {
		TraceMsg("read data in :", 0);
		TraceInt4(_startIdx, 1);
		TraceInt4(_effect_count, 1);
		if (_seek_num > HYDROLOGY_DATA_MAX_IDX)	 //Ѱ�ҵ����������Ѿ��������ֵ���˳�����ֹ���?��
		{
			TraceMsg("seek num out of range", 1);
			// hydrologHEXfree();
			System_Delayms(2000);
			System_Reset();
		}

		_ret = Store_ReadDataItem(_startIdx, _send, 0);	 //��ȡ���ݣ�retΪ���������ݳ���

		if (_ret < 0) {
			TraceMsg("can't read data ! very bad .", 1);
			return -1;  //�޷���ȡ���� ��ֱ������.
		}
		else if (_ret == 1) {
			TraceMsg("It's sended data", 1);
			if (_startIdx
			    >= HYDROLOGY_DATA_MAX_IDX) {  //+++++�����ȡ��startidx�����ɴ�����index������������
				_startIdx = HYDROLOGY_DATA_MIN_IDX;
			}
			else {
				++_startIdx;
			}  //��һ����
			++_seek_num;
			Hydrology_SetStartIdx(_startIdx);  //Ҫ����_startIdx.
			TraceInt4(_startIdx, 1);
			TraceInt4(_endIdx, 1);
			// hydrologyExitSend();
		}
		else  //δ���͵�����
		{
			sendlen = _ret;

			if (IsDebug == 1) {
				hydrologyProcessSend(_send, Test);
				// hydrologyProcessSend(_send,TimerReport);
				IsDebug = 0;
			}
			else {
				hydrologyProcessSend(_send, TimerReport);
			}

			// hydrologyProcessSend(_send,TimerReport);
			Store_MarkDataItemSended(_startIdx);  //���ø������ѷ���
			--_effect_count;
			Hydrology_SetDataPacketCount(_effect_count);  //��������?������Ч����cnt
			if (_startIdx >= HYDROLOGY_DATA_MAX_IDX) {
				_startIdx = HYDROLOGY_DATA_MIN_IDX;
			}
			else {
				++_startIdx;  //��һ����
			}
			++_seek_num;
			// TraceMsg(_send, 1);
			Hydrology_SetStartIdx(_startIdx);  //����_startIdx.

			// TraceInt4(_startIdx, 1);
			// TraceInt4(_endIdx, 1);

			// hydrologyExitSend();           //free������
		}
	}

	TraceMsg("Report done", 1);
	System_Delayms(2000);
	/**************************************************************************************/
	// hydrologyProcessSend(TimerReport);

	/*  if(!IsDebug)
	  {
	    System_Delayms(5000);
	    JudgeServerDataArrived();
	    Hydrology_ProcessGPRSReceieve();
	    JudgeServerDataArrived();
	    Hydrology_ProcessGPRSReceieve();
	    JudgeServerDataArrived();
	    Hydrology_ProcessGPRSReceieve();
	   if(GPRS_Close_TCP_Link() != 0)
		  GPRS_Close_GSM();
	  }
      */

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

/*�����ж�ʱ���Ƿ���ȷ������ȷ��������ʱ��ͬ��*/
int Hydrology_TimeCheck() {
	RTC_ReadTimeBytes5(g_rtc_nowTime);  //��rtc���ȡʱ�䵽g_rtc_nowtime
	// char _temp_str[18];

	/* if(trace_open)
	 {
	     TraceMsg("Device Time is :   ",0);
	     RTC_ReadTimeStr6_A(_temp_str);
	     UART3_Send(_temp_str,17,1);
	 }
     */
	int _time_error = 1;
	for (int i = 0; i < 11; ++i) {
		if (RTC_IsBadTime(g_rtc_nowTime, 1) == 0) {
			// TraceMsg("Time is OK .",1);
			_time_error = 0;
			break;	//ʱ����ȷ��
		}
		TraceMsg("Time is bad once .", 1);
		//ʱ�����Ļ�,�ٶ�
		RTC_ReadTimeBytes5(g_rtc_nowTime);
	}
	if (_time_error > 0) {	//ʱ�仹�Ǵ��� ,����Ҫ������ʱ��ͬ��
		TraceMsg("Device time is bad !", 1);
		TraceMsg("Waiting for config !", 1);
		char newtime[ 6 ] = { 0 };
		if (gprsConfigSuccess == FALSE) {
			GPRS_Close_GSM();
			GPRS_Create_TCP_Link(
				0);  //���Ȳ鿴TCP���ӽ���û�У�û�н�������Ҫ���½�������?
		}
		System_Delayms(5000);  //����ʱ���ѯATָ��ǰ������ʱ�ŷ���ʱ��
		BC95_QueryTime(&newtime[ 0 ], &newtime[ 1 ], &newtime[ 2 ], &newtime[ 3 ],
			       &newtime[ 4 ], &newtime[ 5 ]);
		System_Delayms(300);
		TraceHexMsg(newtime, 6);
		_RTC_SetTime(newtime[ 5 ], newtime[ 4 ], newtime[ 3 ], newtime[ 2 ], newtime[ 1 ],
			     1, newtime[ 0 ], 0);  //��ѯ��Ϊ��ƽ̨ʱ�����RTCʱ��
	}
	if (RTC_IsBadTime(g_rtc_nowTime, 1) < 0) {  //˵��û�ȴ�ʱ��Ķ�����?,������Ȼ�Ǵ����?
		TraceMsg("Still bad time!", 1);
		System_Reset();
		return -2;
	}
	return 0;
}
int HydrologyTask() {

	TimerB_Clear();
	WatchDog_Clear();
	//    JudgeServerDataArrived();
	//    Hydrology_ProcessGPRSReceieve();
	Hydrology_ProcessUARTReceieve();
	Hydrology_TimeCheck();

	if (!IsDebug) {
		if (time_1min)
			time_1min = 0;
		else
			return -1;
	}

	RTC_ReadTimeBytes5(g_rtc_nowTime);
	char rtc_nowTime[ 6 ];
	RTC_ReadTimeBytes6(rtc_nowTime);
	HydrologySample(rtc_nowTime);

	HydrologySaveData(rtc_nowTime, TimerReport);  //��������_saveTime

	HydrologyInstantWaterLevel(rtc_nowTime);  // report

	/*发送图片报*/
	char pictureinterval = 0;
	char _send[ 2 ]	     = { 0 };
	if (s_picture_flag == 0x01) {
		Hydrology_ReadStoreInfo(PICTURE_INTERVAL, &pictureinterval, PICTURE_INTERVAL_LEN);
		pictureinterval = _BCDtoDEC(pictureinterval);
		//判断时间间隔
		// int tmp = rtc_nowTime[3]%pictureinterval;
		if (rtc_nowTime[ 3 ] % (pictureinterval) && rtc_nowTime[ 4 ] == 0) {
			hydrologyProcessSend(_send, Picture);
		}
	}
	// HydrologyOnline();

	// }
	return 0;
}
