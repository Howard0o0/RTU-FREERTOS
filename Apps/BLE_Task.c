#include "BLE_Task.h"


SemaphoreHandle_t xSemaphore_BLE;


void BLE_RE()
{
    xSemaphore_BLE = xSemaphoreCreateMutex();
    pBLE_Dev  ptDevBle =  getIODev();
    while(1){
        xSemaphoreTake(xSemaphore_BLE,portMAX_DELAY);
        if ( s_uart1_type==UART1_BT_TYPE ){
            // printf("1\r\n");
            if ( ptDevBle->isinit() ){
                // printf("2 \r\n");
                if ( ptDevBle->isspp() == 0 ){
                    // printf("3 \r\n");
                    if( !ptDevBle->isConnect() ){
                        // printf("4 \r\n");
                        ptDevBle->adv();
                    }else{
                        // printf("5 \r\n");
                        if( ptDevBle->open() ){
                            ptDevBle->init();
                        }
                    }
                }else{
                    vTaskDelay(10000 / portTICK_PERIOD_MS);
                    ptDevBle->close();
                }
                
            }
            else 
            {
                // printf("6 \r\n");
                ptDevBle->init();
            }
                
        
        }
        xSemaphoreGive(xSemaphore_BLE);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}