#include "gprs.h"
#include "common.h"
#include "msp430f5438a.h"
#include "uart0.h"
#include <stdbool.h>
#include <string.h>

static int check_gprs_module_is_normal();
static void uart0_init();
static void pull_up_pin_batt();
static void pull_down_pin_igt();
static void pull_up_pin_igt();
static void send_at_cmd(char* at_cmd);
static char* rcv_at_response();
static int check_gprs_module_is_normal();

#define BUF_LEN 50

/*
 * return : OK  ERR
 */
int gprs_power_on() {

	debug_printf("gprs is trying to power on , need to wait 42s ...");

	uart0_init();

	pull_up_pin_batt();
	pull_down_pin_igt();
	pull_up_pin_igt();

	return check_gprs_module_is_normal();
}

int gprs_power_off() {}

static void uart0_init() {
	UART0_Open(UART0_GTM900_TYPE);
}

static void pull_up_pin_batt() {
	P10DIR |= BIT7;
	P10OUT &= ~BIT7;
	P10OUT |= BIT7;
	System_Delayms(100);
}

static void pull_down_pin_igt() {
	P3OUT |= BIT0;
	System_Delayms(10);
	P3OUT &= ~BIT0;
	System_Delayms(50);
}

static void pull_up_pin_igt() {
	P3OUT |= BIT0;
	System_Delayms(1200);
	System_Delayms(30000);
}

static void send_at_cmd(char* at_cmd) {
	UART0_Send(at_cmd, strlen(at_cmd), 1);
}

static char* rcv_at_response() {
	char	 buff[ BUF_LEN ];
	int	  real_rcv_len = 0;

	memset(buff, 0, BUF_LEN);
	while (UART0_RecvLineWait(buff + real_rcv_len, BUF_LEN, &real_rcv_len) != -1)
		;

	if (strlen(buff) != 0) {
		return buff;
	}
	else {
		return NULL;
	}
}
static int check_gprs_module_is_normal() {

	char* at_rsp;

	send_at_cmd("AT+CGMI=?");
	at_rsp = rcv_at_response();

	if (strstr(at_rsp, "OK") != NULL) {
		return OK;
	}
	else {
		return ERR;
	}

}