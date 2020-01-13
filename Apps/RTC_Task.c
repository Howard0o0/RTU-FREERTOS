#include "rtc_task.h"
#include "hydrology_task.h"

#include "FreeRTOS.h"
#include "ParTest.h"
#include "semphr.h"

#include "common.h"
#include "hydrologytask.h"
#include "rtc.h"

void rtc_update(void* pvParameters) {

	while (1) {
		// TimerB_Clear();
		WatchDog_Clear();

		check_rtc_time();

		RTC_ReadTimeBytes5(g_rtc_nowTime);
		RTC_ReadTimeBytes6(rtc_nowTime);

		printf("rtc time : %d/%d/%d  %d:%d:%d \r\n", rtc_nowTime[ 0 ],
		       rtc_nowTime[ 1 ], rtc_nowTime[ 2 ], rtc_nowTime[ 3 ], rtc_nowTime[ 4 ],
		       rtc_nowTime[ 5 ]);


		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
}
