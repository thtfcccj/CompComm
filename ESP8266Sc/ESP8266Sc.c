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
  
  if(!Inited){//这里MPLABX memcpy会出错
    memcpyL((char*)&ESP8266Sc.Info, (const char*)&ESP8266_cbInfoDefault, sizeof(struct _ESP8266ScInfo));
    Eeprom_Wr(ESP8266Sc_GetInfoBase(), 
              &ESP8266_cbInfoDefault, sizeof(struct _ESP8266ScInfo));
  }
  else{
    Eeprom_Rd(ESP8266Sc_GetInfoBase(), 
              &ESP8266Sc.Info, sizeof(struct _ESP8266ScInfo));
    #ifdef SUPPORT_PIC//在调试时，不重新开始，会读为全0,
    if(ESP8266Sc.Info.ServerPort == 0)
      memcpyL((char*)&ESP8266Sc.Info, (const char*)&ESP8266_cbInfoDefault, sizeof(struct _ESP8266ScInfo));
    #endif
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
//0x20:要检查接收, 0x10接收OK检查, 0x08 超时不重试直接进入下一模式
static const unsigned char _StateInfo[] = {
  0x00 | 0x00 | 0x00 | 0x00 | 0x00,   //空闲状态
  0x00 | 0x00 | 0x00 | 0x00 | 0x00,   //退出透传模式
  0xC0 | 0x20 | 0x00 | 0x08 | 0x00,   //复位
  0x40 | 0x20 | 0x10 | 0x00 | 0x00,   //关闭回显,返回OK确认模块存在
  0x80 | 0x20 | 0x10 | 0x00 | 0x00,   //置Statin模式,返回OK成功
  0x40 | 0x20 | 0x00 | 0x00 | 0x00,   //获取本地IP地址以确定wifi已连接
  0x80 | 0x20 | 0x10 | 0x00 | 0x00,   //转入智能配网模式,等待OK字样
  0xC0 | 0x20 | 0x00 | 0x00 | 0x00,   //配置服务器,返回connect成功
                                     //(智能配网退出后此步时间较长,重试多次才能进入)
  0x40 | 0x20 | 0x10 | 0x00 | 0x00,   //设置为透传模式,返回OK成功
  0x80 | 0x20 | 0x10 | 0x00 | 0x00,   //开始透传,返回OK成功,并以>开始
  0x80 | 0x00 | 0x00 | 0x00 | 0x00,  //透传开始后的首个固定数据
};

//---------------------------状态gc 状态位-----------------------------------
//按位定义为：0-3Bit:正确时下一状态， 4-7Bit 错误时下一状态
//0x20:要检查接收, 0x10接收OK检查，否则为程序检查, 0x0f下次状态
static const unsigned char _State2State[] = {
  (ESP8266Sc_eIdie << 4)     | ESP8266Sc_eIdie,         //空闲状态
  (ESP8266Sc_eExitPass << 4) | ESP8266Sc_eRst,          //退出透传模式
  (ESP8266Sc_eExitPass << 4) | ESP8266Sc_eIsRdy,        //复位
  (ESP8266Sc_eExitPass << 4) | ESP8266Sc_eSetStation,   //关闭回显,返回OK确认模块存在
  (ESP8266Sc_eRst << 4)      | ESP8266Sc_eIsConn,       //置Statin模式,返回OK成功
  (ESP8266Sc_eIsConn << 4)   | ESP8266Sc_eSetServer,    //获取本地IP地址以确定与wifi已连接
  (ESP8266Sc_eRst << 4)      | ESP8266Sc_eSetServer,    //转入智能配网模式,等待OK字样
  (ESP8266Sc_eSetServer << 4)| ESP8266Sc_eSetPass,      //配置服务器,返回connect成功
  (ESP8266Sc_eSetPass << 4)  | ESP8266Sc_eStartPass,    //设置为透传模式,返回OK成功
  (ESP8266Sc_eExitPass << 4)  | ESP8266Sc_ePassData1st,  //开始透传,若未确认，必须重新开始以退出
  //(ESP8266Sc_eIdie << 4)     | ESP8266Sc_eIdie,         //透传开始后的首个固定数据  
};

//--------------------------等待信息转换为时间----------------------------------
static const unsigned char _WaitInfo2Time[] = {//128ms为单位,含发送时间
  2,  //0等待发送完成
  8,  //1等待通讯应答，(如发送ATE0)
  36, //等待工作响应
  200,// 3等待长时工作响应(如非配网模式复位,此时会连WIFI并成功以及获得IP,应多等会)
};

//-----------------------状态机对应的指令字符串定义-----------------------------
//->MPLABX编译器下,最后字符复制有的会出错，解决方案为：统一最后多加(或多发)一个字符以避免
static const char _DotSpace[] =   {"\", "};      //间隔字符
static const char _RN[] =   {"\r\n\n"};      //回车结束符(空时，填充字节会变为NULL)
static const char _DefaultServer[] = {"192.168.88.101\",\"10001\"\r\n "}; //默认服务器
//状态机对应,完整字符
static const char _ExitPass[] =  {"+++ "};         //退出透传模式,注意没有回车换行符
static const char _Rst[] =       {"AT+RST\r\n "};      //复位
static const char _DisReturn[] = {"ATE0\r\n "};        //禁止回显(IsRdy检查)
static const char _SetStation[] ={"AT+CWMODE=1\r\n "}; //单STA模式
static const char _GetIp[] =     {"AT+CIFSR\r\n "};    //得到本机IP(IsConn检查)
static const char _SmartConn[]=  {"AT+CWSTARTSMART=3\r\n "};//智能配网配式
static const char _SetServer[] =  {"AT+CIPSTART=\"TCP\",\" "}; //配置服务器,需后跟信息
static const char _SetPass[] =   {"AT+CIPMODE=1\r\n "};//设置为透传模式
static const char _EnterPass[] = {"AT+CIPSEND\r\n "};        //开始透传

//新版：
//static const char _SetStation[] ={"AT+CWMODE_DEF=1\r\n "}; //单STA模式，并把配置保存在flash
//static const char _SetAutoAP[]  ={"AT+CWAUTOCONN=1\r\n "}; //使能上电自动连接AP

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

//------------------------强制退出智能配网模式---------------------------------
void ESP8266Sc_ExitSmartConn(void)
{
  if(!(ESP8266Sc.Flag & ESP8266SC_WAIT_SMART_CONN)) return;
  ESP8266Sc.Timer = 0;
  ESP8266Sc.Flag &= ~(ESP8266SC_SMART_CONN | ESP8266SC_WAIT_SMART_CONN);
}

//-------------------------转到下一状态函数----------------------------------
//正确时调用
static void _ToNextState(unsigned char IsErr)
{
  unsigned char NextState = _State2State[ESP8266Sc.eState];
  if(IsErr) //有错误时，错误状态
    ESP8266Sc.eState = (enum _ESP8266Sc_eState)(NextState >> 4);
  else{//置工作模式后，若为智能配网模式,或检查连接但没连上WIFI时，强至转至智能配网模式
    if ((ESP8266Sc.eState == ESP8266Sc_eSetStation) && (ESP8266Sc.Flag & ESP8266SC_SMART_CONN) || 
      (ESP8266Sc.eState == ESP8266Sc_eIsConn) && !(ESP8266Sc.Flag & ESP8266SC_WIFI_FINAL)){
      ESP8266Sc.eState = ESP8266Sc_eSmartConn;
    }
    else{ //正确时，下一状态
      ESP8266Sc.eState = (enum _ESP8266Sc_eState)(NextState & 0x0f);
      ESP8266Sc.ErrIndex = 0;//总故障状态复位
    }
  }
  ESP8266Sc.RetryIndex = 0;//下一状态复位
  ESP8266Sc.Flag |= ESP8266SC_SEND_RDY;//下一状态处理    
}

//---------------------------任务函数-------------------------------------
//128ms调用一次
void ESP8266Sc_Task(void)
{
  if(ESP8266Sc.eState == ESP8266Sc_eIdie) return; //无任务
  //发送未完成或智能配网等待模式
  if(ESP8266Sc.Flag & (ESP8266SC_SEND_RDY | ESP8266SC_WAIT_SMART_CONN)) return; 
  
  if(ESP8266Sc.Timer){//时间未到
    ESP8266Sc.Timer--;
    return;
  }
  
  //等待时间到了，检查超时情况
  unsigned char Info = _StateInfo[ESP8266Sc.eState];
  if((Info & 0x28) == 0x20){//需检查接收时且没有禁止超时检查时进入
    ESP8266Sc.RetryIndex++;
    ESP8266Sc.ErrIndex++;
    if(ESP8266Sc.ErrIndex >= ESP8266SC_ERR_COUNT){ //总故障连续超时了,重新开始
      ESP8266Sc.eState = ESP8266Sc_eExitPass;
      _ToNextState(0);
    }
    else //超过当前次数转错误状态
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
    *pSendBuf = ':';//多加一字符防止发送-1出错
    *(pSendBuf + 1) = '\0';//用户不填充则无字符不发送
    ESP8266Sc_cbFulPassData1st(pSendBuf);   //由用户填充(可不填充即不发送)
    goto _FullEnd;
  }
  
  //发送ESP8266指令
  pSendBuf = strcpyL(pSendBuf, _pCmd[ESP8266Sc.eState - 1]) - 1;//尾部了
  //*pSendBuf = '\0';//这里加了会破坏前面的数
  //要增加服务器信息
  if(ESP8266Sc.eState == ESP8266Sc_eSetServer){
    if(ESP8266Sc_cbIsDefaultServer()){//默认服务器
      strcpyL(pSendBuf, _DefaultServer);
    }
    else{//配置的服务器,格式如：AT+CIPSTART="TCP","192.168.88.250",10001
      pSendBuf = Ip4ToStr(ESP8266Sc.Info.ServerIp, (char*)pSendBuf);//IP
      pSendBuf = strcpyL(pSendBuf, _DotSpace) - 1;
      pSendBuf = Value2StringMin(ESP8266Sc.Info.ServerPort, pSendBuf, 1);//端口号
      strcpyL(pSendBuf, _RN); //加入回车结束字符
    }
  }
  _FullEnd:
  ESP8266Sc_pcbSendBuf(strlen(pESP8266Sc_pcbGetBuf()) - 1); //发出送出,去掉多加的字符
  //配网模式复位防止直接连上，只有减小时间等待能才进入
  if((ESP8266Sc.Flag & ESP8266SC_SMART_CONN) && (ESP8266Sc.eState == ESP8266Sc_eRst))
    ESP8266Sc.Timer = 15;
  else  ESP8266Sc.Timer = _WaitInfo2Time[_StateInfo[ESP8266Sc.eState] >> 6];//超时
}

//----------------------------接收相关字符------------------------------
static const char _OK[] =         {"OK"};          //OK字符
static const char _Got[] =         {"GOT"};          //WIFI GOT IP字符
static const char _CONNECT[] =  {"CONNECT"};//服务器连接成功标志
//本地IP前缀(返回字样：+CIFSR:STAIP,"192.168.88.152"+CIFSRSTAMAC,)
static const char _DotDp[] =       {",\""}; //用于检查格式
static const char _NullIp[] =      {"0.0"}; //示例：MISTAIP,"0.0.0.0"表示无IP
static const char _ctedWifi[] =      {"cted"}; //配网成功后会提示：smartconfig connected wifi
//static const char _PassEnter[] =   {">"};        //开始透传标志\r\n

//--------------------------接收数据处理----------------------------------
//返回0没处理,否则处理完成
signed char ESP8266Sc_RcvData(const unsigned char *pData,
                               unsigned char Len)
{
  if(ESP8266Sc.eState == ESP8266Sc_eIdie) return 0; //无任务
  //注：未检查长度！！！！
  unsigned char Info = _StateInfo[ESP8266Sc.eState];
  if(!(Info & 0x20)) return 1; //不检查接收
  *((char*)pData + Len) = '\0';//强制增加结束字符
  const char *pStr = (const char *)pData;
  //检查接收是否下确(pStr ！= NULL为正确)  
  if(ESP8266Sc.Flag  & ESP8266SC_WAIT_SMART_CONN){//智能配网时检查返回状态
    if(StrFind(pStr, _ctedWifi) == NULL) return 1;
    ESP8266Sc.Flag  &= ~(ESP8266SC_SMART_CONN | ESP8266SC_WAIT_SMART_CONN);//配网成功了,继续
    ESP8266Sc_cbSmartConnFinal();//完成通报
  }
  else if(Info & 0x10){//OK字符检查
    pStr = StrFind(pStr, _OK);
    if(ESP8266Sc.eState == ESP8266Sc_eSmartConn){//智能配网等待模式
      ESP8266Sc.Flag |= ESP8266SC_WAIT_SMART_CONN;
      ESP8266Sc_cbEnterSmartConnNotify();//进入通报
      return 1;
    }
    if(ESP8266Sc.eState == ESP8266Sc_eStartPass){//开始透传时可能错过OK,检查起始
      //if(pStr == NULL) pStr = StrFind((const char *)pData, _PassEnter);
      //if(pStr == NULL) return;//还是没找到，不提前结束，继续等待
      pStr = pData;//修改为只要收到数即认为结束
    }
  }
  else if(ESP8266Sc.eState == ESP8266Sc_eRst){//复位期间获得IP时提前退出
    if(StrFind(pStr, _Got) == NULL) return 1;  
  }
  else if(ESP8266Sc.eState == ESP8266Sc_eSetServer)//设置服务器时
    pStr = StrFind(pStr, _CONNECT);
  else if(ESP8266Sc.eState == ESP8266Sc_eIsConn){//连接成功时
    pStr = StrFind(pStr, _DotDp);
    if(pData != NULL){//数据格式正确了
      if(StrFind(pStr, _NullIp) == NULL){//空IP表示没连上WIFI,否则表示连上WIFI了
        ESP8266Sc.Flag |= ESP8266SC_WIFI_FINAL;
        #ifdef SUPPORT_ESP8266SC_LOCAL_IP//保存本地IP时
          StrToIp4(pStr, ESP8266Sc.LocalIp);
        #endif 
      }
    }
  }
  // else if(ESP8266Sc.eState == ESP8266Sc_eStartPass){//开始透传时
  //  pStr = StrFind(pStr, _PassEnter);//开始透传时
  //}
  
  //结束处理
  if(pStr != NULL) _ToNextState(0);
  else ESP8266Sc.Timer = 0;//提前结束
  return 1;
}
      
      
      
