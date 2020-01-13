#include "hydrology_task.h"
#include "rtc_task.h"

#include "FreeRTOS.h"
#include "ParTest.h"
#include "semphr.h"

#include "common.h"
#include "hydrologytask.h"
#include "message.h"
#include "rtc.h"


int debug = 0;

void hydrology_sample_and_store(void* pvParameters) {

	while (1) {
		// debug_printf("\r\nSample start,freeheap:%d\r\n", xPortGetFreeHeapSize());
		// debug_printf("Sample HWM :%d\r\n", uxTaskGetStackHighWaterMark(NULL));

		if( HydrologySample(rtc_nowTime) != TRUE){
			continue;
		}
		
		HydrologySaveData(TimerReport);

		// debug_printf("\r\nSample end,freeheap:%d\r\n", xPortGetFreeHeapSize());

		vTaskDelay(3000 / portTICK_PERIOD_MS);
	}
}

void hydrology_report(void* pvParameters) {

	while (1) {
		HydrologyInstantWaterLevel(rtc_nowTime);
		vTaskDelay(3000 / portTICK_PERIOD_MS);
	}
}

void hydrology_init() {
	Hydrology_SetStartIdx(1);
	Hydrology_SetEndIdx(1);
	Hydrology_SetDataPacketCount(0);

	Hydrology_InitWaitConfig();
	HydrologyUpdateElementTable();
	HydrologyDataPacketInit();

	printf("hydrology init done \r\n");

}

void create_hydrology_tasks() {
	hydrology_init();
	xTaskCreate(rtc_update, "RTC update", configMINIMAL_STACK_SIZE * 4, NULL,
		    tskIDLE_PRIORITY + 2, NULL);  // 550 333//660 433//440 266
	xTaskCreate(hydrology_sample_and_store, "hydrology sample", configMINIMAL_STACK_SIZE * 4,
		    NULL, tskIDLE_PRIORITY + 2, NULL);  // 550 306//660 396//440 242
	xTaskCreate(hydrology_report, "hydrology report", configMINIMAL_STACK_SIZE * 5, NULL,
		    tskIDLE_PRIORITY + 2, NULL);  // 550 198//660 278//440 118
}