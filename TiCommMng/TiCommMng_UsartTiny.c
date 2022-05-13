/*********************************************************************************

	以时间间隔(Time Interval)作为数据帧判定依据的网络管理-使用UsartTiny时实现

//此模块独立于硬件与应用
*********************************************************************************/

#include "TiCommMng.h"
#include "UsartTiny.h"
#include <string.h>

struct _TiCommMng TiCommMng;
               
/***********************************************************************
                            相关函数实现
***********************************************************************/

//---------------------------------初始化函数----------------------------
void TiCommMng_Init(void)
{
  memset(&TiCommMng, 0, sizeof(struct _TiCommMng));
  TiCommMng.Count = TiCommMng_cbBuadId2FremeOv();
  UsartTiny_Init();//初始化底层通讯
  //后应紧跟配置底层硬件
}

//-------------------------------中断任务----------------------------
//将此函数放入1ms间隔中断进程中
void TiCommMng_IntTask(void)
{
  if(TiCommMng.Index) TiCommMng.Index--; 
}

//-------------------------------普通查询任务----------------------------
//将此函数放入系统1ms进程中
void TiCommMng_Task(void)
{
  #ifdef SUPPORT_TI_COMM_MNG_PRE //挂起时暂停处理
    if(TiCommMng.Flag & TI_COMM_MNG_SUSPEND) return;
  #endif  
  
  //空闲状态启动接收
  if(UsartTiny.eState == UsartTiny_eState_Idie){
    UsartTiny_RcvStart();
    //停止数据收发数据
    TiCommMng.Flag &= ~(TI_COMM_MNG_RCV_DOING | TI_COMM_MNG_SEND_DOING);
    
    return;
  }
  if(TiCommMng.Index) return;//时间未到或在接收等待中
  
  //没在发送或接收等待过程中
  if(!(TiCommMng.Flag & (TI_COMM_MNG_RCV_DOING | 
                          TI_COMM_MNG_SEND_DOING))){
     if(UsartTiny.eState != UsartTiny_eState_Rcv){//异常，重置接收
       UsartTiny_RcvStart();
     }       
     return;
  }
  
  //数据接收完成
  if(TiCommMng.Flag & TI_COMM_MNG_RCV_DOING){
    UsartTiny_Stop(); //强制中止接收
    TiCommMng.Flag &= ~TI_COMM_MNG_RCV_DOING;//停止数据接收
    unsigned char Resume;
    #ifdef SUPPORT_TI_COMM_MNG_PRE 
      Resume = TiCommMng_cbRcvPreDataPro();
      if(Resume == 254){//暂停接收
        UsartTiny_Stop();
        return;
      }
      if(Resume == 255) Resume = TiCommMng_cbDataPro();//正式数据处理
    #else
      Resume = TiCommMng_cbDataPro();//数据处理
    #endif
     if((Resume != 0) && (Resume != 255)){ //数据正确,发送数据
       UsartTiny_SendStart(Resume);
       TiCommMng.Flag |= TI_COMM_MNG_SEND_DOING;//启动发送
       TiCommMng.Index = TiCommMng.Count;//计算发送超时
     }
  }
  else{//发送超时完成了
    UsartTiny_Stop();
  }

  /*/====================-发送0x55测试波特率===================
  if(TiCommMng.Index) TiCommMng.Index--;
  if(TiCommMng.Index) return;//时间未到
  TiCommMng.Index = 10;
  UsartTiny.Data[0] = 0x55;;
  UsartTiny_SendStart(1); */
  //485总线A,B两端应为下述波形(8位数据位、1位停止位)
  //输出波形实测为：(占空比50%,空闲时为中间电平):
  //       起 b0:1 0   1   0   1   0   1 b7:0停
  //   ┏┓  ┏━┓  ┏━┓  ┏━┓  ┏━┓  ┃
  // ━┛┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃━━━
  //     ┗━┛  ┗━┛  ┗━┛  ┗━┛  ┗━┛
  //波特率为9600时，周期为200uS,即对应1bit为100uS
}

//-------------------------接收定时器复位函数-------------------------------
void TiCommMng_ResetRcvTimer(void)
{
  TiCommMng.Flag |= TI_COMM_MNG_RCV_DOING;  
  if(!TiCommMng_cbRcvedNotify()) TiCommMng.Index = TiCommMng.Count;
  else TiCommMng.Index = 0;//数据收完，结束了
}

/******************************************************************************
		                      支持预处理时相关
******************************************************************************/ 
#ifdef SUPPORT_TI_COMM_MNG_PRE

//-------------------------------挂起函数--------------------------------
//挂起后，可才可直接使用缓冲区，但必须调用TiCommMng_PreInsertSend()才能解开
void TiCommMng_Suspend(void)
{
  UsartTiny_Stop(); //强制中止
  TiCommMng.Flag &= ~(TI_COMM_MNG_RCV_DOING |TI_COMM_MNG_SEND_DOING);//停止数据
  TiCommMng.Flag |= TI_COMM_MNG_SUSPEND;//挂起
}

//----------------------------强制插入发送函数--------------------------------
void TiCommMng_InsertSend(unsigned char SendLen) //发送数据长度
{
  TiCommMng.Flag &= ~TI_COMM_MNG_SUSPEND;//无条件取消挂起
  if(!SendLen) return; //无数据要发送
  
  UsartTiny_Stop();//先停止
  UsartTiny_SendStart(SendLen);
  TiCommMng.Flag |= TI_COMM_MNG_SEND_DOING;//启动发送
  TiCommMng.Index = TiCommMng.Count;//计算发送超时
}

#endif //SUPPORT_TI_COMM_MNG_PRE
