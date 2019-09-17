/***********************************************************************

                  At底层通讯接口实现模块实现

***********************************************************************/

#include "AtUsart.h"
#include <string.h>
#include "MemMng.h"

/******************************************************************************
		                           相关函数-系统相关
******************************************************************************/

//-------------------------------初始化函数---------------------------------
//调用此函数前确保硬件通讯参数已初始化准备好
//收发数据前，需指定缓冲区
void AtUsart_Init(struct _AtUsart *pAtUsart,
                  struct _UsartDev *pUsartDev,  //已初始化完成的底层设备
                  unsigned char DevId,         //设备ID号
                  unsigned char ModeMask)      //AT_USART_HALF_MODE_MASK定义  
{
  memset(pAtUsart, 0, sizeof(struct _AtUsart));
  pAtUsart->pUsartDev = pUsartDev;
  pUsartDev->pVoid = pAtUsart;
  pAtUsart->DevId = DevId;  
  pAtUsart->Flag |= ModeMask;
}

//-----------------------------配置发送缓冲区函数-------------------------------
//初始化后调用
void AtUsart_CfgSend(struct _AtUsart *pAtUsart,
                     unsigned short Count,         //缓冲区大小
                     unsigned char *pBuf,          //缓冲区
                     AtUsartNotify_t Notify)        //回调函数
{
  pAtUsart->Send.Count = Count;  
  pAtUsart->Send.pBuf = pBuf; 
  pAtUsart->Send.Notify = Notify; 
  if(pAtUsart->Flag & AT_USART_HALF_DUPLEX){//半双工模式时,默认共享一个缓冲区
    memcpy(&pAtUsart->Rcv, &pAtUsart->Send, sizeof(struct _AtUsartBuf));
  }
}

//-----------------------------配置接收缓冲区函数-------------------------------
//全双式模式，在配置发送缓冲区函数后调用
void AtUsart_CfgRcv(struct _AtUsart *pAtUsart,
                    unsigned short Count,         //缓冲区大小
                     unsigned char *pBuf,          //缓冲区
                     AtUsartNotify_t Notify)    //回调函数
{
  pAtUsart->Rcv.Count = Count;  
  pAtUsart->Rcv.pBuf = pBuf;  
  pAtUsart->Rcv.Notify = Notify;    
}

//------------------------------接收完成调用函数-------------------------------
static void _RcvFinal(struct _AtUsart *pAtUsart, signed char State)
{
  pAtUsart->RcvFlag |= AT_USART_RCV_STATE_FINAL;
  pAtUsart->RcvTimer = 0;
  UsartDev_RcvStop(pAtUsart->pUsartDev);
  AtUsart_cbRcvEndNotify(pAtUsart->DevId); 
  pAtUsart->Rcv.Notify(pAtUsart, State);   //用户通报
}

//---------------------------1ms硬件调用任务函数---------------------------------
//放在硬件定时器中
void AtUsart_1msHwTask(struct _AtUsart *pAtUsart)
{
  //发送超时计数
  if(pAtUsart->SendTimer){
    pAtUsart->SendTimer--;
    if(!pAtUsart->SendTimer){
      pAtUsart->SendFlag |= AT_USART_SEND_STATE_FINAL;
      AtUsart_cbSendEndNotify(pAtUsart->DevId); //发送完成通报
      pAtUsart->Send.Notify(pAtUsart, 1);//超时结束
      AtUsart_SendStop(pAtUsart);//强制停止
    }
  }
  
  //=============================接收相关计时================================
  unsigned char RcvState = pAtUsart->SendFlag & AT_USART_RCV_STATE_MASK;
  if(RcvState == AT_USART_RCV_STATE_IDIE) return;
  
  //接收等待中计数
  if(RcvState == AT_USART_RCV_STATE_WAIT){
    if(pAtUsart->RcvTimer != 0){//0不计数
      pAtUsart->RcvTimer--;
      if(!pAtUsart->RcvTimer){//超时退出
         _RcvFinal(pAtUsart, 1);
      }
    }
  }
  //接收中计数
  if(pAtUsart->RcvTimer != 0){//0不计数
    pAtUsart->RcvTimer--;
    if(!pAtUsart->RcvTimer){//超时退出并通报
      if(pAtUsart->RcvFlag & AT_USART_RCV_ALL_MODE)//超时接收完成
        _RcvFinal(pAtUsart, 0);
      else _RcvFinal(pAtUsart, 1);//错误超时了
    }
  }
}


/******************************************************************************
		                     发送数据操作函数实现
******************************************************************************/

//------------------------UsartDev发送完成中断调用函数实现--------------------
//形参为返struct _UsartDev指针
//回状态定义为:0:继续收发,其它:停止收发
signed char AtUsart_UsartDevSendEndNotify(void *pVoid)
{
  struct _UsartDev *pUsart = pVoid;
  struct _AtUsart *pAtUsart = (struct _AtUsart *)pUsart->pVoid;
  pAtUsart->SendTimer = 0;//结束了
  AtUsart_cbSendEndNotify(pAtUsart->DevId); //发送完成通报
  if(pAtUsart->Send.Notify(pAtUsart, 0) ||  //用户处理,需自启动接收时
    (pAtUsart->Flag & AT_USART_WR_AUTO_RCV)){//自动启用接收时,自启动
    AtUsart_RcvStart(pAtUsart);
  }
  AtUsart_SendStop(pAtUsart);
  return 1; //停止发送
}

//--------------------------------发送操作--------------------------------------
//发送配置，AT_USART_SEND_DIS_ALL字
void AtUsart_SendCfg(struct _AtUsart *pAtUsart, unsigned char Cfg)
{
  pAtUsart->SendFlag &= ~AT_USART_SEND_DIS_ALL;
  pAtUsart->SendFlag |= Cfg;  
  
}
//得到发送缓冲区,带AT指令时，将自动从指令后开始
unsigned char *AtUsart_pGetSendBuf(const struct _AtUsart *pAtUsart)
{
  if(pAtUsart->SendFlag & AT_USART_SEND_DIS_AT) //无AT字
    return pAtUsart->Send.pBuf;
  return pAtUsart->Send.pBuf + 2; //有AT字
}
//得到发送缓冲区大小,带前后缀时，将排除
unsigned short AtUsart_GetSendCount(const struct _AtUsart *pAtUsart)
{
  unsigned short Count = pAtUsart->Send.Count;
  if(!(pAtUsart->SendFlag & AT_USART_SEND_DIS_AT)) Count -= 2; //带AT字
  if(!(pAtUsart->SendFlag & AT_USART_SEND_DIS_CR)) Count--;
  if(!(pAtUsart->SendFlag & AT_USART_SEND_DIS_LF)) Count--;
  return Count;
}
//发送数据,发送前已提前写入缓冲区
void AtUsart_SendBuf(struct _AtUsart *pAtUsart, unsigned short SendLen)
{
  //填充缓冲区
  unsigned char *pBuf = pAtUsart->Send.pBuf;
  if(!(pAtUsart->SendFlag & AT_USART_SEND_DIS_AT)){ //带AT字
    *pBuf = 'A';
    *(pBuf + 1) = 'T';
    SendLen += 2;
  }
  pBuf += SendLen;//指向结束
  if(!(pAtUsart->SendFlag & AT_USART_SEND_DIS_CR)){
    *pBuf++ = '\r';    
    SendLen++;    
  }
  if(!(pAtUsart->SendFlag & AT_USART_SEND_DIS_LF)){
    *pBuf = '\n';    
    SendLen++;    
  }
  pAtUsart->Send.Len = SendLen;
  pAtUsart->SendTimer = AT_USART_SEND_OV;
  pAtUsart->SendFlag &= ~AT_USART_SEND_STATE_MASK;
  pAtUsart->SendFlag |= AT_USART_SEND_STATE_DOING;
  AtUsart_cbSendStartNotify(pAtUsart->DevId);
  UsartDev_SendStart(pAtUsart->pUsartDev,
                     pAtUsart->Send.pBuf,         //发送缓冲区
                     0x8000 | SendLen,            //发送数据大小
                     AtUsart_UsartDevSendEndNotify);     //发送回调函数  
}
//强制停止发送数据
void AtUsart_SendStop(struct _AtUsart *pAtUsart)
{
  if((pAtUsart->SendFlag & AT_USART_SEND_STATE_MASK) != AT_USART_SEND_STATE_DOING)
    return;
  UsartDev_SendStop(pAtUsart->pUsartDev);
  AtUsart_cbSendEndNotify(pAtUsart->DevId);
}

/******************************************************************************
		                     接收数据操作函数实现
******************************************************************************/

//----------------------------状态机操作------------------------------------
static void _SetRcvState(struct _AtUsart *pAtUsart, unsigned char NextState)
{
  pAtUsart->RcvFlag &= ~AT_USART_RCV_STATE_MASK;
  pAtUsart->RcvFlag |= NextState;
}

//---------------------UsartDev接收所有中断调用函数实现------------------------
//形参为返struct _UsartDev指针
//回状态定义为:0:继续收发,其它:停止收发
static signed char _UsartRcvAllNotify(void *pVoid)
{
  struct _UsartDev *pUsart = pVoid;
  struct _AtUsart *pAtUsart = (struct _AtUsart *)pUsart->pVoid;
  //状态机异常
  if(!(pAtUsart->RcvFlag & AT_USART_RCV_ALL_MODE)){
    _RcvFinal(pAtUsart, 3);
    return -1;    
  }
  if((pAtUsart->RcvFlag & AT_USART_RCV_STATE_MASK) != AT_USART_RCV_STATE_DOING){
    _RcvFinal(pAtUsart, 4);
    return -1;
  }
  //接收所有数据时,表示接收满了
  pAtUsart->Rcv.Len += pUsart->RcvLen;  //更新总数
  if(pAtUsart->Rcv.Notify(pAtUsart, 0)){//继续接收接下来的数
    pAtUsart->pUsartDev->RcvLen = 0;//重新开始
    pAtUsart->RcvTimer = pAtUsart->RcvDoingOv;
    UsartDev_RcvStart(pAtUsart->pUsartDev,
                      pAtUsart->Rcv.pBuf,    //接收缓冲区
                      pAtUsart->Rcv.Count,   //接收数据大小,先接收一个以响应
                      _UsartRcvAllNotify);   //接收回调函数
    return 0;
  }
  //接收完成
  pAtUsart->RcvFlag |= AT_USART_RCV_STATE_FINAL;
  AtUsart_RcvStop(pAtUsart);
  return 1;
}

//-------------------------到接收过程中调用函数实现-----------------------------
//接收单个字符过程中时，在调用,返回0以继续操作
static signed char _UsartRcvToDoing(struct _AtUsart *pAtUsart,
                                      unsigned char RcvedLen)//已接收个数
{
  AtUsart_cbRcvValidNotify(pAtUsart->DevId);
  //不用识别后导时,全接收启动
  if((pAtUsart->RcvFlag & AT_USART_RCV_DIS_END) == AT_USART_RCV_DIS_END){
    pAtUsart->Rcv.Len = RcvedLen;  //更新总数,已收完一个数了
    pAtUsart->RcvTimer = pAtUsart->RcvDoingOv;
    UsartDev_RcvStart(pAtUsart->pUsartDev,
                      pAtUsart->Rcv.pBuf + RcvedLen,       //接收缓冲区
                      0x8000 | (pAtUsart->Rcv.Count - RcvedLen),  //接收数据大小
                      _UsartRcvAllNotify);          //接收所有回调函数
    _SetRcvState(pAtUsart, AT_USART_RCV_STATE_DOING);
    pAtUsart->RcvFlag |= AT_USART_RCV_ALL_MODE;//全接收
  }
  else{ //需识别后导,只能还一个个收
    pAtUsart->pUsartDev->RcvLen = 0;//前导不计算长度.重新收
  }
  _SetRcvState(pAtUsart, AT_USART_RCV_STATE_DOING); 
  return 0;
}

//---------------------UsartDev接收单个字中断调用函数实现------------------------
//形参为返struct _UsartDev指针
//回状态定义为:0:继续收发,其它:停止收发
static signed char _UsartRcvNotify(void *pVoid)
{
  struct _UsartDev *pUsart = pVoid;
  struct _AtUsart *pAtUsart = (struct _AtUsart *)pUsart->pVoid;
  
  unsigned char State = pAtUsart->RcvFlag & AT_USART_RCV_STATE_MASK;
  pAtUsart->RcvTimer = AT_USART_RCV_DOING_BYTE_OV;  
  unsigned char RcvChar = pAtUsart->pUsartDev->RcvData;
  
  //======================单个字符接收过程中处理结束字符======================== 
  if(State == AT_USART_RCV_STATE_DOING){
    unsigned char IdentChar;
    if(!(pAtUsart->RcvFlag & AT_USART_RCV_DIS_ECR))//CR后导识别
      IdentChar = '\r';
    else if(!(pAtUsart->RcvFlag & AT_USART_RCV_DIS_ELF))//只有LF时后导识别
      IdentChar = '\n';
    else return -1;//异常进入
    
    //找到结束字符了
    if(pAtUsart->pUsartDev->RcvData == IdentChar){
      //双字符结束识别时
      if((pAtUsart->RcvFlag & AT_USART_RCV_DIS_END) == 0){
        _SetRcvState(pAtUsart, AT_USART_RCV_STATE_DOING_END2);
        return 0;
      }
      //只有一个结束字符时，接收完成
      _RcvFinal(pAtUsart, 0);
      return 1;
    }
    pAtUsart->Rcv.Len++;//正常收一个数计数
    return 0;
  }

  //==========================首次进入时检查===================================
  if(State == AT_USART_RCV_STATE_WAIT){
    //没接前导时到正常
    if((pAtUsart->RcvFlag & AT_USART_RCV_DIS_START) == AT_USART_RCV_DIS_START){
      return _UsartRcvToDoing(pAtUsart, 1);
    }
    //识别收到的前导字符
    unsigned char RcvChar = pAtUsart->pUsartDev->RcvData;
    if(!(pAtUsart->RcvFlag & AT_USART_RCV_DIS_SCR)){//处理CR前导识别
      if(RcvChar != '\r'){//前导错误
        _RcvFinal(pAtUsart, -1);
        return -1;
      }
      //首个识别成功后：
      if(pAtUsart->RcvFlag & AT_USART_RCV_DIS_SLF){//不需要识别第二个时，正式收数
        return _UsartRcvToDoing(pAtUsart, 0);
      }
      //还需要识别第二个
      _SetRcvState(pAtUsart, AT_USART_RCV_STATE_WAIT_START2);
      return 0;
    }
    else if(!(pAtUsart->RcvFlag & AT_USART_RCV_DIS_SLF)){//只有LF时的前导识别
      if(RcvChar != '\n'){//前导错误
        _RcvFinal(pAtUsart, -2);
        return -1;
      }
      //识别成功后
      return _UsartRcvToDoing(pAtUsart, 0);
    }
  }
  
  //==========================第二前导字符识别===================================
  if(State == AT_USART_RCV_STATE_WAIT_START2){
    if(pAtUsart->pUsartDev->RcvData != '\n'){
      _RcvFinal(pAtUsart, -2);
      return -1;
    }
    //识别对了
    return _UsartRcvToDoing(pAtUsart, 0);
  }
  //==========================第二后导字符识别===================================
  if(State == AT_USART_RCV_STATE_DOING_END2){
    if(pAtUsart->pUsartDev->RcvData != '\n'){
      _RcvFinal(pAtUsart, -3);
      return -1;
    }
    //识别对了接收完成
    _RcvFinal(pAtUsart, 0);
    return 1;
  }

  //状态机异常
  _RcvFinal(pAtUsart,3);
  return 1;
}

//--------------------------------接收操作--------------------------------------
//接收配置，AT_USART_RCV_DIS_ALL字
void AtUsart_RcvCfg(struct _AtUsart *pAtUsart, unsigned char Cfg)
{
  pAtUsart->RcvFlag &= ~AT_USART_RCV_DIS_ALL;
  pAtUsart->RcvFlag |= Cfg; 
}

//开始接收数据
void AtUsart_RcvStart(struct _AtUsart *pAtUsart)
{
  pAtUsart->Rcv.Len = 0;
  pAtUsart->RcvTimer = pAtUsart->RcvWaitOv;
  pAtUsart->RcvFlag &= ~(AT_USART_RCV_STATE_MASK | AT_USART_RCV_ALL_MODE);
  pAtUsart->RcvFlag |= AT_USART_RCV_STATE_WAIT;
  UsartDev_RcvStart(pAtUsart->pUsartDev,
                     pAtUsart->Rcv.pBuf,           //接收缓冲区
                     pAtUsart->Rcv.Count,          //接收数据大小,先接收一个以响应
                     _UsartRcvNotify);      //接收回调函数
}
//强制停止接收数据
void AtUsart_RcvStop(struct _AtUsart *pAtUsart)
{
  AtUsart_cbRcvEndNotify(pAtUsart->DevId); 
  UsartDev_RcvStop(pAtUsart->pUsartDev);
  pAtUsart->RcvFlag &= ~AT_USART_RCV_STATE_MASK;
}





