/*******************************************************************************

                      ���߼������ṹ
//�����ػ�
*******************************************************************************/
#ifndef __BUS_COUNT_H
#define	__BUS_COUNT_H

//----------------------------------���ṹ----------------------------------
//������ṹ�� ����ֱ�ӳ�Ա����
struct _BusCount{
  unsigned long Comm;          //ͨѶ��������   
  unsigned long Valid;         //��Ч���ݼ���
  unsigned long Invalid;       //��Ч���ݼ���(����У������ݴ���  
};

//------------------------------��ʼ������-----------------------------------
#include <string.h>
#define BusCount_Init(p) do{memset(p, 0, sizeof(struct _BusCount));} while(0)

//--------------------------������ID�����ṹ--------------------------------
//��Ϊ�ص�������Ӧ��ʵ�֣���ͨ���˺���ת���Ի��������
struct _BusCount *BusCount_pcbGet(unsigned char BusId);



#endif



