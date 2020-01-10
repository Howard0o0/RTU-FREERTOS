#include "Hydrolody_Task.h"
#include "RTC_Task.h"

#include "FreeRTOS.h"
#include "ParTest.h"
#include "semphr.h"

#include "hydrologytask.h"
#include "message.h"
#include "rtc.h"

SemaphoreHandle_t sample_switch_save;

int		  debug = 0;

void hydrology_sample(void* pvParameters) {

	while (1) {
                printf("\r\nSample start,freeheap:%d\r\n",xPortGetFreeHeapSize());
		printf("Sample HWM :%d\r\n", uxTaskGetStackHighWaterMark(NULL));

		HydrologySample(rtc_nowTime);

		printf("\r\nSample end,freeheap:%d\r\n",xPortGetFreeHeapSize());


		if (debug)
			vTaskDelay(1000 / portTICK_PERIOD_MS);
		else
			vTaskDelay(30000 / portTICK_PERIOD_MS);
	}
}

void hydrology_save(void* pvParameters) {

	while (1) {
                xSemaphoreTake(sample_switch_save,portMAX_DELAY);

                printf("\r\nSave start,freeheap:%d\r\n",xPortGetFreeHeapSize());
		printf("Save HWM :%d\r\n", uxTaskGetStackHighWaterMark(NULL));

		HydrologySaveData(rtc_nowTime, TimerReport);

		printf("\r\nSave end,freeheap:%d\r\n",xPortGetFreeHeapSize());


		if (debug)
			vTaskDelay(1000 / portTICK_PERIOD_MS);
		else
			vTaskDelay(30000 / portTICK_PERIOD_MS);
	}
}


void hydrology_report(void* pvParameters) {

	while (1) {
                printf("\r\nReport start,freeheap:%d\r\n",xPortGetFreeHeapSize());
		printf("Report HWM:%d\r\n", uxTaskGetStackHighWaterMark(NULL));

		HydrologyInstantWaterLevel(rtc_nowTime);

                printf("\r\nReport end,freeheap:%d\r\n",xPortGetFreeHeapSize());

                // show_block();
		
		if (debug)
			vTaskDelay(1000 / portTICK_PERIOD_MS);
		else
			vTaskDelay(30000 / portTICK_PERIOD_MS);

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

        vSemaphoreCreateBinary(sample_switch_save);

}

void create_hydrology_tasks() {
	hydrology_init();
	xTaskCreate(rtc_update, "RTC update", configMINIMAL_STACK_SIZE * 4, NULL,
		    tskIDLE_PRIORITY + 2, NULL);//550 333//660 433//440 266
        xTaskCreate(hydrology_sample, "hydrology sample", configMINIMAL_STACK_SIZE * 4, NULL,
	                tskIDLE_PRIORITY + 2, NULL);//550 306//660 396//440 242
	xTaskCreate(hydrology_save, "hydrology save", configMINIMAL_STACK_SIZE * 4, NULL,
	                tskIDLE_PRIORITY + 2, NULL);//550 283//660 363//440 203
	xTaskCreate(hydrology_report, "hydrology report", configMINIMAL_STACK_SIZE * 4, NULL,
	                tskIDLE_PRIORITY + 2, NULL);//550 198//660 278//440 118             

}