/******************************************************************************
                            DA输出调节模块
//此模块界面系统与DA模块之前，完成下列功能:
//1.通过零点与增益， 完成应用与AD的线性转换。
//2.控制输出最大DA值
//3.完成零点与增益标定功能

//线性转换公式为:  输入值 * 转换系数Gain + 零点值
//此模块独立于应用与硬件
******************************************************************************/
  
#ifndef __DA_ADJ_H
#define __DA_ADJ_H

#include "DA.h"
/******************************************************************************
                                 相关配置
******************************************************************************/

//定义由应用数转换为DA值的转换系数的Q值
#ifndef DA_ADJ_GAIN_Q
  #define DA_ADJ_GAIN_Q          10
#endif

//定义自动退出时间,Task调用周期为单位
#ifndef DA_ADJ_QUIT_OV
  #define  DA_ADJ_QUIT_OV         40
#endif

//定义默认零点默认值
#ifndef DA_ADJ_ZERO_DEFAULT
  #define DA_ADJ_ZERO_DEFAULT	  0  //默认DA一一对应
#endif

//定义默认转换系数值,满量程相关,DA_ADJ_GAIN_Q值
#ifndef DA_ADJ_GAIN_DEFAULT
  #define DA_ADJ_GAIN_DEFAULT	  (1 << DA_ADJ_GAIN_Q)  //默认DA一一对应
#endif

//定义用户数值的满值
#ifndef DA_ADJ_USER_FULL
  #define DA_ADJ_USER_FULL      DA_FULL //默认DA对应
#endif

//定义零输出时的DA值(如4mA对应数值)
#ifndef DA_ADJ_OUT_ZERO
  #define DA_ADJ_OUT_ZERO      0 //默认最小
#endif

//定义满量程输出时的DA值(如20mA对应数值)
#ifndef DA_ADJ_OUT_FULL
  #define DA_ADJ_OUT_FULL      4000
#endif

//定义满输出时的DA值
#ifndef DA_ADJ_OUT_MAX
  #define DA_ADJ_OUT_MAX      DA_FULL //默认最大
#endif

/******************************************************************************
                                相关结构
******************************************************************************/
struct _DA_AdjInfo{
  unsigned short Zero;    //零点DA值
  unsigned short Gain;    //转换为DA值的转换系数,Q值
};

struct _DA_Adj{
  struct _DA_AdjInfo Info;
  unsigned char Timer;  //定时器,直接用最高位表示标校模式
};

extern struct _DA_Adj DA_Adj;  //直接实例化

/******************************************************************************
                                 相关函数
******************************************************************************/

//-------------------------------初始化函数------------------------------
void DA_Adj_Init(signed char Inited);

//-------------------------------任务函数------------------------------
//放入512mS进程中
void DA_Adj_Task(void);

//----------------------------更新应用数函数------------------------------
void DA_Adj_UpdateVol(unsigned short Vol);

//----------------------------更新应用为负数函数--------------------------
//输出0点以下值，Vol越大距0点越多
void DA_Adj_UpdateNegaVol(unsigned short Vol);

//----------------------------更新为零输出函数------------------------------
void DA_Adj_UpdateZero(void);

//----------------------------更新为最大值输出函数------------------------------
void DA_Adj_UpdateMax(void);

//------------------------------清零相关函数--------------------------------
//进出清零模式
void DA_Adj_SetZeroMode(void);
//void DA_Adj_ClrZeroMode(void);
#define DA_Adj_ClrZeroMode() do{DA_Adj.Timer = 0;}while(0)
//获得零点DA值
//unsigned short DA_Adj_GetZero(void);
#define DA_Adj_GetZero() (DA_Adj.Info.Zero)
//置零点DA值(不保存EERPOM)
void DA_Adj_SetZero(unsigned short Zero);
//保存当前零点DA值
void DA_Adj_SaveZero(void);

//------------------------------标定相关函数--------------------------------
//进出 模式
void DA_Adj_SetGainMode(void);
//void DA_Adj_ClrGainMode(void);
#define DA_Adj_ClrGainMode() do{DA_Adj.Timer = 0;}while(0)
//获得Gain值
//unsigned short DA_Adj_GetGain(void);
#define DA_Adj_GetGain()   (DA_Adj.Info.Gain)
//置Gain值(不保存EERPOM)
void DA_Adj_SetGain(unsigned short Gain);
//保存当前Gain值
void DA_Adj_SaveGain(void);


#endif //#define __DA_ADJ_H












