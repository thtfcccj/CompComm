/*******************************************************************************

                    ��Ϣ������ʵ��
//��ģ�鸺����Ϣ�����е��������ӹ���
*******************************************************************************/

#include "MsgPacketQ.h"
#include <string.h>

/***********************************************************************
		                       ��غ���ʵ��
***********************************************************************/

//--------------------------��ʼ������---------------------------------
void MsgPacketQ_Init(struct _MsgPacketQ *pMng, //δʵʼ���Ľṹ
                    unsigned char MsgQLen)  //��Ϣ���г���
{
  //memset(pMng, 0, sizeof(struct _MsgPacketQ));
  pMng->pMsgQ = pSoftQCreate(MsgQLen, sizeof(struct _MsgPacket));
}

//--------------------------��Ϣ���---------------------------------
void MsgPacketQ_Push(const struct _MsgPacketQ *pMng,
                     const struct _MsgPacket *pMsg)
{
  SoftQSend(pMng->pMsgQ, pMsg);
}

//--------------------------��Ϣ����---------------------------------
//����NULL����Ϣ�����򷵻��յ�����Ϣ
struct _MsgPacket *MsgPacketQ_Pop(struct _MsgPacketQ *pMng)
{
  if(SoftQReceive(pMng->pMsgQ, &pMng->RdMsg) != 0)//û����Ϣ
    return NULL;
  return &pMng->RdMsg;
}



