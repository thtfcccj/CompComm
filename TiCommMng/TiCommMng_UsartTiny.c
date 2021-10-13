/*********************************************************************************

	��ʱ����(Time Interval)��Ϊ����֡�ж����ݵ��������-ʹ��UsartTinyʱʵ��

//��ģ�������Ӳ����Ӧ��
*********************************************************************************/

#include "TiCommMng.h"
#include "UsartTiny.h"
#include <string.h>

struct _TiCommMng TiCommMng;
               
/***********************************************************************
                            ��غ���ʵ��
***********************************************************************/

//---------------------------------��ʼ������----------------------------
void TiCommMng_Init(void)
{
  memset(&TiCommMng, 0, sizeof(struct _TiCommMng));
  TiCommMng.Count = TiCommMng_cbBuadId2FremeOv();
  UsartTiny_Init();//��ʼ���ײ�ͨѶ
  //��Ӧ�������õײ�Ӳ��
}

//-------------------------------��ͨ��ѯ����----------------------------
//���˺�������ϵͳ1ms������
void TiCommMng_Task(void)
{
  #ifdef SUPPORT_TI_COMM_MNG_PRE //����ʱ��ͣ����
    if(TiCommMng.Flag & TI_COMM_MNG_SUSPEND) return;
  #endif  
  
  //����״̬��������
  if(UsartTiny.eState == UsartTiny_eState_Idie){
    UsartTiny_RcvStart();
    //ֹͣ�����շ�����
    TiCommMng.Flag &= ~(TI_COMM_MNG_RCV_DOING | TI_COMM_MNG_SEND_DOING);
    return;
  }
  //û�ڷ�������չ�����,//��ִ��
  if(!(TiCommMng.Flag & (TI_COMM_MNG_RCV_DOING | 
                          TI_COMM_MNG_SEND_DOING))) return;
  //��������չ��̼�ʱ
  if(TiCommMng.Index) TiCommMng.Index--;
  if(TiCommMng.Index) return;//ʱ��δ��
  
  //���ݽ������
  if(TiCommMng.Flag & TI_COMM_MNG_RCV_DOING){
    UsartTiny_Stop(); //ǿ����ֹ����
    TiCommMng.Flag &= ~TI_COMM_MNG_RCV_DOING;//ֹͣ���ݽ���
    unsigned char Resume;
    #ifdef SUPPORT_TI_COMM_MNG_PRE 
      Resume = TiCommMng_cbRcvPreDataPro();
      if(Resume == 254){//��ͣ����
        UsartTiny_Stop();
        return;
      }
      if(Resume == 255) Resume = TiCommMng_cbDataPro();//��ʽ���ݴ���
    #else
      Resume = TiCommMng_cbDataPro();//���ݴ���
    #endif
     if((Resume != 0) && (Resume != 255)){ //������ȷ,��������
       UsartTiny_SendStart(Resume);
       TiCommMng.Flag |= TI_COMM_MNG_SEND_DOING;//��������
       TiCommMng.Index = TiCommMng.Count;//���㷢�ͳ�ʱ
     }
  }
  else{//���ͳ�ʱ�����
    UsartTiny_Stop();
  }

  /*/====================-����0x55���Բ�����===================
  if(TiCommMng.Index) TiCommMng.Index--;
  if(TiCommMng.Index) return;//ʱ��δ��
  TiCommMng.Index = 10;
  UsartTiny.Data[0] = 0x55;;
  UsartTiny_SendStart(1); */
  //485����A,B����ӦΪ��������(8λ����λ��1λֹͣλ)
  //�������ʵ��Ϊ��(ռ�ձ�50%,����ʱΪ�м��ƽ):
  //       �� b0:1 0   1   0   1   0   1 b7:0ͣ
  //   ����  ������  ������  ������  ������  ��
  // ������  ��  ��  ��  ��  ��  ��  ��  ��  ��������
  //     ������  ������  ������  ������  ������
  //������Ϊ9600ʱ������Ϊ200uS,����Ӧ1bitΪ100uS
}

/******************************************************************************
		                      ֧��Ԥ����ʱ���
******************************************************************************/ 
#ifdef SUPPORT_TI_COMM_MNG_PRE

//-------------------------------������--------------------------------
//����󣬿ɲſ�ֱ��ʹ�û����������������TiCommMng_PreInsertSend()���ܽ⿪
void TiCommMng_Suspend(void)
{
  UsartTiny_Stop(); //ǿ����ֹ
  TiCommMng.Flag &= ~(TI_COMM_MNG_RCV_DOING |TI_COMM_MNG_SEND_DOING);//ֹͣ����
  TiCommMng.Flag |= TI_COMM_MNG_SUSPEND;//����
}

//----------------------------ǿ�Ʋ��뷢�ͺ���--------------------------------
void TiCommMng_InsertSend(unsigned char SendLen) //�������ݳ���
{
  TiCommMng.Flag &= ~TI_COMM_MNG_SUSPEND;//������ȡ������
  if(!SendLen) return; //������Ҫ����
  
  UsartTiny_Stop();//��ֹͣ
  UsartTiny_SendStart(SendLen);
  TiCommMng.Flag |= TI_COMM_MNG_SEND_DOING;//��������
  TiCommMng.Index = TiCommMng.Count;//���㷢�ͳ�ʱ
}

#endif //SUPPORT_TI_COMM_MNG_PRE
