/***************************************************************************

//			                 总线从机-Usart实现

***************************************************************************/

#include "BusSlvUsart.h"
#include "BusId.h"
#include "Usart.h"      //通讯
//通讯配置结构需扩展 #define USART_DEV_CFG_USER UsartDevCfgUser0_t U;
#include "UsartDevCfg.h" 

#include "struct.h"
#include <string.h>

//---------------------------内部定义--------------------------
//启用基类预留变量;
#define STATE     Base.ExU8           //标志,具体定义为：
  #define _RCV_WAIT     1           //接收等待中
  #define _RCV_DOING    2           //接收数据中
  #define _SEND_DOING   3           //发送数据中

#define TIMER     Base.Ex.U8[0]     //当前定时器值
#define TIMER_OV  Base.Ex.U8[1]     //当前定时器超时值

/***********************************************************************
                            相关函数实现
***********************************************************************/

//-----------------------------初始化函数-------------------------------
void BusSlvUsart_Init(struct _BusSlvUsart *pBus,
                      unsigned char BusId)    //分配的总线ID号
{
  memset(pBus, 0, sizeof(struct _BusSlvUsart));
  //初始化基类
  BusBase_Init(&pBus->Base, BusId);
}

//------------------------------接收中断处理函数------------------------------
//形参为返struct _UsartDev指针
//回状态定义为:0:继续收发,其它:停止收发
static signed char _RcvFinal(void *pv)
{
  struct _UsartDev *pUsart = (struct _UsartDev *)pv;
  struct _BusSlvUsart *pBus = struct_get(pUsart->pRcvBuf, 
                                         struct _BusSlvUsart, DataBuf);
  pBus->STATE = _RCV_DOING;//取消等待中
  pBus->TIMER = pBus->TIMER_OV; //有响应，定时器复位
  
  //接收完成了
  if(pUsart->RcvLen >= pUsart->RcvCount){
    pBus->TIMER = 0;//直接中止
    return 1; //停止
  }
  return 0;//接收未完成
}

//-----------------------发送巡检码中断处理函数--------------------------------
//形参为返struct _UsartDev指针
//回状态定义为:0:继续收发,其它:停止收发
static signed char _SendFinal(void *pv)
{
  struct _UsartDev *pUsart = (struct _UsartDev *)pv;
  struct _BusSlvUsart *pBus = struct_get(pUsart->pSendBuf, 
                                         struct _BusSlvUsart, DataBuf);
  pBus->TIMER = pBus->TIMER_OV; //有响应，定时器复位
  //发送完成了
  if(pUsart->SenLen >= pUsart->SendCount){
     pBus->STATE = 0;//空闲状态
     
    //准备普通任务置接收
  }
  return 0;//发送未完成
}

//-------------------------------普通Tick查询任务----------------------------
//将此函数放入系统1ms进程中
void BusSlvUsart_TickTask(struct _BusSlvUsart *pBus)
{
  if(pBus->TIMER) return;//时间未到
  
  unsigned char UsartId = BusId_GetSubId(pBus->Base.Id);
  struct _UsartDev *pUsart = Usart_GetDev(UsartId);
  
  //接收等待中超时
  if(pBus->STATE & _RCV_WAIT){
    UsartDev_RcvStop(pUsart); //强制中止接收
    pBus->STATE = 0;
  }
  //数据接收完成
  else if(pBus->STATE & _RCV_DOING){
    pBus->Base.Count.Comm++;//接收数据计数

    UsartDev_RcvStop(pUsart); //强制中止接收
    pBus->STATE = 0;//停止数据接收
    
    signed short Resume = BusSlvUsart_cbDataPro(pBus, //数据处理
                            pUsart->RcvLen,
                            UsartDevCfg[UsartId].U.S.Adr);
    if(Resume > 0){ //数据正确,发送数据
       pBus->Base.Count.Valid++; //有效计数
       BusId_CtrlUsartRTS(pBus->Base.Id, 1); //发送状态
       UsartDev_SendStart(pUsart,
                          pBus->DataBuf, Resume, _SendFinal);
       
       pBus->STATE = _SEND_DOING;//启动发送
       pBus->TIMER = pBus->TIMER_OV;//计算发送超时,也是一帧为单位
     }
     else if(Resume < 0) pBus->Base.Count.Invalid++;//无效计数
     //else // if(Resume == 0) 广播不计数
  }
  //发送超时完成了
  else if(pBus->STATE){
    UsartDev_SendStop(pUsart); 
    pBus->STATE = 0;//空闲状态
  }
  //最后空闲状态启动接收
  if(pBus->STATE == 0){
    pBus->STATE = _RCV_WAIT; //接收等待    
    BusId_CtrlUsartRTS(pBus->Base.Id, 0); //接收状态
    UsartDev_RcvStart(pUsart, 
                      pBus->DataBuf, BUS_SLV_USART_DATA_SIZE, _RcvFinal);
    pBus->TIMER_OV = UsartDevCfg[UsartId].U.S.SpaceT; //预读
    pBus->TIMER = 255;//等待中,置最长时间防止死机
    return;
  } 
  
  
}

