/***********************************************************************

                  At�ײ�ͨѶ�ӿ�ʵ��ģ��ʵ��

***********************************************************************/

#include "AtUsart.h"
#include <string.h>
#include "MemMng.h"

/******************************************************************************
		                           ��غ���-ϵͳ���
******************************************************************************/

//-------------------------------��ʼ������---------------------------------
//���ô˺���ǰȷ��Ӳ��ͨѶ�����ѳ�ʼ��׼����
//�շ�����ǰ����ָ��������
void AtUsart_Init(struct _AtUsart *pAtUsart,
                  struct _UsartDev *pUsartDev,  //�ѳ�ʼ����ɵĵײ��豸
                  unsigned char DevId,         //�豸ID��
                  unsigned char ModeMask)      //AT_USART_HALF_MODE_MASK����  
{
  memset(pAtUsart, 0, sizeof(struct _AtUsart));
  pAtUsart->pUsartDev = pUsartDev;
  pUsartDev->pVoid = pAtUsart;
  pAtUsart->DevId = DevId;  
  pAtUsart->Flag |= ModeMask;
}

//-----------------------------���÷��ͻ���������-------------------------------
//��ʼ�������
void AtUsart_CfgSend(struct _AtUsart *pAtUsart,
                     unsigned short Count,         //��������С
                     unsigned char *pBuf,          //������
                     AtUsartNotify_t Notify)        //�ص�����
{
  pAtUsart->Send.Count = Count;  
  pAtUsart->Send.pBuf = pBuf; 
  pAtUsart->Send.Notify = Notify; 
  if(pAtUsart->Flag & AT_USART_HALF_DUPLEX){//��˫��ģʽʱ,Ĭ�Ϲ���һ��������
    memcpy(&pAtUsart->Rcv, &pAtUsart->Send, sizeof(struct _AtUsartBuf));
  }
}

//-----------------------------���ý��ջ���������-------------------------------
//ȫ˫ʽģʽ�������÷��ͻ��������������
void AtUsart_CfgRcv(struct _AtUsart *pAtUsart,
                    unsigned short Count,         //��������С
                     unsigned char *pBuf,          //������
                     AtUsartNotify_t Notify)    //�ص�����
{
  pAtUsart->Rcv.Count = Count;  
  pAtUsart->Rcv.pBuf = pBuf;  
  pAtUsart->Rcv.Notify = Notify;    
}

//------------------------------������ɵ��ú���-------------------------------
static void _RcvFinal(struct _AtUsart *pAtUsart, signed char State)
{
  pAtUsart->RcvFlag |= AT_USART_RCV_STATE_FINAL;
  pAtUsart->RcvTimer = 0;
  UsartDev_RcvStop(pAtUsart->pUsartDev);
  AtUsart_cbRcvEndNotify(pAtUsart->DevId); 
  pAtUsart->Rcv.Notify(pAtUsart, State);   //�û�ͨ��
}

//---------------------------1msӲ������������---------------------------------
//����Ӳ����ʱ����
void AtUsart_1msHwTask(struct _AtUsart *pAtUsart)
{
  //���ͳ�ʱ����
  if(pAtUsart->SendTimer){
    pAtUsart->SendTimer--;
    if(!pAtUsart->SendTimer){
      pAtUsart->SendFlag |= AT_USART_SEND_STATE_FINAL;
      AtUsart_cbSendEndNotify(pAtUsart->DevId); //�������ͨ��
      pAtUsart->Send.Notify(pAtUsart, 1);//��ʱ����
      AtUsart_SendStop(pAtUsart);//ǿ��ֹͣ
    }
  }
  
  //=============================������ؼ�ʱ================================
  unsigned char RcvState = pAtUsart->SendFlag & AT_USART_RCV_STATE_MASK;
  if(RcvState == AT_USART_RCV_STATE_IDIE) return;
  
  //���յȴ��м���
  if(RcvState == AT_USART_RCV_STATE_WAIT){
    if(pAtUsart->RcvTimer != 0){//0������
      pAtUsart->RcvTimer--;
      if(!pAtUsart->RcvTimer){//��ʱ�˳�
         _RcvFinal(pAtUsart, 1);
      }
    }
  }
  //�����м���
  if(pAtUsart->RcvTimer != 0){//0������
    pAtUsart->RcvTimer--;
    if(!pAtUsart->RcvTimer){//��ʱ�˳���ͨ��
      if(pAtUsart->RcvFlag & AT_USART_RCV_ALL_MODE)//��ʱ�������
        _RcvFinal(pAtUsart, 0);
      else _RcvFinal(pAtUsart, 1);//����ʱ��
    }
  }
}


/******************************************************************************
		                     �������ݲ�������ʵ��
******************************************************************************/

//------------------------UsartDev��������жϵ��ú���ʵ��--------------------
//�β�Ϊ��struct _UsartDevָ��
//��״̬����Ϊ:0:�����շ�,����:ֹͣ�շ�
signed char AtUsart_UsartDevSendEndNotify(void *pVoid)
{
  struct _UsartDev *pUsart = pVoid;
  struct _AtUsart *pAtUsart = (struct _AtUsart *)pUsart->pVoid;
  pAtUsart->SendTimer = 0;//������
  AtUsart_cbSendEndNotify(pAtUsart->DevId); //�������ͨ��
  if(pAtUsart->Send.Notify(pAtUsart, 0) ||  //�û�����,������������ʱ
    (pAtUsart->Flag & AT_USART_WR_AUTO_RCV)){//�Զ����ý���ʱ,������
    AtUsart_RcvStart(pAtUsart);
  }
  AtUsart_SendStop(pAtUsart);
  return 1; //ֹͣ����
}

//--------------------------------���Ͳ���--------------------------------------
//�������ã�AT_USART_SEND_DIS_ALL��
void AtUsart_SendCfg(struct _AtUsart *pAtUsart, unsigned char Cfg)
{
  pAtUsart->SendFlag &= ~AT_USART_SEND_DIS_ALL;
  pAtUsart->SendFlag |= Cfg;  
  
}
//�õ����ͻ�����,��ATָ��ʱ�����Զ���ָ���ʼ
unsigned char *AtUsart_pGetSendBuf(const struct _AtUsart *pAtUsart)
{
  if(pAtUsart->SendFlag & AT_USART_SEND_DIS_AT) //��AT��
    return pAtUsart->Send.pBuf;
  return pAtUsart->Send.pBuf + 2; //��AT��
}
//�õ����ͻ�������С,��ǰ��׺ʱ�����ų�
unsigned short AtUsart_GetSendCount(const struct _AtUsart *pAtUsart)
{
  unsigned short Count = pAtUsart->Send.Count;
  if(!(pAtUsart->SendFlag & AT_USART_SEND_DIS_AT)) Count -= 2; //��AT��
  if(!(pAtUsart->SendFlag & AT_USART_SEND_DIS_CR)) Count--;
  if(!(pAtUsart->SendFlag & AT_USART_SEND_DIS_LF)) Count--;
  return Count;
}
//��������,����ǰ����ǰд�뻺����
void AtUsart_SendBuf(struct _AtUsart *pAtUsart, unsigned short SendLen)
{
  //��仺����
  unsigned char *pBuf = pAtUsart->Send.pBuf;
  if(!(pAtUsart->SendFlag & AT_USART_SEND_DIS_AT)){ //��AT��
    *pBuf = 'A';
    *(pBuf + 1) = 'T';
    SendLen += 2;
  }
  pBuf += SendLen;//ָ�����
  if(!(pAtUsart->SendFlag & AT_USART_SEND_DIS_CR)){
    *pBuf++ = '\r';    
    SendLen++;    
  }
  if(!(pAtUsart->SendFlag & AT_USART_SEND_DIS_LF)){
    *pBuf = '\n';    
    SendLen++;    
  }
  pAtUsart->Send.Len = SendLen;
  pAtUsart->SendTimer = AT_USART_SEND_OV;
  pAtUsart->SendFlag &= ~AT_USART_SEND_STATE_MASK;
  pAtUsart->SendFlag |= AT_USART_SEND_STATE_DOING;
  AtUsart_cbSendStartNotify(pAtUsart->DevId);
  UsartDev_SendStart(pAtUsart->pUsartDev,
                     pAtUsart->Send.pBuf,         //���ͻ�����
                     0x8000 | SendLen,            //�������ݴ�С
                     AtUsart_UsartDevSendEndNotify);     //���ͻص�����  
}
//ǿ��ֹͣ��������
void AtUsart_SendStop(struct _AtUsart *pAtUsart)
{
  if((pAtUsart->SendFlag & AT_USART_SEND_STATE_MASK) != AT_USART_SEND_STATE_DOING)
    return;
  UsartDev_SendStop(pAtUsart->pUsartDev);
  AtUsart_cbSendEndNotify(pAtUsart->DevId);
}

/******************************************************************************
		                     �������ݲ�������ʵ��
******************************************************************************/

//----------------------------״̬������------------------------------------
static void _SetRcvState(struct _AtUsart *pAtUsart, unsigned char NextState)
{
  pAtUsart->RcvFlag &= ~AT_USART_RCV_STATE_MASK;
  pAtUsart->RcvFlag |= NextState;
}

//---------------------UsartDev���������жϵ��ú���ʵ��------------------------
//�β�Ϊ��struct _UsartDevָ��
//��״̬����Ϊ:0:�����շ�,����:ֹͣ�շ�
static signed char _UsartRcvAllNotify(void *pVoid)
{
  struct _UsartDev *pUsart = pVoid;
  struct _AtUsart *pAtUsart = (struct _AtUsart *)pUsart->pVoid;
  //״̬���쳣
  if(!(pAtUsart->RcvFlag & AT_USART_RCV_ALL_MODE)){
    _RcvFinal(pAtUsart, 3);
    return -1;    
  }
  if((pAtUsart->RcvFlag & AT_USART_RCV_STATE_MASK) != AT_USART_RCV_STATE_DOING){
    _RcvFinal(pAtUsart, 4);
    return -1;
  }
  //������������ʱ,��ʾ��������
  pAtUsart->Rcv.Len += pUsart->RcvLen;  //��������
  if(pAtUsart->Rcv.Notify(pAtUsart, 0)){//�������ս���������
    pAtUsart->pUsartDev->RcvLen = 0;//���¿�ʼ
    pAtUsart->RcvTimer = pAtUsart->RcvDoingOv;
    UsartDev_RcvStart(pAtUsart->pUsartDev,
                      pAtUsart->Rcv.pBuf,    //���ջ�����
                      pAtUsart->Rcv.Count,   //�������ݴ�С,�Ƚ���һ������Ӧ
                      _UsartRcvAllNotify);   //���ջص�����
    return 0;
  }
  //�������
  pAtUsart->RcvFlag |= AT_USART_RCV_STATE_FINAL;
  AtUsart_RcvStop(pAtUsart);
  return 1;
}

//-------------------------�����չ����е��ú���ʵ��-----------------------------
//���յ����ַ�������ʱ���ڵ���,����0�Լ�������
static signed char _UsartRcvToDoing(struct _AtUsart *pAtUsart,
                                      unsigned char RcvedLen)//�ѽ��ո���
{
  AtUsart_cbRcvValidNotify(pAtUsart->DevId);
  //����ʶ���ʱ,ȫ��������
  if((pAtUsart->RcvFlag & AT_USART_RCV_DIS_END) == AT_USART_RCV_DIS_END){
    pAtUsart->Rcv.Len = RcvedLen;  //��������,������һ������
    pAtUsart->RcvTimer = pAtUsart->RcvDoingOv;
    UsartDev_RcvStart(pAtUsart->pUsartDev,
                      pAtUsart->Rcv.pBuf + RcvedLen,       //���ջ�����
                      0x8000 | (pAtUsart->Rcv.Count - RcvedLen),  //�������ݴ�С
                      _UsartRcvAllNotify);          //�������лص�����
    _SetRcvState(pAtUsart, AT_USART_RCV_STATE_DOING);
    pAtUsart->RcvFlag |= AT_USART_RCV_ALL_MODE;//ȫ����
  }
  else{ //��ʶ���,ֻ�ܻ�һ������
    pAtUsart->pUsartDev->RcvLen = 0;//ǰ�������㳤��.������
  }
  _SetRcvState(pAtUsart, AT_USART_RCV_STATE_DOING); 
  return 0;
}

//---------------------UsartDev���յ������жϵ��ú���ʵ��------------------------
//�β�Ϊ��struct _UsartDevָ��
//��״̬����Ϊ:0:�����շ�,����:ֹͣ�շ�
static signed char _UsartRcvNotify(void *pVoid)
{
  struct _UsartDev *pUsart = pVoid;
  struct _AtUsart *pAtUsart = (struct _AtUsart *)pUsart->pVoid;
  
  unsigned char State = pAtUsart->RcvFlag & AT_USART_RCV_STATE_MASK;
  pAtUsart->RcvTimer = AT_USART_RCV_DOING_BYTE_OV;  
  unsigned char RcvChar = pAtUsart->pUsartDev->RcvData;
  
  //======================�����ַ����չ����д�������ַ�======================== 
  if(State == AT_USART_RCV_STATE_DOING){
    unsigned char IdentChar;
    if(!(pAtUsart->RcvFlag & AT_USART_RCV_DIS_ECR))//CR��ʶ��
      IdentChar = '\r';
    else if(!(pAtUsart->RcvFlag & AT_USART_RCV_DIS_ELF))//ֻ��LFʱ��ʶ��
      IdentChar = '\n';
    else return -1;//�쳣����
    
    //�ҵ������ַ���
    if(pAtUsart->pUsartDev->RcvData == IdentChar){
      //˫�ַ�����ʶ��ʱ
      if((pAtUsart->RcvFlag & AT_USART_RCV_DIS_END) == 0){
        _SetRcvState(pAtUsart, AT_USART_RCV_STATE_DOING_END2);
        return 0;
      }
      //ֻ��һ�������ַ�ʱ���������
      _RcvFinal(pAtUsart, 0);
      return 1;
    }
    pAtUsart->Rcv.Len++;//������һ��������
    return 0;
  }

  //==========================�״ν���ʱ���===================================
  if(State == AT_USART_RCV_STATE_WAIT){
    //û��ǰ��ʱ������
    if((pAtUsart->RcvFlag & AT_USART_RCV_DIS_START) == AT_USART_RCV_DIS_START){
      return _UsartRcvToDoing(pAtUsart, 1);
    }
    //ʶ���յ���ǰ���ַ�
    unsigned char RcvChar = pAtUsart->pUsartDev->RcvData;
    if(!(pAtUsart->RcvFlag & AT_USART_RCV_DIS_SCR)){//����CRǰ��ʶ��
      if(RcvChar != '\r'){//ǰ������
        _RcvFinal(pAtUsart, -1);
        return -1;
      }
      //�׸�ʶ��ɹ���
      if(pAtUsart->RcvFlag & AT_USART_RCV_DIS_SLF){//����Ҫʶ��ڶ���ʱ����ʽ����
        return _UsartRcvToDoing(pAtUsart, 0);
      }
      //����Ҫʶ��ڶ���
      _SetRcvState(pAtUsart, AT_USART_RCV_STATE_WAIT_START2);
      return 0;
    }
    else if(!(pAtUsart->RcvFlag & AT_USART_RCV_DIS_SLF)){//ֻ��LFʱ��ǰ��ʶ��
      if(RcvChar != '\n'){//ǰ������
        _RcvFinal(pAtUsart, -2);
        return -1;
      }
      //ʶ��ɹ���
      return _UsartRcvToDoing(pAtUsart, 0);
    }
  }
  
  //==========================�ڶ�ǰ���ַ�ʶ��===================================
  if(State == AT_USART_RCV_STATE_WAIT_START2){
    if(pAtUsart->pUsartDev->RcvData != '\n'){
      _RcvFinal(pAtUsart, -2);
      return -1;
    }
    //ʶ�����
    return _UsartRcvToDoing(pAtUsart, 0);
  }
  //==========================�ڶ����ַ�ʶ��===================================
  if(State == AT_USART_RCV_STATE_DOING_END2){
    if(pAtUsart->pUsartDev->RcvData != '\n'){
      _RcvFinal(pAtUsart, -3);
      return -1;
    }
    //ʶ����˽������
    _RcvFinal(pAtUsart, 0);
    return 1;
  }

  //״̬���쳣
  _RcvFinal(pAtUsart,3);
  return 1;
}

//--------------------------------���ղ���--------------------------------------
//�������ã�AT_USART_RCV_DIS_ALL��
void AtUsart_RcvCfg(struct _AtUsart *pAtUsart, unsigned char Cfg)
{
  pAtUsart->RcvFlag &= ~AT_USART_RCV_DIS_ALL;
  pAtUsart->RcvFlag |= Cfg; 
}

//��ʼ��������
void AtUsart_RcvStart(struct _AtUsart *pAtUsart)
{
  pAtUsart->Rcv.Len = 0;
  pAtUsart->RcvTimer = pAtUsart->RcvWaitOv;
  pAtUsart->RcvFlag &= ~(AT_USART_RCV_STATE_MASK | AT_USART_RCV_ALL_MODE);
  pAtUsart->RcvFlag |= AT_USART_RCV_STATE_WAIT;
  UsartDev_RcvStart(pAtUsart->pUsartDev,
                     pAtUsart->Rcv.pBuf,           //���ջ�����
                     pAtUsart->Rcv.Count,          //�������ݴ�С,�Ƚ���һ������Ӧ
                     _UsartRcvNotify);      //���ջص�����
}
//ǿ��ֹͣ��������
void AtUsart_RcvStop(struct _AtUsart *pAtUsart)
{
  AtUsart_cbRcvEndNotify(pAtUsart->DevId); 
  UsartDev_RcvStop(pAtUsart->pUsartDev);
  pAtUsart->RcvFlag &= ~AT_USART_RCV_STATE_MASK;
}





