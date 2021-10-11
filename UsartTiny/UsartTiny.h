/******************************************************************

//               USART精简模块对外接口
//说明:
//UsartTiny模块为原UsartDev的简要功能实现,主要变化有:
//1.此模块直接将驱动实例化,无指针操作以节省代码空间与时间效率
//2.仅不支持自动发送模式(发送过程中不需上层干预)。
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
  enum _UsartTiny_eState  eState; //工作状态机
  unsigned char Data[USART_TINY_DATA_MAX]; //收发数据缓冲
  unsigned char Index;   //收发数据位置
  unsigned char SendLen; //发送数据个数
};

extern struct _UsartTiny UsartTiny; //直接实例化

/*********************************************************************
                             相关行为函数
***********************************************************************/

//-----------------------------初始化函数------------------------------
//调用此函数前,需对Usart参数进行配置
void UsartTiny_Init(void);

//-----------------------底层硬件配置函数----------------------
//Cfg参数位含义同与UsartDevCfg.h
void UsartTiny_CfgHw(unsigned char Cfg);

//---------------------------停止数据收发函数-------------------------
//收发数据过程中中止数据收发
void UsartTiny_Stop(void); 

//----------------------------启动接收函数------------------------------
void UsartTiny_RcvStart(void);

//----------------------------启动发送函数------------------------------
void UsartTiny_SendStart(unsigned char SendLen);

/*********************************************************************
                             相关成员函数
***********************************************************************/
//注:暂直接上层调用

/************************************************************************
                             相关回调函数
***********************************************************************/

//部分函数直接实现:
//1.实现485的RT控制 2.指示灯控制(借用FireFreme的通讯指示灯软件接口),
//3上层接口
#include "IOCtrl.h"
#ifdef SUPPORT_TI_COMM_MNG
  #include "TiCommMng.h"
#else
  #include "ModbusRtuMng.h"
#endif
//-----------------------------初始化附加函数------------------------------
//void UsartTiny_cbInit(void);
#define UsartTiny_cbInit() do{CfgUsartIo();}while(0)

//-----------------------------停止通报函数------------------------------
//void UsartTiny_cbStopNotify(void);
#define UsartTiny_cbStopNotify() do{ClrRTS();}while(0)

//-----------------------------接收启动通报函数------------------------------
//void UsartTiny_cbRcvStartNotify(void);
#define UsartTiny_cbRcvStartNotify() UsartTiny_cbStopNotify()

//-----------------------------发送启动通报函数------------------------------
//void UsartTiny_cbSendStartNotify(void);
#define UsartTiny_cbSendStartNotify() do{SetRTS();}while(0)

//----------------------------中断内接收到数据通报函数------------------------------
//void UsartTiny_cbRcvDataNotify(void);
#ifdef SUPPORT_TI_COMM_MNG
#define UsartTiny_cbRcvDataNotify() do{TiCommMng_ResetRcvTimer();}while(0)
#else
#define UsartTiny_cbRcvDataNotify() do{ModbusRtuMng_ResetRcvTimer();}while(0)
#endif

//----------------------------中断内接收到数据溢出函数------------------------------
//void UsartTiny_cbRcvDataOv(void);
#define UsartTiny_cbRcvDataOv() do{}while(0)

//----------------------------中断内发送出一个数据通报函数------------------------------
//void UsartTiny_cbSendDataNotify(void);
#ifdef SUPPORT_TI_COMM_MNG
  #define UsartTiny_cbSendDataNotify() do{TiCommMng_ResetSendTimer();}while(0)
#else
  #define UsartTiny_cbSendDataNotify() do{ModbusRtuMng_ResetSendTimer();}while(0)
#endif
 
//------------------------数据发送到最后通报函数------------------------------
//void UsartTiny_cbSendLastDataNotify(void);
#define UsartTiny_cbSendLastNotify() do{}while(0)

//------------------------数据发送完成通报函数------------------------------
//void UsartTiny_cbSendFinalNotify(void);
#define UsartTiny_cbSendFinalNotify() do{}while(0)

//----------------------------收发数据错误通报函数------------------------------
//void UsartTiny_cbErrNotify(void);
#define UsartTiny_cbErrNotify(err) do{}while(0)

#endif  //#ifndef __USART_TINY_H

