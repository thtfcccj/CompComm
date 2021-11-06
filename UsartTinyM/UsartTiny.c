/******************************************************************

*         ������USART����ģ�����ӿ�
* ��֧��SUPPORT_USART_DEV_CFG_TINY
* ��ʵ�ֶ�����Ӳ��
*******************************************************************/

#include "UsartTiny.h"
#include "IOCtrl.h"
#include "UsartDev.h"
#include <string.h>

/*********************************************************************
                             ��غ���ʵ��
***********************************************************************/

//-----------------------------��ʼ������------------------------------
//���ô˺���ǰ,���ʼ��UsartDev(��UsartId)����IO������Usart������������
void UsartTiny_Init(struct _UsartTiny *pTiny,
                    const struct _UsartDevPt *pFun, //��̬��������
                    struct _UsartDev *pDev)  ///���еĶ���
{
  memset(pTiny, 0, sizeof(struct _UsartTiny));
  pTiny->pFun = pFun;  
  pTiny->pDev = pDev;
  UsartTiny_Stop(pTiny);
  pTiny->eState =  UsartTiny_eState_Idie;
}

//---------------------------ֹͣ�����շ�����-------------------------
//�շ����ݹ�������ֹ�����շ�
void UsartTiny_Stop(struct _UsartTiny *pTiny)
{
  pTiny->pFun->RcvStop(pTiny->pDev);
  pTiny->pFun->SendStop(pTiny->pDev);  
  //��λ״̬�����ڲ�����
  //���ﲻ��pTiny->Index�Ա���ֹͣ���Կɻ�֪��ǰ�շ����ݸ������
  pTiny->eState = UsartTiny_eState_Idie;
  UsartTiny_cbStopNotify(pTiny);//ֹͣͨ��
}

//-----------------------�����жϴ�����--------------------------------
//�β�Ϊ��struct _UsartDevָ��
//��״̬����Ϊ:0:�����շ�,����:ֹͣ�շ�
static signed char _RcvEndInt(void *pv)
{
  struct _UsartDev *pDev = (struct _UsartDev *)pv;
  struct _UsartTiny *pTiny = (struct _UsartTiny *)(pDev->pVoid);
  if(pTiny->eState != UsartTiny_eState_Rcv){
    UsartTiny_cbErrNotify(pTiny, -1);
    return -1;//�쳣����
  }
  pTiny->Index++;
  UsartTiny_cbRcvDataNotify(pTiny);//��������ͨ��  
  //��ֹ�������
  if(pTiny->Index >= USART_TINY_DATA_MAX){
    UsartTiny_cbRcvDataOv();
    return -1;//ֹͣ����
  }
  return 0;  
}

//----------------------------�������պ���------------------------------
void UsartTiny_RcvStart(struct _UsartTiny *pTiny)
{
  UsartTiny_Stop(pTiny);//������ֹͣ
  pTiny->Index = 0;
  pTiny->eState = UsartTiny_eState_Rcv;
  
  UsartTiny_cbRcvStartNotify(pTiny);//��������ͨ��
  pTiny->pDev->pVoid = pTiny;//�ص�ʹ��
  pTiny->pFun->RcvStart(pTiny->pDev, 
                    pTiny->Data, 
                    USART_TINY_DATA_MAX, //�ֶ�����
                    _RcvEndInt);
}

//-----------------------�����жϴ���--------------------------------
//�β�Ϊ��struct _UsartDevָ��
//��״̬����Ϊ:0:�����շ�,����:
static signed char _SendEndInt(void *pv)
{
  struct _UsartDev *pDev = (struct _UsartDev *)pv;
  struct _UsartTiny *pTiny = (struct _UsartTiny *)(pDev->pVoid);
  //״̬������
  if(pTiny->eState != UsartTiny_eState_Send){   
     UsartTiny_cbErrNotify(pTiny, -2); 
     return -1; //ֹͣ�շ�
  }
  
  if(pDev->SenLen < pDev->SendCount)//���������һ������
    UsartTiny_cbSendDataNotify(pTiny);
  else{//���������
    pTiny->eState = UsartTiny_eState_Idie;
  }
  
  return 0;  
}

//----------------------------�������ͺ���------------------------------
void UsartTiny_SendStart(struct _UsartTiny *pTiny,
                         unsigned char SendLen)
{
  //UsartTiny_Stop();//������ֹͣ

  //���üĴ���Ϊ����ģʽ,��ʼ��������
  pTiny->SendLen = SendLen;
  pTiny->Index = 0;
  pTiny->eState = UsartTiny_eState_Send;

  UsartTiny_cbSendStartNotify(pTiny);   //��������ͨ��
  pTiny->pDev->pVoid = pTiny;//�ص�ʹ��  
  pTiny->pFun->SendStart(pTiny->pDev, 
                         pTiny->Data, 
                         SendLen, //�ֶ�ģʽ
                          _SendEndInt);
}

