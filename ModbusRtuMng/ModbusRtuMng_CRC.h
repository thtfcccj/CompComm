/******************************************************************
//					
//           ModbusRtu������֮CRCУ��ʵ��
//��ģ��ΪModbusRtuMng����ģ��
****************************************************************/
#ifndef __MODBUS_RTU_MNG_CRC_H
#define __MODBUS_RTU_MNG_CRC_H


//-----------------------�õ�CRC16��ѭ���߳���⣩����-------------------
//���صõ���CRC16����
unsigned short ModbusRtuMng_GetCRC16(unsigned char *pBuf,  //����֡
                                     unsigned short Len);   //����֡����


#endif


