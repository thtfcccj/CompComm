/******************************************************************

//               USART精简接口在PIC Usar中的实现
//
*******************************************************************/

#include "UsartTiny.h"
#include "IOCtrl.h"
#include "IOCtrl.h"
#include "ModbusRtuMng.h"
#include <string.h>

struct _UsartTiny UsartTiny; //直接实例化
static unsigned char _Cfg = 0;   //UsartTiny_CfgHw的配置

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
  RCSTA |= PICB_SPEN;    //使能串口,TX,RX引脚自动配置
  TXSTA &= ~PICB_SYNC;   //使用异步USART
  INTCON |= PICB_PEIE;    //开总中断
}

//-----------------------底层硬件配置函数----------------------
void UsartTiny_CfgHw(unsigned char Cfg)
{
  //Cfg = 0; //此硬件配置其它模式暂不支持
  _Cfg = Cfg; //记住配置以使用
  //基本配置:
  TXSTA = PICB_CSRC;    //时钟频率模式选为主模式(内部时钟) 异步通信方式 低波特率
  RCSTA = PICB_SPEN;     //开串口, 取消地址检测,关接收与地址检测

  //计算并配置数据位数(硬件自带1个起始位+1个停止位,没有奇偶校验与2个停止位功能)
  unsigned char Bit;
  if(Cfg & USART_TINY_BIT_7) Bit = 7;
  else  Bit = 8;
  if(Cfg & USART_TINY_STOP_2) Bit++;  //两个停止位
  if(Cfg &USART_TINY_PARITY_EN) Bit++;//硬件不支持校验，但需要占位
  if(Bit > 8){//启用第9位充当多出的位
    TXSTA |= PICB_TX9; 
    RCSTA |= PICB_RX9; 
  }

  //配置波特率: 32M时钟时的配置(SYNC = 0; BRGH = 0; BRG16 = 0;)： 
  BAUDCON = 0;    //睡眠唤醒关闭 8位波特率设置,关闭自动波特率检测，发送级性不反
  SPBRGH = 0X00;
  unsigned char Shift = Cfg & USART_TINY_BAUD_MASK;
  if((Shift == 0) || (Shift > USART_TINY_BAUD_19200)) //置默认波特率,不支持高波特率
    Shift = USART_TINY_BAUD_9600; 
                      
  SPBRGL = 207 >> (Shift - 1);//2400:207 4800:103 9600:51 19200:25
}


//---------------------------停止数据收发函数-------------------------
//收发数据过程中中止数据收发
void UsartTiny_Stop(void)
{
  //关闭所有中断，在收发数据时打开
  PIE1 &= ~(PICB_TXIE | PICB_RCIE);   //禁止发送与接收中断
  RCSTA &= ~PICB_CREN;               //取消接收使能
  TXSTA &= ~PICB_TXEN;               //取消发送使能
	
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
  RCSTA |= PICB_CREN;         //打开接收使能
  UsartTiny.Index = 0;
  UsartTiny.eState = UsartTiny_eState_Rcv;
  
  UsartTiny_cbRcvStartNotify();//启动接收通报
  PIE1 |= PICB_RCIE;           //最后开启接收中断
}

//----------------------------启动发送函数------------------------------
void UsartTiny_SendStart(unsigned char SendLen)
{
  //UsartTiny_Stop();//无条件停止

  //配置寄存器为发送模式,开始发送数据
  UsartTiny.SendLen = SendLen;
  UsartTiny.Index = 0;
  UsartTiny.eState = UsartTiny_eState_Send;
    
  //最后开启发送中断 
  TXSTA |= PICB_TXEN;             //打开发送使能 
  UsartTiny_cbSendStartNotify();   //启动发送通报
  TXREG = UsartTiny.Data[0];     //开始发送首个数据     
  PIE1 |= PICB_TXIE;              //开发送中断
}

//---------------------USART硬件接收中断处理函数----------------------------
void UsartTiny_RcvInt(void)
{
  //读寄存器清中断
  volatile unsigned char Rcsta = RCSTA;//先读状态
  volatile unsigned char Data =  RCREG; 
  if(UsartTiny.eState != UsartTiny_eState_Rcv) return;//异常丢弃
  
  //软件奇校偶校验,数据位数与停止位处理
  if(_Cfg & USART_TINY_BIT_7) Data &= 0x7f; //强制最高位
  //软件奇校偶校验略

  //状态机错误 底帧错误(帧错误FERR或FIFO溢出错误OERR)
  if((Rcsta & (PICB_FERR | PICB_OERR)) || 
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
  PIR1 &= ~PICB_TXIF;//无条件清标志
  //状态机错误
  if(UsartTiny.eState != UsartTiny_eState_Send){   
     UsartTiny_cbErrNotify(-2); 
     return;
  }

  UsartTiny.Index++; //发送完成一个了
  UsartTiny_cbSendDataNotify();//发送完成了一个数据

  if(UsartTiny.Index < UsartTiny.SendLen){ //还有数据时,发送下一个数
    TXREG = UsartTiny.Data[UsartTiny.Index];
  }
  else{ //最后一个数压入移位寄存器完成,没有数可入了，停止中断，由上层处理最后一字节发送
    UsartTiny_cbSendLastNotify();
    PIE1 &= ~PICB_TXIE;   //禁止发送中断     
  }
}
