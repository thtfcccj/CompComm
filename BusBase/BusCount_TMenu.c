/*******************************************************************************

                     ComCount�ṹ�˵�ʵ��
�˲˵�����Ϊֻ��
*******************************************************************************/

#include "BusCount_TMenu.h"

#include "TMenu_EditSel.h"
#include "stringEx.h"
#include <string.h>
#include "TMenuBuf.h"

//--------------------------------�ڲ��ַ���Դ--------------------------------- 
#ifdef TM_EN_MUTI_LAN            //����ѡ���Ͷ������ʱ
  //�ݲ�֧�ֳ���ѡ�����ַ���
#elif defined(TM_RESOURCE_LAN)  //����Դ�ļ���ȡ�������ʱ
  //�˵�
  #include "lsAry.h"
#else  //��һ�����ַ���ʱ
  //�˵�
  static const char ls_PacketCount[]   = {"ͨѶ����"};
  static const char ls_PacketValid[]   = {"��Ч����"};
  static const char ls_PacketInvalid[] = {"��Ч����"};
  static const char ls_PacketQuality[] = {"����ͳ��"};
#endif

const LanCode_t* const lsAry_Packet[] = {
  ls_PacketCount, ls_PacketValid, ls_PacketInvalid};
  
//----------------------------�ص�����ʵ��--------------------------------------
//ö���͵�����ʽ
static void _Notify(unsigned char Type, void *pv)
{
  struct _EditSelUser *pUser = (struct _EditSelUser *)pv;
  switch(Type){
  //�õ��Ӳ˵�ͷ
  case TM_NOTIFY_GET_SUB_HEADER:
    strcpy((char*)pv, pLanCodeToChar(lsAry_Packet[*((unsigned char*)pv)]));
    break;
  case TM_NOTIFY_GET_DATA:{ //����ǰֵװ��
    unsigned long Data;
    const struct _BusCount *pBus = BusCountTMenu_pcbGet(); 
    if(pUser->CurItem == 0) Data = pBus->Comm;
    else if(pUser->CurItem == 1) Data = pBus->Valid;  
    else  Data = pBus->Invalid; 
    pUser->pData = Value4StringMin(Data, TMenuBuf.Str, 8);
    break;
  }
  default:break;
  }
}

//---------------------------------�˵��ṹ----------------------------------
const TMenu_t BusCount_TMenu = {//�˵��ṹ
  TMTYPE_EDITSEL,          //�˵�����Ϊ�༭ѡ��ģʽģʽ���û�����־,�����ұ�
  3,                       //�ɹ�ѡ��ĵ��������
  ls_PacketQuality,        //�˵�ͷ,ΪNULLʱ�ӻص����ȡ
  BUS_COUNT_TMENU_PARENT,  //���ѵĸ��˵�
  NULL,                    //������ѵ��Ӳ˵���������ͷ,ֻ��ʱ��Ϊstruct _TMenuSelRO
  _Notify,                 //���û��ռ佻����ͨ������
};


