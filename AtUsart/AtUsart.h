/***********************************************************************

                  AT(或类AT)指令在通讯上的实现模块
1. 此模块建在AT(或类AT)指令模块与底层UsartDev之间,实现其双向通讯
2. 标准AT指令格式为:
     发送:“AT....<CR>” 
    接收:“<CR><LF><response><CR><LF>”
3. 此模块实现为多例化以同时支持不同AT指令
4. 支持半大工状态,此状态下，可共用接收缓冲区
5. 支持阻塞操作。
***********************************************************************/
#ifndef __AT_USART_H
#define	__AT_USART_H

/****************************************************************************
		                      相关配置
****************************************************************************/

//接收数据时，每个数的等待时间，1ms为单位, >= 2
#define	AT_USART_RCV_DOING_BYTE_OV        32

//发送数据时，发送完等待时间，1ms为单位, >= 2
#define	AT_USART_SEND_OV                  512

/****************************************************************************
		                      相关结构
****************************************************************************/
#include "UsartDev.h"

//定义回调函数功能类型 
typedef  signed char(*AtUsartNotify_t)(const void *, signed char State);

//收发缓冲区定义:
struct _AtUsartBuf{
  unsigned short Count;         //缓冲区大小
  unsigned short Len;           //要处理的数据长度
  unsigned char *pBuf;          //缓冲区
  AtUsartNotify_t Notify;        //回调函数
};

//主结构
struct _AtUsart{
  //内部管理： 
  struct _UsartDev *pUsartDev; //挂接的底层设备   
  unsigned char DevId;         //设备ID号
  unsigned char Flag;          //相关标志,见定义
  unsigned char SendFlag;       //发送相关标志,见定义
  unsigned char RcvFlag;       //接收相关标志,见定义
  
  unsigned short SendTimer;      //发送定时器值
  unsigned short RcvTimer;      //按收定时器值
  unsigned short RcvWaitOv;     //接收等待时间, 0一直等待
  unsigned short RcvDoingOv;    //接收中超时时间,0一直接收直到缓冲区满  
  //缓冲区  
  struct _AtUsartBuf Send;      //发送区
  struct _AtUsartBuf Rcv;       //接收区
};

//相关标志定义为:
#define AT_USART_HALF_MODE_MASK   0xC0   //工作模式,定义为:
#define AT_USART_HALF_DUPLEX      0x80   //标识在半双工工作状态(共用一个缓冲区)
#define AT_USART_WR_AUTO_RCV      0x40   //发送完自动置接收,否则停止。


//发送相关标志定义为:
#define AT_USART_SEND_DIS_AT     0x80    //不自动加前导字符"AT"
#define AT_USART_SEND_DIS_CR     0x40    //不自动加后导字符"<CR>"
#define AT_USART_SEND_DIS_LF     0x20    //不自动加后导字符"<LF>"
#define AT_USART_SEND_DIS_ALL    0xE0    //不自动加所有前后导字符(可实现透传)
//控制相关:
#define AT_USART_SEND_STATE_MASK   0x03   //发送状态,定义为:
#define AT_USART_SEND_STATE_IDIE   0x00   //空闲
#define AT_USART_SEND_STATE_DOING  0x01   //发送中
#define AT_USART_SEND_STATE_FINAL  0x03   //发送完成

//接收相关标志定义为:
//符号识别相关:
#define AT_USART_RCV_DIS_SCR     0x80     //不自动识别前导字符"<CR>"
#define AT_USART_RCV_DIS_SLF     0x40     //不自动识别前导字符"<LF>"
#define AT_USART_RCV_DIS_ECR     0x20     //不自动识别后导字符"<CR>"
#define AT_USART_RCV_DIS_ELF     0x10     //不自动识别后导字符"<LF>"
#define AT_USART_RCV_DIS_ALL     0xF0    //不自动加所有前后导字符(可实现全接收)
//控制相关:
#define AT_USART_RCV_ALL_MODE           0x08   //标识在所有字接收状态
  
#define AT_USART_RCV_STATE_MASK         0x07   //发送状态,定义为:
#define AT_USART_RCV_STATE_IDIE         0x00   //空闲
#define AT_USART_RCV_STATE_WAIT         0x01   //接收等待中
#define AT_USART_RCV_STATE_WAIT_START2  0x02   //接收前导字符2
#define AT_USART_RCV_STATE_DOING        0x03   //接收中
#define AT_USART_RCV_STATE_DOING_END2   0x04   //接收中结束字符2 
#define AT_USART_RCV_STATE_FINAL        0x07   //接收完成

/******************************************************************************
		                           相关函数-系统相关
******************************************************************************/

//-------------------------------初始化函数---------------------------------
//调用此函数前确保硬件通讯参数已初始化准备好
//收发数据前，需指定缓冲区
void AtUsart_Init(struct _AtUsart *pAtUsart,
                  struct _UsartDev *pUsartDev,  //已初始化完成的底层设备
                  unsigned char DevId,         //设备ID号
                  unsigned char ModeMask);   //AT_USART_HALF_MODE_MASK定义      

//---------------------------1ms硬件调用任务函数---------------------------------
//放在硬件定时器中
void AtUsart_1msHwTask(struct _AtUsart *pAtUsart);

//-----------------------------配置发送缓冲区函数-------------------------------
//初始化后调用
void AtUsart_CfgSend(struct _AtUsart *pAtUsart,
                     unsigned short Count,         //缓冲区大小
                     unsigned char *pBuf,          //缓冲区
                     AtUsartNotify_t Notify);    //回调函数

//-----------------------------配置接收缓冲区函数-------------------------------
//全双式模式，在配置发送缓冲区函数后调用
void AtUsart_CfgRcv(struct _AtUsart *pAtUsart,
                    unsigned short Count,          //缓冲区大小
                    unsigned char *pBuf,           //缓冲区 
                    AtUsartNotify_t Notify);    //回调函数

//------------------------此模块UsartDev发送完成中断调用函数实现--------------------
//形参为返struct _UsartDev指针
//回状态定义为:0:继续收发,其它:停止收发
//可用于判断
signed char AtUsart_UsartDevSendEndNotify(void *pVoid);

/******************************************************************************
		                     收发数据操作函数
******************************************************************************/
//--------------------------------发送操作--------------------------------------
//发送配置，AT_USART_SEND_DIS_ALL字
void AtUsart_SendCfg(struct _AtUsart *pAtUsart, unsigned char Cfg);
//得到发送缓冲区,带AT指令时，将自动从指令后开始
unsigned char *AtUsart_pGetSendBuf(const struct _AtUsart *pAtUsart);
//得到发送缓冲区大小,带前后缀时，将排除
unsigned short AtUsart_GetSendCount(const struct _AtUsart *pAtUsart);
//发送数据,发送前已提前写入缓冲区
void AtUsart_SendBuf(struct _AtUsart *pAtUsart, unsigned short SendLen);
//强制停止发送数据
void AtUsart_SendStop(struct _AtUsart *pAtUsart);
//是否完成,用于阻塞操作时查询
signed char  AtUsart_IsSendFinal(const struct _AtUsart *pAtUsart);
//置自动接收
#define AtUsart_EnWrAutoRcv(atUsart) do{(atUsart)->Flag |= AT_USART_WR_AUTO_RCV;}while(0) 
//取消自动接收
#define AtUsart_DisWrAutoRcv(atUsart) do{(atUsart)->Flag &= ~AT_USART_WR_AUTO_RCV;}while(0) 
//--------------------------------接收操作--------------------------------------
//接收配置，AT_USART_RCV_DIS_ALL字
void AtUsart_RcvCfg(struct _AtUsart *pAtUsart, unsigned char Cfg);
//置接收数据等待时间,超时将调用回调, 以1ms为单位
#define AtUsart_SetRcvWaitOv(atUsart, ms) do{(atUsart)->RcvWaitOv = ms;}while(0)
//置接收数据过程中超时时间,超时认为一帧数据接束, 以1ms为单位
#define AtUsart_SetRcvDoingOv(atUsart, ms) do{(atUsart)->RcvDoingOv = ms;}while(0)
//开始接收数据
void AtUsart_RcvStart(struct _AtUsart *pAtUsart);
//强制停止接收数据
void AtUsart_RcvStop(struct _AtUsart *pAtUsart);

//得到接收缓冲区,带前导时，将自动从前导后开始
const unsigned char *AtUsart_pGetRcvBuf(const struct _AtUsart *pAtUsart);
//得到接收缓冲区大小,带前后缀时，将排除
unsigned short AtUsart_GetRcvCount(const struct _AtUsart *pAtUsart);
//数据正确时，得到接收到的数据大小
unsigned short AtUsart_GetRcvSize(const struct _AtUsart *pAtUsart);
//是否完成,用于阻塞操作时查询
signed char  AtUsart_IsRcvFinal(const struct _AtUsart *pAtUsart);

/******************************************************************************
		                          回调函数
******************************************************************************/

//-------------------------发送完成回调函数定义说明-----------------------------
//pAtUsart->Send.Notify(, State)
//在中断里调用,返回是否继续接收， State定义为：
//0: 完成,1: 数据超时未发送完

//---------------------------接收完成回调函数-----------------------------------
//pAtUsart->Rcv.Notify(, State)
//在中断里调用,返回是否继续接收， State定义为：
//Err定义为:
// 0: 正确
// 1: 接收等待无响应(即无开始字符)
// 2: 数据接收超时(没有结束字符或不完整)
//负值为接收错误状态或系统异常
//-1: 接收起始字符1错误
//-2: 接收起始字符2错误                       
//-3: 接收结束符字2错误
//其它: 底层通讯异常
//返回是否重新接收

//------------------------开始发送字符通报函数--------------------------------
//可用于点亮发送指示灯
//void AtUsart_cbSendStartNotify(unsigned char DevId);//设备ID号
#define AtUsart_cbSendStartNotify(devId) do{}while(0) //不启用时

//--------------------------------发送结束通报函数------------------------------
//可用于关闭发送指示灯
//void AtUsart_cbSendFinalNotify(unsigned char DevId);//设备ID号
#define AtUsart_cbSendEndNotify(devId) do{}while(0) //不启用时

//----------------------接收到有效超始字符后通报函数----------------------------
//可用于点亮接收指示灯
//void AtUsart_cbRcvValidNotify(unsigned char DevId);//设备ID号
#define AtUsart_cbRcvValidNotify(devId) do{}while(0) //不启用时

//--------------------------------接收结束通报函数------------------------------
//可用于关闭接收指示灯
//void AtUsart_cbRcvEndNotify(unsigned char DevId);//设备ID号
#define AtUsart_cbRcvEndNotify(devId) do{}while(0) //不启用时

#endif




