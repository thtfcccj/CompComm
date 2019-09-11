/***********************************************************************

                  Sms�û�ͨѶ��
***********************************************************************/


#include "AtCmd.h"
#include <string.h>

/******************************************************************************
		                     ��������ʽ��������
******************************************************************************/

//-----------------------------����д����---------------------------------
//��������ʽ�����������󼴷���
void AtCmd_WrStart(struct _AtUsart *pAtUsart,
                   const unsigned char *pWrData, //��д�������
                   unsigned short Len,//���ݳ���
                   unsigned char Cfg)//AtUsart�����д����
{
  //δʹ��Ĭ�ϻ�����ʱ
  unsigned char *pSendBuf = AtUsart_pGetSendBuf(pAtUsart); 
  if(pWrData != pSendBuf) memcpy(pSendBuf, pWrData, Len);
  AtUsart_SendCfg(pAtUsart, Cfg);
  AtUsart_SendBuf(pAtUsart, Len);
}
  
//-----------------------------����дAT����---------------------------------
//��������ʽ�����������󼴷���
void AtCmd_WrAtStart(struct _AtUsart *pAtUsart,
                     const char *pCmd)  //д����,����AT��ʼ������ַ�
{
  AtUsart_DisWrAutoRcv(pAtUsart);         //��ֹ�Զ�����
  AtCmd_WrStart(pAtUsart, (const unsigned char*)pCmd, strlen(pCmd), 0);             
}

//-----------------------------����������---------------------------------
//��������ʽ�����������󼴷���
void AtCmd_RdStart(struct _AtUsart *pAtUsart,
                   unsigned char Cfg,//AtUsart����Ķ�����
                   unsigned short WaitOv,//�ȴ�����ms��λʱ�䣬0һֱ�ȴ�
                   unsigned short DongOv)//���ݽ��ռ��ms��λʱ�䣬0һֱ��  
{
  AtUsart_RcvCfg(pAtUsart, Cfg);
  AtUsart_SetRcvWaitOv(pAtUsart, WaitOv);
  AtUsart_SetRcvDoingOv(pAtUsart, DongOv);  
  AtUsart_RcvStart(pAtUsart);
}

//-----------------------------������AT����---------------------------------
//��������ʽ�����������󼴷���
void AtCmd_RdAtStart(struct _AtUsart *pAtUsart)
{
  AtCmd_RdStart(pAtUsart, 0, AT_CMD_BLOCKING_RD_WAIT_OV,
                             AT_CMD_BLOCKING_RD_DOING_OV);
}

//-----------------------------����д���Զ���AT����-----------------------------
//��������ʽ�����������󼴷���
void AtCmd_RwAtStart(struct _AtUsart *pAtUsart,
                     const char *pCmd)  //д����,����AT��ʼ������ַ�
{
  //���ý���
  AtUsart_EnWrAutoRcv(pAtUsart);         //�����Զ�����
  AtUsart_RcvCfg(pAtUsart, 0);            //��׼ATģʽ
  AtUsart_SetRcvWaitOv(pAtUsart, AT_CMD_BLOCKING_RD_WAIT_OV); 
  AtUsart_SetRcvDoingOv(pAtUsart, AT_CMD_BLOCKING_RD_DOING_OV); 
  //��������
  AtCmd_WrStart(pAtUsart, (const unsigned char*)pCmd, strlen(pCmd), 0);  
}

/******************************************************************************
		                     ������ʽ��д���ݺ���
******************************************************************************/

//--------------------------������д����---------------------------------
//������ʽ����������0�ɹ�,����δд��
signed char AtCmd_CfgWr(struct _AtUsart *pAtUsart,
                         const unsigned char *pWrData, //��д�������
                         unsigned short Len,//���ݳ���
                         unsigned char Cfg)//AtUsart�����д����
{
  AtUsart_DisWrAutoRcv(pAtUsart);         //��ֹ�Զ�����
  AtCmd_WrStart(pAtUsart, pWrData, Len, Cfg);     //����д
  while(!AtUsart_IsSendFinal(pAtUsart)); //�ȴ����
  return AtCmd_cbGetState(pAtUsart);     //������ֱ�ӷ���
}

//--------------------------�����ö�����---------------------------------
//������ʽ����������0�ɹ�,����δд��
signed char AtCmd_CfgRd(struct _AtUsart *pAtUsart,
                         unsigned char Cfg)//AtUsart����Ķ�����
{
  AtCmd_RdStart(pAtUsart, Cfg,AT_CMD_BLOCKING_RD_WAIT_OV,
                AT_CMD_BLOCKING_RD_DOING_OV);
  while(!AtUsart_IsRcvFinal(pAtUsart)); //�ȴ��������
  return AtCmd_cbGetState(pAtUsart);     //������ֱ�ӷ���
}

//------------------------������д�������״̬����---------------------------------
//������ʽ����������0�ɹ�,���򷵻ش�����
signed char AtCmd_CfgRw(struct _AtUsart *pAtUsart,
                         const unsigned char *pWrData, //��д�������
                         unsigned short Len,//���ݳ���
                         unsigned char WrCfg,  //AtUsart����Ķ�����
                         unsigned char RdCfg)  //AtUsart����Ķ�����                      
{
  unsigned char WrState = AtCmd_CfgWr(pAtUsart, pWrData, Len, WrCfg);
  if(WrState) return WrState;     //д����
  return AtCmd_CfgRd(pAtUsart, RdCfg);
}

/******************************************************************************
		                     ������ʽ��д�ַ�����,�����ַ���
******************************************************************************/
//-----------------------�������󷵻ؽ���ַ�����---------------------------------
//������ʽ�������ɹ�ʱ���ض�ȡ���ַ���,���򷵻�NULL
static const char *_pGetRdStr(struct _AtUsart *pAtUsart) 
{
  unsigned short Len = AtUsart_GetRcvSize(pAtUsart);
  char *pStr = (char*)AtUsart_pGetRcvBuf(pAtUsart);
  *(pStr + Len) = '\0';//ǿ�Ƽӽ����ַ�
  return pStr;  
}

//------------------------������д������ַ�������---------------------------------
//������ʽ�������ɹ�ʱ���ض�ȡ���ַ���,���򷵻�NULL
const char *AtCmd_pCfgRwStr(struct _AtUsart *pAtUsart,
                            const char *pCmd,     //д����,����AT��ʼ������ַ�
                            unsigned char WrCfg,  //AtUsart����Ķ�����
                            unsigned char RdCfg)  //AtUsart����Ķ�����                      
{
  unsigned char State = AtCmd_CfgWr(pAtUsart,(const unsigned char*)pCmd, 
                                    strlen(pCmd), WrCfg);
  if(State) return NULL;     //д����
  State = AtCmd_CfgRd(pAtUsart, RdCfg);
  if(State) return NULL;     //������
  return _pGetRdStr(pAtUsart);
}

//---------------------------д��׼ATָ���---------------------------------
//������ʽ���������ؽ����0��ȷ
signed char AtCmd_WrAt(struct _AtUsart *pAtUsart,
                        const char *pCmd)       //д����,������ʼ�����ַ�
{
  return AtCmd_CfgWr(pAtUsart,(const unsigned char*)pCmd, strlen(pCmd), 0);
}

//---------------------------д����������׼ATָ���---------------------------------
//������ʽ���������ؽ����0��ȷ
signed char AtCmd_RwAtStrStart(struct _AtUsart *pAtUsart,
                               const char *pCmd)       //д����,������ʼ�����ַ�
{
  unsigned char State = AtCmd_CfgWr(pAtUsart, (const unsigned char*)pCmd, 
                                    strlen(pCmd), 0);
  if(State) return NULL;     //д����
  return AtCmd_CfgRd(pAtUsart, 0);
}

//---------------------------д�����ر�׼ATָ���---------------------------------
//������ʽ���������ض��ص����ݻ�����(�������صĿ�ʼ�����ַ�)��NULL��ʾ����
//������ȷʱ�����ճ���ͨ��AtCmd_GetRdLen()���
const char *AtCmd_pRwAt(struct _AtUsart *pAtUsart,
                         const char *pCmd)       //д����,������ʼ�����ַ�
{
  unsigned char State = AtCmd_CfgWr(pAtUsart, (const unsigned char*)pCmd, 
                                    strlen(pCmd), 0);
  if(State) return NULL;     //д����
  State = AtCmd_CfgRd(pAtUsart, 0);
  if(State) return NULL;     //������
  return _pGetRdStr(pAtUsart);
}



