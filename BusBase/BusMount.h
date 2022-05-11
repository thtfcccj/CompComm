/***********************************************************************

                    Bus Id ��Ӧ�Ĺҽ�
�˽ӿ���ʵ�ֶ�����Ӧ��
***********************************************************************/
#ifndef __BUS_MOUNT_H
#define	__BUS_MOUNT_H

/***********************************************************************
                        �������
***********************************************************************/

//#define SUPPORT_BUS_MOUNT_TYPE      //֧���������ͱ�ʶʱ����

/***********************************************************************
                        ��ؽṹ
***********************************************************************/
#include "BusIdDef.h" //����ID�����Ͷ���(Ӧ�����)
#include "BusCount.h" //����

//----------------------------���ṹ-----------------------------------
struct _BusMount{
  #ifdef SUPPORT_BUS_MOUNT_TYPE      //֧�����ͱ�ʶʱ
    //��ǰ�ҽӵ���������
    unsigned char Type[BUS_ID_COUNT];    
  #endif
  //��BusId��Ӧ������ָ��, ������ṹ����������struct _BusCount
  struct _BusCount *pBusMount[BUS_ID_COUNT];
};

extern struct _BusMount BusMount; //ֱ�ӵ�����

/***********************************************************************
                           ��غ���
***********************************************************************/

//------------------------------������ʼ��-----------------------------
void BusMount_Init(void); 

//----------------------------------�ҽ�-------------------------------
//���ߴ��������
#ifdef SUPPORT_BUS_MOUNT_TYPE      //֧�����ͱ�ʶʱ
  void BusMount_Mount(unsigned char BusId, 
                      unsigned char Type,   //�ҽ����ߵ�����
                      struct _BusCount *pBase); 
#else
  void BusMount_Mount(unsigned char BusId, 
                      struct _BusCount *pBase);   
#endif

//----------------------------�õ��ҽӵ���������-------------------------
#define BusMount_GetType(busId)   (BusMount.Type[busId])

//--------------------------------�õ��ṹ------------------------------
//����struct _BusCount���������ֱ��ת��Ϊ��ṹ
//����NULLδ�ҽӻ�֧��
struct _BusCount *BusMount_GetBase(unsigned char BusId); 
//ֱ�Ӳ���ʱ
#define BusMount_pGetBase(busId)   (BusMount.pBusMount[busId])


#endif //#define	__BUS_MOUNT_H

