/*******************************************************************************

                    消息包队列实现
//此模块负责消息包队列的入队与出队管理
*******************************************************************************/

#include "MsgPacketQ.h"
#include <string.h>

/***********************************************************************
		                       相关函数实现
***********************************************************************/

//--------------------------初始化函数---------------------------------
void MsgPacketQ_Init(struct _MsgPacketQ *pMng, //未实始化的结构
                    unsigned char MsgQLen)  //消息队列长度
{
  //memset(pMng, 0, sizeof(struct _MsgPacketQ));
  pMng->pMsgQ = pSoftQCreate(MsgQLen, sizeof(struct _MsgPacket));
}

//--------------------------消息入队---------------------------------
void MsgPacketQ_Push(const struct _MsgPacketQ *pMng,
                     const struct _MsgPacket *pMsg)
{
  SoftQSend(pMng->pMsgQ, pMsg);
}

//--------------------------消息出队---------------------------------
//返回NULL无消息，否则返回收到的消息
struct _MsgPacket *MsgPacketQ_Pop(struct _MsgPacketQ *pMng)
{
  if(SoftQReceive(pMng->pMsgQ, &pMng->RdMsg) != 0)//没有消息
    return NULL;
  return &pMng->RdMsg;
}



