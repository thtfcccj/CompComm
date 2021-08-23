/******************************************************************

*         USART精简接口-在使用单例化的UsartDev时的实现
* 需支持SUPPORT_USART_DEV_CFG_TINY
* 此实现独立于硬件
*******************************************************************/

#include "UsartTiny.h"
#include "IOCtrl.h"
#include "UsartDev.h"
#include <string.h>

struct _UsartTiny UsartTiny; //直接实例化
struct _UsartDev UsartDev;  //直接单例化

/*********************************************************************
                        IoCtrl需要的其它资源
***********************************************************************/

//#define UsartTiny_pcbGetHw()   //硬件指针
//#define UsartTiny_cbUsartCfg(cfg)   //struct _UsartDevCfg硬件配置函数

/*********************************************************************
                             相关函数实现
***********************************************************************/

//-----------------------------初始化函数------------------------------
//调用此函数前,需对Usart参数进行配置
void UsartTiny_Init(void)
{
  memset(&UsartTiny, 0, sizeof(struct _UsartTiny));
  UsartTiny_cbInit(); //相关初始化
  UsartTiny_Stop();
  UsartTiny.eState =  UsartTiny_eState_Idie;

  //准备好串口
  UsartDev_Init(&UsartDev,
                UsartTiny_pcbGetHw());  //挂接的硬件
}

//-----------------------底层硬件配置函数----------------------
void UsartTiny_CfgHw(unsigned char Cfg)
{
  UsartTiny_cbUsartCfg((struct _UsartDevCfg*)&Cfg);
}

//---------------------------停止数据收发函数-------------------------
//收发数据过程中中止数据收发
void UsartTiny_Stop(void)
{
  UsartDev_RcvStop(&UsartDev);
  UsartDev_SendStop(&UsartDev);  
  //复位状态机与内部变量
  //这里不清UsartTiny.Index以便在停止后仍可获知当前收发数据个数情况
  UsartTiny.eState = UsartTiny_eState_Idie;
  UsartTiny_cbStopNotify();//停止通报
}

//-----------------------接收中断处理函数--------------------------------
//形参为返struct _UsartDev指针
//回状态定义为:0:继续收发,其它:停止收发
static signed char _RcvEndInt(void *pv)
{
  if(UsartTiny.eState != UsartTiny_eState_Rcv){
    UsartTiny_cbErrNotify(-1);
    return -1;//异常丢弃
  }
  UsartTiny.Index++;
  UsartTiny_cbRcvDataNotify();//接收数据通报  
  //防止数据溢出
  if(UsartTiny.Index >= USART_TINY_DATA_MAX){
    UsartTiny_cbRcvDataOv();
    return -1;//停止接收
  }
  return 0;  
}

//----------------------------启动接收函数------------------------------
void UsartTiny_RcvStart(void)
{
  UsartTiny_Stop();//无条件停止
  UsartTiny.Index = 0;
  UsartTiny.eState = UsartTiny_eState_Rcv;
  
  UsartTiny_cbRcvStartNotify();//启动接收通报
  UsartDev_RcvStart(&UsartDev, 
                    UsartTiny.Data, 
                    USART_TINY_DATA_MAX, //手动接收
                    _RcvEndInt);
}

//-----------------------发送中断处理函--------------------------------
//形参为返struct _UsartDev指针
//回状态定义为:0:继续收发,其它:
static signed char _SendEndInt(void *pv)
{
  //状态机错误
  if(UsartTiny.eState != UsartTiny_eState_Send){   
     UsartTiny_cbErrNotify(-2); 
     return -1; //停止收发
  }
  
  if(UsartDev.SenLen < UsartDev.SendCount)//发送完成了一个数据
    UsartTiny_cbSendDataNotify();
  else{//发送完成了
    UsartTiny.eState = UsartTiny_eState_Idie;
  }
  
  return 0;  
}

//----------------------------启动发送函数------------------------------
void UsartTiny_SendStart(unsigned char SendLen)
{
  //UsartTiny_Stop();//无条件停止

  //配置寄存器为发送模式,开始发送数据
  UsartTiny.SendLen = SendLen;
  UsartTiny.Index = 0;
  UsartTiny.eState = UsartTiny_eState_Send;

  UsartTiny_cbSendStartNotify();   //启动发送通报 
  UsartDev_SendStart(&UsartDev, 
                     UsartTiny.Data, 
                     SendLen, //手动模式
                     _SendEndInt);
}

