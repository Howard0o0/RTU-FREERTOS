/*
 * author   :   howard
 * date     :   2019/12/30
 * Desc     :
 */

#ifndef __COMMUNICATION_OPR_H
#define __COMMUNICATION_OPR_H

#include "FreeRTOS.h"
#include "semphr.h"


typedef struct communication_dev_t {

	char* name;
	SemaphoreHandle_t lock;

	int (*power_on)(void);
	int (*power_off)(void);
	int (*sleep)(void);
	int (*wake_up)(void);
	int (*send_msg)(char* pSend, int sendDataLen, int isLastPacket, int center);
	char* (*rcv_msg)(void);
	
} communication_dev_t;

/******Interfaces for up class **************************/

/*!
 * @brief : an IO device driver call this functin to register into Dev NodeList
 *
 * @param[in] ptIODev  : Structure instance of a specific device.
 *
 * @retval zero -> Success / other value -> Error.
 */
int register_communication_dev_t(communication_dev_t *comm_dev);

/*!
 * @brief : show IO device available
 */
void show_communication_dev(void);

/*!
 * @brief : get a IO device
 *
 * @retval NULL -> appointed device is not registed / other value -> address of device's handler.
 */
communication_dev_t *get_communication_dev(void);

int register_communication_module(void);

#endif