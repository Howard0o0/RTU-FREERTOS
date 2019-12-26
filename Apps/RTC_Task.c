#include "RTC_Task.h"
#include "Hydrolody_Task.h"

#include "FreeRTOS.h"
#include "ParTest.h"
#include "semphr.h"

#include "hydrologytask.h"
#include "rtc.h"

extern SemaphoreHandle_t GPRS_Lock;


void rtc_update(void* pvParameters) {

	while (1) {
                printf("\r\nRTC start,freeheap:%d\r\n",xPortGetFreeHeapSize());
                printf("RTC HWM:%d\r\n", uxTaskGetStackHighWaterMark(NULL));


		TimerB_Clear();
		WatchDog_Clear();

                xSemaphoreTake(GPRS_Lock,portMAX_DELAY);
		Hydrology_TimeCheck();
                xSemaphoreGive(GPRS_Lock);

		RTC_ReadTimeBytes5(g_rtc_nowTime);
		RTC_ReadTimeBytes6(rtc_nowTime);

                printf("RTC now time is: %d/%d/%d  %d:%d:%d \r\n", rtc_nowTime[ 0 ],
		       rtc_nowTime[ 1 ], rtc_nowTime[ 2 ], rtc_nowTime[ 3 ], rtc_nowTime[ 4 ], rtc_nowTime[ 5 ]);

		printf("\r\nRTC end,freeheap:%d\r\n",xPortGetFreeHeapSize());


		vTaskDelay(10000 / portTICK_PERIOD_MS);
	}
}



