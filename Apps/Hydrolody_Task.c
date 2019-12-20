#include "Hydrolody_Task.h"
#include "RTC_Task.h"

#include "FreeRTOS.h"
#include "ParTest.h"
#include "semphr.h"

#include "hydrologytask.h"
#include "message.h"
#include "rtc.h"


int		  debug = 0;
SemaphoreHandle_t test_sem;

void hydrology_sample_and_save(void* pvParameters) {

	while (1) {
		
		printf("Sample and Save HWM IN:%d\r\n", uxTaskGetStackHighWaterMark(NULL));

		HydrologySample(rtc_nowTime);
		HydrologySaveData(rtc_nowTime, TimerReport);

                xSemaphoreGive(test_sem);

		
		if (debug)
			vTaskDelay(1000 / portTICK_PERIOD_MS);
		else
			vTaskDelay(60000 / portTICK_PERIOD_MS);
	}
}


void hydrology_report(void* pvParameters) {

	while (1) {
                xSemaphoreTake(test_sem,portMAX_DELAY);

		printf("Report HWM:%d\r\n", uxTaskGetStackHighWaterMark(NULL));

		HydrologyInstantWaterLevel(rtc_nowTime);


		
		if (debug)
			vTaskDelay(1000 / portTICK_PERIOD_MS);
		else
			vTaskDelay(60000 / portTICK_PERIOD_MS);

	}
}

void hydrology_init() {
#if 1
	Hydrology_SetStartIdx(1);
	Hydrology_SetEndIdx(1);
	Hydrology_SetDataPacketCount(0);
#endif
	Hydrology_InitWaitConfig();
	HydrologyUpdateElementTable();
	HydrologyDataPacketInit();

	printf("hydrology init done \r\n");

        vSemaphoreCreateBinary(test_sem);
}

void hydrology_run() {
	hydrology_init();
	xTaskCreate(rtc_update, "RTC update", configMINIMAL_STACK_SIZE * 5, NULL,
		    tskIDLE_PRIORITY + 1, NULL);//330 197 //110 39
	xTaskCreate(hydrology_sample_and_save, "hydrology sample", configMINIMAL_STACK_SIZE * 10, NULL,
	                tskIDLE_PRIORITY + 1, NULL);//330 197 //110 39
	xTaskCreate(hydrology_report, "hydrology report", configMINIMAL_STACK_SIZE * 5, NULL,
	                tskIDLE_PRIORITY + 1, NULL);//660 289 //330 49

}