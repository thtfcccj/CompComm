/*******************************************************************************

//		ModbusRtu管理器实现

*******************************************************************************/

#include "ModbusRtuMng.h"
#include <string.h>

#include "Eeprom.h"
#include "InfoBase.h"

struct _ModbusRtuMng ModbusRtuMng;

/***********************************************************************
                       内部函数声明
***********************************************************************/
//CRC校验
unsigned short ModbusRtuMng_GetCRC16(unsigned char *pBuf,  //数据帧
                                     unsigned short Len);   //数据帧长度
//波特率配置
#define _GetBuadCfg() ((ModbusRtuMng.Info.CommCfg >> 4) & 0x07)                       

/***********************************************************************
                   相关函数实现
***********************************************************************/

//各波特率RTC超时时间,ms为单位
static const unsigned char _Baud2Ov[] = {
    3,  //9600,//   0  //默认
    6,  //4800,//   1
    12, //2400,//   2
    24, //1200,//   3
    3,  //19200,//  4
    3,  //38400,//  5
    3,  //57600,//  6
    3,  //115200,// 7
};

#ifdef SUPPORT_MODBUS_BUND_ADR67 //支持地址高两位保为波特率时
static const unsigned char _Adr67ToBaud[] = {
  0,  //全不拔为9600
  2,  //高位拔为2400
  1,  //低位拔为4800
  4,  //全拔为19200
};
#endif

//------------------------初始化函数----------------------------
void ModbusRtuMng_Init(unsigned char Inited)
{
  memset(&ModbusRtuMng, 0, sizeof(struct _ModbusRtuMng));
  if(!Inited){
    //写入Eeprom,Adr默认为0,只接受254地址通讯
    Eeprom_Wr(ModbusRtuMng_GetInfoBase(),
              &ModbusRtuMng.Info,
              sizeof(struct _ModbusRtuMngInfo));
  }
  else{
    //读出
    Eeprom_Rd(ModbusRtuMng_GetInfoBase(),
               &ModbusRtuMng.Info,
               sizeof(struct _ModbusRtuMngInfo));
    #ifdef SUPPORT_MODBUS_BUND_ADR67 //支持地址高两位保为波特率时
      ModbusRtuMng.Info.CommCfg = 0; //强制默认(9600 + 无校验8数据位1停止位)
    #endif
  }
  ModbusRtuMng.Count = _Baud2Ov[_GetBuadCfg()];
  UsartTiny_CfgHw(ModbusRtuMng.Info.CommCfg);//配置底层硬件
  UsartTiny_Init();//初始化底层通讯
}

//--------------------------通讯地址设置函数-----------------------------
void ModbusRtuMng_SetAdr(unsigned char Adr)
{
  #ifdef SUPPORT_MODBUS_BUND_ADR67 //支持地址高两位保为波特率时,强制无校验8数据位
     ModbusRtuMng_SetCommCfg(_Adr67ToBaud[(Adr >> 2) & 0x30]); 
     Adr &= ~0xC0;  //去除高两位  
  #endif 

  ModbusRtuMng.Info.Adr = Adr;
  #ifdef SUPPORT_MODBUS_SW_ADR
    Eeprom_Wr(ModbusRtuMng_GetInfoBase() + 
                struct_offset(struct _ModbusRtuMngInfo, Adr),
              &ModbusRtuMng.Info.Adr, 1);
  #endif
}

//--------------------------通讯参数设置函数-----------------------------
void ModbusRtuMng_SetCommCfg(unsigned char CommCfg)
{
  ModbusRtuMng.Info.CommCfg = CommCfg;
  ModbusRtuMng.Count = _Baud2Ov[_GetBuadCfg()];
  Eeprom_Wr(ModbusRtuMng_GetInfoBase() + 
            struct_offset(struct _ModbusRtuMngInfo, CommCfg),
            &ModbusRtuMng.Info.CommCfg, 1);
  UsartTiny_CfgHw(ModbusRtuMng.Info.CommCfg);//重配置底层硬件
  UsartTiny_Init();//初始化底层
}

//--------------------------数据处理函数-------------------------------
//接收完数据后调用此函数
//此函数负责处理UsartTiny.Data缓冲区里的数据,并将处理结果返回
//返回255表示错误;  0:数据正确，但不返回数据 
//其它:返回数据个数
unsigned char ModbusRtuMng_Data(void)
{
  //先检查地址是否正确
  unsigned char CurAdr = UsartTiny.Data[0];
  //地址错误,254为默认地址,用于重设通讯址与波特率等
  if((CurAdr != 0) && (CurAdr != ModbusRtuMng.Info.Adr)&& (CurAdr != 254))
    return -1;
    
  //检查长度是否过短,地址1+功能码1+数据[0~n]+校验码2 >=4
  if(UsartTiny.Index < 4) return -1; //个数错误
  
  //CRC校验
  unsigned char Len = UsartTiny.Index - 2;//CRC点两位
  unsigned short CRC;
  CRC = (unsigned short)(UsartTiny.Data[Len] << 8) | 
         (unsigned short)(UsartTiny.Data[Len + 1]); 
  if(CRC != ModbusRtuMng_GetCRC16(UsartTiny.Data,Len)) 
    return (unsigned char)-3;   //CRC数据校验码错误
      
  //数据正确,交由应用层处理,数据在UsartTiny.Data中,0为地址,Len为长度
  Len = ModbusRtuMng_cbDataNotify(UsartTiny.Data, Len);
  #ifdef SUPPORT_MODBUS_DATA_REMAIN//不能识别的数据时后处理
  if(Len == 255)
     Len = ModbusRtuMng_cbDataRemain(UsartTiny.Data, Len);
  #endif
  if((Len == 0) || (Len == 255)) return Len;//无数据要发送
  if(CurAdr == 0) return 0;//广播地址无数据发送
  //有数据需发送了
  //加CRC校验码
  CRC = ModbusRtuMng_GetCRC16(UsartTiny.Data, Len);
  UsartTiny.Data[Len] = (unsigned char)(CRC >> 8);//CRC高位
  UsartTiny.Data[Len + 1] = (unsigned char)(CRC & 0xff);//CRC低位
  return Len + 2; //返回需发送的数据个数CRC16占2位
}
//-------------------------------普通查询任务----------------------------
//将此函数放入系统1ms进程中
void ModbusRtuMng_Task(void)
{
  #ifdef SUPPORT_MODBUS_PRE //挂起时暂停处理
    if(ModbusRtuMng.Flag & MODBUS_RTU_MNG_SUSPEND) return;
  #endif  
  
  //空闲状态启动接收
  if(UsartTiny.eState == UsartTiny_eState_Idie){
    UsartTiny_RcvStart();
    //停止数据收发数据
    ModbusRtuMng.Flag &= ~(MODBUS_RTU_MNG_RCV_DOING | MODBUS_RTU_MNG_SEND_DOING);
    return;
  }
  //没在发送与接收过程中,//不执行
  if(!(ModbusRtuMng.Flag & (MODBUS_RTU_MNG_RCV_DOING | 
                          MODBUS_RTU_MNG_SEND_DOING))) return;
  //发送与接收过程计时
  if(ModbusRtuMng.Index) ModbusRtuMng.Index--;
  if(ModbusRtuMng.Index) return;//时间未到
  
  //数据接收完成
  if(ModbusRtuMng.Flag & MODBUS_RTU_MNG_RCV_DOING){
    UsartTiny_Stop(); //强制中止接收
    ModbusRtuMng.Flag &= ~MODBUS_RTU_MNG_RCV_DOING;//停止数据接收
     unsigned char Resume;
    #ifdef SUPPORT_MODBUS_PRE 
      Resume = ModbusRtuMng_cbRcvPreDataPro();
      if(Resume == 254){//暂停接收
        UsartTiny_Stop();
        return;
      }
      if(Resume == 255) Resume = ModbusRtuMng_Data();
    #else
      Resume = ModbusRtuMng_Data();
    #endif
     if((Resume != 0) && (Resume != 255)){ //数据正确,发送数据
       UsartTiny_SendStart(Resume);
       ModbusRtuMng.Flag |= MODBUS_RTU_MNG_SEND_DOING;//启动发送
       ModbusRtuMng.Index = ModbusRtuMng.Count;//计算发送超时
     }
  }
  else{//发送超时完成了
    UsartTiny_Stop();
  }

  /*/====================-发送0x55测试波特率===================
  if(ModbusRtuMng.Index) ModbusRtuMng.Index--;
  if(ModbusRtuMng.Index) return;//时间未到
  ModbusRtuMng.Index = 10;
  UsartTiny.Data[0] = 0x55;;
  UsartTiny_SendStart(1); */
  //485总线A,B两端应为下述波形(8位数据位、1位停止位)
  //输出波形实测为：(占空比50%,空闲时为中间电平):
  //       起 b0:1 0   1   0   1   0   1 b7:0停
  //   ┏┓  ┏━┓  ┏━┓  ┏━┓  ┏━┓  ┃
  // ━┛┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃  ┃━━━
  //     ┗━┛  ┗━┛  ┗━┛  ┗━┛  ┗━┛
  //波特率为9600时，周期为200uS,即对应1bit为100uS
}

/******************************************************************************
		                      支持预处理时相关
******************************************************************************/ 
#ifdef SUPPORT_MODBUS_PRE

//-------------------------------挂起函数--------------------------------
//挂起后，可才可直接使用缓冲区，但必须调用ModbusRtuMng_PreInsertSend()才能解开
void ModbusRtuMng_Suspend(void)
{
  UsartTiny_Stop(); //强制中止
  ModbusRtuMng.Flag &= ~(MODBUS_RTU_MNG_RCV_DOING |MODBUS_RTU_MNG_SEND_DOING);//停止数据
  ModbusRtuMng.Flag |= MODBUS_RTU_MNG_SUSPEND;//挂起
}

//----------------------------强制插入发送函数--------------------------------
void ModbusRtuMng_InsertSend(unsigned char SendLen) //发送数据长度
{
  ModbusRtuMng.Flag &= ~MODBUS_RTU_MNG_SUSPEND;//无条件取消挂起
  if(!SendLen) return; //无数据要发送
  
  UsartTiny_Stop();//先停止
  UsartTiny_SendStart(SendLen);
  ModbusRtuMng.Flag |= MODBUS_RTU_MNG_SEND_DOING;//启动发送
  ModbusRtuMng.Index = ModbusRtuMng.Count;//计算发送超时
}

#endif //SUPPORT_MODBUS_PRE
