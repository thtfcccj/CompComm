/*********************************************************************************

//					以时间间隔(Time Interval)作为数据帧判定依据的网络管理实现
//此模块同时识别首位为0时认为是
//此模块独立于硬件与应用
*********************************************************************************/
#ifndef _TI_COMM_MNG_H
#define	_TI_COMM_MNG_H

/*****************************************************************************
                             相关配置
******************************************************************************/

//仅数据预处理时，允许先判断数据，或直接插入发送数据,全局定义
//#define SUPPORT_TI_COMM_MNG_PRE   

/*******************************************************************************
                              相关结构
*******************************************************************************/

struct _TiCommMng{
  unsigned char Count;   //数数据收发定时器装载值
  unsigned char Index;   //Rtu模数数据收发定时器

  unsigned char Flag;//相关标志,见定义
};

//相关标志定义为:
#define TI_COMM_MNG_RCV_DOING    0x01 //数据接收过程中标志
#define TI_COMM_MNG_SEND_DOING   0x02 //数据发送过程中标志
#define TI_COMM_MNG_SUSPEND      0x04 //支持预处理时，挂起标志

extern struct _TiCommMng TiCommMng;//直接实例化

/*****************************************************************************
                             相关函数
******************************************************************************/

//-----------------------------初始化函数-------------------------------
void TiCommMng_Init(void);

//-------------------------------普通查询任务----------------------------
//将此函数放入系统1ms进程中
void TiCommMng_Task(void);

//-------------------------接收定时器复位函数-------------------------------
#define TiCommMng_ResetRcvTimer() \
  do{TiCommMng.Index = TiCommMng.Count;\
     TiCommMng.Flag |= TI_COMM_MNG_RCV_DOING;}while(0)

//-------------------------发送定时器复位函数-------------------------------
#define TiCommMng_ResetSendTimer() \
  do{TiCommMng.Index = TiCommMng.Count;}while(0)

//-------------------------------挂起函数--------------------------------
//挂起后，可才可直接使用UsartTiny.Data缓冲区
//调用此函数后，必须调用TiCommMng_PreInsertSend()才能解开
#ifdef SUPPORT_TI_COMM_MNG_PRE
   void TiCommMng_Suspend(void);
#else //不支持时
   #define TiCommMng_Suspend() do{}while(0)
#endif

//----------------------------强制插入发送函数--------------------------------
#ifdef SUPPORT_TI_COMM_MNG_PRE
  void TiCommMng_InsertSend(unsigned char SendLen); //发送数据长度
#else //不支持时
   #define TiCommMng_PreInsertSend(sendLen ) do{}while(0)
#endif

/*******************************************************************************
                            回调函数
********************************************************************************/

//----------------------得到数据帧间隔ms时间----------------------------
//与波特率有关，超过此时间，认为是一帧数据的结束
unsigned char TiCommMng_cbBuadId2FremeOv(void);

//---------------------------------数据预处理函数-------------------------------
//数据在UsartTiny.Data中,0为地址,Len为长度,返回需发送的字节数(含地址)
//返回: 255未能识别;  254暂停接收, 0:数据正确，但不返回数据 
//其它:返回数据个数
#ifdef SUPPORT_TI_COMM_MNG_PRE 
  unsigned char  TiCommMng_cbRcvPreDataPro(void);
#endif

//--------------------------数据处理函数-------------------------------
//接收完数据后将调用此函数
//此函数内，应负责处理UsartTiny.Data缓冲区里的数据,并将处理结果返回
//返回255表示错误;  0:数据正确，但不返回数据 
//其它:返回数据个数
unsigned char TiCommMng_cbDataPro(void);

  
#endif //_TI_COMM_MNG_H




