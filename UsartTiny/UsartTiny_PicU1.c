/******************************************************************

//               USART精简接口在PIC Usar1中的实现
//
*******************************************************************/

#include "UsartTiny.h"
#include "IOCtrl.h"
#include "ModbusRtuMng.h"
#include <string.h>

struct _UsartTiny UsartTiny; //直接实例化

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
  RC1STA |= PICB_SPEN;    //使能串口
  TX1STA &= ~PICB_SYNC;   //使用异步USART
  INTCON |= PICB_PEIE;    //开总中断
}
//---------------------------停止数据收发函数-------------------------
//收发数据过程中中止数据收发
void UsartTiny_Stop(void)
{
  //关闭所有中断，在收发数据时打开
	PIE1 &= ~(PICB_TXIE | PICB_RCIE);   //禁止发送与接收中断
	RC1STA &= ~PICB_CREN;               //取消接收使能
	TX1STA &= ~PICB_TXEN;               //取消发送使能
	
  //复位状态机与内部变量
  //这里不清UsartTiny.Index以便在停止后仍可获知当前收发数据个数情况
  UsartTiny.eState = UsartTiny_eState_Idie;
  UsartTiny_cbStopNotify();//停止通报
}

//----------------------------启动接收函数------------------------------
void UsartTiny_RcvStart(void)
{
  UsartTiny_Stop();//无条件停止
  
  //配置寄存器为接收模式,开始接收数据
  RC1STA |= PICB_CREN;         //打开接收使能
  UsartTiny.Index = 0;
  UsartTiny.eState = UsartTiny_eState_Rcv;
  
  UsartTiny_cbRcvStartNotify();//启动接收通报
	PIE1 |= PICB_RCIE;           //最后开启接收中断
}

//----------------------------启动发送函数------------------------------
void UsartTiny_SendStart(unsigned char SendLen)
{
  UsartTiny_Stop();//无条件停止

  //配置寄存器为接收模式,开始接收数据
  UsartTiny.SendLen = SendLen;
  UsartTiny.Index = 0;
  UsartTiny.eState = UsartTiny_eState_Send;
  
  UsartTiny_cbSendStartNotify();   //启动发送通报
  
  //最后开启发送中断 
	TX1STA |= PICB_TXEN;             //打开发送使能  
	TX1REG = UsartTiny.Data[0];     //开始发送首个数据     
  PIE1 |= PICB_TXIE;              //开发送中断
}

//---------------------USART硬件接收中断处理函数----------------------------
void UsartTiny_RcvInt(void)
{
  //读寄存器清中断
  volatile unsigned char Data =  RC1REG; 

  //状态机错误 底帧错误(帧错误FERR或FIFO溢出错误OERR)
  if((RCSTA & (PICB_FERR | PICB_OERR)) || 
    (UsartTiny.eState != UsartTiny_eState_Rcv)){
    UsartTiny_cbErrNotify(-1); 
    return;
  }
  //防止数据溢出
  if(UsartTiny.Index < USART_TINY_DATA_MAX){
    UsartTiny.Data[UsartTiny.Index] = Data;
    UsartTiny.Index++;
  }
  else{//接收缓冲区满
     UsartTiny_cbRcvDataOv();
     UsartTiny_Stop();//无条件停止
  }   
  UsartTiny_cbRcvDataNotify();//接收数据通报  
}

//---------------------USART硬件发送中断处理函数----------------------------
void UsartTiny_SendInt(void)
{
  //状态机错误
  if(UsartTiny.eState != UsartTiny_eState_Send){
     UsartTiny_cbErrNotify(-2); 
     return;
  }
  UsartTiny.Index++; //发送完成一个了
  UsartTiny_cbSendDataNotify();//发送完成了一个数据

  if(UsartTiny.Index < UsartTiny.SendLen){ //还有数据时,发送下一个数
    TX1REG = UsartTiny.Data[UsartTiny.Index];
  }
  //最后一个数被移出了,发送一个空等待数据结束
  else if(UsartTiny.Index == UsartTiny.SendLen){
    TX1REG = 0xff;
  }  
  else{//发送完成
    UsartTiny_cbSendFinalNotify();
    UsartTiny_Stop();//无条件停止
  }
}
