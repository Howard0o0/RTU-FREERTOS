#include "FreeRTOS.h"
#include "ParTest.h"
#include "semphr.h"
#include "ioDev.h"
#include "uart1.h"

extern int s_uart1_type;  
void BLE_RE();