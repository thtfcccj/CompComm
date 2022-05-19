/*********************************************************************************

//			以时间间隔(Time Interval)作为数据帧判定依据的网络管理-多例化时实现

//此模块为原TiCommMng的多例化，用于多个串口时的管理，并与其互斥(1个项目内2先1)
//此模块使用UsartDrv通讯,故独立于硬件与应用
*********************************************************************************/
#ifndef _TI_COMM_MNG_H
#define	_TI_COMM_MNG_H

/*****************************************************************************
                             相关配置
******************************************************************************/

//此模块支持：
//#define SUPPORT_TI_COMM_MNG 

//仅数据预处理时，允许先判断数据，或直接插入发送数据,全局定义
//#define SUPPORT_TI_COMM_MNG_PRE   

/*******************************************************************************
                              相关结构
*******************************************************************************/
#include "UsartTiny.h" //通过此多例化模块控制底层
#include "UsartDev.h" //

struct _TiCommMng{
  struct _UsartTiny UsartTiny; 
  unsigned char cbData;  //上层应用需要缓存的数据(4byte对齐)
  unsigned char Count;   //数数据收发定时器装载值
  unsigned char Index;   //Rtu模数数据收发定时器
  
  unsigned char Flag;//相关标志,见定义
};

//相关标志定义为:
#define TI_COMM_MNG_RCV_DOING    0x01 //数据接收过程中标志
#define TI_COMM_MNG_SEND_DOING   0x02 //数据发送过程中标志
#define TI_COMM_MNG_SUSPEND      0x04 //支持预处理时，挂起标志

/*****************************************************************************
                             相关函数
******************************************************************************/

//-----------------------------初始化函数-------------------------------
//调用此函数前,需初始化UsartDev(含UsartId)及其IO，及对Usart参数进行配置
void TiCommMng_Init(struct _TiCommMng *pMng,
                    const struct _UsartDevPt *pFun, //多态操作函数
                    struct _UsartDev *pDev);

//-------------------------------中断任务----------------------------
//将此函数放入1ms间隔中断进程中
void TiCommMng_IntTask(struct _TiCommMng *pMng);

//-------------------------------普通查询任务----------------------------
//将此函数放入系统1ms进程中
void TiCommMng_Task(struct _TiCommMng *pMng);

//-------------------------接收定时器复位函数-------------------------------
void TiCommMng_ResetRcvTimer(struct _TiCommMng *pMng);

//-------------------------发送定时器复位函数-------------------------------
#define TiCommMng_ResetSendTimer(mng) \
  do{(mng)->Index = (mng)->Count;}while(0)

//-------------------------------挂起函数--------------------------------
//挂起后，可才可直接使用pUsartDev中的收发数据缓冲区
//调用此函数后，必须调用TiCommMng_PreInsertSend()才能解开
#ifdef SUPPORT_TI_COMM_MNG_PRE
   void TiCommMng_Suspend(struct _TiCommMng *pMng);
#else //不支持时
   #define TiCommMng_Suspend(mng) do{}while(0)
#endif

//----------------------------强制插入发送函数--------------------------------
#ifdef SUPPORT_TI_COMM_MNG_PRE
  void TiCommMng_InsertSend(struct _TiCommMng *pMng,
                            unsigned char SendLen); //发送数据长度
#else //不支持时
   #define TiCommMng_PreInsertSend(mng, sendLen) do{}while(0)
#endif
  
//----------------------------得到设备ID--------------------------------
#define TiCommMng_GetDevId(mng)  UsartTiny_GetDevId(&(mng)->UsartTiny)
  
/*******************************************************************************
                            回调函数
********************************************************************************/

//----------------------得到数据帧间隔ms时间----------------------------
//与波特率有关，超过此时间，认为是一帧数据的结束
unsigned char TiCommMng_cbBuadId2FremeOv(const struct _TiCommMng *pMng);

//---------------------------------数据预处理函数-------------------------------
//数据在UsartTiny.Data中,0为地址,Len为长度,返回需发送的字节数(含地址)
//返回: 255未能识别;  254暂停接收, 0:数据正确，但不返回数据 
//其它:返回数据个数
#ifdef SUPPORT_TI_COMM_MNG_PRE 
  unsigned char  TiCommMng_cbRcvPreDataPro(struct _TiCommMng *pMng);
#endif

//--------------------------收到每个数据后的通报-------------------------------
//返回非0时，数据主动结束，否则继续收数
unsigned char TiCommMng_cbRcvedNotify(struct _TiCommMng *pMng);   

//--------------------------数据处理函数-------------------------------
//接收完数据后将调用此函数
//此函数内，应负责处理UsartTiny.Data缓冲区里的数据,并将处理结果返回
//返回255表示错误;  0:数据正确，但不返回数据 
//其它:返回数据个数
unsigned char TiCommMng_cbDataPro(struct _TiCommMng *pMng);

  
#endif //_TI_COMM_MNG_H




