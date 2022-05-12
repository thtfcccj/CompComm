/*****************************************************************************
	
//                     Modbus�����ʵ��ģ��

******************************************************************************/
#include "ModbusCodec.h"
#include "ModbusRtuMng_CRC.h" //CRC16

//-----------------------�õ�CRC16��ѭ���߳���⣩����-------------------
//���صõ���CRC16����

#define ModbusCodec_GetCRC16(buf,len) do{\
   ModbusRtuMng_GetCRC16(buf,len); }while(0)



/*****************************************************************************
                          ��غ���
******************************************************************************/
    
//--------------------------��Ϊ�ӻ�ʱ����뺯��--------------------------------
//ֻ֧��RTUģʽ
//����-1��ʾ����;  0:������ȷ��������������, ����:�������ݸ���
signed short ModbusCodec_Slv(unsigned char  *pData, //�յ�������
                             unsigned short RcvLen,//�������ݳ���
                             unsigned char SlvAdr) //�ӻ���ַ
{
  //������ݷ�����
  if(RcvLen < 4) return -1;//���ȴ���
  unsigned char CurAdr =  pData[0];
  //��ַ����,255ΪP2P��ַ,��������ͨѶַ�벨���ʵ�
  if((CurAdr != 0) && (CurAdr != SlvAdr) && (CurAdr != 255))
    return (unsigned char)-1;    

  RcvLen -= 2;//ȥ��CRC��
  unsigned short CRC16 = ModbusRtuMng_GetCRC16(pData, RcvLen);
  signed char CrcAnti;
  if(CRC16 == (((unsigned short)pData[RcvLen] << 8) | pData[RcvLen + 1]))
    CrcAnti = 0;
  else if(CRC16 == (((unsigned short)pData[RcvLen + 1] << 8) | pData[RcvLen]))
    CrcAnti = 1; //����
  else return (unsigned char)-1;   //CRC����У�������

  //Ӧ�ò����(ȥ��ַ����)
  signed short Resume = ModbusCodec_cbEncoder(pData + 1, RcvLen - 1);
  if(Resume <= 0) return Resume; //���践�ػ���������
  //������ӣ�
  CRC16 = ModbusRtuMng_GetCRC16(pData, Resume);
  if(CrcAnti){//����
    pData[Resume + 1] = (unsigned char)(CRC16 >> 8);//CRC��λ
    pData[Resume] = (unsigned char)(CRC16 & 0xff);//CRC��λ
  }
  else{//û��
    pData[Resume] = (unsigned char)(CRC16 >> 8);//CRC��λ
    pData[Resume + 1] = (unsigned char)(CRC16 & 0xff);//CRC��λ
  }
  return Resume + 2; //�����跢�͵����ݸ���CRC16ռ2λ
}

