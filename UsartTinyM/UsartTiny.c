/******************************************************************

*         多例化USART精简模块对外接口
* 需支持SUPPORT_USART_DEV_CFG_TINY
* 此实现独立于硬件
*******************************************************************/

#include "UsartTiny.h"
#include "IOCtrl.h"
#include "UsartDev.h"
#include <string.h>

/*********************************************************************
                             相关函数实现
***********************************************************************/

//-----------------------------初始化函数------------------------------
//调用此函数前,需初始化UsartDev(含UsartId)及其IO，及对Usart参数进行配置
void UsartTiny_Init(struct _UsartTiny *pTiny,
                    const struct _UsartDevPt *pFun, //多态操作函数
                    struct _UsartDev *pDev)  ///持有的对像
{
  memset(pTiny, 0, sizeof(struct _UsartTiny));
  pTiny->pFun = pFun;  
  pTiny->pDev = pDev;
  UsartTiny_Stop(pTiny);
  pTiny->eState =  UsartTiny_eState_Idie;
}

//---------------------------停止数据收发函数-------------------------
//收发数据过程中中止数据收发
void UsartTiny_Stop(struct _UsartTiny *pTiny)
{
  pTiny->pFun->RcvStop(pTiny->pDev);
  pTiny->pFun->SendStop(pTiny->pDev);  
  //复位状态机与内部变量
  //这里不清pTiny->Index以便在停止后仍可获知当前收发数据个数情况
  pTiny->eState = UsartTiny_eState_Idie;
  UsartTiny_cbStopNotify(pTiny);//停止通报
}

//-----------------------接收中断处理函数--------------------------------
//形参为返struct _UsartDev指针
//回状态定义为:0:继续收发,其它:停止收发
static signed char _RcvEndInt(void *pv)
{
  struct _UsartDev *pDev = (struct _UsartDev *)pv;
  struct _UsartTiny *pTiny = (struct _UsartTiny *)(pDev->pVoid);
  if(pTiny->eState != UsartTiny_eState_Rcv){
    UsartTiny_cbErrNotify(pTiny, -1);
    return -1;//异常丢弃
  }
  pTiny->Index++;
  UsartTiny_cbRcvDataNotify(pTiny);//接收数据通报  
  //防止数据溢出
  if(pTiny->Index >= USART_TINY_DATA_MAX){
    UsartTiny_cbRcvDataOv();
    return -1;//停止接收
  }
  return 0;  
}

//----------------------------启动接收函数------------------------------
void UsartTiny_RcvStart(struct _UsartTiny *pTiny)
{
  UsartTiny_Stop(pTiny);//无条件停止
  pTiny->Index = 0;
  pTiny->eState = UsartTiny_eState_Rcv;
  
  UsartTiny_cbRcvStartNotify(pTiny);//启动接收通报
  pTiny->pDev->pVoid = pTiny;//回调使用
  pTiny->pFun->RcvStart(pTiny->pDev, 
                    pTiny->Data, 
                    USART_TINY_DATA_MAX, //手动接收
                    _RcvEndInt);
}

//-----------------------发送中断处理函--------------------------------
//形参为返struct _UsartDev指针
//回状态定义为:0:继续收发,其它:
static signed char _SendEndInt(void *pv)
{
  struct _UsartDev *pDev = (struct _UsartDev *)pv;
  struct _UsartTiny *pTiny = (struct _UsartTiny *)(pDev->pVoid);
  //状态机错误
  if(pTiny->eState != UsartTiny_eState_Send){   
     UsartTiny_cbErrNotify(pTiny, -2); 
     return -1; //停止收发
  }
  
  if(pDev->SenLen < pDev->SendCount)//发送完成了一个数据
    UsartTiny_cbSendDataNotify(pTiny);
  else{//发送完成了
    pTiny->eState = UsartTiny_eState_Idie;
  }
  
  return 0;  
}

//----------------------------启动发送函数------------------------------
void UsartTiny_SendStart(struct _UsartTiny *pTiny,
                         unsigned char SendLen)
{
  //UsartTiny_Stop();//无条件停止

  //配置寄存器为发送模式,开始发送数据
  pTiny->SendLen = SendLen;
  pTiny->Index = 0;
  pTiny->eState = UsartTiny_eState_Send;

  UsartTiny_cbSendStartNotify(pTiny);   //启动发送通报
  pTiny->pDev->pVoid = pTiny;//回调使用  
  pTiny->pFun->SendStart(pTiny->pDev, 
                         pTiny->Data, 
                         SendLen, //手动模式
                          _SendEndInt);
}

