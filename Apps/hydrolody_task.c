
#include "rtc_task.h"
#include "hydrolody_task.h"
#include "FreeRTOS.h"
#include "ParTest.h"
#include "semphr.h"

#include "common.h"
#include "hydrologytask.h"
#include "message.h"
#include "rtc.h"

int debug = 0;

void hydrology_sample(void* pvParameters) {

	while (1) {
		HydrologySample();

		unsigned int interval = get_sample_interval_form_flash();
		for(unsigned int i=0; i<interval; i++){
			vTaskDelay(30 * 1000 / portTICK_PERIOD_MS);
			vTaskDelay(30 * 1000 / portTICK_PERIOD_MS);
			printf("sample still alive, already %d mins, total interval time %d mins\n", i, interval);
		}
	}
}

void hydrology_store(void* pvParameters) {

	while (1) {
		HydrologySaveData(TimerReport);

		unsigned int interval = get_store_interval();
		for(unsigned int i=0; i<interval; i++){
			vTaskDelay(30 * 1000 / portTICK_PERIOD_MS);
			vTaskDelay(30 * 1000 / portTICK_PERIOD_MS);
			printf("store still alive, already %d mins, total interval time %d mins\n", i, interval);
		}
	}
}

void hydrology_report(void* pvParameters) {

	while (1) {
		hydrologyReport(rtc_nowTime);

		unsigned int interval = get_report_interval();
		for(unsigned int i=0; i<interval; i++){
			vTaskDelay(30 * 1000 / portTICK_PERIOD_MS);
			vTaskDelay(30 * 1000 / portTICK_PERIOD_MS);
			printf("report still alive, already %d mins, total interval time %d mins\n", i, interval);
		}
	}
}

void hydrology_init() {
	Hydrology_SetStartIdx(1);
	Hydrology_SetEndIdx(1);
	Hydrology_SetDataPacketCount(0);

	Hydrology_InitWaitConfig();
	HydrologyUpdateElementTable();
	HydrologyDataPacketInit();

	printf("hydrology init done \n\n");

}

void create_hydrology_tasks() {
	hydrology_init();
	// xTaskCreate(rtc_update, "RTC update", configMINIMAL_STACK_SIZE * 4, NULL,
	// 	    tskIDLE_PRIORITY + 2, NULL);  // 550 333//660 433//440 266

	xTaskCreate(hydrology_sample, "hydrology sample", configMINIMAL_STACK_SIZE * 4, NULL,
		    tskIDLE_PRIORITY + 2, NULL);  // 550 306//660 396//440 242

	xTaskCreate(hydrology_store, "hydrology store", configMINIMAL_STACK_SIZE * 4, NULL,
		    tskIDLE_PRIORITY + 2, NULL);  // 550 306//660 396//440 242

	xTaskCreate(hydrology_report, "hydrology report", configMINIMAL_STACK_SIZE * 6, NULL,
		    tskIDLE_PRIORITY + 2, NULL);  // 550 198//660 278//440 118
}