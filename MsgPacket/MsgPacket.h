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
  unsigned char  Fun;          //功能码，具体定义
  unsigned short Id;           //阵列ID号
};

//功能码最高两位定义为功能类型:
#define MSG_PACKET_RD_RW  0x00  //读读写类数据,如读EEPROM
#define MSG_PACKET_RD_RO  0x40  //读只读类数据,如读状态信息
#define MSG_PACKET_WR_RW  0x80  //写读写类数据,如写EEPROM
#define MSG_PACKET_WR_WO  0xC0  //写只写类数据,如指令


#endif //#define	__MSG_PACKET_H

