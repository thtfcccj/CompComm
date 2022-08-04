/***************************************************************************

//			                 ���ߴӻ�-Usartʵ��

***************************************************************************/

#include "BusSlvUsart.h"
#include "struct.h"
#include <string.h>

//---------------------------�ڲ�����--------------------------
//���û���Ԥ������;
#define STATE     Base.ExU8           //��־,���嶨��Ϊ��
  #define _RCV_WAIT     1           //���յȴ���
  #define _RCV_DOING    2           //����������
  #define _SEND_DOING   3           //����������

#define TIMER     Base.Ex.U8[0]     //��ǰ��ʱ��ֵ
#define TIMER_OV  Base.Ex.U8[1]     //��ǰ��ʱ����ʱֵ

/***********************************************************************
                            ��غ���ʵ��
***********************************************************************/

//-----------------------------��ʼ������-------------------------------
void BusSlvUsart_Init(struct _BusSlvUsart *pBus,
                      unsigned char BusId)    //���������ID��
{
  memset(pBus, 0, sizeof(struct _BusSlvUsart));
  //��ʼ������
  BusBase_Init(&pBus->Base, BusId);
}

//------------------------------�����жϴ�����------------------------------
//�β�Ϊ��struct _UsartDevָ��
//��״̬����Ϊ:0:�����շ�,����:ֹͣ�շ�
static signed char _RcvFinal(void *pv)
{
  struct _UsartDev *pUsart = (struct _UsartDev *)pv;
  struct _BusSlvUsart *pBus = struct_get(pUsart->pRcvBuf, 
                                         struct _BusSlvUsart, DataBuf);
  pBus->STATE = _RCV_DOING;//ȡ���ȴ���
  pBus->TIMER = pBus->TIMER_OV; //����Ӧ����ʱ����λ
  
  //���������
  if(pUsart->RcvLen >= pUsart->RcvCount){
    pBus->TIMER = 0;//ֱ����ֹ
    return 1; //ֹͣ
  }
  return 0;//����δ���
}

//-----------------------����Ѳ�����жϴ�����--------------------------------
//�β�Ϊ��struct _UsartDevָ��
//��״̬����Ϊ:0:�����շ�,����:ֹͣ�շ�
static signed char _SendFinal(void *pv)
{
  struct _UsartDev *pUsart = (struct _UsartDev *)pv;
  struct _BusSlvUsart *pBus = struct_get(pUsart->pSendBuf, 
                                         struct _BusSlvUsart, DataBuf);
  pBus->TIMER = pBus->TIMER_OV; //����Ӧ����ʱ����λ
  //���������
  if(pUsart->SenLen >= pUsart->SendCount){
     pBus->STATE = 0;//����״̬
     
    //׼����ͨ�����ý���
  }
  return 0;//����δ���
}

//-------------------------------��ͨTick��ѯ����----------------------------
//���˺�������ϵͳ1ms������
void BusSlvUsart_TickTask(struct _BusSlvUsart *pBus)
{
  if(pBus->TIMER) return;//ʱ��δ��
  
  struct _UsartDev *pUsart = BusSlvUsart_cbGetDev(pBus->Base.Id);
  
  //���յȴ��г�ʱ
  if(pBus->STATE & _RCV_WAIT){
    UsartDev_RcvStop(pUsart); //ǿ����ֹ����
    pBus->STATE = 0;
  }
  //���ݽ������
  else if(pBus->STATE & _RCV_DOING){
    pBus->Base.Count.Comm++;//�������ݼ���

    UsartDev_RcvStop(pUsart); //ǿ����ֹ����
    pBus->STATE = 0;//ֹͣ���ݽ���
    
    signed short Resume = BusSlvUsart_cbDataPro(pBus, //���ݴ���
                            pUsart->RcvLen,
                            BusSlvUsart_cbGetAdr(pBus->Base.Id));
    if(Resume > 0){ //������ȷ,��������
       pBus->Base.Count.Valid++; //��Ч����
       BusSlvUsart_cbRTS(pBus->Base.Id, 1); //����״̬
       UsartDev_SendStart(pUsart,
                          pBus->DataBuf, Resume, _SendFinal);
       
       pBus->STATE = _SEND_DOING;//��������
       pBus->TIMER = pBus->TIMER_OV;//���㷢�ͳ�ʱ,Ҳ��һ֡Ϊ��λ
     }
     else if(Resume < 0) pBus->Base.Count.Invalid++;//��Ч����
     //else // if(Resume == 0) �㲥������
  }
  //���ͳ�ʱ�����
  else if(pBus->STATE){
    UsartDev_SendStop(pUsart); 
    pBus->STATE = 0;//����״̬
  }
  //������״̬��������
  if(pBus->STATE == 0){
    pBus->STATE = _RCV_WAIT; //���յȴ�    
    BusSlvUsart_cbRTS(pBus->Base.Id, 0); //����״̬
    UsartDev_RcvStart(pUsart, 
                      pBus->DataBuf, BUS_SLV_USART_DATA_SIZE, _RcvFinal);
    pBus->TIMER_OV = BusSlvUsart_cbGetSpaceT(pBus->Base.Id);
    pBus->TIMER = 255;//�ȴ���,���ʱ���ֹ����
    return;
  } 
}

