/******************************************************************

//               USART����ӿ���PIC Usar�е�ʵ��
//
*******************************************************************/

#include "UsartTiny.h"
#include "IOCtrl.h"
#include "IOCtrl.h"
#include "ModbusRtuMng.h"
#include <string.h>

struct _UsartTiny UsartTiny; //ֱ��ʵ����
static unsigned char _Cfg = 0;   //UsartTiny_CfgHw������

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
  RCSTA |= PICB_SPEN;    //ʹ�ܴ���,TX,RX�����Զ�����
  TXSTA &= ~PICB_SYNC;   //ʹ���첽USART
  INTCON |= PICB_PEIE;    //�����ж�
}

//-----------------------�ײ�Ӳ�����ú���----------------------
void UsartTiny_CfgHw(unsigned char Cfg)
{
  //Cfg = 0; //��Ӳ����������ģʽ�ݲ�֧��
  _Cfg = Cfg; //��ס������ʹ��
  //��������:
  TXSTA = PICB_CSRC;    //ʱ��Ƶ��ģʽѡΪ��ģʽ(�ڲ�ʱ��) �첽ͨ�ŷ�ʽ �Ͳ�����
  RCSTA = PICB_SPEN;     //������, ȡ����ַ���,�ؽ������ַ���

  //���㲢��������λ��(Ӳ���Դ�1����ʼλ+1��ֹͣλ,û����żУ����2��ֹͣλ����)
  unsigned char Bit;
  if(Cfg & USART_TINY_BIT_7) Bit = 7;
  else  Bit = 8;
  if(Cfg & USART_TINY_STOP_2) Bit++;  //����ֹͣλ
  if(Cfg &USART_TINY_PARITY_EN) Bit++;//Ӳ����֧��У�飬����Ҫռλ
  if(Bit > 8){//���õ�9λ�䵱�����λ
    TXSTA |= PICB_TX9; 
    RCSTA |= PICB_RX9; 
  }

  //���ò�����: 32Mʱ��ʱ������(SYNC = 0; BRGH = 0; BRG16 = 0;)�� 
  BAUDCON = 0;    //˯�߻��ѹر� 8λ����������,�ر��Զ������ʼ�⣬���ͼ��Բ���
  SPBRGH = 0X00;
  unsigned char Shift = Cfg & USART_TINY_BAUD_MASK;
  if((Shift == 0) || (Shift > USART_TINY_BAUD_19200)) //��Ĭ�ϲ�����,��֧�ָ߲�����
    Shift = USART_TINY_BAUD_9600; 
                      
  SPBRGL = 207 >> (Shift - 1);//2400:207 4800:103 9600:51 19200:25
}


//---------------------------ֹͣ�����շ�����-------------------------
//�շ����ݹ�������ֹ�����շ�
void UsartTiny_Stop(void)
{
  //�ر������жϣ����շ�����ʱ��
  PIE1 &= ~(PICB_TXIE | PICB_RCIE);   //��ֹ����������ж�
  RCSTA &= ~PICB_CREN;               //ȡ������ʹ��
  TXSTA &= ~PICB_TXEN;               //ȡ������ʹ��
	
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
  RCSTA |= PICB_CREN;         //�򿪽���ʹ��
  UsartTiny.Index = 0;
  UsartTiny.eState = UsartTiny_eState_Rcv;
  
  UsartTiny_cbRcvStartNotify();//��������ͨ��
  PIE1 |= PICB_RCIE;           //����������ж�
}

//----------------------------�������ͺ���------------------------------
void UsartTiny_SendStart(unsigned char SendLen)
{
  //UsartTiny_Stop();//������ֹͣ

  //���üĴ���Ϊ����ģʽ,��ʼ��������
  UsartTiny.SendLen = SendLen;
  UsartTiny.Index = 0;
  UsartTiny.eState = UsartTiny_eState_Send;
    
  //����������ж� 
  TXSTA |= PICB_TXEN;             //�򿪷���ʹ�� 
  UsartTiny_cbSendStartNotify();   //��������ͨ��
  TXREG = UsartTiny.Data[0];     //��ʼ�����׸�����     
  PIE1 |= PICB_TXIE;              //�������ж�
}

//---------------------USARTӲ�������жϴ�����----------------------------
void UsartTiny_RcvInt(void)
{
  //���Ĵ������ж�
  volatile unsigned char Rcsta = RCSTA;//�ȶ�״̬
  volatile unsigned char Data =  RCREG; 
  if(UsartTiny.eState != UsartTiny_eState_Rcv) return;//�쳣����
  
  //�����УżУ��,����λ����ֹͣλ����
  if(_Cfg & USART_TINY_BIT_7) Data &= 0x7f; //ǿ�����λ
  //�����УżУ����

  //״̬������ ��֡����(֡����FERR��FIFO�������OERR)
  if((Rcsta & (PICB_FERR | PICB_OERR)) || 
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
  PIR1 &= ~PICB_TXIF;//���������־
  //״̬������
  if(UsartTiny.eState != UsartTiny_eState_Send){   
     UsartTiny_cbErrNotify(-2); 
     return;
  }

  UsartTiny.Index++; //�������һ����
  UsartTiny_cbSendDataNotify();//���������һ������

  if(UsartTiny.Index < UsartTiny.SendLen){ //��������ʱ,������һ����
    TXREG = UsartTiny.Data[UsartTiny.Index];
  }
  else{ //���һ����ѹ����λ�Ĵ������,û���������ˣ�ֹͣ�жϣ����ϲ㴦�����һ�ֽڷ���
    UsartTiny_cbSendLastNotify();
    PIE1 &= ~PICB_TXIE;   //��ֹ�����ж�     
  }
}
