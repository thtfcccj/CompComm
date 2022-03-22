/*******************************************************************************

                        ���߻���ṹ
�˽ṹ���������߹���ʱ����Ϊ�����ʹ��
*******************************************************************************/
#ifndef __BUS_BASE_H
#define	__BUS_BASE_H

#include "BusCount.h"

//----------------------------���ṹ-------------------------------
//������ṹ�� ����ֱ�ӳ�Ա����
struct _BusBase{
  struct _BusCount Count;         //���߼������ͻ����Է����ⲿʹ��
  unsigned char Id;               //Ϊ�����߷����ID��
  unsigned char ExU8;             //�����������ֵ
  union{
    unsigned char U8[2];          //�����������ֵ
    unsigned short U16;           //�����������ֵ
    signed short S16;             //�����������ֵ           
  }Ex;
};

//------------------------------��ʼ������---------------------------
//������ʹ�õı�����ȫ����ʼ��Ϊ0
#include <string.h>
#define BusBase_Init(bus, id) do{\
  memset(bus, 0, sizeof(struct _BusBase)); (bus)->Id = id; }while(0)

//--------------------------������ID�����ṹ--------------------------------
#define BusBase_cbGet(busId) ((struct _BusBase*)BusCount_pcbGet(busId))
    
#endif




