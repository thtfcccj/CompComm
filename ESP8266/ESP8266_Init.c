/***********************************************************************

                  ESP8266驱动程序-初始化模式实现

***********************************************************************/

#include "ESP8266.h"
#include "ESP8266_Private.h"
#include "AtCmd.h"
#include "MemMng.h"
#include "StringEx.h"
#include <string.h>

/******************************************************************************
		                          初始化模式相关
******************************************************************************/

//--------------------------填充本地IP------------------------------------
//pStr字样:+CIFSR:STAIP,"192.168.88.152"+CIFSRSTAMAC,
const char En_DotDp[] =                 {",\""};
static void _FullLocalIp(char *pStr)
{
  pStr = StrFind(pStr, En_DotDp);
  if(pStr == NULL) return; //异常
  StrToIp4(pStr, pESP8266->LocalIp);
}

//-----------------------内部使用到的相关指令------------------------------------
//不含AT字样
static const char _Rst[] =             {"+RST"};
static const char _DisReturn[] =       {"E0"};   //禁止回显
static const char _GetIp[] =           {"+CIFSR"};   //得到本机IP
static const char _AutoSetWifi[] =     {"+CWSMARTSTART=2"};   //智能配网配式
//工作模式
static const char _GetCWMODE_Sta[] =   {"+CWMODE?"};   //得到工作模式
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
  En_PassExit,
  _Rst,
  _DisReturn,
  _GetCWMODE_Sta,
  NULL,
  _GetIp,
  _AutoSetWifi,  
};

//初始化模式各状态位查找表,高4bit为DisALL
static const unsigned char  _InitLut[] = {
  AT_USART_RCV_DIS_ALL,//强制退出透传模式，接收所有  
  AT_USART_RCV_DIS_ALL,//返回信息，接收所有
  0,                   //回显返回返回OK
  AT_USART_RCV_DIS_ALL,//返回+CWMODE，接收所有
  0,                   //返回OK
  AT_USART_RCV_DIS_ALL,//,返回+CIFSR
  0,                   //返回OK成功
};

//-----------------------初始化模式写函数------------------------------------
void ESP8266_InitModeWr(void)
{
  //无条件写+++
  if(pESP8266->ModeState == ESP8266_MODE_INIT_ENTER){
    AtCmd_WrStart(&pESP8266->AtUsart, (const unsigned char*)En_PassExit, 
                  strlen(En_PassExit), AT_USART_SEND_DIS_ALL);
    pESP8266->Timer = 8;// 1s为单位
    pESP8266->Flag |= ESP8266_RD_WAIT;
    return;
  }
  //正常AT写
  const char *pCmd;
  if(pESP8266->ModeState == ESP8266_MODE_INIT_SET_CWMODE)//配置工作模式
    pCmd = _pSetCWMODE[pESP8266->Flag & ESP8266_CWMODE_MASK];
  else pCmd = _pInitStr[pESP8266->ModeState];
  if(pESP8266->ModeState == ESP8266_MODE_INIT_RST)
    pESP8266->Timer = 80; //复位时要慢些
  else pESP8266->Timer = 8;// 1s为单位
  AtCmd_RwAtStart(&pESP8266->AtUsart, pCmd, 
                  _InitLut[pESP8266->ModeState] & AT_USART_RCV_DIS_ALL); 
  
  pESP8266->Flag |= ESP8266_RD_WAIT;
}

//-----------------------初始化模式读检查函数----------------------------------
void ESP8266_InitModeRd(void)
{
  //无条件写+++完成
  if(pESP8266->ModeState == ESP8266_MODE_INIT_ENTER){
    pESP8266->ModeState = ESP8266_MODE_INIT_ATE0;//因ESP8266有记忆功能，故可以直接就连上了
    pESP8266->Timer = 40; //开机时要慢些
    ESP8266_InitModeWr();
    return;
  }
  
  //先获得字符
  unsigned short RcvSize = AtUsart_GetRcvSize(&pESP8266->AtUsart);
  char *pStr = ESP8266_pGetRcvStr(RcvSize);
  
  //智能配网等待结果中
  if(pESP8266->ModeState == ESP8266_MODE_INIT_SMART_WAIT){
    if(ESP8266_GetRdRusume()){//没有结果,继续等待
      pESP8266->Timer = 8;// 1s为单位检查
    }
    else{//有返回了
      if(StrFind(pStr, En_OK) != NULL){//有OK， 则正常转入
        pESP8266->Flag |= ESP8266_WIFI_RDY;
        pESP8266->ModeState = ESP8266_MODE_INIT_CIFSR;//读IP信息
        ESP8266_InitModeWr();
      }
      else{//其它字符不成功,重新开始
        
      }
    }
    return;
  }
  
  //其它读写检查
  if(ESP8266_GetRdRusume() || ESP8266_GetWrRusume()){//失败重新写
    if(!(pESP8266->Flag & ESP8266_HW_RDY) && 
       (pESP8266->ModeState == ESP8266_MODE_INIT_ATE0)){//退出透传不成功时
      pESP8266->ModeState = ESP8266_MODE_INIT_ENTER;
    }
    ESP8266_InitModeWr();
    return;
  }
  
  //复位成功时，返回字符串为设备信息
  if(pESP8266->ModeState == ESP8266_MODE_INIT_RST){//这里只检查长度
    if(RcvSize >= 100) pESP8266->ModeState = ESP8266_MODE_INIT_ATE0;
  }
  //ATE0返回OK成功
  else if(pESP8266->ModeState == ESP8266_MODE_INIT_ATE0){
    if(ESP8266_IsOk(pStr)){
      if(pESP8266->Flag & ESP8266_HW_RDY)//正常配置进入时
        pESP8266->ModeState = ESP8266_MODE_INIT_SET_CWMODE;
      else //开机进入时,到获取IP
        pESP8266->ModeState = ESP8266_MODE_INIT_CIFSR;
    }
  }
  //写模式成功返回OK
  else if((pESP8266->ModeState == ESP8266_MODE_INIT_SET_CWMODE)){
    if(ESP8266_IsOk(pStr)){
      unsigned char CmMode = pESP8266->Flag & ESP8266_CWMODE_MASK;
      if(CmMode >= 2)//AP模式初始化完成
        pESP8266->Flag |= ESP8266_AP_RDY;
      if(CmMode == 2){//AP模式强致到配置模式
        pESP8266->CurMode = ESP8266_MODE_CFG;
        return;
      }
      //其它含STA模式，继续
      pESP8266->ModeState = ESP8266_MODE_INIT_CIFSR; 
    }
  }
  //返回IP信息(同时WIFI成功)
  else if((pESP8266->ModeState == ESP8266_MODE_INIT_CIFSR)){
    if(RcvSize >= 30){//IP成功了
      _FullLocalIp(pStr);
      if(pESP8266->LocalIp[3] != 0){//IP有效了，否则为0.0.0.0表示未接入网络
        pESP8266->Flag |= ESP8266_WIFI_RDY | ESP8266_HW_RDY;
        if(pESP8266->PreMode <= ESP8266_MODE_CFG)//直接进入配置模式
          pESP8266->CurMode = ESP8266_MODE_CFG;
        else//进入用户指定模式
          pESP8266->CurMode = pESP8266->PreMode;
        pESP8266->ModeState = 0; 
        pESP8266->Flag &= ~ESP8266_RD_WAIT; //重新开始
        return;
      }
    }
    //IP获取不成功
    if(pESP8266->Flag & ESP8266_HW_RDY){//正常配置进入时
      if(!(pESP8266->Flag & ESP8266_WIFI_RDY))//没有配置WIFI时
        pESP8266->ModeState = ESP8266_MODE_INIT_SMART_START;//进入配网状态
      //else 继续查询
    }
    else{//开机进入时
      pESP8266->Flag |= ESP8266_HW_RDY;//启动准备
      pESP8266->ModeState = ESP8266_MODE_INIT_RST;//进入复位状态
    }
  }
  //智能配网指令结果,OK指令成功
  else if((pESP8266->ModeState == ESP8266_MODE_INIT_SMART_START)){
    if(ESP8266_IsOk(pStr)){
      AtCmd_RdStart(&pESP8266->AtUsart,AT_USART_RCV_DIS_ALL,0,500);//AT模式全等
      pESP8266->Timer = 80;//首次等10s
      pESP8266->ModeState = ESP8266_MODE_INIT_SMART_WAIT;
      return;//等待结果
    }
  }
  ESP8266_InitModeWr(); //写参数
}


