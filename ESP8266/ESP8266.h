/***********************************************************************

                  ESP8266��������
ȫ˫������
***********************************************************************/
#ifndef __ESP8266_H
#define	__ESP8266_H

/****************************************************************************
		                      �������
****************************************************************************/

#ifndef ESP8266_WR_BUF_SIZE     //д�����С
  #define ESP8266_WR_BUF_SIZE          128
#endif 

#ifndef ESP8266_RD_BUF_SIZE     //�������С
  #define ESP8266_RD_BUF_SIZE          256
#endif 

/****************************************************************************
		                      ��ؽṹ
****************************************************************************/

#include "AtUsart.h"

//���ṹ
struct _ESP8266{
  //�ײ�ʵ��:
  struct _AtUsart AtUsart; //��ռ��(Usart����ռ)
  unsigned char WrBuf[ESP8266_WR_BUF_SIZE];               //д����
  unsigned char RdBuf[ESP8266_RD_BUF_SIZE];               //������
  volatile signed char HwRdResume;        //AtUsart�����
  volatile signed char HwWrResume;        //AtUsartд���  
  unsigned char PreMode;               //Ԥ�õĹ���ģʽ��������
  unsigned char CurMode;               //��ǰ����ģʽ��������
  unsigned char ModeState;                 //��ǰ����ģʽ��Ӧ״̬��������ģʽ����
  unsigned char Timer;                 //��ʱ��
  volatile unsigned char Flag;       //��ر�־��������  
  
  unsigned long LocalIp;              //��ȡ���ı���IP��ַ�����ھ������ڷ���
};

//��ر�־����Ϊ:
#define ESP8266_RD_WAIT      0x20           //�����ѷ��������ȴ�

#define ESP8266_HW_RDY       0x10           //ESP8266Ӳ����⵽
#define ESP8266_WIFI_RDY     0x08           //ESP8266��WIFI׼������
#define ESP8266_AP_RDY       0x04           //ESP8266��APʱ��AP׼������
#define ESP8266_HW_RDY_MASK  0x1C            //

#define ESP8266_CWMODE_MASK  0x03           //ESP8266����ģʽ0��,1:STA 2:AP 3:AP+STA


//����ģʽ����Ӧ��״̬����Ϊ:
#define ESP8266_MODE_INIT         0     //��ʼ��ģʽ(����ʱ),״̬����Ϊ:
  //ModeState=0: ����"AT+RST"ָ�״̬��1
  //ModeState=1: ����"ATE0"ָ��رջ��ԣ�����OK����ESP8266_HW_RDY,״̬��2
  //ModeState=2: ����"AT+CWMODE="ָ���OK, ��STA��AP+STAʱ״̬��3,����ת������ģʽ
  //ModeState=3: ����"AT+CIFSR"��ȡ�����汾��IP��ַ����ʱת��4,��ʱ��ESP8266_WIFI_RDYת������ģʽ
  //ModeState=4: ����"AT+CWSMARTSTART=2"ת����������ģʽ���ȴ��û�����WIFI��ת��5
  //             (�ֻ��������ֻ�΢�������������ſɿƼ��� ���wifi����)
  //ModeState=5: �غ���գ��û����ú󣬽����յ���OK����ʼ�ַ�����ESP8266_WIFI_RDYת������ģʽ
#define ESP8266_MODE_CFG                0     //��������ģʽ(����ģʽ�˳�ʱ)�ȴ�

#define ESP8266_MODE_TCP_PASS_LOCAL     1     //����TCP͸��ģʽ,�������������豸����
  //1�ڱ��ص�������TCP/IPת���ڷ�����
  //ModeState=0: ����"AT+CIPSTART="TCP","LocalIp4.LocalIp3.LocalIp2.485��ַ",10002"ָ��
  //�ɹ�����CONNECT��ת��1
  //ModeState=1: ���͡�AT+CIPMODE=1������Ϊ͸��ģʽ,����OK, ת��2
  //ModeState=2: ���͡�AT+CIPSEND����ʼ͸��,����<,����UsartDev����Ȩ��ת��3
  //ModeState=3: ͸��ģʽ��,��ģʽ����Modbus�Ƚ��ӣ�ESP8266_ModeExit()ʱ��ת��4
  //ModeState=4: ���UsartDev����Ȩ,����+++�˳�
#define ESP8266_MODE_TCP_PASS_GLOBAL    3     //ȫ��TCP͸��ģʽ,��������Internet
  //ͬ����ģʽ�����滻���ص�ַΪȫ�ֵ�ַ��



//���ǵ��󲿷�ϵͳֻ��һ��8266,��ֱ�ӵ�����,�����ǵ��������⣬����ָ��
extern struct _ESP8266 *pESP8266; 

/******************************************************************************
		                        ��غ���
******************************************************************************/

//-------------------------------��ʼ������---------------------------------
//���ô˺���ǰȷ��Ӳ��ͨѶ�����ѳ�ʼ��׼����
void ESP8266_Init(struct _UsartDev *pUsartDev, //�ѳ�ʼ����ɵĵײ��豸 
                  unsigned char DevId,         //�豸���ص�ID��
                  unsigned char CwMode,       //ESP8266����ģʽ,0��,1:STA 2:AP 3:AP+STA
                  unsigned char PreMode);     //��ģ��Ԥ�õĹ���ģʽ

//---------------------------1msӲ������������---------------------------------
//����Ӳ����ʱ����
#define ESP8266_1msHwTask() \
  do{if(pESP8266 != NULL){AtUsart_1msHwTask(&pESP8266->AtUsart);}}while(0)

//---------------------------����ĳ��ģʽ---------------------------------
void ESP8266_ModeEnter(unsigned char Mode);

//---------------------------�˳�ԭ��ģʽ---------------------------------
void ESP8266_ModeExit(void);

//---------------------------������-------------------------------------
//128ms������һ��
void ESP8266_Task(void);

/******************************************************************************
		                            �ص�����
******************************************************************************/

//---------------------------�ͷű�������豸----------------------------------
void ESP8266_cbRealseUsartDev(void);

#endif




