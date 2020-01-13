//////////////////////////////////////////////////////
//     �ļ���: flash.c
//   �ļ��汾: 1.0.0
//   ����ʱ��: 09��11��30��
//   ��������:  
//       ����: ����
//       ��ע: ��
//
//////////////////////////////////////////////////////
//���ڶ�дƬ��flash
#include "msp430x54x.h"
#include "flash.h"

// *********************************************************
//����flash,ÿ�β�ɾ��һ���Σ�Ϊ512bt��Ҳ����0x200
//����index:�����λ��
//����value:�����������ָ��
//����size:����������Ĵ�
// *****************************************************
void FLASH_EraseOneSEG ( unsigned long int Address ) //����flash,ÿ�β�ɾ��һ����
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

//����flash,ÿ�β�ɾ��һ��bank
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
//��FLASH����,����һ���ֽ�����
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
//������д�����ݵ�Ƭ��FLASH
//����index:���������λ��
//����value:�����ָ��
//����size:����Ĵ�С
//�Ȳ�����д
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
