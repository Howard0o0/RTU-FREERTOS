#include "RTC_Task.h"
#include "Hydrolody_Task.h"

#include "FreeRTOS.h"
#include "ParTest.h"
#include "semphr.h"

#include "hydrologytask.h"
#include "rtc.h"



void rtc_update(void* pvParameters) {

	while (1) {

                printf("RTC HWM:%d\r\n", uxTaskGetStackHighWaterMark(NULL));


		TimerB_Clear();
		WatchDog_Clear();

		Hydrology_TimeCheck();
		RTC_ReadTimeBytes5(g_rtc_nowTime);
		RTC_ReadTimeBytes6(rtc_nowTime);

		

		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
}



