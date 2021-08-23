/******************************************************************

//               USART����ӿ���PIC Usar1�е�ʵ��
//
*******************************************************************/

#include "UsartTiny.h"
#include "IOCtrl.h"
#include "ModbusRtuMng.h"
#include <string.h>

struct _UsartTiny UsartTiny; //ֱ��ʵ����

/*********************************************************************
                             ��غ���ʵ��
***********************************************************************/

//-----------------------------��ʼ������------------------------------
//���ô˺���ǰ,���Usart������������
void UsartTiny_Init(void)
{
  memset(&UsartTiny, 0, sizeof(struct _UsartTiny));
  UsartTiny_cbInit(); //��س�ʼ��
  UsartTiny_Stop();
  UsartTiny.eState =  UsartTiny_eState_Idie;
  //׼���ô���
  RC1STA |= PICB_SPEN;    //ʹ�ܴ���
  TX1STA &= ~PICB_SYNC;   //ʹ���첽USART
  INTCON |= PICB_PEIE;    //�����ж�
}
//---------------------------ֹͣ�����շ�����-------------------------
//�շ����ݹ�������ֹ�����շ�
void UsartTiny_Stop(void)
{
  //�ر������жϣ����շ�����ʱ��
	PIE1 &= ~(PICB_TXIE | PICB_RCIE);   //��ֹ����������ж�
	RC1STA &= ~PICB_CREN;               //ȡ������ʹ��
	TX1STA &= ~PICB_TXEN;               //ȡ������ʹ��
	
  //��λ״̬�����ڲ�����
  //���ﲻ��UsartTiny.Index�Ա���ֹͣ���Կɻ�֪��ǰ�շ����ݸ������
  UsartTiny.eState = UsartTiny_eState_Idie;
  UsartTiny_cbStopNotify();//ֹͣͨ��
}

//----------------------------�������պ���------------------------------
void UsartTiny_RcvStart(void)
{
  UsartTiny_Stop();//������ֹͣ
  
  //���üĴ���Ϊ����ģʽ,��ʼ��������
  RC1STA |= PICB_CREN;         //�򿪽���ʹ��
  UsartTiny.Index = 0;
  UsartTiny.eState = UsartTiny_eState_Rcv;
  
  UsartTiny_cbRcvStartNotify();//��������ͨ��
	PIE1 |= PICB_RCIE;           //����������ж�
}

//----------------------------�������ͺ���------------------------------
void UsartTiny_SendStart(unsigned char SendLen)
{
  UsartTiny_Stop();//������ֹͣ

  //���üĴ���Ϊ����ģʽ,��ʼ��������
  UsartTiny.SendLen = SendLen;
  UsartTiny.Index = 0;
  UsartTiny.eState = UsartTiny_eState_Send;
  
  UsartTiny_cbSendStartNotify();   //��������ͨ��
  
  //����������ж� 
	TX1STA |= PICB_TXEN;             //�򿪷���ʹ��  
	TX1REG = UsartTiny.Data[0];     //��ʼ�����׸�����     
  PIE1 |= PICB_TXIE;              //�������ж�
}

//---------------------USARTӲ�������жϴ�����----------------------------
void UsartTiny_RcvInt(void)
{
  //���Ĵ������ж�
  volatile unsigned char Data =  RC1REG; 

  //״̬������ ��֡����(֡����FERR��FIFO�������OERR)
  if((RCSTA & (PICB_FERR | PICB_OERR)) || 
    (UsartTiny.eState != UsartTiny_eState_Rcv)){
    UsartTiny_cbErrNotify(-1); 
    return;
  }
  //��ֹ�������
  if(UsartTiny.Index < USART_TINY_DATA_MAX){
    UsartTiny.Data[UsartTiny.Index] = Data;
    UsartTiny.Index++;
  }
  else{//���ջ�������
     UsartTiny_cbRcvDataOv();
     UsartTiny_Stop();//������ֹͣ
  }   
  UsartTiny_cbRcvDataNotify();//��������ͨ��  
}

//---------------------USARTӲ�������жϴ�����----------------------------
void UsartTiny_SendInt(void)
{
  //״̬������
  if(UsartTiny.eState != UsartTiny_eState_Send){
     UsartTiny_cbErrNotify(-2); 
     return;
  }
  UsartTiny.Index++; //�������һ����
  UsartTiny_cbSendDataNotify();//���������һ������

  if(UsartTiny.Index < UsartTiny.SendLen){ //��������ʱ,������һ����
    TX1REG = UsartTiny.Data[UsartTiny.Index];
  }
  //���һ�������Ƴ���,����һ���յȴ����ݽ���
  else if(UsartTiny.Index == UsartTiny.SendLen){
    TX1REG = 0xff;
  }  
  else{//�������
    UsartTiny_cbSendFinalNotify();
    UsartTiny_Stop();//������ֹͣ
  }
}
