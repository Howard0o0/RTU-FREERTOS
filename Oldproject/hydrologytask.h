#ifndef  _HYDROLOGYTASK_H_
#define  _HYDROLOGYTASK_H_

#include "stdint.h"


#define G_SAMPLE_SAVE_MAX_NUM   3    //ky,��������
int HydrologySample();

int Hydrology_TimeCheck();   
int HydrologyTask();

int HydrologyPacketJudgment();

int HydrologyCancelMark();

void HydrologyTimeBase();

void convertSendTimetoHydrology(char* src,char* dst);
void convert_array_bcd2dec(char* array,int len);

int HydrologyRecord(int index);


extern uint16_t time_1s;
extern uint16_t time_10min;
extern char Picture_Flag;

int hydrologyReport(char now_time[ 6 ]);
int HydrologyInstantWaterLevel(char* _saveTime);
void HydrologyUpdateElementTable() ;
void HydrologyDataPacketInit();
int HydrologySaveData(char funcode);
extern void getElementDd(char ID,char *D,char *d); 
//extern int BC95_QueryTime(char *year,char *month,char *date,char *hour,char *min,char *second);
extern void hydrologyExitSend();
extern int FlowCheckSampleData(int* pstart,int* pend);
extern int converToHexElement(double input, int D,int d,char* out);
//extern int mallocElement(char element,char D,char d,hydrologyElement* ele);
extern void Hydrology_ReadSwitch(char *value);
extern void Hydrology_ReadPulse(long *value,int index);
extern void Hydrology_ReadAnalog(float *value,int index);
extern int hydrologyJudgeType(char funcode);
extern void UART1_Open_9600(int _type);

void task_hydrology_init(void *pvParameters);
void task_hydrology_run(void *pvParameters);
// void task_hydrology_check_time(void *pvParameters);

// void task_hydrology_sample(void *pvParameters);

// void task_hydrology_save_data(void *pvParameters);

// void task_hydrology_instant_waterlevel(void *pvParameters);

unsigned int get_sample_interval_form_flash();   //minutes
uint8_t get_store_interval(void);                //minutes
unsigned int get_report_interval();              //minutes


#endif /* _HYDROLOGYTASK_H_ */