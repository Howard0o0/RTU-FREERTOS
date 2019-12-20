#include "Hydrolody_Task.h"
#include "RTC_Task.h"

#include "FreeRTOS.h"
#include "ParTest.h"
#include "semphr.h"

#include "hydrologytask.h"
#include "message.h"
#include "rtc.h"


int		  debug = 0;

void hydrology_sample(void* pvParameters) {

	while (1) {
                printf("Sample start\r\n");
		printf("Sample HWM :%d\r\n", uxTaskGetStackHighWaterMark(NULL));

		HydrologySample(rtc_nowTime);

		printf("Sample end\r\n");
		if (debug)
			vTaskDelay(1000 / portTICK_PERIOD_MS);
		else
			vTaskDelay(30000 / portTICK_PERIOD_MS);
	}
}

void hydrology_save(void* pvParameters) {

	while (1) {
                printf("Save start\r\n");
		printf("Save HWM :%d\r\n", uxTaskGetStackHighWaterMark(NULL));

		HydrologySaveData(rtc_nowTime, TimerReport);

		printf("Save end\r\n");
		if (debug)
			vTaskDelay(1000 / portTICK_PERIOD_MS);
		else
			vTaskDelay(30000 / portTICK_PERIOD_MS);
	}
}


void hydrology_report(void* pvParameters) {

	while (1) {
                printf("Report start\r\n");
		printf("Report HWM:%d\r\n", uxTaskGetStackHighWaterMark(NULL));

		HydrologyInstantWaterLevel(rtc_nowTime);

                printf("Report end\r\n");
		
		if (debug)
			vTaskDelay(1000 / portTICK_PERIOD_MS);
		else
			vTaskDelay(30000 / portTICK_PERIOD_MS);

	}
}


extern SemaphoreHandle_t sample_switch_save;

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

        vSemaphoreCreateBinary(sample_switch_save);

}

void hydrology_run() {
	hydrology_init();
	xTaskCreate(rtc_update, "RTC update", configMINIMAL_STACK_SIZE * 5, NULL,
		    tskIDLE_PRIORITY + 2, NULL);
        xTaskCreate(hydrology_sample, "hydrology sample", configMINIMAL_STACK_SIZE * 5, NULL,
	                tskIDLE_PRIORITY + 2, NULL);
	xTaskCreate(hydrology_save, "hydrology save", configMINIMAL_STACK_SIZE * 5, NULL,
	                tskIDLE_PRIORITY + 2, NULL);
	xTaskCreate(hydrology_report, "hydrology report", configMINIMAL_STACK_SIZE * 5, NULL,
	                tskIDLE_PRIORITY + 2, NULL);

}