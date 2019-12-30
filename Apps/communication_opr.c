#include "common.h"
#include "communicaiton_opr.h"
#include "gprs.h"
#include "module_config.h"
#include <string.h>

static communication_dev_t g_communication_handler = NULL;

int register_communication_dev_t(communication_dev_t* comm_dev) {
	if (comm_dev == NULL) {
		return -1;
	}
	g_communication_handler = comm_dev;

	return 0;
}

void show_communication_dev(void) {
	communication_dev_t comm_dev = g_communication_handler;
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
#ifdef GPRS
	return gprs_module_driver_install();

#else
	debug_printf("要在module_config.h文件里选择通信模块的类型 ！ \r\n");
	return -1;

#endif
}
