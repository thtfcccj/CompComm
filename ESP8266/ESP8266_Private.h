/******************************************************************************

                  ESP8266驱动程序-内部私有接口

******************************************************************************/
#ifndef __ESP8266_PRIVATE_H
#define	__ESP8266_PRIVATE_H

/******************************************************************************
                  ESP8266 公共函数
******************************************************************************/
//------------------------------得到读结果---------------------------------
//只能读一次
signed char ESP8266_GetRdRusume(void);

//------------------------------得到写结果---------------------------------
//只能读一次
signed char ESP8266_GetWrRusume(void);

//----------------------------相关字符------------------------------------
extern const char En_OK[];// =                    {"OK"};
extern const char En_PassExit[];// =              {"+++"};

//-----------------------判断字符是否为OK字样----------------------------------
signed char ESP8266_IsOk(const char *pStr);

//-----------------------获得接收到的字符------------------------------------
//自动在尾部增加结束字符
char *ESP8266_pGetRcvStr(unsigned short RcvSize);

//---------------------------释放被管理的设备----------------------------------
//返回是否成功释放
signed char ESP8266_RealseUsartDev(struct _UsartDev *pUsartDev);

//---------------------------释放后重新获得被管理的设备------------------------
void ESP8266_ReGetUsartDev(struct _UsartDev *pUsartDev);

/******************************************************************************
                  ESP8266 各子模式函数声明 
******************************************************************************/

//-----------------------初始化模式写函数------------------------------------
void ESP8266_InitModeWr(void);

//-----------------------初始化模式读检查函数------------------------------------
void ESP8266_InitModeRd(void);

//-----------------------透传模式写函数------------------------------------
void ESP8266_PassModeWr(void);

//-------------------------透传模式读检查函数----------------------------------
void ESP8266_PassModeRd(void);




#endif




