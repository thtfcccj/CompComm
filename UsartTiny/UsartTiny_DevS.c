/******************************************************************

*         USART����ӿ�-��ʹ�õ�������UsartDevʱ��ʵ��
* ��֧��SUPPORT_USART_DEV_CFG_TINY
* ��ʵ�ֶ�����Ӳ��
*******************************************************************/

#include "UsartTiny.h"
#include "IOCtrl.h"
#include "UsartDev.h"
#include <string.h>

struct _UsartTiny UsartTiny; //ֱ��ʵ����
struct _UsartDev UsartDev;  //ֱ�ӵ�����

/*********************************************************************
                        IoCtrl��Ҫ��������Դ
***********************************************************************/

//#define UsartTiny_pcbGetHw()   //Ӳ��ָ��
//#define UsartTiny_cbUsartCfg(cfg)   //struct _UsartDevCfgӲ�����ú���

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
  UsartDev_Init(&UsartDev,
                UsartTiny_pcbGetHw());  //�ҽӵ�Ӳ��
}

//-----------------------�ײ�Ӳ�����ú���----------------------
void UsartTiny_CfgHw(unsigned char Cfg)
{
  UsartTiny_cbUsartCfg((struct _UsartDevCfg*)&Cfg);
}

//---------------------------ֹͣ�����շ�����-------------------------
//�շ����ݹ�������ֹ�����շ�
void UsartTiny_Stop(void)
{
  UsartDev_RcvStop(&UsartDev);
  UsartDev_SendStop(&UsartDev);  
  //��λ״̬�����ڲ�����
  //���ﲻ��UsartTiny.Index�Ա���ֹͣ���Կɻ�֪��ǰ�շ����ݸ������
  UsartTiny.eState = UsartTiny_eState_Idie;
  UsartTiny_cbStopNotify();//ֹͣͨ��
}

//-----------------------�����жϴ�����--------------------------------
//�β�Ϊ��struct _UsartDevָ��
//��״̬����Ϊ:0:�����շ�,����:ֹͣ�շ�
static signed char _RcvEndInt(void *pv)
{
  if(UsartTiny.eState != UsartTiny_eState_Rcv){
    UsartTiny_cbErrNotify(-1);
    return -1;//�쳣����
  }
  UsartTiny.Index++;
  UsartTiny_cbRcvDataNotify();//��������ͨ��  
  //��ֹ�������
  if(UsartTiny.Index >= USART_TINY_DATA_MAX){
    UsartTiny_cbRcvDataOv();
    return -1;//ֹͣ����
  }
  return 0;  
}

//----------------------------�������պ���------------------------------
void UsartTiny_RcvStart(void)
{
  UsartTiny_Stop();//������ֹͣ
  UsartTiny.Index = 0;
  UsartTiny.eState = UsartTiny_eState_Rcv;
  
  UsartTiny_cbRcvStartNotify();//��������ͨ��
  UsartDev_RcvStart(&UsartDev, 
                    UsartTiny.Data, 
                    USART_TINY_DATA_MAX, //�ֶ�����
                    _RcvEndInt);
}

//-----------------------�����жϴ���--------------------------------
//�β�Ϊ��struct _UsartDevָ��
//��״̬����Ϊ:0:�����շ�,����:
static signed char _SendEndInt(void *pv)
{
  //״̬������
  if(UsartTiny.eState != UsartTiny_eState_Send){   
     UsartTiny_cbErrNotify(-2); 
     return -1; //ֹͣ�շ�
  }
  
  if(UsartDev.SenLen < UsartDev.SendCount)//���������һ������
    UsartTiny_cbSendDataNotify();
  else{//���������
    UsartTiny.eState = UsartTiny_eState_Idie;
  }
  
  return 0;  
}

//----------------------------�������ͺ���------------------------------
void UsartTiny_SendStart(unsigned char SendLen)
{
  //UsartTiny_Stop();//������ֹͣ

  //���üĴ���Ϊ����ģʽ,��ʼ��������
  UsartTiny.SendLen = SendLen;
  UsartTiny.Index = 0;
  UsartTiny.eState = UsartTiny_eState_Send;

  UsartTiny_cbSendStartNotify();   //��������ͨ�� 
  UsartDev_SendStart(&UsartDev, 
                     UsartTiny.Data, 
                     SendLen, //�ֶ�ģʽ
                     _SendEndInt);
}

