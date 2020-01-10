/*
 * author :  howard
 * date :  2019/12/30
 * desc :	接口的返回值参照枚举类型return_code
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
#include <stdint.h>

typedef struct time_struct {
	uint8_t year;
	uint8_t month;
	uint8_t date;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
} time_t;

enum return_code { ERROR = -1, OK = 0 };

typedef struct communication_module_t {

	char*		  name;
	SemaphoreHandle_t lock;

	int (*power_on)(void);	//0成功 其他失败
	int (*power_off)(void);	//0成功 其他失败
	int (*sleep)(void);	//0成功 其他失败
	int (*wake_up)(void);	//0成功 其他失败
	int (*check_if_module_is_normal)(void);	//0成功 其他失败
	int (*send_msg)(char* pSend, int sendDataLen, int isLastPacket, int center);	//0成功 其他失败
	char* (*rcv_msg)(void);	//非NULL成功 NULL失败
	time_t (*get_real_time)(void); //若获取时间失败，则将time_t.year置零

} communication_module_t;


/*================供上层应用调用接口=======================*/

/* 0成功 其他失败 
 * ！！！ 底层驱动开发人员需要维护该接口内的select_communication_dev_and_install_driver()接口 ！！！
*/
int install_communication_module_driver(void);

void lock_communication_dev(void);

void unlock_communication_dev(void);

/* 非NULL成功 NULL失败 */
communication_module_t* get_communication_dev(void);

void show_current_communication_dev(void);

void communication_module_unit_test(void);
/*==================================================================*/




/*===================供底层驱动调用接口================================*/

/* 0成功 其他失败 */
int register_communication_module(communication_module_t* comm_dev);

/*==================================================================*/



#endif