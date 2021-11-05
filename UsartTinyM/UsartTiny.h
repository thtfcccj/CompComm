/******************************************************************

//               多例化USART精简模块对外接口
//此模块为UsartTiny的多例化，用于驱动多个UsartDev硬件
//2.仅不支持自动发送模式(发送过程中需上层干预)。
//3.仅支持半双工模式
//注:此模块仅负责数据收发,不负责底层通讯参数的配置
*******************************************************************/

#ifndef __USART_TINY_H
#define __USART_TINY_H
#ifdef SUPPORT_EX_PREINCLUDE//不支持Preinlude時
  #include "Preinclude.h"
#endif

/*********************************************************************
                             相关配置与定义
***********************************************************************/

//数据最大个数, < 255
#ifndef USART_TINY_DATA_MAX
  #define USART_TINY_DATA_MAX      (32 + 5)	 //3号寄存器返回最大个数
#endif

/*********************************************************************
                             相关结构
***********************************************************************/

//工作状态机
enum _UsartTiny_eState{
  UsartTiny_eState_Idie      = 0,  //空闲状态，等待外部外部收发指令
  UsartTiny_eState_Rcv       = 1,  //接收数据状态
  //UsartTiny_eState_RcvFinal  = 2,  //接收数据完成状态
  UsartTiny_eState_Send      = 3,  //发送数据状态
  //UsartTiny_eState_SendFinal = 4,  //发送数据完成状态
};

struct _UsartTiny{
  struct _UsartDev *pDev;  //持用不拥有允许外部使用()
  enum _UsartTiny_eState  eState; //工作状态机
  unsigned char Index;   //收发数据位置
  unsigned char SendLen; //发送数据个数  
  unsigned char Data[USART_TINY_DATA_MAX]; //收发数据缓冲
};

/*********************************************************************
                             相关行为函数
***********************************************************************/

//-----------------------------初始化函数------------------------------
//调用此函数前,需初始化UsartDev(含UsartId)及其IO，及对Usart参数进行配置
void UsartTiny_Init(struct _UsartTiny *pTiny,
                    struct _UsartDev *pDev);

//---------------------------停止数据收发函数-------------------------
//收发数据过程中中止数据收发
void UsartTiny_Stop(struct _UsartTiny *pTiny); 

//----------------------------启动接收函数------------------------------
void UsartTiny_RcvStart(struct _UsartTiny *pTiny);

//----------------------------启动发送函数------------------------------
void UsartTiny_SendStart(struct _UsartTiny *pTiny,
                         unsigned char SendLen);

/*********************************************************************
                             相关成员函数
***********************************************************************/
//注:暂直接上层调用

//----------------------------得到设备ID------------------------------
#define UsartTiny_GetDevId(tiny) ((tiny)->pDev->UsartId)

/************************************************************************
                             相关回调函数
***********************************************************************/

//部分函数直接实现:
//1.实现485的RT控制 2.指示灯控制(借用FireFreme的通讯指示灯软件接口),
//3上层接口
#include "IOCtrl.h" //RTS直接控制底层
#ifdef SUPPORT_TI_COMM_MNG
  #include "TiCommMng.h" 
  #define UTtoTC(tiny)  ((struct _TiCommMng*)tiny) //需对应使用多例化继承结构
#endif

//-----------------------------停止通报函数------------------------------
//void UsartTiny_cbStopNotify(struct _UsartTiny *pTiny);
#define UsartTiny_cbStopNotify(tiny) do{ClrRTS(UsartTiny_GetDevId(tiny));}while(0)

//-----------------------------接收启动通报函数------------------------------
//void UsartTiny_cbRcvStartNotify(struct _UsartTiny *pTiny);
#define UsartTiny_cbRcvStartNotify(tiny) UsartTiny_cbStopNotify(tiny)

//-----------------------------发送启动通报函数------------------------------
//void UsartTiny_cbSendStartNotify(struct _UsartTiny *pTiny);
#define UsartTiny_cbSendStartNotify(tiny) do{SetRTS(UsartTiny_GetDevId(tiny));}while(0)

//----------------------------中断内接收到数据通报函数------------------------------
#ifdef SUPPORT_TI_COMM_MNG
  #define UsartTiny_cbRcvDataNotify(tiny) do{TiCommMng_ResetRcvTimer(UTtoTC(tiny));}while(0)
#else
  void UsartTiny_cbRcvDataNotify(struct _UsartTiny *pTiny);  
#endif

//----------------------------中断内接收到数据溢出函数------------------------------
//void UsartTiny_cbRcvDataOv(struct _UsartTiny *pTiny);
#define UsartTiny_cbRcvDataOv(tiny) do{}while(0)

//----------------------------中断内发送出一个数据通报函数------------------------------
//void UsartTiny_cbSendDataNotify(struct _UsartTiny *pTiny);
#ifdef SUPPORT_TI_COMM_MNG
  #define UsartTiny_cbSendDataNotify(tiny) do{TiCommMng_ResetSendTimer(UTtoTC(tiny));}while(0)
#else
  void UsartTiny_cbSendDataNotify(const struct _UsartTiny *pTiny);
#endif
 
//----------------------------收发数据错误通报函数------------------------------
//void UsartTiny_cbErrNotify(struct _UsartTiny *pTiny, unsigned char Err);
#define UsartTiny_cbErrNotify(tiny, err) do{}while(0)

#endif  //#ifndef __USART_TINY_H

