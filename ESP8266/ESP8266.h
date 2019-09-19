/***********************************************************************

                  ESP8266��������
ȫ˫������
***********************************************************************/
#ifndef __ESP8266_H
#define	__ESP8266_H

/****************************************************************************
		                      �������
****************************************************************************/

//��ģ��Ϊ��ѡʱ������ģ�鲻�ã����ⲿʹ��, �������ȫ�������
//#define SUPPORT_ESP8266

//֧�ֻ���ʱ(����ͨѶͨ���ȣ� �������ȫ��������)
//#define SUPPORT_ESP8266_BASE     

#ifndef ESP8266_WR_BUF_SIZE     //д�����С
  #define ESP8266_WR_BUF_SIZE          256
#endif 

#ifndef ESP8266_RD_BUF_SIZE     //�������С
  #define ESP8266_RD_BUF_SIZE          256
#endif 

/****************************************************************************
		                      ��ؽṹ
****************************************************************************/

#include "AtUsart.h"
#ifdef SUPPORT_ESP8266_BASE
  #include "ComBase.h"
#endif

//���ṹ
struct _ESP8266{
  #ifdef SUPPORT_ESP8266_BASE
    struct _ComBase Base;        //ͨѶ����
  #endif
  //�ײ�ʵ��:
  struct _AtUsart AtUsart;       //��ռ��(Usart����ռ)
  struct _UsartDev OrgUsartDev; //���屻�ٳֵĵײ��豸��Ϣ�Ա��ڻָ�  
  
  unsigned char WrBuf[ESP8266_WR_BUF_SIZE];               //д����
  unsigned char RdBuf[ESP8266_RD_BUF_SIZE];               //������
  volatile signed char HwRdResume;        //AtUsart�����
  volatile signed char HwWrResume;        //AtUsartд���  
  unsigned char PreMode;               //Ԥ�õĹ���ģʽ��������
  unsigned char CurMode;               //��ǰ����ģʽ��������
  unsigned char ModeState;             //��ǰ����ģʽ��Ӧ״̬��������ģʽ����
  unsigned char Timer;                 //��ʱ��
  unsigned char PrvModeState;         //�ϴε�ǰ����ģʽ��Ӧ״̬������ͨѶ����
  unsigned char CurCommErrIndex;      //��ǰͨѶ������,������ͬ״̬����һ��ʱ���Զ���λ
  volatile unsigned char Flag;       //��ر�־��������  
  
  unsigned char LocalIp[4];           //��ȡ���ı���IP��ַ�����ھ������ڷ���
};

//��ر�־����Ϊ:
#define ESP8266_PASS_RCV_FINAL  0x40        //͸��ģʽ�յ����ݱ�־
#define ESP8266_RD_WAIT      0x20           //�����ѷ��������ȴ�

#define ESP8266_HW_RDY       0x10           //ESP8266Ӳ����⵽
#define ESP8266_WIFI_RDY     0x08           //ESP8266��WIFI׼������
#define ESP8266_AP_RDY       0x04           //ESP8266��APʱ��AP׼������
#define ESP8266_HW_RDY_MASK  0x1C            //

#define ESP8266_CWMODE_MASK  0x03           //ESP8266����ģʽ0��,1:STA 2:AP 3:AP+STA


//����ģʽ����Ӧ��״̬����Ϊ:
#define ESP8266_MODE_INIT            0     //��ʼ��ģʽ(����ʱ),״̬����Ϊ:
  //����"+++"ָ��ǿ���˳�͸��ģʽ�Խ����ʼ��ģʽ
  #define  ESP8266_MODE_INIT_ENTER         0 
  //����"AT+RST"ָ�״̬��1
  #define  ESP8266_MODE_INIT_RST           1 
  //����"ATE0"ָ��رջ��ԣ�����OK����ESP8266_HW_RDY,״̬��2
  #define  ESP8266_MODE_INIT_ATE0          2
  //����"AT+CWMODE?"ָ��񵽵�ǰģʽ,���뵱ǰģʽ��ͬ��������������
  #define  ESP8266_MODE_INIT_GET_CWMODE    3  
  //����"AT+CWMODE="ָ���OK, ��STA��AP+STAʱ״̬��3,����ת������ģʽ
  #define  ESP8266_MODE_INIT_SET_CWMODE    4
  //����"AT+CIFSR"��ȡ�����汾��IP��ַ����ʱת��4,��ʱ��ESP8266_WIFI_RDYת������ģʽ
  #define  ESP8266_MODE_INIT_CIFSR         5     
  //ModeState=4: ����"AT+CWSMARTSTART=2"ת����������ģʽ���ȴ��û�����WIFI��ת��5
  //             (�ֻ��������ֻ�΢�������������ſɿƼ��� ���wifi����)
  #define  ESP8266_MODE_INIT_SMART_START   6   
  //ModeState=5: �غ���գ��û����ú󣬽����յ���OK����ʼ�ַ�����ESP8266_WIFI_RDYת������ģʽ
  #define  ESP8266_MODE_INIT_SMART_WAIT    7   
  
  //ע������ʱ,������������"+++"�Կɽ������ȶ�ȡIP���򲻳ɹ�����λ���ٶ�IP�������ɹ�
  //      ��ת���Զ�����ģʽ����ʱ���¸�λ����ѫ��

#define ESP8266_MODE_CFG                1     //��������ģʽ(����ģʽ�˳�ʱ)�ȴ�

#define ESP8266_MODE_TCP_PASS           2     //TCP͸��ģʽ(֧�־�������ȫ��TCP͸��)
  //1�ڱ��ص�������TCP/IPת���ڷ�����
  //����"AT+CIPSTART="TCP","Ip4.Ip3.Ip2.Ip4",port"ָ��
  //�ɹ�����CONNECT��ת��1,����һֱ�ڴ�״̬ 
  #define  ESP8266_MODE_PASS_SERVER           0 
  //���͡�AT+CIPMODE=1������Ϊ͸��ģʽ,����OK, ת��2
  #define  ESP8266_MODE_PASS_RDY         1 
  //���͡�AT+CIPSEND����ʼ͸��,����<,����UsartDev����Ȩ��ת��3
  #define  ESP8266_MODE_PASS_ENTER       2 
  //͸��ģʽ��,��ģʽ����Modbus�Ƚ��ӣ�ESP8266_ModeExit()ʱ��ת��4
  #define  ESP8266_MODE_PASS_DOING       3 
  //ModeState=4: ���UsartDev����Ȩ,����+++�˳�
  #define  ESP8266_MODE_PASS_EXIT        4 

#define ESP8266_MODE_MQTT            2     //MQTTģʽ(�ݲ�֧��)

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

//-----------------------����������-------------------------------------
//���ڽ�����ɨ��
void ESP8266_FastTask(void);

//---------------------------������-------------------------------------
//128ms������һ��
void ESP8266_Task(void);

/******************************************************************************
		                            �ص�����
******************************************************************************/

//--------------------��ȡȫ��TCP/IP����������IP��ַ���λ---------------------
//����NULLΪ����ģʽ
unsigned char *pESP8266_cbGetGlobalServerIp(unsigned char DevId);

//--------------------��ȡ����TCP/IP����������IP��ַ���λ---------------------
//��3λ�ñ���IP����
unsigned char ESP8266_cbGetLocalServerIpLowest(unsigned char DevId);

//--------------------------��ȡTCP/IP�����������˿ں�---------------------
//���ػ�ȫ��
unsigned short ESP8266_cbGetServerPort(unsigned char DevId);

//-------------------------�ڲ�͸��ʱ�����ݴ���----------------------------
//���ط������ݳ��ȣ�0��ʾ������
unsigned short ESP8266_cbPassEncoder(unsigned char *pData,  //������
                                     unsigned short RcvLen,   //�յ������ݳ���
                                     unsigned short BufSize); //��������С
#endif




