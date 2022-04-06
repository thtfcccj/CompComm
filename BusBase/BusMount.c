/***********************************************************************

                    Bus Id ��Ӧ�Ĺҽ�ʵ��

***********************************************************************/

#include "BusMount.h"
#include <string.h>

struct _BusMount BusMount; //ֱ�ӵ�����

/***********************************************************************
                           ��غ���
***********************************************************************/

//------------------------------������ʼ��-----------------------------
void BusMount_Init(void)
{
  memset(&BusMount, 0, sizeof(struct _BusMount));
}

//----------------------------------�ҽ�-------------------------------
//���ߴ��������
void BusMount_Mount(unsigned char BusId, struct _BusCount *pBase)
{
  if(BusId >= BUS_ID_COUNT) return;
  BusMount.pBusMount[BusId] = pBase;
}

//--------------------------------�õ��ṹ------------------------------
//����struct _BusCount���������ֱ��ת��Ϊ��ṹ
//����NULLδ�ҽӻ�֧��
struct _BusCount *BusMount_GetBase(unsigned char BusId)
{
  if(BusId >= BUS_ID_COUNT) return NULL;
  return BusMount.pBusMount[BusId];  
}

