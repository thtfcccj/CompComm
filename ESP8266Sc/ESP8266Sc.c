/***********************************************************************

        置ESP8266Station(S首字母)与+TCP Client(C首字母)实现
 * 注：strcpy在此函数内调用时,在xc8 1.37中编译器中会链接不过
***********************************************************************/

#include "ESP8266Sc.h"
#include "Eeprom.h"
#include "InfoBase.h"

#include "StringEx.h"
#include <string.h>

struct _ESP8266Sc ESP8266Sc;  //单例化

/******************************************************************************
		                        相关函数
******************************************************************************/

//-------------------------------初始化函数---------------------------------
void ESP8266Sc_Init(signed char Inited)
{
  memset(&ESP8266Sc, 0, sizeof(struct _ESP8266Sc));
  if(!Inited){
    memcpy(&ESP8266Sc.Info, 
          ESP8266Sc_pcbGetInfoDefault(), sizeof(struct _ESP8266ScInfo));
    Eeprom_Wr(ESP8266Sc_GetInfoBase(), 
              &ESP8266Sc.Info, sizeof(struct _ESP8266ScInfo));
  }
  else{
    Eeprom_Rd(ESP8266Sc_GetInfoBase(), 
              &ESP8266Sc.Info, sizeof(struct _ESP8266ScInfo));
  }
}

//---------------------------启动重配置-----------------------------------
//形参为是否进入智能配网模式
void ESP8266Sc_ReConfigStart(unsigned char IsSmartConn)
{
  //所有标志一起复位
  if(IsSmartConn) 
    ESP8266Sc.Flag = ESP8266SC_STATE_RUNNED | ESP8266SC_SEND_RDY | ESP8266SC_SMART_CONN;
  else ESP8266Sc.Flag = ESP8266SC_STATE_RUNNED | ESP8266SC_SEND_RDY;
  ESP8266Sc.eState = ESP8266Sc_eExitPass;
  ESP8266Sc.Timer = 0;//下周期开始
  ESP8266Sc.RetryIndex = 0;
}

//---------------------------状态信息位-----------------------------------
//按位定义为：//0xC0:等待状态(0:不等，1等待通讯回应，2等待工作响应，3等待用户响应) 
//0x20:要检查接收, 0x10接收OK检查
static const unsigned char _StateInfo[] = {
  0x00 | 0x00 | 0x00 | 0x00 | 0x00,   //空闲状态
  0x40 | 0x00 | 0x00 | 0x00 | 0x00,   //退出透传模式
  0x80 | 0x00 | 0x00 | 0x00 | 0x00,   //复位
  0x40 | 0x20 | 0x10 | 0x00 | 0x00,   //关闭回显,返回OK确认模块存在
  0x80 | 0x20 | 0x10 | 0x00 | 0x00,   //置Statin模式,返回OK成功
  0x40 | 0x20 | 0x00 | 0x00 | 0x00,   //获取本地IP地址以确定与服务器已连接
  0xC0 | 0x20 | 0x10 | 0x00 | 0x00,   //转入智能配网模式,等待OK字样
  0x80 | 0x20 | 0x00 | 0x00 | 0x00,   //配置服务器,返回connect成功
  0x40 | 0x20 | 0x10 | 0x00 | 0x00,   //设置为透传模式,返回OK成功
  0x40 | 0x00 | 0x00 | 0x00 | 0x00,   //开始透传
  0x40 | 0x00 | 0x00 | 0x00 | 0x00,  //透传开始后的首个固定数据
};

//---------------------------状态gc 状态位-----------------------------------
//按位定义为：0-3Bit:正确时下一状态， 4-7Bit 错误时下一状态
//0x20:要检查接收, 0x10接收OK检查，否则为程序检查, 0x0f下次状态
static const unsigned char _State2State[] = {
  ESP8266Sc_eIdie     | ESP8266Sc_eIdie,         //空闲状态
  ESP8266Sc_eExitPass | ESP8266Sc_eRst,          //退出透传模式
  ESP8266Sc_eExitPass | ESP8266Sc_eIsRdy,        //复位
  ESP8266Sc_eExitPass | ESP8266Sc_eSetStation,   //关闭回显,返回OK确认模块存在
  ESP8266Sc_eRst      | ESP8266Sc_eIsConn,       //置Statin模式,返回OK成功
  ESP8266Sc_eIsConn   | ESP8266Sc_eSetServer,    //获取本地IP地址以确定与服务器已连接
  ESP8266Sc_eRst      | ESP8266Sc_eSetServer,    //转入智能配网模式,等待OK字样
  ESP8266Sc_eSetServer| ESP8266Sc_eSetPass,      //配置服务器,返回connect成功
  ESP8266Sc_eSetPass  | ESP8266Sc_eStartPass,    //设置为透传模式,返回OK成功
  ESP8266Sc_eSetPass  | ESP8266Sc_ePassData1st,  //开始透传
  //ESP8266Sc_eIdie     | ESP8266Sc_eIdie,         //透传开始后的首个固定数据  
};

//--------------------------等待信息转换为时间----------------------------------
static const unsigned char _WaitInfo2Time[] = {
  0,//0:不等，
  3,  //1等待通讯回应(发发送ATE0，将直接获得响应)
  15, //等待工作响应(如复位)
  255,// 3等待用户响应(如智能配网)
};

//-----------------------状态机对应的指令字符串定义-----------------------------
static const char _DotSpace[] =   {"\",\""};      //间隔字符
static const char _DefaultServer[] = {"192.168.88.101\",\"10001\"\r\n"}; //默认服务器

//状态机对应,完整字符
static const char _ExitPass[] =  {"+++"};         //退出透传模式
static const char _Rst[] =       {"AT+RST\r\n"};      //复位
static const char _DisReturn[] = {"ATE0\r\n"};        //禁止回显(IsRdy检查)
static const char _SetStation[] ={"AT+CWMODE=1\r\n"}; //置Statin模式
static const char _GetIp[] =     {"AT+CIFSR\r\n"};    //得到本机IP(IsConn检查)
static const char _SmartConn[]=  {"AT+CWSMARTSTART=2\r\n"};//智能配网配式
static const char _SetServer[] =  {"AT+CIPSTART=\"TCP\",\""}; //配置服务器,需跟信息
static const char _SetPass[] =   {"AT++CIPMODE=1"};//设置为透传模式
static const char _EnterPass[] = {"\r\n>"};        //开始透传

//初始化模式各状态对应字符
static const char * const _pCmd[] = {
  _ExitPass,             //退出透传模式
  _Rst,                  //复位
  _DisReturn,            //关闭回显,等待OK确认模块存在
  _SetStation,           //置Statin模式
  _GetIp,                //获取本地IP地址以确定与服务器已连接
  _SmartConn,            //转入智能配网模式
  _SetServer,            //配置服务器
  _SetPass,              //设置为透传模式
  _EnterPass,            //开始透传  
};

//-------------------------转到下一状态函数----------------------------------
//正确时调用
static void _ToNextState(unsigned char IsErr)
{
  unsigned char NextState = _State2State[ESP8266Sc.eState];
  if(IsErr) //有错误时，错误状态
    ESP8266Sc.eState = (enum _ESP8266Sc_eState)(NextState >> 4);
  else //正确时，下一状态
    ESP8266Sc.eState = (enum _ESP8266Sc_eState)(NextState & 0x0f);
  ESP8266Sc.RetryIndex = 0;//下一状态复位
  ESP8266Sc.Flag |= ESP8266SC_SEND_RDY;//下一状态处理    
}

//---------------------------任务函数-------------------------------------
//128ms调用一次
void ESP8266Sc_Task(void)
{
  if(ESP8266Sc.eState == ESP8266Sc_eIdie) return; //无任务
  if(ESP8266Sc.Flag & ESP8266SC_SEND_RDY) return; //发送未完成
  if(ESP8266Sc.Timer){//时间未到
    ESP8266Sc.Timer--;
    return;
  }
  
  //等待时间到了，检查超时情况
  unsigned char Info = _StateInfo[ESP8266Sc.eState];
  if(Info & 0x20){//需检查接收时超时了
    ESP8266Sc.RetryIndex++;
    //超过次数转错误状态
    if(ESP8266Sc.RetryIndex >= ESP8266SC_RETRY_COUNT) _ToNextState(1);
    else ESP8266Sc.Flag |= ESP8266SC_SEND_RDY;//继续当前状态重试
    return;
  }
  //等待响应时间到了，下一状态
  //透传模式首个数据结束了
  if(ESP8266Sc.eState == ESP8266Sc_ePassData1st){
    ESP8266Sc.Flag |= ESP8266SC_SERVER_FINAL;
    ESP8266Sc.eState = ESP8266Sc_eIdie;   
    ESP8266Sc_cbConfigFinal(); //通报完成
  }
  else _ToNextState(0);  //下一状态
}
  
//------------------------快速任务函数-------------------------------------
void ESP8266Sc_FastTask(void)
{
  //只处理发送
  if(!(ESP8266Sc.Flag & ESP8266SC_SEND_RDY)) return;
  ESP8266Sc.Flag &= ~ESP8266SC_SEND_RDY;
  
  char *pSendBuf = pESP8266Sc_pcbGetBuf();
  
  //透传模式了，填充用户欢迎信息后直接结束状态
  if(ESP8266Sc.eState == ESP8266Sc_ePassData1st){
    *pSendBuf = '\0';//用户不填充则无字符不发送
    ESP8266Sc_cbFulPassData1st(pSendBuf);   //由用户填充
    unsigned short Len = strlen(pSendBuf);
    if(Len) ESP8266Sc_pcbSendBuf(Len); //发出
  }
  
  //发送ESP8266指令
  pSendBuf = strcpyL(pSendBuf, _pCmd[ESP8266Sc.eState - 1]);//尾部了
  //要增加服务器信息
  if(ESP8266Sc.eState == ESP8266Sc_eSetServer){
    if(ESP8266Sc_cbIsDefaultServer()){//默认服务器
      strcpyL(pSendBuf, _DefaultServer);
    }
    else{//配置的服务器
      pSendBuf = Ip4ToStr(ESP8266Sc.Info.ServerIp, (char*)pSendBuf);//IP
      pSendBuf = strcpyL((char*)pSendBuf, _DotSpace); //","  
      pSendBuf = Value2StringMin(ESP8266Sc.Info.ServerPort, pSendBuf, 1);//端口号
      strcpyL(pSendBuf, _DefaultServer + (sizeof(_DefaultServer) - 4)); //结束字符
    }
  }
  ESP8266Sc_pcbSendBuf(strlen(pESP8266Sc_pcbGetBuf())); //发出送出
  ESP8266Sc.Timer = _WaitInfo2Time[_StateInfo[ESP8266Sc.eState] >> 6];//超时
}

//----------------------------接收相关字符------------------------------
static const char _OK[] =         {"OK"};          //OK字符
static const char _CONNECTED[] =  {"CONNECTED"};//服务器连接成功标志
//本地IP前缀(返回字样：+CIFSR:STAIP,"192.168.88.152"+CIFSRSTAMAC,)
static const char _DotDp[] =       {",\""}; 

//--------------------------接收数据处理----------------------------------
//返回0没处理,否则处理完成
signed char ESP8266Sc_RcvData(const unsigned char *pData,
                               unsigned char Len)
{
  if(ESP8266Sc.eState == ESP8266Sc_eIdie) return 0; //无任务
  //注：未检查长度！！！！
  unsigned char Info = _StateInfo[ESP8266Sc.eState];
  if(!(Info & 0x80)) return 1; //不处理接收
  const char *pStr = (const char *)pData;
  //检查接收是否下确(pStr ！= NULL为正确)
  if(Info & 0x40)//OK字符检查
    pStr = StrFind(pStr, _OK); 
  else if(ESP8266Sc.eState == ESP8266Sc_eSetServer)//设置服务器时
    pStr = StrFind(pStr, _CONNECTED); 
  else if(ESP8266Sc.eState == ESP8266Sc_eIsConn){//连接成功时
    pStr = StrFind(pStr, _DotDp);
    if(pData != NULL){//连接上本地了
      ESP8266Sc.eState |= ESP8266SC_WIFI_FINAL;
      #ifdef SUPPORT_ESP8266SC_LOCAL_IP//保存本地IP时
        StrToIp4(pStr, ESP8266Sc.LocalIp);
      #endif 
    }
  }
  //结束处理
  if(pStr != NULL) _ToNextState(0);  
  else ESP8266Sc.Timer = 0;//提前结束
  return 1;
}
      
      
      
