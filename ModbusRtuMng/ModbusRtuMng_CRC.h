/******************************************************************
//					
//           ModbusRtu������֮CRCУ��ʵ��
//��ģ��ΪModbusRtuMng����ģ��
****************************************************************/
#ifndef __MODBUS_RTU_MNG_CRC_H
#define __MODBUS_RTU_MNG_CRC_H

#ifdef SUPPORT_EX_PREINCLUDE//��֧��Preinlude�r
  #include "Preinclude.h"
#endif


//ȫ��������ã�ModbusCRC16У���λʹ��ѹ����ʽ(�ռ任ʱ����204Byte)
//#define MODBUS_CRC16_H_ZIP

//-----------------------�õ�CRC16��ѭ���߳���⣩����-------------------
//���صõ���CRC16����
unsigned short ModbusRtuMng_GetCRC16(unsigned char *pBuf,  //����֡
                                     unsigned short Len);   //����֡����


#endif


