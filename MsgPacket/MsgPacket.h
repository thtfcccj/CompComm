/***********************************************************************

                        消息包定义
此结构主要用于系统进程或网络交换数据用
***********************************************************************/
#ifndef __MSG_PACKET_H
#define	__MSG_PACKET_H

/***********************************************************************
		                          消息结构定义
***********************************************************************/

//此包含义为：针对数据对像(Object)第Id个，进行某个功能(Fun)的读,写等操作
struct _MsgPacket{
  unsigned char  Object;       //由用户定义的数据对像
  unsigned char  TypeFun;      //类型与功能码，具体定义
  unsigned short Id;           //阵列ID号
};

//功能码由应用层决定，高两位定义为读写及类型:
#define MSG_PACKET_RD_RW  0x00  //读读写类数据,如读EEPROM
#define MSG_PACKET_RD_RO  0x40  //读只读类数据,如读状态信息
#define MSG_PACKET_WR_RW  0x80  //写读写类数据,如写EEPROM
#define MSG_PACKET_WR_WO  0xC0  //写只写类数据,如指令
#define MSG_PACKET_TYPE   0xC0 

/*******************************************************************************
		                         相关函数
*******************************************************************************/

//--------------------------------对像主体------------------------------------- 
#define MsgPacket_GetObject(msgPacket) ((msgPacket)->Object)

//------------------------------功读写及类型---------------------------------
#define MsgPacket_GetType(msgPacket) ((msgPacket)->TypeFun & MSG_PACKET_TYPE)
#define MsgPacket_IsRdRw(msgPacket) \
                       (MsgPacket_GetType(msgPacket) == MSG_PACKET_RD_RW)
#define MsgPacket_IsRdRo(msgPacket) \
                       (MsgPacket_GetType(msgPacket) == MSG_PACKET_RD_RO)
#define MsgPacket_IsWrRw(msgPacket) \
                       (MsgPacket_GetType(msgPacket) == MSG_PACKET_WR_RW)                         
#define MsgPacket_IsWrWo(msgPacket) \
                       (MsgPacket_GetType(msgPacket) == MSG_PACKET_WR_WO) 
//方向判断
#define MsgPacket_IsWr(msgPacket) ((msgPacket)->TypeFun &  0x80)                         
#define MsgPacket_IsRd(msgPacket) (!MsgPacket_IsWr(msgPacket))
                         
//----------------------------------功能码------------------------------------- 
#define MsgPacket_GetFun(msgPacket) ((msgPacket)->TypeFun & 0x3F)
                         
//----------------------------------ID------------------------------------- 
#define MsgPacket_GetId(msgPacket) ((msgPacket)->Id)                         


#endif //#define	__MSG_PACKET_H

