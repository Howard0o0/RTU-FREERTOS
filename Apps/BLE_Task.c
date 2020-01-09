#include "BLE_Task.h"

#include "FreeRTOS.h"
#include "task.h"
#include "ParTest.h"
#include "semphr.h"

#include "ioDev.h"
#include "uart1.h"
#include "msp430common.h"
#include "blueTooth.h"
#include "uart3.h"
// #include "Console.h"
#include "common.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>



void BLE_RE(void* pvParameters)
{

    BleDriverInstall();
    pBLE_Dev  ptDevBle =  getIODev();
    
    while(1){

        // printf("BLE HWM:%d\r\n",uxTaskGetStackHighWaterMark(NULL)); 
        
        if(BLE_isexist() && (s_uart1_type == UART1_BT_TYPE)){
                if(ptDevBle->isinit() == 0){
                        ptDevBle->init();
                        vTaskDelay(100 / portTICK_PERIOD_MS);
                        continue;
                }

                if(ptDevBle->isspp() != 0){
                        vTaskDelay(10000 / portTICK_PERIOD_MS);
                        ptDevBle->close();
                        vTaskDelay(100 / portTICK_PERIOD_MS);
                        continue;
                }

                if(ptDevBle->isConnect() != 0){
                        if(ptDevBle->open()){
                            ptDevBle->init();    
                        }
                        vTaskDelay(100 / portTICK_PERIOD_MS);
                        continue;
                }

                ptDevBle->adv();

        }



        // if(BLE_isexist()){
        //        if ( s_uart1_type==UART1_BT_TYPE ){
        //         // printf("1\r\n");
        //                 if ( ptDevBle->isinit() ){
        //                         // printf("2 \r\n");
        //                         if ( ptDevBle->isspp() == 0 ){
        //                                 // printf("3 \r\n");
        //                                 if( !ptDevBle->isConnect() ){
        //                                         // printf("4 \r\n");
        //                                         ptDevBle->adv();
        //                                 }else{
        //                                         // printf("5 \r\n");
        //                                         if( ptDevBle->open() ){
        //                                         ptDevBle->init();
        //                                         }
        //                                 }
        //                         }else{
        //                         vTaskDelay(10000 / portTICK_PERIOD_MS);
        //                         ptDevBle->close();
        //                         }
                                
        //                 }
        //                 else 
        //                 {
        //                         // printf("6 \r\n");
        //                         ptDevBle->init();
        //                 }
        //         } 
        // }
        
        vTaskDelay(100 / portTICK_PERIOD_MS);

    }
}


