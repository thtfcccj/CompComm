/***********************************************************************

                  Sms用户通讯层
***********************************************************************/


#include "AtCmd.h"
#include <string.h>

/******************************************************************************
		                     非阻塞方式操作函数
******************************************************************************/

//-----------------------------启动写函数---------------------------------
//非阻塞方式工作，启动后即返回
void AtCmd_WrStart(struct _AtUsart *pAtUsart,
                   const unsigned char *pWrData, //待写入的数据
                   unsigned short Len,//数据长度
                   unsigned char Cfg)//AtUsart定义的写配置
{
  //未使用默认缓冲区时
  unsigned char *pSendBuf = AtUsart_pGetSendBuf(pAtUsart); 
  if(pWrData != pSendBuf) memcpy(pSendBuf, pWrData, Len);
  AtUsart_SendCfg(pAtUsart, Cfg);
  AtUsart_SendBuf(pAtUsart, Len);
}
  
//-----------------------------启动写AT函数---------------------------------
//非阻塞方式工作，启动后即返回
void AtCmd_WrAtStart(struct _AtUsart *pAtUsart,
                     const char *pCmd)  //写命令,不含AT开始与结束字符
{
  AtUsart_DisWrAutoRcv(pAtUsart);         //禁止自动接收
  AtCmd_WrStart(pAtUsart, (const unsigned char*)pCmd, strlen(pCmd), 0);             
}

//-----------------------------启动读函数---------------------------------
//非阻塞方式工作，启动后即返回
void AtCmd_RdStart(struct _AtUsart *pAtUsart,
                   unsigned char Cfg,//AtUsart定义的读配置
                   unsigned short WaitOv,//等待数据ms单位时间，0一直等待
                   unsigned short DongOv)//数据接收间隔ms单位时间，0一直收  
{
  AtUsart_RcvCfg(pAtUsart, Cfg);
  AtUsart_SetRcvWaitOv(pAtUsart, WaitOv);
  AtUsart_SetRcvDoingOv(pAtUsart, DongOv);  
  AtUsart_RcvStart(pAtUsart);
}

//-----------------------------启动读AT函数---------------------------------
//非阻塞方式工作，启动后即返回
void AtCmd_RdAtStart(struct _AtUsart *pAtUsart)
{
  AtCmd_RdStart(pAtUsart, 0, AT_CMD_BLOCKING_RD_WAIT_OV,
                             AT_CMD_BLOCKING_RD_DOING_OV);
}

//-----------------------------启动写后自动读AT函数-----------------------------
//非阻塞方式工作，启动后即返回
void AtCmd_RwAtStart(struct _AtUsart *pAtUsart,
                     const char *pCmd)  //写命令,不含AT开始与结束字符
{
  //配置接收
  AtUsart_EnWrAutoRcv(pAtUsart);         //允许自动接收
  AtUsart_RcvCfg(pAtUsart, 0);            //标准AT模式
  AtUsart_SetRcvWaitOv(pAtUsart, AT_CMD_BLOCKING_RD_WAIT_OV); 
  AtUsart_SetRcvDoingOv(pAtUsart, AT_CMD_BLOCKING_RD_DOING_OV); 
  //启动发送
  AtCmd_WrStart(pAtUsart, (const unsigned char*)pCmd, strlen(pCmd), 0);  
}

/******************************************************************************
		                     阻塞方式读写数据函数
******************************************************************************/

//--------------------------带配置写函数---------------------------------
//阻塞方式工作，返回0成功,否则未写入
signed char AtCmd_CfgWr(struct _AtUsart *pAtUsart,
                         const unsigned char *pWrData, //待写入的数据
                         unsigned short Len,//数据长度
                         unsigned char Cfg)//AtUsart定义的写配置
{
  AtUsart_DisWrAutoRcv(pAtUsart);         //禁止自动接收
  AtCmd_WrStart(pAtUsart, pWrData, Len, Cfg);     //启动写
  while(!AtUsart_IsSendFinal(pAtUsart)); //等待完成
  return AtCmd_cbGetState(pAtUsart);     //故障码直接返回
}

//--------------------------带配置读函数---------------------------------
//阻塞方式工作，返回0成功,否则未写入
signed char AtCmd_CfgRd(struct _AtUsart *pAtUsart,
                         unsigned char Cfg)//AtUsart定义的读配置
{
  AtCmd_RdStart(pAtUsart, Cfg,AT_CMD_BLOCKING_RD_WAIT_OV,
                AT_CMD_BLOCKING_RD_DOING_OV);
  while(!AtUsart_IsRcvFinal(pAtUsart)); //等待接收完成
  return AtCmd_cbGetState(pAtUsart);     //故障码直接返回
}

//------------------------带配置写后读返回状态函数---------------------------------
//阻塞方式工作，返回0成功,否则返回错误码
signed char AtCmd_CfgRw(struct _AtUsart *pAtUsart,
                         const unsigned char *pWrData, //待写入的数据
                         unsigned short Len,//数据长度
                         unsigned char WrCfg,  //AtUsart定义的读配置
                         unsigned char RdCfg)  //AtUsart定义的读配置                      
{
  unsigned char WrState = AtCmd_CfgWr(pAtUsart, pWrData, Len, WrCfg);
  if(WrState) return WrState;     //写有误
  return AtCmd_CfgRd(pAtUsart, RdCfg);
}

/******************************************************************************
		                     阻塞方式读写字符函数,返回字符串
******************************************************************************/
//-----------------------读结束后返回结果字符函数---------------------------------
//阻塞方式工作，成功时返回读取的字符串,否则返回NULL
static const char *_pGetRdStr(struct _AtUsart *pAtUsart) 
{
  unsigned short Len = AtUsart_GetRcvSize(pAtUsart);
  char *pStr = (char*)AtUsart_pGetRcvBuf(pAtUsart);
  *(pStr + Len) = '\0';//强制加结束字符
  return pStr;  
}

//------------------------带配置写后读回字符串函数---------------------------------
//阻塞方式工作，成功时返回读取的字符串,否则返回NULL
const char *AtCmd_pCfgRwStr(struct _AtUsart *pAtUsart,
                            const char *pCmd,     //写命令,不含AT开始与结束字符
                            unsigned char WrCfg,  //AtUsart定义的读配置
                            unsigned char RdCfg)  //AtUsart定义的读配置                      
{
  unsigned char State = AtCmd_CfgWr(pAtUsart,(const unsigned char*)pCmd, 
                                    strlen(pCmd), WrCfg);
  if(State) return NULL;     //写有误
  State = AtCmd_CfgRd(pAtUsart, RdCfg);
  if(State) return NULL;     //读错误
  return _pGetRdStr(pAtUsart);
}

//---------------------------写标准AT指令函数---------------------------------
//阻塞方式工作，返回结果，0正确
signed char AtCmd_WrAt(struct _AtUsart *pAtUsart,
                        const char *pCmd)       //写命令,不含开始结束字符
{
  return AtCmd_CfgWr(pAtUsart,(const unsigned char*)pCmd, strlen(pCmd), 0);
}

//---------------------------写并读启动标准AT指令函数---------------------------------
//阻塞方式工作，返回结果，0正确
signed char AtCmd_RwAtStrStart(struct _AtUsart *pAtUsart,
                               const char *pCmd)       //写命令,不含开始结束字符
{
  unsigned char State = AtCmd_CfgWr(pAtUsart, (const unsigned char*)pCmd, 
                                    strlen(pCmd), 0);
  if(State) return NULL;     //写有误
  return AtCmd_CfgRd(pAtUsart, 0);
}

//---------------------------写并读回标准AT指令函数---------------------------------
//阻塞方式工作，返回读回的数据缓冲区(不含返回的开始结束字符)，NULL表示错误
//数据正确时，接收长度通过AtCmd_GetRdLen()获得
const char *AtCmd_pRwAt(struct _AtUsart *pAtUsart,
                         const char *pCmd)       //写命令,不含开始结束字符
{
  unsigned char State = AtCmd_CfgWr(pAtUsart, (const unsigned char*)pCmd, 
                                    strlen(pCmd), 0);
  if(State) return NULL;     //写有误
  State = AtCmd_CfgRd(pAtUsart, 0);
  if(State) return NULL;     //读错误
  return _pGetRdStr(pAtUsart);
}



