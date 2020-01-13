#ifndef _HYDROLODYTASK_
#define _HYDROLODYTASK_

#include "FreeRTOS.h"
#include "semphr.h"

void hydrology_sample(void* pvParameters);
void hydrology_save(void* pvParameters);
void hydrology_report(void* pvParameters);
void hydrology_init();
void create_hydrology_tasks();

#endif