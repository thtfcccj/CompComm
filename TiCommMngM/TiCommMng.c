/*********************************************************************************

//		以时间间隔(Time Interval)作为数据帧判定依据的网络管理-多例化时实现

//此模块独立于硬件与应用
*********************************************************************************/

#include "TiCommMng.h"
#include <string.h>

/***********************************************************************
                            相关函数实现
***********************************************************************/

//-----------------------------初始化函数-------------------------------
//调用此函数前,需初始化UsartDev(含UsartId)及其IO，及对Usart参数进行配置
void TiCommMng_Init(struct _TiCommMng *pMng,
                    struct _UsartDev *pDev)
{
  memset(pMng, 0, sizeof(struct _TiCommMng));
  pMng->Count = TiCommMng_cbBuadId2FremeOv(pMng);
  UsartTiny_Init(&pMng->UsartTiny, pDev);//初始化底层通讯
  //后应紧跟配置底层硬件
}

//-------------------------------中断任务----------------------------
//将此函数放入1ms间隔中断进程中
void TiCommMng_IntTask(struct _TiCommMng *pMng)
{
  if(pMng->Index) pMng->Index--; 
}

//-------------------------------普通查询任务----------------------------
//将此函数放入系统1ms进程中
void TiCommMng_Task(struct _TiCommMng *pMng)
{
  #ifdef SUPPORT_TI_COMM_MNG_PRE //挂起时暂停处理
    if(pMng->Flag & TI_COMM_MNG_SUSPEND) return;
  #endif  
  
  //空闲状态启动接收
  if(pMng->UsartTiny.eState == UsartTiny_eState_Idie){
    UsartTiny_RcvStart(&pMng->UsartTiny);
    //停止数据收发数据
    pMng->Flag &= ~(TI_COMM_MNG_RCV_DOING | TI_COMM_MNG_SEND_DOING);
    
    return;
  }
  if(pMng->Index) return;//时间未到或在接收等待中  
  //没在发送或接收等待过程中
  if(!(pMng->Flag & (TI_COMM_MNG_RCV_DOING | 
                          TI_COMM_MNG_SEND_DOING))){
     if(pMng->UsartTiny.eState != UsartTiny_eState_Rcv){//异常，重置接收
       UsartTiny_RcvStart(&pMng->UsartTiny);
     }       
     return;
  }
  
  //数据接收完成
  if(pMng->Flag & TI_COMM_MNG_RCV_DOING){
    UsartTiny_Stop(&pMng->UsartTiny); //强制中止接收
    pMng->Flag &= ~TI_COMM_MNG_RCV_DOING;//停止数据接收
    unsigned char Resume;
    #ifdef SUPPORT_TI_COMM_MNG_PRE 
      Resume = TiCommMng_cbRcvPreDataPro(pMng);
      if(Resume == 254){//暂停接收
        UsartTiny_Stop(&pMng->UsartTiny);
        return;
      }
      if(Resume == 255) Resume = TiCommMng_cbDataPro(pMng);//正式数据处理
    #else
      Resume = TiCommMng_cbDataPro(pMng);//数据处理
    #endif
     if((Resume != 0) && (Resume != 255)){ //数据正确,发送数据
       UsartTiny_SendStart(&pMng->UsartTiny, Resume);
       pMng->Flag |= TI_COMM_MNG_SEND_DOING;//启动发送
       pMng->Index = pMng->Count;//计算发送超时
     }
  }
  else{//发送超时完成了
    UsartTiny_Stop(&pMng->UsartTiny);
  }
}

/******************************************************************************
		                      支持预处理时相关
******************************************************************************/ 
#ifdef SUPPORT_TI_COMM_MNG_PRE

//-------------------------------挂起函数--------------------------------
//挂起后，可才可直接使用缓冲区，但必须调用TiCommMng_PreInsertSend()才能解开
void TiCommMng_Suspend(struct _TiCommMng *pMng)
{
  UsartTiny_Stop(&pMng->UsartTiny); //强制中止
  pMng->Flag &= ~(TI_COMM_MNG_RCV_DOING |TI_COMM_MNG_SEND_DOING);//停止数据
  pMng->Flag |= TI_COMM_MNG_SUSPEND;//挂起
}

//----------------------------强制插入发送函数--------------------------------
void TiCommMng_InsertSend(struct _TiCommMng *pMng,
                          unsigned char SendLen) //发送数据长度
{
  pMng->Flag &= ~TI_COMM_MNG_SUSPEND;//无条件取消挂起
  if(!SendLen) return; //无数据要发送
  
  UsartTiny_Stop(&pMng->UsartTiny);//先停止
  UsartTiny_SendStart(&pMng->UsartTiny, SendLen);
  pMng->Flag |= TI_COMM_MNG_SEND_DOING;//启动发送
  pMng->Index = pMng->Count;//计算发送超时
}

#endif //SUPPORT_TI_COMM_MNG_PRE
