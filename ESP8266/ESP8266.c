/***********************************************************************

                  Sms用户通讯层
***********************************************************************/

#include "ESP8266.h"
#include "AtCmd.h"
#include "MemMng.h"
#include <string.h>

struct _ESP8266 *pESP8266 = NULL; //单例化

/******************************************************************************
		                        相关函数
******************************************************************************/

//-------------------------------写回调实现---------------------------------
//调用此函数前确保硬件通讯参数已初始化准备好
static signed char _AtUsartWrNotify(const void *pv,
                                     signed char State)
{ 
  pESP8266->HwWrResume = State;
  return 0;
}

//-------------------------------读回调实现---------------------------------
//调用此函数前确保硬件通讯参数已初始化准备好
static signed char _AtUsartRdNotify(const void *pv,
                                     signed char State)
{
  pESP8266->HwRdResume = State;
  return 0;
}

//------------------------------得到读结果---------------------------------
//只能读一次
static signed char _GetRdRusume(void)
{
  signed char Resume = pESP8266->HwRdResume;
  pESP8266->HwRdResume = 127;//读完复位
  return Resume;
}
//------------------------------得到写结果---------------------------------
//只能读一次
static signed char _GetWrRusume(void)
{
  signed char Resume = pESP8266->HwWrResume;
  pESP8266->HwWrResume = 127;//读完复位
  return Resume;
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
  AtUsart_Init(&pESP8266->AtUsart, pUsartDev, DevId, 0); //自动得到串口
  AtUsart_CfgSend(&pESP8266->AtUsart, ESP8266_WR_BUF_SIZE, 
                  pESP8266->WrBuf, _AtUsartWrNotify);
  AtUsart_CfgRcv(&pESP8266->AtUsart, ESP8266_RD_BUF_SIZE, 
                  pESP8266->RdBuf, _AtUsartRdNotify);
}
                                  
//-----------------------判断字符是否为OK字样------------------------------------
static signed char _IsOk(const char *pStr)
{
  if(*pStr++ != 'O') return 0;
  if(*pStr++ != 'K') return 0;
  return 1;
}

/******************************************************************************
		                          初始化模式相关
******************************************************************************/
//-----------------------内部使用到的相关指令------------------------------------
//不含AT字样
static const char _Rst[] =             {"+RST"};
static const char _DisReturn[] =       {"E0"};   //禁止回显
static const char _GetIp[] =           {"+=CIFSR"};   //得到本机IP
static const char _AutoSetWifi[] =     {"+CWSMARTSTART=2"};   //智能配网配式
//工作模式
static const char _SetCWMODE_Sta[] =   {"+CWMODE=1"};   //设置工作模式为STA
static const char _SetCWMODE_AP[] =    {"+CWMODE=2"};   //设置工作模式为AP
static const char _SetCWMODE_StaAP[] = {"+CWMODE=3"};   //设置工作模式为STA+AP
static const char * const _pSetCWMODE[] = {
  _SetCWMODE_Sta, //关闭时
  _SetCWMODE_Sta,
  _SetCWMODE_AP,
  _SetCWMODE_StaAP,
};
//初始化模式各状态对应字符
static const char * const _pInitStr[] = {
  _Rst,
  _DisReturn,
  NULL,
  _GetIp,
  _AutoSetWifi,  
};

//-----------------------初始化模式写函数------------------------------------
static void _InitModeWr(void)
{
  const char *pCmd;
  if(pESP8266->ModeState == 2)//配置工作模式
    pCmd = _pSetCWMODE[pESP8266->Flag & ESP8266_CWMODE_MASK];
  else pCmd = _pInitStr[pESP8266->ModeState];
  unsigned char RdCfg;
  if(pESP8266->ModeState == 0) RdCfg = AT_USART_RCV_DIS_ALL; //复位时字符不定有前导
  else RdCfg = 0; 
  AtCmd_RwAtStart(&pESP8266->AtUsart, pCmd, RdCfg); 
  pESP8266->Flag |= ESP8266_RD_WAIT;
  if(pESP8266->ModeState == 0) pESP8266->Timer = 80; //复位时要慢些
  else pESP8266->Timer = 8;// 1s为单位
}

//-----------------------初始化模式读检查函数------------------------------------
static void _InitModeRd(void)
{
  //智能配网等待结果中
  if(pESP8266->ModeState == 5){
    if(_GetRdRusume()){//没有结果,继续等待
      pESP8266->Timer = 8;// 1s为单位检查
    }
    else{//有返回了
      if(_IsOk((char*)AtUsart_pGetRcvBuf(&pESP8266->AtUsart))){//OK则正常转入
        pESP8266->ModeState = 3;//读IP信息
        _InitModeWr();
      }
      else{//其它字符复位不成功,重新开始
        pESP8266->Flag &= ~ESP8266_HW_RDY_MASK;
        pESP8266->ModeState = 0;
      }
    }
    return;
  }
  
  //其它读写检查
  if(_GetRdRusume() || _GetWrRusume()){//失败重新写
    _InitModeWr();
    return;
  }
  
  //结果决定状态
  unsigned short RcvSize = AtUsart_GetRcvSize(&pESP8266->AtUsart);
  const char *pStr = (char*)AtUsart_pGetRcvBuf(&pESP8266->AtUsart);
  
  //复位成功时，返回字符串为设备信息
  if(pESP8266->ModeState == 0){//这里只检查长度
    if(RcvSize >= 100) pESP8266->ModeState = 1;
  }
  //ATE0返回OK成功
  else if((pESP8266->ModeState == 1)){
    if(_IsOk(pStr)){
      pESP8266->ModeState = 2;
      pESP8266->Flag |= ESP8266_HW_RDY;
    }
  }
  //写模式成功返回OK
  else if((pESP8266->ModeState == 2)){
    if(_IsOk(pStr)){
      unsigned char CmMode = pESP8266->Flag & ESP8266_CWMODE_MASK;
      if(CmMode >= 2)//AP模式初始化完成
        pESP8266->Flag |= ESP8266_AP_RDY;
      if(CmMode == 2){//AP模式强致到配置模式
        pESP8266->CurMode = ESP8266_MODE_CFG;
        return;
      }
      //其它含STA模式，继续
      pESP8266->ModeState = 3; 
    }
  }
  //返回IP信息(同时WIFI成功)
  else if((pESP8266->ModeState == 3)){
    if(RcvSize >= 10){//IP成功了,进入配置模式
      pESP8266->Flag |= ESP8266_WIFI_RDY;
      pESP8266->CurMode = ESP8266_MODE_CFG;
      pESP8266->ModeState = 0;  
      return;
    }
    else pESP8266->ModeState = 4;//进入配网状态
  }
  //智能配网指令结果,OK指令成功
  else if((pESP8266->ModeState == 4)){
    if(_IsOk(pStr)){
      AtCmd_RdStart(&pESP8266->AtUsart,0,0,0);//AT模式全等
      pESP8266->Timer = 80;//首次等10s
      pESP8266->ModeState = 5;
      return;//等待结果
    }
  }
  _InitModeWr(); //写参数
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
  
  //根据工作状态执行任务
  switch(pESP8266->CurMode){
    case ESP8266_MODE_INIT:  //写完返回时,检查结果
      if((pESP8266->Flag & ESP8266_RD_WAIT)) _InitModeRd();
      else _InitModeWr(); break;
    default: break;
  }
}

/******************************************************************************
		                          配置模式相关
******************************************************************************/
#include "UsartMng.h"
//----------------------接收到有效超始字符后通报函数----------------------------
//可用于点亮接收指示灯
void AtUsart_cbRcvValidNotify(unsigned char DevId)//设备ID号
{
  UsartMng_cbSetLight(DevId);
}

//--------------------------------接收结束通报函数------------------------------
//可用于关闭接收指示灯
void AtUsart_cbRcvEndNotify(unsigned char DevId)//设备ID号
{
  UsartMng_cbClrLight(DevId);   
}


