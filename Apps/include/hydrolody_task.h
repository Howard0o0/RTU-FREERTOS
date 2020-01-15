#ifndef _HYDROLODYTASK_H
#define _HYDROLODYTASK_H

#include "FreeRTOS.h"
#include "semphr.h"

void hydrology_sample(void* pvParameters);

void hydrology_store(void* pvParameters);

void hydrology_report(void* pvParameters);

void hydrology_init();

void create_hydrology_tasks();

#endif