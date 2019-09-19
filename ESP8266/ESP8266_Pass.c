/***********************************************************************

                  ESP8266驱动程序-透传模式实现

***********************************************************************/

#include "ESP8266.h"
#include "ESP8266_Private.h"
#include "AtCmd.h"
#include "MemMng.h"
#include "StringEx.h"
#include <string.h>

/******************************************************************************
		                     本地TCP透传模式相关
******************************************************************************/
//-----------------------内部使用到的相关指令------------------------------------
//不含AT字样
static const char _Dot3[] =           {"..."};
static const char _Port[] =           {"Port"};
static const char _CONNECTED[] =           {"CONNECTED"};
static const char _EnterPass[] =           {"\r\n>"};
static const char _PassDataRdy[] =           {"Pass Rdy!"};

static const char _SetServer[] =           {"+CIPSTART=\"TCP\",\"...\",Port"};
static const char _PassRdy[] =             {"+CIPMODE=1"};
static const char _PassEnter[] =           {"+CIPSEND"};

//各模式各状态对应字符
static const char * const _pPassStr[] = {
  _SetServer, 
  _PassRdy,
  _PassEnter,
  NULL,
  En_PassExit,
};

//各状态位查找表,高4bit为DisALL
static const unsigned char  _PassLut[] = {
  AT_USART_RCV_DIS_ALL,//返回连接情况，接收所有
  0,                   //成功返回OK
  AT_USART_RCV_DIS_ALL,//进入后，返回OK,再返回<
  AT_USART_RCV_DIS_ALL,   //透传中
  AT_USART_RCV_DIS_ALL,//退出返回
};

//-----------------------透传模式写函数------------------------------------
void ESP8266_PassModeWr(void)
{
  const char *pCmd;
  if(pESP8266->ModeState == ESP8266_MODE_PASS_SERVER){//服务器连接时
    //组织字符串
    char *pBuf = (char*)AtUsart_pGetSendBuf(&pESP8266->AtUsart);    
    memcpy(pBuf, _SetServer, sizeof(_SetServer));
    //IP地址替换
    char *pCurBuf = (char*)pESP8266->RdBuf; //临时借用此缓冲
    
    const unsigned char *pIp = pESP8266_cbGetGlobalServerIp(pESP8266->AtUsart.DevId);
    if(pIp == NULL){//本地时
      unsigned char LocalServerIp[4];
      memcpy(LocalServerIp, pESP8266->LocalIp, 3);
      LocalServerIp[3] = ESP8266_cbGetLocalServerIpLowest(pESP8266->AtUsart.DevId);;
      Ip4ToStr(LocalServerIp, pCurBuf);
    }
    else{ //全局时
      Ip4ToStr(pIp,pCurBuf);
    }
      
    StringReplace(pBuf, _Dot3, pCurBuf);
    //端口替换(<=32767)
    Value2StringMin(ESP8266_cbGetServerPort(pESP8266->AtUsart.DevId),pCurBuf, 1);
    StringReplace(pBuf, _Port, pCurBuf);
    pCmd = pBuf;
    pESP8266->Timer = 50; //时间长点
  }
  else{
    pCmd = _pPassStr[pESP8266->ModeState];
    if(pCmd == NULL) return; //异常
    if(pESP8266->ModeState == ESP8266_MODE_PASS_ENTER) 
      pESP8266->Timer = 30;//等待时间长点
    else pESP8266->Timer = 8;// 1s为单位
  }
  AtCmd_RwAtStart(&pESP8266->AtUsart, pCmd, 
                  _PassLut[pESP8266->ModeState] & AT_USART_RCV_DIS_ALL); 
  pESP8266->Flag |= ESP8266_RD_WAIT;
}

//-------------------------透传模式读检查函数----------------------------------
void ESP8266_PassModeRd(void)
{
  unsigned short RcvSize = AtUsart_GetRcvSize(&pESP8266->AtUsart);
  //透传模式等待中
  if(pESP8266->ModeState == ESP8266_MODE_PASS_DOING){
    pESP8266->Timer = 255;//最长等待
    if(ESP8266_GetRdRusume()) return;//无数据
    unsigned char *pBuf = AtUsart_pGetRcvBuf(&pESP8266->AtUsart);
    if(RcvSize){//收到数据时
      RcvSize = ESP8266_cbPassEncoder(pBuf, RcvSize, ESP8266_WR_BUF_SIZE);
      if(RcvSize){//有回数时直接透传发送出去
        AtCmd_WrStart(&pESP8266->AtUsart, pBuf,RcvSize, AT_USART_SEND_DIS_ALL);
        //全双工可在发送时接收
      }
    }
    //再次启动内部全接收
    AtCmd_RdStart(&pESP8266->AtUsart,AT_USART_RCV_DIS_ALL, 0, 200);
    return;
  }
  
  char *pStr = ESP8266_pGetRcvStr(RcvSize);

  //其它读写检查
  if(ESP8266_GetRdRusume() || ESP8266_GetWrRusume()){//失败重新写
    ESP8266_PassModeWr();
    return;
  }
  
  //连接服务器状态检查
  if(pESP8266->ModeState == ESP8266_MODE_PASS_SERVER){
    if(StrFind(pStr, _CONNECTED) != NULL)//有连接标识， 则正常转入
      pESP8266->ModeState = ESP8266_MODE_PASS_RDY;
  }
  //设置为透传模式,返回OK
  else if((pESP8266->ModeState == ESP8266_MODE_PASS_RDY)){
    if(ESP8266_IsOk(pStr))
      pESP8266->ModeState = ESP8266_MODE_PASS_ENTER;
  }
  //开始透传,返回<
  else if((pESP8266->ModeState == ESP8266_MODE_PASS_ENTER)){
    if(StrFind(pStr, _EnterPass) != NULL){//有换行及>标识， 则正常转入
      pESP8266->ModeState = ESP8266_MODE_PASS_DOING; 
      if(!ESP8266_RealseUsartDev(pESP8266->AtUsart.pUsartDev)){//没有交出控制权时
        //通报准备好以交互
        AtCmd_WrStart(&pESP8266->AtUsart, (const unsigned char*)_PassDataRdy,
                      sizeof(_PassDataRdy) - 1, AT_USART_SEND_DIS_ALL);
        AtCmd_RdStart(&pESP8266->AtUsart,AT_USART_RCV_DIS_ALL, 0, 200);//启动内部全接收
        pESP8266->Timer = 255;//最长等待
      }
      return;
    }
  }
  //透传退出
  else if((pESP8266->ModeState == ESP8266_MODE_PASS_EXIT)){
    if(ESP8266_IsOk(pStr)){
      pESP8266->ModeState = 0;
      pESP8266->CurMode = ESP8266_MODE_CFG; //强制退到配置模式
    }
  }

  ESP8266_PassModeWr(); //写参数
}




