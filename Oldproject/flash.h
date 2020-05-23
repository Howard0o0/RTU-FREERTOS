//////////////////////////////////////////////////////
//     �ļ���: flash.h
//   �ļ��汾: 0.0.0
//   ����ʱ��: 09�� 8��10��
//   ��������:  
//       ����: ����
//       ��ע: ��
//
//////////////////////////////////////////////////////
#pragma once 

#define BANK_D_START_ADDR       0x30000
#define BANK_D_END_ADDR         0x3FFFF
#define BANK_D_SIZE             0x10000
#define FLASH_SEGMENT_SIZE      512


void FLASH_EraseOneSEG ( unsigned long int Address ); //����flash,ÿ�β�ɾ��һ����

// *****************************************************
//�������Ƭ��FLASH����
//����waddr:�����λ��
//����value:�����������ָ��
//����size:����������Ĵ�С
// *****************************************************

void FLASH_EraseOneBank ( unsigned long int Address );//����flash,ÿ�β�ɾ��һ��BANK

void FLASH_Writew ( unsigned long int Address,unsigned int nValue ); //д��һ���ֽ�

// *****************************************************
//������д�����ݵ�Ƭ��FLASH
//����Address:���������λ��
//����value:�����ָ��
//����size:����Ĵ�С
//�Ȳ�����д
// *****************************************************
void Write_flash_Segment ( unsigned long int Address,  char* value,  int size );
int Read_Flashw ( unsigned long int waddr ); //��ȡһ���ֽ�


void Write_Flash_Erase_Segment ( unsigned long int Address,  char* value,  int size );
void Write_Flash_Segment ( unsigned long int Address,  char* value,  int size );
void Read_Flash_Segment(unsigned long int Address, char* value, int size);