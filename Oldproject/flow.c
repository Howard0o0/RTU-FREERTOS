#include "msp430common.h"
#include "uart0.h"
#include "uart1.h"
#include "common.h"
#include "rom.h"
#include "flash.h"
#include "rtc.h"
#include "store.h"
#include "Sampler.h"
#include "flow.h"
#include "debug.h"
#include "message.h"
#include "hydrologycommand.h"
#include "FreeRTOS.h"

int FlowCheckSampleData(int* pstart,int* pend)
{
    //������ݴ洢�ϱ�� �±� 
    int _startIdx=*pstart;
    int _endIdx=*pend;
    
    if(Hydrology_ReadStartIdx(&_startIdx)<0 || Hydrology_ReadEndIdx(&_endIdx)<0 )
    {//������������������ʧ��
        TraceMsg("read idx error ,reGenerate .",1);
        
        if(Hydrology_RetrieveIndex()<0)                                        //�����������ʧ�ܣ���Ҫ�����������++++++++++++++++++++++
        {
            TraceMsg("reGen failed .",1);
            Hydrology_SetStartIdx(HYDROLOGY_DATA_MIN_IDX);
            Hydrology_SetEndIdx(HYDROLOGY_DATA_MIN_IDX);
        }
        TraceMsg("StartIdx:",0); 
        TraceInt4(_startIdx,1);
        TraceMsg("EndIdx:",0);
        TraceInt4(_endIdx,1);
        Hydrology_ReadStartIdx(&_startIdx);//���¶���
        Hydrology_ReadEndIdx(&_endIdx);//���¶���
    }
    //�±���ȷ��
    if( _endIdx<HYDROLOGY_DATA_MIN_IDX || _endIdx >HYDROLOGY_DATA_MAX_IDX || _startIdx<HYDROLOGY_DATA_MIN_IDX || _startIdx >HYDROLOGY_DATA_MAX_IDX)
    {
        TraceMsg("Idx bad .",1);
        return -1;
    }
    TraceMsg("startIdx=",1);
    TraceInt4(_startIdx,1);
    TraceMsg("endIdx=",1);
    TraceInt4(_endIdx,1);

      *pstart = _startIdx;
    *pend = _endIdx;
    
    return 0;
}

int FlowProcess()    //δʹ��  
{
    //������ݴ洢�ϱ���±� 
    int _startIdx=0;
    int _endIdx=0;
    char _effect_endIdx=0;//ky,ǰ���_endIdx�ĵ�ַ�ڵ��������Ѿ������˵ģ������ٶ���һ����ַ���Ǳ�����Ч���ݵ����һ����ַ
    char _send[120] = {0}; 
    char _specSend[200] = {0};
    int  _ret=0;   
    int  _seek_num=0;//��ֹ��ѭ��
    int sendlen = 0;
   
    _ret = FlowCheckSampleData(&_startIdx,&_endIdx);           //���startidx endidx
    if (_ret !=0 )
    {
        return -1;   
    }
    
    if(_endIdx == 1)
    {
        _effect_endIdx = 20;
    }
    else
    {
        _effect_endIdx = _endIdx-1;
    }

    while(1)
    {
        TraceMsg("read data in :",0);
        TraceInt4(_startIdx,1);
        if(_seek_num > DATA_ITEM_LEN)//Ѱ�ҵ����������Ѿ��������ֵ���˳�     //+++Ҫ��Ϊ��ַ�߽��ж�
        {
            TraceMsg("seek num out of range",1);
			//hydrologHEXvPortFree();
                        System_Delayms(2000);
			System_Reset();
            //break;
        }

        //����
        _ret = Store_ReadDataItem(_startIdx,_send,0);                           //+++++��ȡ����+++��startIDx++++retΪ���������ݳ���
        if(_ret<0)
        {
            TraceMsg("can't read data ! very bad .",1);
            return -1; //�޷���ȡ���� ��ֱ������.
        }
        else if(_ret==1)
        {//�����һ���Ѿ����͹�������,                                          //+++++++++++
            TraceMsg("It's sended data",1);
          //  if(_startIdx == _endIdx)
           // {//����Ƿ���  _endIdx, ����ǾͲ�����ѭ����. 
               // _ret = hydrologHEXProcess(_send,42,TimerReport,_specSend,0);
				
               // break;
           // }
            //������һ��λ��
            if(_startIdx >= HYDROLOGY_DATA_MAX_IDX)                     //+++++�����ȡ��startidx�����ɴ�����index������������
                _startIdx=HYDROLOGY_DATA_MIN_IDX;
            else   
                ++ _startIdx;//��һ����

            ++_seek_num;
			
                                                              //�������,Ҫ����_startIdx.          
			Hydrology_SetStartIdx(_startIdx);
			
			TraceInt4(_startIdx, 1);
			TraceInt4(_endIdx, 1);
			
			hydrologHEXvPortFree();

            //continue;
        }
        //�������ķ�������,������_idx
        //������һ�ε� _startIdx   

	else //δ���͵����� //ÿ�����ݶ�Ҫ����ͻ��˷����������飬�ڷ��͸����ط����� 
		{
	        sendlen = _ret;

	        if (_startIdx == _effect_endIdx)   //����ʧ�ܾ�����ѭ��    
	        {
				Store_MarkDataItemSended(_startIdx);      
				_ret = hydrologHEXProcess(_send,_ret,TimerReport,_specSend,0);//�������һ�����ݣ�����ͻ��˷������򱾵ط�++++++++++++++++

				if(_ret !=0)
				{
					break;
				}
	        }
			
	        else
	        { 
				_ret = hydrologHEXProcess(_send,_ret,TimerReport,_specSend,1);//����һ�����ݣ���ͻ��˷�+++++++++++++
				
				if(_ret !=0)
	            {
	                break;
	            }	
	        }

	        //�����ø������ѷ���
	        Store_MarkDataItemSended(_startIdx);

	        if(_startIdx>=HYDROLOGY_DATA_MAX_IDX) 
	            _startIdx=HYDROLOGY_DATA_MIN_IDX;
	        else   
	            ++ _startIdx;//��һ����

	        ++_seek_num;

	        TraceMsg(_send, 1);

	        //�������,Ҫ����_startIdx. 
	        Hydrology_SetStartIdx(_startIdx);

	        TraceInt4(_startIdx, 1);
	        TraceInt4(_endIdx, 1);

	        hydrologHEXvPortFree();

		}
    }

    TraceMsg("Report done",1);

    return 0;
}


