#include "common.h"
#include "communication_opr.h"
#include "gprs.h"
#include "module_config.h"
#include <string.h>

static communication_dev_t *g_communication_handler = NULL;

static int init_communication_dev_lock(void);
static int select_communication_dev_and_install_driver(void);

int register_communication_dev_t(communication_dev_t* comm_dev) {
	if (comm_dev == NULL) {
		return ERROR;
	}
	g_communication_handler = comm_dev;

	return OK;
}

void show_communication_dev(void) {
	communication_dev_t *comm_dev = g_communication_handler;
	if (comm_dev) {
		printf("communication dev : %s\n", comm_dev->name);
	}
	else {
		printf("no communication available \r\n");
	}
}

communication_dev_t* get_communication_dev(void) {
	return g_communication_handler;
}

int register_communication_module(void) {

	if (select_communication_dev_and_install_driver() != OK){
		return ERROR;
	}

	return init_communication_dev_lock();
}


void lock_communication_dev(void){
	
	if ((g_communication_handler == NULL) ||( g_communication_handler->lock == NULL))
	{
		return;
	}
	
	xSemaphoreTake(g_communication_handler->lock,portMAX_DELAY);
}

void unlock_communication_dev(void){

	if ((g_communication_handler == NULL) ||( g_communication_handler->lock == NULL))
	{
		return;
	}
	xSemaphoreGive(g_communication_handler->lock);
}

static int init_communication_dev_lock(void) {
	if(g_communication_handler ==  NULL){
		return ERROR;
	}

	g_communication_handler->lock = xSemaphoreCreateMutex();
	if (g_communication_handler->lock == NULL) {
		debug_printf("insufficient heap memory available,communication's lock init failed! "
			     "\r\n");
		return -ERROR;
	}

	return OK;
}

static int select_communication_dev_and_install_driver(void){
#ifdef GPRS
	return gprs_module_driver_install();
#else
	debug_printf("要在module_config.h文件里选择通信模块的类型 ！ \r\n");
	return ERROR;
#endif
}