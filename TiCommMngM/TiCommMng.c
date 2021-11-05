/*********************************************************************************

//		��ʱ����(Time Interval)��Ϊ����֡�ж����ݵ��������-������ʱʵ��

//��ģ�������Ӳ����Ӧ��
*********************************************************************************/

#include "TiCommMng.h"
#include <string.h>

/***********************************************************************
                            ��غ���ʵ��
***********************************************************************/

//-----------------------------��ʼ������-------------------------------
//���ô˺���ǰ,���ʼ��UsartDev(��UsartId)����IO������Usart������������
void TiCommMng_Init(struct _TiCommMng *pMng,
                    struct _UsartDev *pDev)
{
  memset(pMng, 0, sizeof(struct _TiCommMng));
  pMng->Count = TiCommMng_cbBuadId2FremeOv(pMng);
  UsartTiny_Init(&pMng->UsartTiny, pDev);//��ʼ���ײ�ͨѶ
  //��Ӧ�������õײ�Ӳ��
}

//-------------------------------�ж�����----------------------------
//���˺�������1ms����жϽ�����
void TiCommMng_IntTask(struct _TiCommMng *pMng)
{
  if(pMng->Index) pMng->Index--; 
}

//-------------------------------��ͨ��ѯ����----------------------------
//���˺�������ϵͳ1ms������
void TiCommMng_Task(struct _TiCommMng *pMng)
{
  #ifdef SUPPORT_TI_COMM_MNG_PRE //����ʱ��ͣ����
    if(pMng->Flag & TI_COMM_MNG_SUSPEND) return;
  #endif  
  
  //����״̬��������
  if(pMng->UsartTiny.eState == UsartTiny_eState_Idie){
    UsartTiny_RcvStart(&pMng->UsartTiny);
    //ֹͣ�����շ�����
    pMng->Flag &= ~(TI_COMM_MNG_RCV_DOING | TI_COMM_MNG_SEND_DOING);
    
    return;
  }
  if(pMng->Index) return;//ʱ��δ�����ڽ��յȴ���  
  //û�ڷ��ͻ���յȴ�������
  if(!(pMng->Flag & (TI_COMM_MNG_RCV_DOING | 
                          TI_COMM_MNG_SEND_DOING))){
     if(pMng->UsartTiny.eState != UsartTiny_eState_Rcv){//�쳣�����ý���
       UsartTiny_RcvStart(&pMng->UsartTiny);
     }       
     return;
  }
  
  //���ݽ������
  if(pMng->Flag & TI_COMM_MNG_RCV_DOING){
    UsartTiny_Stop(&pMng->UsartTiny); //ǿ����ֹ����
    pMng->Flag &= ~TI_COMM_MNG_RCV_DOING;//ֹͣ���ݽ���
    unsigned char Resume;
    #ifdef SUPPORT_TI_COMM_MNG_PRE 
      Resume = TiCommMng_cbRcvPreDataPro(pMng);
      if(Resume == 254){//��ͣ����
        UsartTiny_Stop(&pMng->UsartTiny);
        return;
      }
      if(Resume == 255) Resume = TiCommMng_cbDataPro(pMng);//��ʽ���ݴ���
    #else
      Resume = TiCommMng_cbDataPro(pMng);//���ݴ���
    #endif
     if((Resume != 0) && (Resume != 255)){ //������ȷ,��������
       UsartTiny_SendStart(&pMng->UsartTiny, Resume);
       pMng->Flag |= TI_COMM_MNG_SEND_DOING;//��������
       pMng->Index = pMng->Count;//���㷢�ͳ�ʱ
     }
  }
  else{//���ͳ�ʱ�����
    UsartTiny_Stop(&pMng->UsartTiny);
  }
}

/******************************************************************************
		                      ֧��Ԥ����ʱ���
******************************************************************************/ 
#ifdef SUPPORT_TI_COMM_MNG_PRE

//-------------------------------������--------------------------------
//����󣬿ɲſ�ֱ��ʹ�û����������������TiCommMng_PreInsertSend()���ܽ⿪
void TiCommMng_Suspend(struct _TiCommMng *pMng)
{
  UsartTiny_Stop(&pMng->UsartTiny); //ǿ����ֹ
  pMng->Flag &= ~(TI_COMM_MNG_RCV_DOING |TI_COMM_MNG_SEND_DOING);//ֹͣ����
  pMng->Flag |= TI_COMM_MNG_SUSPEND;//����
}

//----------------------------ǿ�Ʋ��뷢�ͺ���--------------------------------
void TiCommMng_InsertSend(struct _TiCommMng *pMng,
                          unsigned char SendLen) //�������ݳ���
{
  pMng->Flag &= ~TI_COMM_MNG_SUSPEND;//������ȡ������
  if(!SendLen) return; //������Ҫ����
  
  UsartTiny_Stop(&pMng->UsartTiny);//��ֹͣ
  UsartTiny_SendStart(&pMng->UsartTiny, SendLen);
  pMng->Flag |= TI_COMM_MNG_SEND_DOING;//��������
  pMng->Index = pMng->Count;//���㷢�ͳ�ʱ
}

#endif //SUPPORT_TI_COMM_MNG_PRE
