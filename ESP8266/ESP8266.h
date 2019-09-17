/***********************************************************************

                  ESP8266驱动程序
全双工驱动
***********************************************************************/
#ifndef __ESP8266_H
#define	__ESP8266_H

/****************************************************************************
		                      相关配置
****************************************************************************/

#ifndef ESP8266_WR_BUF_SIZE     //写缓冲大小
  #define ESP8266_WR_BUF_SIZE          128
#endif 

#ifndef ESP8266_RD_BUF_SIZE     //读缓冲大小
  #define ESP8266_RD_BUF_SIZE          256
#endif 

/****************************************************************************
		                      相关结构
****************************************************************************/

#include "AtUsart.h"

//主结构
struct _ESP8266{
  //底层实例:
  struct _AtUsart AtUsart; //独占型(Usart不独占)
  unsigned char WrBuf[ESP8266_WR_BUF_SIZE];               //写缓冲
  unsigned char RdBuf[ESP8266_RD_BUF_SIZE];               //读缓冲
  volatile signed char HwRdResume;        //AtUsart读结果
  volatile signed char HwWrResume;        //AtUsart写结果  
  unsigned char PreMode;               //预置的工作模式，见定义
  unsigned char CurMode;               //当前工作模式，见定义
  unsigned char ModeState;                 //当前工作模式对应状态，见工作模式定义
  unsigned char Timer;                 //定时器
  volatile unsigned char Flag;       //相关标志，见定义  
  
  unsigned long LocalIp;              //获取到的本地IP地址，用于局域网内访问
};

//相关标志定义为:
#define ESP8266_RD_WAIT      0x20           //数据已发出，读等待

#define ESP8266_HW_RDY       0x10           //ESP8266硬件检测到
#define ESP8266_WIFI_RDY     0x08           //ESP8266的WIFI准备好了
#define ESP8266_AP_RDY       0x04           //ESP8266带AP时，AP准备好了
#define ESP8266_HW_RDY_MASK  0x1C            //

#define ESP8266_CWMODE_MASK  0x03           //ESP8266工作模式0关,1:STA 2:AP 3:AP+STA


//工作模式及对应的状态定义为:
#define ESP8266_MODE_INIT         0     //初始化模式(开机时),状态定义为:
  //ModeState=0: 发送"AT+RST"指令，状态到1
  //ModeState=1: 发送"ATE0"指令关闭回显，返回OK则置ESP8266_HW_RDY,状态到2
  //ModeState=2: 发送"AT+CWMODE="指令返回OK, 则STA或AP+STA时状态到3,否则转至配置模式
  //ModeState=3: 发送"AT+CIFSR"获取并保存本地IP地址，无时转到4,有时置ESP8266_WIFI_RDY转至配置模式
  //ModeState=4: 发送"AT+CWSMARTSTART=2"转入智能配网模式，等待用户接下WIFI，转至5
  //             (手机配网：手机微信上搜索“安信可科技” 点击wifi配置)
  //ModeState=5: 守候接收，用户配置后，将接收到“OK”开始字符，置ESP8266_WIFI_RDY转至配置模式
#define ESP8266_MODE_CFG                0     //命令配置模式(其它模式退出时)等待

#define ESP8266_MODE_TCP_PASS_LOCAL     1     //本地TCP透传模式,即跟局域网内设备相连
  //1在本地电脑做好TCP/IP转串口服务器
  //ModeState=0: 发送"AT+CIPSTART="TCP","LocalIp4.LocalIp3.LocalIp2.485地址",10002"指令
  //成功返回CONNECT，转到1
  //ModeState=1: 发送“AT+CIPMODE=1”设置为透传模式,返回OK, 转到2
  //ModeState=2: 发送“AT+CIPSEND”开始透传,返回<,交出UsartDev控制权，转到3
  //ModeState=3: 透传模式中,此模式可由Modbus等交接，ESP8266_ModeExit()时，转到4
  //ModeState=4: 夺回UsartDev控制权,发送+++退出
#define ESP8266_MODE_TCP_PASS_GLOBAL    3     //全球TCP透传模式,即连接入Internet
  //同本地模式，仅替换本地地址为全局地址。



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

//---------------------------任务函数-------------------------------------
//128ms调吸入一次
void ESP8266_Task(void);

/******************************************************************************
		                            回调函数
******************************************************************************/

//---------------------------释放被管理的设备----------------------------------
void ESP8266_cbRealseUsartDev(void);

#endif




