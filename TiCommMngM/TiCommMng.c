/*********************************************************************************

//		以时间间隔(Time Interval)作为数据帧判定依据的网络管理-多例化时实现

//此模块独立于硬件与应用
*********************************************************************************/

#include "TiCommMng.h"
#include <string.h>

//支持通讯计数时
#ifdef SUPPORT_COMM_COUNT 
  unsigned long _CommCount = 0;//接收总数 
  unsigned long _ValidCount = 0;//有效计数
  unsigned long _InValidCount = 0;//无效计数  
#endif

/***********************************************************************
                            相关函数实现
***********************************************************************/

//-----------------------------初始化函数-------------------------------
//调用此函数前,需初始化UsartDev(含UsartId)及其IO，及对Usart参数进行配置
void TiCommMng_Init(struct _TiCommMng *pMng,
                    const struct _UsartDevPt *pFun, //多态操作函数
                    struct _UsartDev *pDev)
{
  memset(pMng, 0, sizeof(struct _TiCommMng));
  UsartTiny_Init(&pMng->UsartTiny, pFun, pDev);//初始化底层通讯  
  //最后装载超时值
  pMng->Count = TiCommMng_cbBuadId2FremeOv(pMng); 
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
    pMng->Index = 0;//等待中
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

    #ifdef SUPPORT_COMM_COUNT //支持通讯计数时
      _CommCount++;//接收总数 
    #endif
      
     if((Resume != 0) && (Resume != 255)){ //数据正确,发送数据
       UsartTiny_SendStart(&pMng->UsartTiny, Resume);
       pMng->Flag |= TI_COMM_MNG_SEND_DOING;//启动发送
       pMng->Index = pMng->Count;//计算发送超时
       #ifdef SUPPORT_COMM_COUNT //支持通讯计数时
        _ValidCount++;//有效计数
       #endif
     }
     else{
      #ifdef SUPPORT_COMM_COUNT //支持通讯计数时
        if(Resume == 255) _InValidCount++;//无效计数
      #endif
     }
     
  }
  else{//发送超时完成了
    UsartTiny_Stop(&pMng->UsartTiny);
  }
}

//-------------------------接收定时器复位函数-------------------------------
void TiCommMng_ResetRcvTimer(struct _TiCommMng *pMng)
{
  pMng->Flag |= TI_COMM_MNG_RCV_DOING;
  if(!TiCommMng_cbRcvedNotify(pMng)) pMng->Index = pMng->Count;
  else pMng->Index = 0;//数据收完，结束了
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
