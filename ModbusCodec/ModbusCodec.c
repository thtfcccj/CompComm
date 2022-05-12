/*****************************************************************************
	
//                     Modbus编解码实现模块

******************************************************************************/
#include "ModbusCodec.h"
#include "ModbusRtuMng_CRC.h" //CRC16

//-----------------------得到CRC16（循环冗长检测）函数-------------------
//返回得到的CRC16数据

#define ModbusCodec_GetCRC16(buf,len) do{\
   ModbusRtuMng_GetCRC16(buf,len); }while(0)



/*****************************************************************************
                          相关函数
******************************************************************************/
    
//--------------------------作为从机时编解码函数--------------------------------
//只支持RTU模式
//返回-1表示错误;  0:数据正确，但不返回数据, 其它:返回数据个数
signed short ModbusCodec_Slv(unsigned char  *pData, //收到的数据
                             unsigned short RcvLen,//接收数据长度
                             unsigned char SlvAdr) //从机地址
{
  //检查数据符合性
  if(RcvLen < 4) return -1;//长度错误
  unsigned char CurAdr =  pData[0];
  //地址错误,255为P2P地址,用于重设通讯址与波特率等
  if((CurAdr != 0) && (CurAdr != SlvAdr) && (CurAdr != 255))
    return (unsigned char)-1;    

  RcvLen -= 2;//去除CRC域
  unsigned short CRC16 = ModbusRtuMng_GetCRC16(pData, RcvLen);
  signed char CrcAnti;
  if(CRC16 == (((unsigned short)pData[RcvLen] << 8) | pData[RcvLen + 1]))
    CrcAnti = 0;
  else if(CRC16 == (((unsigned short)pData[RcvLen + 1] << 8) | pData[RcvLen]))
    CrcAnti = 1; //反了
  else return (unsigned char)-1;   //CRC数据校验码错误

  //应用层编码(去地址传入)
  signed short Resume = ModbusCodec_cbEncoder(pData + 1, RcvLen - 1);
  if(Resume <= 0) return Resume; //无需返回或数据有误
  //结果附加：
  CRC16 = ModbusRtuMng_GetCRC16(pData, Resume);
  if(CrcAnti){//反了
    pData[Resume + 1] = (unsigned char)(CRC16 >> 8);//CRC高位
    pData[Resume] = (unsigned char)(CRC16 & 0xff);//CRC低位
  }
  else{//没反
    pData[Resume] = (unsigned char)(CRC16 >> 8);//CRC高位
    pData[Resume + 1] = (unsigned char)(CRC16 & 0xff);//CRC低位
  }
  return Resume + 2; //返回需发送的数据个数CRC16占2位
}

