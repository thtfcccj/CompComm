/***********************************************************************

                  ESP8266驱动程序
全双工驱动
***********************************************************************/
#ifndef __ESP8266_H
#define	__ESP8266_H

/****************************************************************************
		                      相关配置
****************************************************************************/

//此模块为可选时，（此模块不用，供外部使用, 定义需放全局配置里）
//#define SUPPORT_ESP8266

//支持基类时(用于通讯通数等， 定义需放全局配置里)
//#define SUPPORT_ESP8266_BASE     

#ifndef ESP8266_WR_BUF_SIZE     //写缓冲大小
  #define ESP8266_WR_BUF_SIZE          256
#endif 

#ifndef ESP8266_RD_BUF_SIZE     //读缓冲大小
  #define ESP8266_RD_BUF_SIZE          256
#endif 

/****************************************************************************
		                      相关结构
****************************************************************************/

#include "AtUsart.h"
#ifdef SUPPORT_ESP8266_BASE
  #include "ComBase.h"
#endif

//主结构
struct _ESP8266{
  #ifdef SUPPORT_ESP8266_BASE
    struct _ComBase Base;        //通讯基类
  #endif
  //底层实例:
  struct _AtUsart AtUsart;       //独占型(Usart不独占)
  struct _UsartDev OrgUsartDev; //缓冲被劫持的底层设备信息以便于恢复  
  
  unsigned char WrBuf[ESP8266_WR_BUF_SIZE];               //写缓冲
  unsigned char RdBuf[ESP8266_RD_BUF_SIZE];               //读缓冲
  volatile signed char HwRdResume;        //AtUsart读结果
  volatile signed char HwWrResume;        //AtUsart写结果  
  unsigned char PreMode;               //预置的工作模式，见定义
  unsigned char CurMode;               //当前工作模式，见定义
  unsigned char ModeState;             //当前工作模式对应状态，见工作模式定义
  unsigned char Timer;                 //定时器
  unsigned char PrvModeState;         //上次当前工作模式对应状态，用于通讯计数
  unsigned char CurCommErrIndex;      //当前通讯计数器,连续相同状态保持一定时间自动复位
  volatile unsigned char Flag;       //相关标志，见定义  
  
  unsigned char LocalIp[4];           //获取到的本地IP地址，用于局域网内访问
};

//相关标志定义为:
#define ESP8266_PASS_RCV_FINAL  0x40        //透传模式收到数据标志
#define ESP8266_RD_WAIT      0x20           //数据已发出，读等待

#define ESP8266_HW_RDY       0x10           //ESP8266硬件检测到
#define ESP8266_WIFI_RDY     0x08           //ESP8266的WIFI准备好了
#define ESP8266_AP_RDY       0x04           //ESP8266带AP时，AP准备好了
#define ESP8266_HW_RDY_MASK  0x1C            //

#define ESP8266_CWMODE_MASK  0x03           //ESP8266工作模式0关,1:STA 2:AP 3:AP+STA


//工作模式及对应的状态定义为:
#define ESP8266_MODE_INIT            0     //初始化模式(开机时),状态定义为:
  //发送"+++"指令强制退出透传模式以进入初始化模式
  #define  ESP8266_MODE_INIT_ENTER         0 
  //发送"AT+RST"指令，状态到1
  #define  ESP8266_MODE_INIT_RST           1 
  //发送"ATE0"指令关闭回显，返回OK则置ESP8266_HW_RDY,状态到2
  #define  ESP8266_MODE_INIT_ATE0          2
  //发送"AT+CWMODE?"指令获到当前模式,若与当前模式相同，则已正常工作
  #define  ESP8266_MODE_INIT_GET_CWMODE    3  
  //发送"AT+CWMODE="指令返回OK, 则STA或AP+STA时状态到3,否则转至配置模式
  #define  ESP8266_MODE_INIT_SET_CWMODE    4
  //发送"AT+CIFSR"获取并保存本地IP地址，无时转到4,有时置ESP8266_WIFI_RDY转至配置模式
  #define  ESP8266_MODE_INIT_CIFSR         5     
  //ModeState=4: 发送"AT+CWSMARTSTART=2"转入智能配网模式，等待用户接下WIFI，转至5
  //             (手机配网：手机微信上搜索“安信可科技” 点击wifi配置)
  #define  ESP8266_MODE_INIT_SMART_START   6   
  //ModeState=5: 守候接收，用户配置后，将接收到“OK”开始字符，置ESP8266_WIFI_RDY转至配置模式
  #define  ESP8266_MODE_INIT_SMART_WAIT    7   
  
  //注：开机时,先无条件发送"+++"以可交流后，先读取IP，或不成功，则复位，再读IP若还不成功
  //      将转入自动配网模式，超时重新复位开马建勋。

#define ESP8266_MODE_CFG                1     //命令配置模式(其它模式退出时)等待

#define ESP8266_MODE_TCP_PASS           2     //TCP透传模式(支持局域网与全球TCP透传)
  //1在本地电脑做好TCP/IP转串口服务器
  //发送"AT+CIPSTART="TCP","Ip4.Ip3.Ip2.Ip4",port"指令
  //成功返回CONNECT，转到1,否则一直在此状态 
  #define  ESP8266_MODE_PASS_SERVER           0 
  //发送“AT+CIPMODE=1”设置为透传模式,返回OK, 转到2
  #define  ESP8266_MODE_PASS_RDY         1 
  //发送“AT+CIPSEND”开始透传,返回<,交出UsartDev控制权，转到3
  #define  ESP8266_MODE_PASS_ENTER       2 
  //透传模式中,此模式可由Modbus等交接，ESP8266_ModeExit()时，转到4
  #define  ESP8266_MODE_PASS_DOING       3 
  //ModeState=4: 夺回UsartDev控制权,发送+++退出
  #define  ESP8266_MODE_PASS_EXIT        4 

#define ESP8266_MODE_MQTT            2     //MQTT模式(暂不支持)

//考虑到大部分系统只有一个8266,故直接单例化,但考虑到挂载问题，故用指针
extern struct _ESP8266 *pESP8266; 

/******************************************************************************
		                        相关函数
******************************************************************************/

//-------------------------------初始化函数---------------------------------
//调用此函数前确保硬件通讯参数已初始化准备好
void ESP8266_Init(struct _UsartDev *pUsartDev, //已初始化完成的底层设备 
                  unsigned char DevId,         //设备挂载的ID号
                  unsigned char CwMode,       //ESP8266工作模式,0关,1:STA 2:AP 3:AP+STA
                  unsigned char PreMode);     //本模块预置的工作模式

//---------------------------1ms硬件调用任务函数---------------------------------
//放在硬件定时器中
#define ESP8266_1msHwTask() \
  do{if(pESP8266 != NULL){AtUsart_1msHwTask(&pESP8266->AtUsart);}}while(0)

//---------------------------进入某个模式---------------------------------
void ESP8266_ModeEnter(unsigned char Mode);

//---------------------------退出原有模式---------------------------------
void ESP8266_ModeExit(void);

//-----------------------快速任务函数-------------------------------------
//放在进程中扫描
void ESP8266_FastTask(void);

//---------------------------任务函数-------------------------------------
//128ms调吸入一次
void ESP8266_Task(void);

/******************************************************************************
		                            回调函数
******************************************************************************/

//--------------------获取全球TCP/IP服务器主机IP地址最低位---------------------
//返回NULL为本地模式
unsigned char *pESP8266_cbGetGlobalServerIp(unsigned char DevId);

//--------------------获取本地TCP/IP服务器主机IP地址最低位---------------------
//高3位用本地IP代替
unsigned char ESP8266_cbGetLocalServerIpLowest(unsigned char DevId);

//--------------------------获取TCP/IP服务器主机端口号---------------------
//本地或全球
unsigned short ESP8266_cbGetServerPort(unsigned char DevId);

//-------------------------内部透传时，数据处理----------------------------
//返回发送数据长度，0表示不回数
unsigned short ESP8266_cbPassEncoder(unsigned char *pData,  //数据区
                                     unsigned short RcvLen,   //收到的数据长度
                                     unsigned short BufSize); //缓冲区大小
#endif




