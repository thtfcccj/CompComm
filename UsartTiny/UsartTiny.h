/******************************************************************

//               USART����ģ�����ӿ�
//˵��:
//UsartTinyģ��ΪԭUsartDev�ļ�Ҫ����ʵ��,��Ҫ�仯��:
//1.��ģ��ֱ�ӽ�����ʵ����,��ָ������Խ�ʡ����ռ���ʱ��Ч��
//2.����֧���Զ�����ģʽ(���͹����в����ϲ��Ԥ)��
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
  enum _UsartTiny_eState  eState; //����״̬��
  unsigned char Data[USART_TINY_DATA_MAX]; //�շ����ݻ���
  unsigned char Index;   //�շ�����λ��
  unsigned char SendLen; //�������ݸ���
};

extern struct _UsartTiny UsartTiny; //ֱ��ʵ����

/*********************************************************************
                             �����Ϊ����
***********************************************************************/

//-----------------------------��ʼ������------------------------------
//���ô˺���ǰ,���Usart������������
void UsartTiny_Init(void);

//-----------------------�ײ�Ӳ�����ú���----------------------
//Cfg����λ����ͬ��UsartDevCfg.h
void UsartTiny_CfgHw(unsigned char Cfg);

//---------------------------ֹͣ�����շ�����-------------------------
//�շ����ݹ�������ֹ�����շ�
void UsartTiny_Stop(void); 

//----------------------------�������պ���------------------------------
void UsartTiny_RcvStart(void);

//----------------------------�������ͺ���------------------------------
void UsartTiny_SendStart(unsigned char SendLen);

/*********************************************************************
                             ��س�Ա����
***********************************************************************/
//ע:��ֱ���ϲ����

/************************************************************************
                             ��ػص�����
***********************************************************************/

//���ֺ���ֱ��ʵ��:
//1.ʵ��485��RT���� 2.ָʾ�ƿ���(����FireFreme��ͨѶָʾ������ӿ�),
//3�ϲ�ӿ�
#include "IOCtrl.h"
#ifdef SUPPORT_TI_COMM_MNG
  #include "TiCommMng.h"
#else
  #include "ModbusRtuMng.h"
#endif
//-----------------------------��ʼ�����Ӻ���------------------------------
//void UsartTiny_cbInit(void);
#define UsartTiny_cbInit() do{CfgUsartIo();}while(0)

//-----------------------------ֹͣͨ������------------------------------
//void UsartTiny_cbStopNotify(void);
#define UsartTiny_cbStopNotify() do{ClrRTS();}while(0)

//-----------------------------��������ͨ������------------------------------
//void UsartTiny_cbRcvStartNotify(void);
#define UsartTiny_cbRcvStartNotify() UsartTiny_cbStopNotify()

//-----------------------------��������ͨ������------------------------------
//void UsartTiny_cbSendStartNotify(void);
#define UsartTiny_cbSendStartNotify() do{SetRTS();}while(0)

//----------------------------�ж��ڽ��յ�����ͨ������------------------------------
//void UsartTiny_cbRcvDataNotify(void);
#ifdef SUPPORT_TI_COMM_MNG
#define UsartTiny_cbRcvDataNotify() do{TiCommMng_ResetRcvTimer();}while(0)
#else
#define UsartTiny_cbRcvDataNotify() do{ModbusRtuMng_ResetRcvTimer();}while(0)
#endif

//----------------------------�ж��ڽ��յ������������------------------------------
//void UsartTiny_cbRcvDataOv(void);
#define UsartTiny_cbRcvDataOv() do{}while(0)

//----------------------------�ж��ڷ��ͳ�һ������ͨ������------------------------------
//void UsartTiny_cbSendDataNotify(void);
#ifdef SUPPORT_TI_COMM_MNG
  #define UsartTiny_cbSendDataNotify() do{TiCommMng_ResetSendTimer();}while(0)
#else
  #define UsartTiny_cbSendDataNotify() do{ModbusRtuMng_ResetSendTimer();}while(0)
#endif
 
//------------------------���ݷ��͵����ͨ������------------------------------
//void UsartTiny_cbSendLastDataNotify(void);
#define UsartTiny_cbSendLastNotify() do{}while(0)

//------------------------���ݷ������ͨ������------------------------------
//void UsartTiny_cbSendFinalNotify(void);
#define UsartTiny_cbSendFinalNotify() do{}while(0)

//----------------------------�շ����ݴ���ͨ������------------------------------
//void UsartTiny_cbErrNotify(void);
#define UsartTiny_cbErrNotify(err) do{}while(0)

#endif  //#ifndef __USART_TINY_H

