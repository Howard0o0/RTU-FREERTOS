#include "communication_opr.h"
#include "common.h"
#include "gprs.h"
#include "module_config.h"
#include <string.h>
#include "hydrologycommand.h"

static communication_module_t* g_communication_handler = NULL;

static int init_communication_dev_lock(void);
static int select_communication_dev_and_install_driver(void);

int register_communication_module(communication_module_t* comm_dev) {
	if (comm_dev == NULL) {
		return ERROR;
	}
	g_communication_handler = comm_dev;

	return OK;
}

void show_current_communication_dev(void) {
	communication_module_t* comm_dev = g_communication_handler;
	if (comm_dev) {
		printf("communication dev : %s\n", comm_dev->name);
	}
	else {
		printf("no communication available \r\n");
	}
}

communication_module_t* get_communication_dev(void) {
	return g_communication_handler;
}

int install_communication_module_driver(void) {

	if (select_communication_dev_and_install_driver() != OK) {
		return ERROR;
	}

	return init_communication_dev_lock();
}

void lock_communication_dev(void) {

	if ((g_communication_handler == NULL) || (g_communication_handler->lock == NULL)) {
		return;
	}

	xSemaphoreTake(g_communication_handler->lock, portMAX_DELAY);
}

void unlock_communication_dev(void) {

	if ((g_communication_handler == NULL) || (g_communication_handler->lock == NULL)) {
		return;
	}
	xSemaphoreGive(g_communication_handler->lock);
}

void communication_module_unit_test(void) {
	char* data_to_send = "haha";

	install_communication_module_driver();
	communication_module_t* comm_dev = get_communication_dev();
	if (comm_dev->power_on() == 0){
		printf("%s power on \r\n",comm_dev->name);
	}
	else{
		printf("%s power failed \r\n",comm_dev->name);
	}

	if(comm_dev->check_if_module_is_normal() == 0){
		printf("%s module normal \r\n",comm_dev->name);
	}
	else{
		printf("%s module is not normal \r\n",comm_dev->name);
	}

	while (1) {
		comm_dev->send_msg(data_to_send, strlen(data_to_send), 0, HYDROLOGY_CENTER1_IP );
		comm_dev->send_msg(data_to_send, strlen(data_to_send), 0, HYDROLOGY_BACKUP1_IP );
	}
}

static int init_communication_dev_lock(void) {
	if (g_communication_handler == NULL) {
		return ERROR;
	}

	g_communication_handler->lock = xSemaphoreCreateMutex();
	if (g_communication_handler->lock == NULL) {
		debug_printf("insufficient heap memory available,communication's lock init failed! "
			     "\r\n");
		return ERROR;
	}

	return OK;
}

static int select_communication_dev_and_install_driver(void) {
#if GPRS
	return gprs_module_driver_install();
#elif BC95
        return bc95_module_driver_install();
#else
	debug_printf("要在module_config.h文件里选择通信模块的类型 ！ \r\n");
	return ERROR;
#endif
}
