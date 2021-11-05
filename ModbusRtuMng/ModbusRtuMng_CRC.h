/******************************************************************
//					
//           ModbusRtu管理器之CRC校验实现
//此模块为ModbusRtuMng的子模块
****************************************************************/
#ifndef __MODBUS_RTU_MNG_CRC_H
#define __MODBUS_RTU_MNG_CRC_H


//-----------------------得到CRC16（循环冗长检测）函数-------------------
//返回得到的CRC16数据
unsigned short ModbusRtuMng_GetCRC16(unsigned char *pBuf,  //数据帧
                                     unsigned short Len);   //数据帧长度


#endif


