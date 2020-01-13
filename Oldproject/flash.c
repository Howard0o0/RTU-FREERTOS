//////////////////////////////////////////////////////
//     文件名: flash.c
//   文件版本: 1.0.0
//   创建时间: 09年11月30日
//   更新内容:  
//       作者: 林智
//       附注: 无
//
//////////////////////////////////////////////////////
//用于读写片内flash
#include "msp430x54x.h"
#include "flash.h"

// *********************************************************
//擦除flash,每次擦删除一个段，为512bt，也就是0x200
//参数index:数组的位置
//参数value:读出的数组的指针
//参数size:读出的数组的大
// *****************************************************
void FLASH_EraseOneSEG ( unsigned long int Address ) //擦除flash,每次擦删除一个段
{
	_DINT();
	FCTL3=FWKEY;								// LOCK = 0
	while ( ( FCTL3&BUSY ) ==BUSY );					// Waitint for FLASH
	FCTL1=FWKEY+ERASE;							// ERASE=1
	__data20_write_char ( Address,0 );                                      // Write to the SEGment
	FCTL1=FWKEY;
	FCTL3=FWKEY+LOCK;
	while ( ( FCTL3&BUSY ) ==BUSY );					// Waitint for FLASH
	_EINT();
}

//擦除flash,每次擦删除一个bank
void FLASH_EraseOneBank ( unsigned long int Address ) 
{
	_DINT();
	FCTL3=FWKEY;								// LOCK = 0
	while ( ( FCTL3&BUSY ) ==BUSY );					// Waitint for FLASH
	FCTL1=FWKEY+MERAS;							// MRASE=1
	__data20_write_char ( Address,0 );                                      
	FCTL1=FWKEY;
	FCTL3=FWKEY+LOCK;
	while ( ( FCTL3&BUSY ) ==BUSY );					// Waitint for FLASH
	_EINT();
}

/* *********************************************
//读FLASH操作,返回一个字节数据
************************************************/
int Read_Flashw ( unsigned long int waddr )
{
	int  value;
	
	while ( FCTL3 & BUSY );
	
	value = __data20_read_char ( waddr );
	
	return value;
}

/*******************************************************************************
// Write a word (nValue) to Address
*******************************************************************************/
void FLASH_Writew ( unsigned long int Address,unsigned int nValue )
{

	FCTL1=FWKEY+WRT;							// WRT = 1
	FCTL3=FWKEY;								// LOCK = 0
	
	while ( ( FCTL3&BUSY ) ==BUSY );					// Waitint for FLASH
	__data20_write_short ( Address,nValue );
	
	FCTL1=FWKEY;								// WRT = 0
	FCTL3=FWKEY+LOCK;							// LOCK = 1
	while ( ( FCTL3&BUSY ) ==BUSY );					// Waitint for FLASH
}

// *****************************************************
//把数组写入数据到片内FLASH
//参数index:保存数组的位置
//参数value:数组的指针
//参数size:数组的大小
//先擦除再写
// *****************************************************

void Write_flash_Segment ( unsigned long int Address,  char* value,  int size )
{
	int i;
	
	FCTL1=FWKEY+WRT;							// WRT = 1
	FCTL3=FWKEY;								// LOCK = 0
	
	for ( i = 0; i < size; i++ )
	{
		while ( ( FCTL3&BUSY ) ==BUSY );				// Waitint for FLASH
		__data20_write_char ( Address+i,value[i] );
	}
	
	FCTL1=FWKEY;								// WRT = 0
	FCTL3=FWKEY+LOCK;							// LOCK = 1
	while ( ( FCTL3&BUSY ) ==BUSY );					// Waitint for FLASH
}
