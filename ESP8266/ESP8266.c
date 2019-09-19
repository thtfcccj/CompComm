/***********************************************************************

                  ESP8266驱动程序
全双工驱动
***********************************************************************/

#include "ESP8266.h"
#include "ESP8266_Private.h"
#include "AtCmd.h"
#include "MemMng.h"
#include "StringEx.h"
#include <string.h>

struct _ESP8266 *pESP8266 = NULL; //单例化

/******************************************************************************
		                        相关函数
******************************************************************************/

//---------------------------释放被管理的设备----------------------------------
//返回是否成功释放
signed char ESP8266_RealseUsartDev(struct _UsartDev *pUsartDev)
{
  if(pESP8266->OrgUsartDev.pVoid == NULL) return 0;//没在透传模式
  memcpy(pESP8266->AtUsart.pUsartDev,
         &pESP8266->OrgUsartDev, 
         sizeof(struct _UsartDev));
  return 1;
}

//---------------------------释放后重新获得被管理的设备------------------------
void ESP8266_ReGetUsartDev(struct _UsartDev *pUsartDev)
{
  //无条件保存
  memcpy(&pESP8266->OrgUsartDev, pUsartDev, sizeof(struct _UsartDev));
}

//-------------------------------写回调实现---------------------------------
//调用此函数前确保硬件通讯参数已初始化准备好
static signed char _AtUsartWrNotify(const void *pv,
                                     signed char State)
{ 
  pESP8266->HwWrResume = State;
  #ifdef SUPPORT_ESP8266_BASE
    pESP8266->Base.CommCount++; 
  #endif
  return 0;
}

//-------------------------------读回调实现---------------------------------
//调用此函数前确保硬件通讯参数已初始化准备好
static signed char _AtUsartRdNotify(const void *pv,
                                     signed char State)
{
  pESP8266->HwRdResume = State;
  //透传模式立即执行
  if((pESP8266->CurMode == ESP8266_MODE_TCP_PASS) && 
     (pESP8266->ModeState == ESP8266_MODE_PASS_DOING))
    pESP8266->Flag |= ESP8266_PASS_RCV_FINAL;
  
  #ifdef SUPPORT_ESP8266_BASE
    if(State) pESP8266->Base.InvalidCount++; 
    else pESP8266->Base.ValidCount++;
  #endif
  
  return 0;
}

//------------------------------得到读结果---------------------------------
//只能读一次
signed char ESP8266_GetRdRusume(void)
{
  signed char Resume = pESP8266->HwRdResume;
  pESP8266->HwRdResume = 127;//读完复位
  return Resume;
}

//------------------------------得到写结果---------------------------------
//只能读一次
signed char ESP8266_GetWrRusume(void)
{
  signed char Resume = pESP8266->HwWrResume;
  pESP8266->HwWrResume = 127;//读完复位
  return Resume;
}

//----------------------------重启通讯---------------------------------
void _ReStartComm(void)
{
  pESP8266->Flag &= ~(ESP8266_HW_RDY_MASK | ESP8266_PASS_RCV_FINAL | ESP8266_RD_WAIT);
  pESP8266->CurCommErrIndex = 0;
  pESP8266->CurMode = ESP8266_MODE_INIT;
  pESP8266->ModeState = ESP8266_MODE_INIT_ENTER;//因ESP8266有记忆功能，故可以直接就连上了
  pESP8266->Timer = 0;//下周期开始
}     
         
//-------------------------------初始化函数---------------------------------
//调用此函数前确保硬件通讯参数已初始化准备好
void ESP8266_Init(struct _UsartDev *pUsartDev, //已初始化完成的底层设备 
                  unsigned char DevId,         //设备挂载的ID号
                  unsigned char CwMode,       //ESP8266工作模式,0关,1:STA 2:AP 3:AP+STA
                  unsigned char PreMode)     //本模块预置的工作模式 
{
  //分配struct _SenMng内存
  pESP8266 = MemMng_pvMalloc(sizeof(struct _ESP8266));
  memset(pESP8266, 0, sizeof(struct _ESP8266));
  //初始化相关
  pESP8266->Flag = CwMode;
  pESP8266->PreMode = PreMode; 
  
  #ifdef SUPPORT_ESP8266_BASE
    pESP8266->Base.ComId = DevId;
  #endif
  
  ESP8266_ReGetUsartDev(pUsartDev); //先获取控制权
  AtUsart_Init(&pESP8266->AtUsart, pUsartDev, DevId, 0); //自动得到串口
  AtUsart_CfgSend(&pESP8266->AtUsart, ESP8266_WR_BUF_SIZE, 
                  pESP8266->WrBuf, _AtUsartWrNotify);
  AtUsart_CfgRcv(&pESP8266->AtUsart, ESP8266_RD_BUF_SIZE, 
                  pESP8266->RdBuf, _AtUsartRdNotify);
  //开始通讯
  _ReStartComm();
}
                                  
//-----------------------判断字符是否为OK字样------------------------------------
signed char ESP8266_IsOk(const char *pStr)
{
  if(*pStr++ != 'O') return 0;
  if(*pStr++ != 'K') return 0;
  return 1;
}

//----------------------------相关字符------------------------------------
const char En_OK[] =                    {"OK"};
const char En_PassExit[] =              {"+++"};

//-----------------------获得接收到的字符------------------------------------
//自动在尾部增加结束字符
char *ESP8266_pGetRcvStr(unsigned short RcvSize)
{
  char *pStr = (char*)AtUsart_pGetRcvBuf(&pESP8266->AtUsart);
  if(RcvSize < (ESP8266_RD_BUF_SIZE - 1)) //强制增加结束字符
    *(pStr + RcvSize) = '\0'; 
  else  *(pStr + RcvSize - 1) = '\0';
  return pStr;
}

//-----------------------快速任务函数-------------------------------------
//放在进程中扫描
void ESP8266_FastTask(void)
{
  if(pESP8266 == NULL) return; //未挂接
  //只处理透传模式立即执行
  if(!(pESP8266->Flag & ESP8266_PASS_RCV_FINAL)) return;
  pESP8266->Flag &= ~ESP8266_PASS_RCV_FINAL;
  ESP8266_PassModeRd();
}

//---------------------------任务函数-------------------------------------
//128ms调吸入一次
void ESP8266_Task(void)
{
  if(pESP8266 == NULL) return; //未挂接
  
  if((pESP8266->Flag & ESP8266_CWMODE_MASK) == 0) return;//关闭时不处理
  
  //接收等待中 
  if(pESP8266->Timer){
    pESP8266->Timer--;
    return;
  }
  
  //=======================根据工作状态执行任务=======================
  switch(pESP8266->CurMode){
    case ESP8266_MODE_INIT:  //初始化模式
      if((pESP8266->Flag & ESP8266_RD_WAIT)) //写完返回时,检查结果
        ESP8266_InitModeRd();
      else ESP8266_InitModeWr(); break;
    case ESP8266_MODE_TCP_PASS:  //TCP透传模式  
      if((pESP8266->Flag & ESP8266_RD_WAIT))//写完返回时,检查结果
        ESP8266_PassModeRd();
      else ESP8266_PassModeWr(); break;      
    default: break;
  }
  
  
  //============在暂态时，相同状态达到一定阶段重新启动状态防止异常========
  if(!(pESP8266->Flag & ESP8266_PASS_RCV_FINAL) && 
     (pESP8266->CurMode != ESP8266_MODE_CFG)){
    if(pESP8266->ModeState == pESP8266->PrvModeState){
      pESP8266->CurCommErrIndex++;
      if(pESP8266->CurCommErrIndex == 255){//计时到了
        _ReStartComm();
      }
    }
    else{
      pESP8266->PrvModeState = pESP8266->ModeState;
      pESP8266->CurCommErrIndex = 0;
    }
  }
  else  pESP8266->CurCommErrIndex = 0;
  
  
}

/******************************************************************************
		                          配置模式相关
******************************************************************************/
#include "UsartMng.h"
//----------------------接收到有效超始字符后通报函数----------------------------
//可用于点亮接收指示灯
void AtUsart_cbRcvValidNotify(unsigned char DevId)//设备ID号
{
  //智能配网模式反亮提示
  if((pESP8266->CurMode == ESP8266_MODE_INIT) &&
     (pESP8266->ModeState >= ESP8266_MODE_INIT_SMART_START))
    UsartMng_cbClrLight(DevId);
  else UsartMng_cbSetLight(DevId);
}

//--------------------------------接收结束通报函数------------------------------
//可用于关闭接收指示灯
void AtUsart_cbRcvEndNotify(unsigned char DevId)//设备ID号
{
  //智能配网模式反亮提示
  if((pESP8266->CurMode == ESP8266_MODE_INIT) &&
     (pESP8266->ModeState >= ESP8266_MODE_INIT_SMART_START))
    UsartMng_cbSetLight(DevId);
  else UsartMng_cbClrLight(DevId);   
}


