/*
 * author :  howard
 * date :  2019/12/30
 * desc :
 * how to use : register_communication_module(); -> get_communication_dev(); ->
 * 		lock_communication_dev(); -> 设备操作 -> unlock_communication_dev();
 * 
 * 
 * NOTE	:
 * 在对通信模块进行任何操作前都要先对设备上互斥锁lock_communication_dev，避免线程并发使用，使用完设备后要释放锁unlock_communication_dev，这样别的线程才能使用
 */

#ifndef __COMMUNICATION_OPR_H
#define __COMMUNICATION_OPR_H

#include "FreeRTOS.h"
#include "semphr.h"

typedef struct communication_dev_t {

	char*		  name;
	SemaphoreHandle_t lock;

	int (*power_on)(void);
	int (*power_off)(void);
	int (*sleep)(void);
	int (*wake_up)(void);
	int (*send_msg)(char* pSend, int sendDataLen, int isLastPacket, int center);
	char* (*rcv_msg)(void);

} communication_dev_t;

enum return_code { ERROR = -1, OK = 0 };

/******Interfaces for up class **************************/

int register_communication_module(void);

void lock_communication_dev(void);

void unlock_communication_dev(void);
/*!
 * @brief : get a IO device
 *
 * @retval NULL -> appointed device is not registed / other value -> address of device's handler.
 */
communication_dev_t* get_communication_dev(void);

/*!
 * @brief : an IO device driver call this functin to register into Dev NodeList
 *
 * @param[in] ptIODev  : Structure instance of a specific device.
 *
 * @retval zero -> Success / other value -> Error.
 */
int register_communication_dev_t(communication_dev_t* comm_dev);

/*!
 * @brief : show IO device available
 */
void show_communication_dev(void);

#endif