/*
 * author   :   howard
 * date     :   2019/09/29
 * Desc     :   这是一个用于输入输出调试信息的抽象层，只要是用于输入输出调试信息的模块设备，都需要按照本抽象层实例化对象。
 *              底层驱动需要向ioDev注册，上层便可使用IODev的操作函数。驱动注册详情参考blueTooth.c的691行-最后一行
*/

#ifndef __IO_MANAGER_H
#define __IO_MANAGER_H

typedef struct BLEDev
{
    char *name;
    
    int (*init)(void);  // 0 success
    int (*open)(void);  // 0 success
    void (*write)(char *msgRecv,int *len);
    int (*read)(char *msgSend,int len);  // 0 success
    int (*close)(void); // 0 success

    int (*isConnect)(void);  //1 canUse, 0 can't use
    void (*restart)(void);
    int (*isinit)(void);
    int (*isspp)(void);
    void (*adv)(void);
}BLE_Dev,*pBLE_Dev;

/******Interfaces for up class **************************/

/*!
 * @brief : an IO device driver call this functin to register into Dev NodeList
 *
 * @param[in] ptIODev  : Structure instance of a specific device.
 *
 * @retval zero -> Success / other value -> Error.
 */
int RegisterIODev(pBLE_Dev ptIODev);

/*!
 * @brief : show IO device available
 */
void ShowIODevs(void);

/*!
 * @brief : get a IO device 
 *
 * @retval NULL -> appointed device is not registed / other value -> address of device's handler.
 */
pBLE_Dev getIODev(void);


#endif