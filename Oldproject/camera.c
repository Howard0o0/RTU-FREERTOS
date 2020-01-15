//////////////////////////
//���ߣ�������
//ʱ�䣺19.10
//���ܣ����㡢�洢������ͼƬ
//////////////////////////

#include "camera.h"
#include "msp430common.h"
#include "common.h"
#include "uart2.h"
#include "uart0.h"
#include "uart3.h"
#include "string.h"
#include "flash.h"
#include "message.h"
#include "hydrologycommand.h"
#include "hydrologytask.h"
#include "rtc.h"
#include "memoryleakcheck.h"
#include "communication_opr.h"
//����ͷָ���еĳ���
const static char reso = RESOLUTION1280_1024;             //�ֱ���

//��������ͷ���ص���Ϣ
char ack[4] = {0,};                                        //���ڴ洢ack֡
char pic_info[10]={0,};                                    //���ڴ洢���ص�ͼƬ��Ϣ֡
char *pic_packet;                                          //���ڴ洢ͼƬ����֡�е�����
char device_id;                                            //����ͷid

//����ͼƬ�����õ���ȫ�ֱ���
uint8_t   pack_num_sd;                                      //����ͼƬ�İ����� 
char serial_num = 0x00;                                    //��ǰ�����к�
char send_time[6] = {0,};                                  //����ʱ��
char shoot_time[5] = {0,};                                 //����ʱ��

extern void convertSampleTimetoHydrology(char* src,char* dst);
bool last_packet ;
cameratype camera_i, camera_ii;

//��������ָ��
void camera_TakeCmdSend(char device )
{
  uint16_t size = PAC_SIZE;
  char *pack_size;                                        //����ÿ��ͼƬ����
  pack_size = (char*) (&size);
  char take_cmd [TAKECMD_LEN]= {0x55,0x48,0x00,0x00,0x00,0x00,0x23};
  
  take_cmd[2] = device;
  take_cmd[3] = reso;
  take_cmd[4] = pack_size [0];
  take_cmd[5] = pack_size [1];
  
  UART2_Send(take_cmd,TAKECMD_LEN,0);
  TraceHexMsg(take_cmd,TAKECMD_LEN);
}
//����ȡ��ָ��
void camera_GetCmdSend(char *pack_cnt,char device )
{
  char get_cmd [GETCMD_LEN]= {0x55,0x45,0x00,0x00,0x00,0x23};
  get_cmd[2] = device;
  get_cmd[3] = pack_cnt [0];
  get_cmd[4] = pack_cnt [1];
  UART2_Send(get_cmd,GETCMD_LEN,0);
  TraceHexMsg(get_cmd,GETCMD_LEN);
}
//��ȡack
int camera_AckRead()
{
  int num=0;
  if (UART2_RecvLineLongWait(ack,ACK_LEN,&num)<0)
    return -1; 
  //TraceHexMsg(ack,ACK_LEN);
  return 0;
}
//��ȡͼ����Ϣ���ݰ�
int camera_InfoRead()
{
  int num=0;
  char pic_size[4] = {0,};                                   //���ڴ洢ͼƬ��Ϣ֡�е�ͼƬ��С
  cameratype *camera;
  
  if (UART2_RecvLineLongWait(pic_info,INFO_LEN,&num)<0)
    return -1;
  TraceHexMsg(pic_info,INFO_LEN);
  
  //����Ϣ֡�л�ȡͼƬ�����Ϣ
  pic_size[0] = pic_info[3];
  pic_size[1] = pic_info[4];
  pic_size[2] = pic_info[5];
  pic_size[3] = pic_info[6];
  
  device_id = pic_info[2];

  switch (device_id)
  {
    case 0 : {camera = &camera_i;break;}
    case 1 : {camera = &camera_ii;break;}
    default:{camera = &camera_i;break;}
  }
  for(int i = 3;i >= 0;i--)                             
  {//pic_size ����ת�浽int������
    camera->picture_len <<=8;
    camera->picture_len += (uint8_t)pic_size[i];   
  }
  camera->last_len = camera->picture_len % PAC_SIZE;
  //��ȡ�ְ�����
  camera->pack_num_rc = (uint8_t)pic_info[8];
  camera->pack_num_rc <<= 8;
  camera->pack_num_rc += (uint8_t)pic_info[7];
  
  switch (device_id)
  {
    case 0 : 
      {
        FLASH_EraseOneBank (BANK_D_START_ADDR);                       //��������BANK D
        camera_ii.addr_r=camera_ii.addr_w += camera_i.picture_len;
        break;
      }
    case 1 : 
      { 
        long extra_bytes = camera_i.picture_len+camera_ii.picture_len-BANK_D_SIZE;
        if(extra_bytes > 0)
        {
          TraceMsg("BANK_D IS NOT BIG ENOUGH!",1);
          
          printf("picture1's length is %ldB",camera_i.picture_len);
          printf("picture2's length is %ldB",camera_ii.picture_len);
          for (int i=1;i<=extra_bytes;i+=64)
            FLASH_EraseOneSEG(BANK_D_END_ADDR+i);
        }
        break; 
      }
    default: ;
  }
    
  return 0;
}

//��ȡͼ�����ݰ�
int camera_DatapackRead(char device)
{
  int num=0;
  int len=PAC_MAXSIZE;
  cameratype *camera;
  
  switch (device)
  {
    case 0 : {camera = &camera_i;break;}
    case 1 : {camera = &camera_ii;break;}
    default:{camera = &camera_i;break;}
  }
  if (last_packet) 
    len = camera->last_len+REST_SIZE;
  if (UART2_RecvLineLongWait(pic_packet,len,&num)<0)
    return -1;
  //TraceHexMsg(pic_packet,len);
  return len;
}

//��������ָ���ȡ������Ϣ
int camera_PictureTake(char device)
{ 
  camera_TakeCmdSend(device);
  //���ack��info
  if (camera_AckRead()<0)return -1;
  if(ack[1] != 0x48) return -1;
  if (camera_InfoRead()<0)return -1;
  if(pic_info[1] != 0x52) return -1;
  
  return 0;
}

//��ȡһ���ְ����洢
int camera_PictureGet(unsigned long addr, char device)
{ 
  cameratype *camera;
  switch (device)
  {
    case 0x00 : {camera = &camera_i;break;}
    case 0x01 : {camera = &camera_ii;break;}
    default:{camera = &camera_i;break;}
  }
  int len;
  camera_GetCmdSend((char *)(&camera->count), device);
  
  if (camera_AckRead()<0) return -1;
  if(ack[1] != 0x45) return -1;
  
  len = camera_DatapackRead(device);//��ȡ pic_packet ��pic_bytes ֵ
  if (len <0) return -1;                                                 
  if (pic_packet[1] != 0x46 ) return -1;
  
  Write_flash_Segment (addr,&pic_packet[7],len-REST_SIZE);
  return 0;
}

//�������
//��װ����ͷ��
int camera_PicturePacketHead(char *head,int pic_len,bool fstflg)        
{
  char frame_start[2] = {SOH1,SOH2};                                     //֡��ʼ��
  uint16_t packetlen_int ;                                               //���ĳ���
  if (fstflg)                                                            //ֻ�е�һ�����ĳ����� PACKET_INFO_LEN
    packetlen_int = pic_len + PACKET_INFO_LEN +3;
  else
    packetlen_int = pic_len + 3;
  char *packet_len;                                                   
  char packet_start = SYN;                                              //������ʼ��
  char pac_totalnum_serialnum[3];                                       //M3ģʽ�µİ����������к�                   
  
  packet_len = (char*)(&packetlen_int);                                 //���ĳ���
  
  pac_totalnum_serialnum[0] = (pack_num_sd & 0xf0)>>4;
  pac_totalnum_serialnum[1] = (pack_num_sd & 0x0f)<<4; 
  pac_totalnum_serialnum[2] = ++serial_num;
  
  int index = 0;
  head[index++] = frame_start[0];
  head[index++] = frame_start[1];
  
  if (Hydrology_ReadStoreInfo(HYDROLOGY_CENTER_ADDR,&head[index],HYDROLOGY_CENTER_LEN-3)<0)     //������վ��ַ
    return -1;   
  index += (HYDROLOGY_CENTER_LEN-3);
  if (Hydrology_ReadStoreInfo(HYDROLOGY_REMOTE_ADDR,&head[index],HYDROLOGY_REMOTE_LEN)<0)       //��ң��վ��ַ
    return -1;     
  index += HYDROLOGY_REMOTE_LEN;
  if (Hydrology_ReadStoreInfo(HYDROLOGY_PASSWORD_ADDR,&head[index],HYDROLOGY_PASSWORD_LEN)<0)   //������
    return -1; 
  index += HYDROLOGY_PASSWORD_LEN;
  head[index++] = Picture;                                                                      //ͼƬ������
  head[index++] = packet_len[1];
  head[index++] = packet_len[0];
  head[index++] = packet_start; 
  head[index++] = pac_totalnum_serialnum[0];
  head[index++] = pac_totalnum_serialnum[1];
  head[index++] = pac_totalnum_serialnum[2];
  return 0;
}
//��װ������Ϣ
int camera_PicturePacketInfo(char *packet_info)
{
  char ser_num [2];                                                                     //��ˮ��
  char remoteaddr_id = 0xF1;                                                            //ң��վ��ַ��ʶ��
  char observationtime_id = 0xF0;                                                       //�۲�ʱ��Ҫ�ر�ʶ��
  char picture_id = 0xF3;                                                               //ͼƬҪ�ر�ʶ��
  
  int index= 0;
  getstreamid(ser_num);                                                                 //������ˮ��
  packet_info[index++] = ser_num[0];
  packet_info[index++] = ser_num[1];
  
  memcpy(&packet_info[index],send_time,6);                                              //��װ����ʱ��
  index += 6;
  
  packet_info[index++] = remoteaddr_id;
  packet_info[index++] = remoteaddr_id;                                                 //ң��վ��ַ��ʶ��
  
  if (Hydrology_ReadStoreInfo(HYDROLOGY_REMOTE_ADDR,&packet_info[index],HYDROLOGY_REMOTE_LEN)<0) //��ң��վ��ַ
    return -1;  
  index += HYDROLOGY_REMOTE_LEN;
  
  if (Hydrology_ReadStoreInfo(HYDROLOGY_RTUTYPE,&packet_info[index++],HYDROLOGY_RTUTYPE_LEN)<0) //��ң��վ������
    return -1; 
  
  packet_info[index++] = observationtime_id;
  packet_info[index++] = observationtime_id;                                                    //�۲�ʱ��Ҫ�ر�ʶ��
  
  memcpy(&packet_info[index],shoot_time,5);                                                     //��װ�۲�ʱ��
  index += 5;
  
  packet_info[index++] = picture_id;
  packet_info[index++] = picture_id;                                                            //ͼƬҪ�ر�ʶ��
  return 0;
}

//��װ����
void camera_PicturePacketElement(long addr,int pic_len,char *element)//pic_len ��ͼƬ���ݳ���
{
  int i = 0;
  while (i < pic_len)
    element[i++] = Read_Flashw (addr++);                              //��ͼƬҪ�� 
}

//��װ����
int camera_PicturePacket(long addr,int pic_len,bool lstflg,bool fstflg)//�β� pic_len ͼƬ���ݳ��ȣ� lstflg �Ƿ�Ϊ���һ����־
{
  //��������
  char head[PACKET_HEAD_LEN]={0,};                                  //����ͷ��
  char packet_info[PACKET_INFO_LEN]={0,};                           //����ͷ��
  char element[PACKET_ELEMENT_LEN]={0,};                            //Ҫ��
  char up_picture_packet[PACKET_PACKET_LEN];                        //����
  
  char packet_end;                                                  //���Ľ�����
  short CRC;                                                        //У����
  int index = 0;

  if (camera_PicturePacketHead(head,pic_len,fstflg)<0)              //��װ����ͷ��
    return -1;               
  memcpy(up_picture_packet,head,PACKET_HEAD_LEN);
  index += PACKET_HEAD_LEN;
  if (fstflg)                                                       //ֻ�е�һ����Ҫ������Ϣ
  {
    if (camera_PicturePacketInfo(packet_info)<0)                    //��װ����ͷ��
      return -1;
    memcpy(&up_picture_packet[index],packet_info,PACKET_INFO_LEN);
    index += PACKET_INFO_LEN;
    fstflg = false;                                                //��һ����־���
  }
  
  camera_PicturePacketElement(addr,pic_len,element);               //��װͼƬҪ��
  
  memcpy(&up_picture_packet[index],element,pic_len);   
  index += pic_len;
  //������������
  if (lstflg) packet_end =ETX;                                    //���һ���Ľ�����
  else packet_end =ETB;                                           //��ʾ�������а�
  
  up_picture_packet[index++] = packet_end;
  CRC = hydrologyCRC16(up_picture_packet,index);
  
  up_picture_packet[index++] = CRC >> 8;
  up_picture_packet[index++] = CRC & 0xFF;
  
  //����һ֡����
  communication_module_t* comm_dev = get_communication_dev();
  int i = 2;
  while (comm_dev->send_msg(up_picture_packet,index,0, HYDROLOGY_CENTER1_IP ) == FALSE && i>0) i--;
  if (i<=0) return -1;

  TraceHexMsg(up_picture_packet,index);
  return 0;
}

//���㲢�洢ͼƬ
int camera_Take(char device)
{
  UART2_Open(1);
  //UART0_Open(1);
  char shoot_time_DEC[5] = {0,};
  RTC_ReadTimeBytes5(shoot_time_DEC);
  convertSampleTimetoHydrology(shoot_time_DEC,shoot_time);      //��ȡ����ʱ��
  
  int i = 4;
  while (camera_PictureTake(device)<0 && i>0)
    i--;
  if (i==0) return -1;

  pic_packet = (char*)os_malloc(PAC_MAXSIZE); 
  
  cameratype *camera;
  switch (device)
  {
    case 0 : {camera = &camera_i;break;}
    case 1 : {camera = &camera_ii;break;}
    default:{camera = &camera_i;break;}
  }

  while (camera->count < camera->pack_num_rc) //�����а�û�л�ȡʱѭ��
  { 
    camera->count++;
    if (camera->count >= camera->pack_num_rc) 
      last_packet = true;
    else
      last_packet = false;
    
    int j = 4;
    while (camera_PictureGet(camera->addr_w,device)<0 && j>0)
      j--;
    if (j==0)  
    {
      os_free(pic_packet);
      return -1;
    }
    camera->addr_w += PAC_SIZE;
  }                                 

  os_free(pic_packet);
  UART2_Close();
  return 0;
}

//����ͼƬ����
int camera_PacketSend(char device)
{
  Hydrology_ReadTime(send_time);                        //��ȡ����ʱ��
  cameratype *camera;
  serial_num = 0;
  switch (device)
  {
    case 0x00 : {camera = &camera_i;break;}
    case 0x01 : {camera = &camera_ii;break;}
    default:{camera = &camera_i;break;}
  }
  const int rest_bytes = camera->picture_len % READ_LEN;       //���һ�����ݵĳ���
  int cnt;
 
  pack_num_sd = camera->picture_len / READ_LEN;                 //��������
  cnt = pack_num_sd; 
  if(rest_bytes != 0) pack_num_sd++;                    //����������Ҫ�������һ��
  
  while (cnt-->0)
  {
   if (camera_PicturePacket(camera->addr_r,READ_LEN,false,camera->fstflg)<0) 
     return -1;  
   if(camera->fstflg) 
     camera->fstflg = false;
   camera->addr_r += READ_LEN;
  }
  if (camera_PicturePacket(camera->addr_r,rest_bytes,true,camera->fstflg)<0) 
   return -1;
  
  return 0;
}

//��ʼ��ȫ�ֱ�����ÿ������ǰ����Ҫ���õ�һЩ������
void camera_Init()
{  
  P10DIR |= BIT2;
  camera_i.count  = camera_ii.count = 0;
  camera_i.fstflg = camera_ii.fstflg = true;
  camera_i.addr_r = camera_ii.addr_r = BANK_D_START_ADDR;
  camera_i.addr_w = camera_ii.addr_w = BANK_D_START_ADDR;
}

char camera_change(char device)
{
  switch (device)
  {
    case 0: 
    {
      P10OUT |= BIT2;//p102�߾�ѡͨJ14 1 3 5����
      break;
    }
    case 1: 
    {
      P10OUT &=~ BIT2;//p102�;�ѡͨJ14 7 9 11����
      break;
    }
    default: 
    {
      P10OUT &=~ BIT2;
      break;
    }
  }
  return device;
}

void camera()
{
  camera_Init();
  
  if (camera_Take(camera_change(0))<0 || camera_Take(camera_change(1))<0) 
    TraceMsg("Camera Write Failed!",1);
  else {
     TraceMsg("A",1);
    if (camera_PacketSend(0)<0 || camera_PacketSend(1)<0) 
      TraceMsg("Camera Send Failed!",1);
}
  //System_Delayms(5000);                                  //��ʱ5s

}