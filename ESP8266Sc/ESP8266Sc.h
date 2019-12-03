/***********************************************************************

        置ESP8266Station(S首字母)与+TCP Client(C首字母)
此驱动为“ESP8266”模块的专用版与精简版本，适用性极简MCU
此模块独立于应用与底层串口
***********************************************************************/
#ifndef __ESP8266SC_H
#define	__ESP8266SC_H

/****************************************************************************
		                     相关配置
****************************************************************************/

//此模块建议全局定义
//#define SUPPORT_ESP8266

//定义是否支持获取本地IP地址,若在局域网内有多个设备联网，(RAM多占用4Byte,Flash若干)
//#define SUPPORT_ESP8266SC_LOCAL_IP

//定义每种状态没有接收后的重试次数，若超过此次数，则重新跑
#ifndef ESP8266SC_RETRY_COUNT
  #define ESP8266SC_RETRY_COUNT 2
#endif

/****************************************************************************
		                     流程说明
****************************************************************************/

//1. (0. (eIdie)开机为，什么都不做(利用ESP8266的保存功能，自动连接,此状态运行信息不置位)
//2. 连接WIFI(自动IP已分配方式,成功将置WIFI完成标志):
//  外部通讯不上或智能配网时，触发ESP8266Sc_ReConfigStart(IsSmartConn)开始配置，此流程为:
//  (1. (eExitPass)发送"+++"指令强制退出透传模式以进入初始化模式
//  (2. (eRst)发送"AT+RST"指令复位
//  (3. (eIsRdy)发送"ATE0"指令关闭回显，返回OK则继续，否则返回(1.(eExitPass)
//  (4. (eSetStation)发送"AT+CWMODE=1"置Statin模式，返回OK则继续，否则返回(1.(eExitPass)
//  (5. (eGetLocalIp)发送"AT+CIFSR"获取本地IP地址：若IsSmartConn置位,则转至配网模式
//               否则无时继续发送查询，有时转到配置服务器(eSetServer)
//  (6. (eSmartConn)发送"AT+CWSMARTSTART=2"转入智能配网模式,长时等待结果
//                  若用户配网成功，将接收到“OK”开始字符,否则返回(1.(eExitPass)
//3. 连接服务器(成功将置服务器完成标志): 
//   (7. (eSetServer) 发送"AT+CIPSTART="TCP","Ip4.Ip3.Ip2.Ip4",port"指令(Info里)
//   (8. (eSetPass) 发送“AT+CIPMODE=1”设置为透传模式
//   (9. (eStartPass) 发送“AT+CIPSEND”开始透传,返回<,
//   (10.(ePassData1St) 发送首个透传数据(如从机信息，版本,欢迎等)，置标志并转回空闲

/****************************************************************************
		                      相关结构
****************************************************************************/

//状态机,见流程描述
enum _ESP8266Sc_eState{
  ESP8266Sc_eIdie           = 0,//空闲状态
  ESP8266Sc_eExitPass       = 1,//退出透传模式
  ESP8266Sc_eRst            = 2,//复位
  ESP8266Sc_eIsRdy          = 3,//关闭回显,等待OK确认模块存在
  ESP8266Sc_eSetStation     = 4,//置Statin模式
  ESP8266Sc_eIsConn         = 5,//获取本地IP地址以确定与服务器已连接
  ESP8266Sc_eSmartConn      = 6,//转入智能配网模式
  ESP8266Sc_eSetServer      = 7,//配置服务器
  ESP8266Sc_eSetPass        = 8,//设置为透传模式
  ESP8266Sc_eStartPass      = 9,//开始透传
  ESP8266Sc_ePassData1st    = 10,//透传开始后的首个固定数据,如从机信息，版本,欢迎等
};

//信息位
struct _ESP8266ScInfo{
  unsigned char ServerIp[4]; //服务器IPv4地址
  unsigned short ServerPort; //服务器端口号
};

//主结构
struct _ESP8266Sc{
  struct _ESP8266ScInfo Info;
  enum _ESP8266Sc_eState eState;
  unsigned char Timer;            //定时器
  unsigned char RetryIndex;       //同一状态查询次数
  unsigned char Flag;             //相关标志，见定义
  
  #ifdef SUPPORT_ESP8266SC_LOCAL_IP//保存本地IP时
  unsigned char LocalIp[4]; //本机IPv4地址
  #endif
};
//相关标志定义为:
#define ESP8266SC_STATE_RUNNED  0x80    //此状态跑过了，否则开机即透传
#define ESP8266SC_WIFI_FINAL    0x40    //ESP8266的WIFI准备好了
#define ESP8266SC_SERVER_FINAL  0x20    //ESP8266与服务器连接成功了


#define ESP8266SC_SMART_CONN  0x20    //用户带入的强制进入智能配网模式
#define ESP8266SC_SEND_RDY    0x10    //发送准备好了

extern struct _ESP8266Sc ESP8266Sc; 

/******************************************************************************
		                        相关函数
******************************************************************************/

//-------------------------------初始化函数---------------------------------
void ESP8266Sc_Init(signed char Inited);

//---------------------------启动重配置-----------------------------------
//形参为是否进入智能配网模式
void ESP8266Sc_ReConfigStart(unsigned char IsSmartConn);

//---------------------------任务函数-------------------------------------
//128ms调用一次
void ESP8266Sc_Task(void);

//------------------------快速任务函数-------------------------------------
void ESP8266Sc_FastTask(void);

//--------------------------接收数据处理----------------------------------
//返回0没处理,否则处理完成
signed char ESP8266Sc_RcvData(const unsigned char *pData,
                               unsigned char Len);

//------------------------是否空闲-------------------------------------
#define ESP8266Sc_IsIdie()    (ESP8266Sc.eState == ESP8266Sc_eIdie)

/******************************************************************************
		                   回调函数-上层
******************************************************************************/

//---------------------------得到默认Info结构-------------------------------
//为防止服务器信息泄露，留外部定义
const struct _ESP8266ScInfo *ESP8266Sc_pcbGetInfoDefault(void);

//---------------------------填充首个透传数据-------------------------------
//可填充 如从机信息，版本,欢迎等以告知从机连接上了
//数据必须以结束字符结束，不填充将不返回数据
void ESP8266Sc_cbFulPassData1st(char *pStr);

//----------------------------------配置完成通报----------------------------
void ESP8266Sc_cbConfigFinal(void);

//-----------------------------是否为默认服务器-----------------------------
//此功能用于在不连接英特网服务器情况下，连接本地默认服务器进行配置
signed char ESP8266Sc_cbIsDefaultServer(void);

/******************************************************************************
		                   回调函数-底层通讯
******************************************************************************/
//---------------------------得到发送缓冲区---------------------------------
char *pESP8266Sc_pcbGetBuf(void);

//------------------------------发送缓冲区数据------------------------------
void ESP8266Sc_pcbSendBuf(unsigned char Len);






#endif




