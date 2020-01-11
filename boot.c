#include "boot.h"
#include "blueTooth.h"
#include "communication_opr.h"
#include "common.h"
#include "msp430common.h"
#include <stdio.h>

void install_io_devs_driver(void) {
	install_communication_module_driver();

	UART1_Open(1);
	BleDriverInstall();
}

static void boot_communication_module(void) {

	printf("%s module is powering on , waiting for about 42s...\n",
	       get_communication_dev()->name);
	get_communication_dev()->power_on();
}

static void check_communication_module(void) {

	communication_module_t* comm_module = get_communication_dev();
	if (comm_module->check_if_module_is_normal() == OK) {
		printf("[OK] %s module\n", comm_module->name);
	}
	else {
		printf("[ERROR] %s module not respond\n", comm_module->name);
	}
}

static void check_rtc_module(void){
	if(check_if_rtc_normal() == TRUE){
		printf("[OK] rtc module \n");
	}
	else
	{
		printf("[ERROR] rtc module \n");
	}
	
}
void bios_check(void) {

	boot_communication_module();

	check_communication_module();
	check_rtc_module();
}