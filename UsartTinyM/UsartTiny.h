/******************************************************************

//               ������USART����ģ�����ӿ�
//��ģ��ΪUsartTiny�Ķ������������������UsartDevӲ��
//2.����֧���Զ�����ģʽ(���͹��������ϲ��Ԥ)��
//3.��֧�ְ�˫��ģʽ
//ע:��ģ������������շ�,������ײ�ͨѶ����������
*******************************************************************/

#ifndef __USART_TINY_H
#define __USART_TINY_H
#ifdef SUPPORT_EX_PREINCLUDE//��֧��Preinlude�r
  #include "Preinclude.h"
#endif

/*********************************************************************
                             ��������붨��
***********************************************************************/

//����������, < 255
#ifndef USART_TINY_DATA_MAX
  #define USART_TINY_DATA_MAX      (32 + 5)	 //3�żĴ�������������
#endif

/*********************************************************************
                             ��ؽṹ
***********************************************************************/

//����״̬��
enum _UsartTiny_eState{
  UsartTiny_eState_Idie      = 0,  //����״̬���ȴ��ⲿ�ⲿ�շ�ָ��
  UsartTiny_eState_Rcv       = 1,  //��������״̬
  //UsartTiny_eState_RcvFinal  = 2,  //�����������״̬
  UsartTiny_eState_Send      = 3,  //��������״̬
  //UsartTiny_eState_SendFinal = 4,  //�����������״̬
};

struct _UsartTiny{
  struct _UsartDev *pDev;  //���ò�ӵ�������ⲿʹ��()
  enum _UsartTiny_eState  eState; //����״̬��
  unsigned char Index;   //�շ�����λ��
  unsigned char SendLen; //�������ݸ���  
  unsigned char Data[USART_TINY_DATA_MAX]; //�շ����ݻ���
};

/*********************************************************************
                             �����Ϊ����
***********************************************************************/

//-----------------------------��ʼ������------------------------------
//���ô˺���ǰ,���ʼ��UsartDev(��UsartId)����IO������Usart������������
void UsartTiny_Init(struct _UsartTiny *pTiny,
                    struct _UsartDev *pDev);

//---------------------------ֹͣ�����շ�����-------------------------
//�շ����ݹ�������ֹ�����շ�
void UsartTiny_Stop(struct _UsartTiny *pTiny); 

//----------------------------�������պ���------------------------------
void UsartTiny_RcvStart(struct _UsartTiny *pTiny);

//----------------------------�������ͺ���------------------------------
void UsartTiny_SendStart(struct _UsartTiny *pTiny,
                         unsigned char SendLen);

/*********************************************************************
                             ��س�Ա����
***********************************************************************/
//ע:��ֱ���ϲ����

//----------------------------�õ��豸ID------------------------------
#define UsartTiny_GetDevId(tiny) ((tiny)->pDev->UsartId)

/************************************************************************
                             ��ػص�����
***********************************************************************/

//���ֺ���ֱ��ʵ��:
//1.ʵ��485��RT���� 2.ָʾ�ƿ���(����FireFreme��ͨѶָʾ������ӿ�),
//3�ϲ�ӿ�
#include "IOCtrl.h" //RTSֱ�ӿ��Ƶײ�
#ifdef SUPPORT_TI_COMM_MNG
  #include "TiCommMng.h" 
  #define UTtoTC(tiny)  ((struct _TiCommMng*)tiny) //���Ӧʹ�ö������̳нṹ
#endif

//-----------------------------ֹͣͨ������------------------------------
//void UsartTiny_cbStopNotify(struct _UsartTiny *pTiny);
#define UsartTiny_cbStopNotify(tiny) do{ClrRTS(UsartTiny_GetDevId(tiny));}while(0)

//-----------------------------��������ͨ������------------------------------
//void UsartTiny_cbRcvStartNotify(struct _UsartTiny *pTiny);
#define UsartTiny_cbRcvStartNotify(tiny) UsartTiny_cbStopNotify(tiny)

//-----------------------------��������ͨ������------------------------------
//void UsartTiny_cbSendStartNotify(struct _UsartTiny *pTiny);
#define UsartTiny_cbSendStartNotify(tiny) do{SetRTS(UsartTiny_GetDevId(tiny));}while(0)

//----------------------------�ж��ڽ��յ�����ͨ������------------------------------
#ifdef SUPPORT_TI_COMM_MNG
  #define UsartTiny_cbRcvDataNotify(tiny) do{TiCommMng_ResetRcvTimer(UTtoTC(tiny));}while(0)
#else
  void UsartTiny_cbRcvDataNotify(struct _UsartTiny *pTiny);  
#endif

//----------------------------�ж��ڽ��յ������������------------------------------
//void UsartTiny_cbRcvDataOv(struct _UsartTiny *pTiny);
#define UsartTiny_cbRcvDataOv(tiny) do{}while(0)

//----------------------------�ж��ڷ��ͳ�һ������ͨ������------------------------------
//void UsartTiny_cbSendDataNotify(struct _UsartTiny *pTiny);
#ifdef SUPPORT_TI_COMM_MNG
  #define UsartTiny_cbSendDataNotify(tiny) do{TiCommMng_ResetSendTimer(UTtoTC(tiny));}while(0)
#else
  void UsartTiny_cbSendDataNotify(const struct _UsartTiny *pTiny);
#endif
 
//----------------------------�շ����ݴ���ͨ������------------------------------
//void UsartTiny_cbErrNotify(struct _UsartTiny *pTiny, unsigned char Err);
#define UsartTiny_cbErrNotify(tiny, err) do{}while(0)

#endif  //#ifndef __USART_TINY_H

