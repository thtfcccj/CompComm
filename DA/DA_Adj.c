/******************************************************************************

                            DA输出调节模块实现

******************************************************************************/

#include "DA_Adj.h"
#include <string.h>

#include "InfoBase.h"
#include "Eeprom.h"

struct _DA_Adj DA_Adj;  //直接实例化

/******************************************************************************
                               相关函数
******************************************************************************/

//-------------------------------初始化函数------------------------------
void DA_Adj_Init(signed char Inited)
{
  memset(&DA_Adj, 0,  sizeof(struct _DA_Adj));
  if(!Inited){
    DA_Adj.Info.Zero = DA_ADJ_ZERO_DEFAULT;
    DA_Adj.Info.Gain = DA_ADJ_GAIN_DEFAULT;
    Eeprom_Wr(DA_Adj_GetInfoBase(),
              &DA_Adj.Info,
              sizeof(struct _DA_AdjInfo));
  }
  else{
    Eeprom_Rd(DA_Adj_GetInfoBase(),
              &DA_Adj.Info,
              sizeof(struct _DA_AdjInfo));  
  }
  DA_Init();//DA初始化
  DA_Adj_UpdateVol(0);//立即输出零点值
}

//-------------------------------任务函数------------------------------
//放入512mS进程中
void DA_Adj_Task(void)
{
  if(!DA_Adj.Timer) return;
  DA_Adj.Timer--;
  if(DA_Adj.Timer == 0x80) DA_Adj.Timer = 0;
}

//-------------------------由浓度值更新DA值函数------------------------------
//返回对应的DA值
static void _UpdateVol(unsigned short Vol)
{
  unsigned long Data = (unsigned long)Vol;
  Data *= DA_Adj.Info.Gain;
  Data >>= DA_ADJ_GAIN_Q;
  Data += DA_Adj.Info.Zero;
  if(Data >= 65535) DA_SetDA(65535);
  else DA_SetDA(Data);
}

//----------------------------更新浓度值函数------------------------------
void DA_Adj_UpdateVol(unsigned short Vol)
{
  if(DA_Adj.Timer) return; //标定中不能更新
  _UpdateVol(Vol);
}

//----------------------------更新应用为负数函数--------------------------
//输出0点以下值，Vol越大距0点越多
void DA_Adj_UpdateNegaVol(unsigned short Vol)
{
  if(DA_Adj.Timer) return; //标定中不能更新  
  unsigned long Data = (unsigned long)Vol;
  Data *= DA_Adj.Info.Gain;
  Vol = Data >> DA_ADJ_GAIN_Q;
  
  if(DA_Adj.Info.Zero > Vol) DA_SetDA(Vol);
  else DA_SetDA(0);//异常
}

//----------------------------更新为零输出函数------------------------------
void DA_Adj_UpdateZero(void)
{
  if(DA_Adj.Timer) return; //标定中不能更新
  DA_SetDA(DA_ADJ_OUT_ZERO);  
}

//----------------------------更新为最大值输出函数------------------------------
void DA_Adj_UpdateMax(void)
{
  DA_SetDA(DA_ADJ_OUT_MAX);
}

//------------------------------清零相关函数--------------------------------
//进出清零模式
void DA_Adj_SetZeroMode(void)
{
  DA_Adj.Timer = DA_ADJ_QUIT_OV;
  DA_SetDA(DA_Adj.Info.Zero);
}

//置零点DA值(不保存EERPOM)
void DA_Adj_SetZero(unsigned short Zero)
{
  DA_Adj.Timer = DA_ADJ_QUIT_OV;
  DA_Adj.Info.Zero = Zero;
  DA_SetDA(Zero);
}
//保存当前零点DA值
void DA_Adj_SaveZero(void)
{
	Eeprom_Wr(DA_Adj_GetInfoBase() + struct_offset(struct _DA_AdjInfo,Zero),
			     &DA_Adj.Info.Zero,
           2);
  DA_Adj.Timer = 0;
}

//------------------------------标定相关函数--------------------------------
//进出 模式
void DA_Adj_SetGainMode(void)
{
  DA_Adj.Timer = 0x80 | DA_ADJ_QUIT_OV;
  _UpdateVol(DA_ADJ_OUT_FULL);
}
//置Gain值(不保存EERPOM)
void DA_Adj_SetGain(unsigned short Gain)
{
  DA_Adj.Timer = 0x80 | DA_ADJ_QUIT_OV;
  DA_Adj.Info.Gain = Gain;
  _UpdateVol(DA_ADJ_OUT_FULL);
}
//保存当前Gain值
void DA_Adj_SaveGain(void)
{
  Eeprom_Wr(DA_Adj_GetInfoBase() + struct_offset(struct _DA_AdjInfo,Gain),
			   &DA_Adj.Info.Gain,
               2); 
  DA_Adj.Timer = 0;
}





