/*******************************************************************************

//		ModbusRtu������ʵ��

*******************************************************************************/

#include "ModbusRtuMng.h"
#include <string.h>

#include "Eeprom.h"
#include "InfoBase.h"

struct _ModbusRtuMng ModbusRtuMng;

/***********************************************************************
                       �ڲ���������
***********************************************************************/
//CRCУ��
unsigned short ModbusRtuMng_GetCRC16(unsigned char *pBuf,  //����֡
                                     unsigned short Len);   //����֡����
//����������
#define _GetBuadCfg() ((ModbusRtuMng.Info.CommCfg >> 4) & 0x07)                       

/***********************************************************************
                   ��غ���ʵ��
***********************************************************************/

//��������RTC��ʱʱ��,msΪ��λ
static const unsigned char _Baud2Ov[] = {
    3,  //9600,//   0  //Ĭ��
    6,  //4800,//   1
    12, //2400,//   2
    24, //1200,//   3
    3,  //19200,//  4
    3,  //38400,//  5
    3,  //57600,//  6
    3,  //115200,// 7
};

#ifdef SUPPORT_MODBUS_BUND_ADR67 //֧�ֵ�ַ����λ��Ϊ������ʱ
static const unsigned char _Adr67ToBaud[] = {
  0,  //ȫ����Ϊ9600
  2,  //��λ��Ϊ2400
  1,  //��λ��Ϊ4800
  4,  //ȫ��Ϊ19200
};
#endif

//------------------------��ʼ������----------------------------
void ModbusRtuMng_Init(unsigned char Inited)
{
  memset(&ModbusRtuMng, 0, sizeof(struct _ModbusRtuMng));
  if(!Inited){
    //д��Eeprom,AdrĬ��Ϊ0,ֻ����254��ַͨѶ
    Eeprom_Wr(ModbusRtuMng_GetInfoBase(),
              &ModbusRtuMng.Info,
              sizeof(struct _ModbusRtuMngInfo));
  }
  else{
    //����
    Eeprom_Rd(ModbusRtuMng_GetInfoBase(),
               &ModbusRtuMng.Info,
               sizeof(struct _ModbusRtuMngInfo));
    #ifdef SUPPORT_MODBUS_BUND_ADR67 //֧�ֵ�ַ����λ��Ϊ������ʱ
      ModbusRtuMng.Info.CommCfg = 0; //ǿ��Ĭ��(9600 + ��У��8����λ1ֹͣλ)
    #endif
  }
  ModbusRtuMng.Count = _Baud2Ov[_GetBuadCfg()];
  UsartTiny_CfgHw(ModbusRtuMng.Info.CommCfg);//���õײ�Ӳ��
  UsartTiny_Init();//��ʼ���ײ�ͨѶ
}

//--------------------------ͨѶ��ַ���ú���-----------------------------
void ModbusRtuMng_SetAdr(unsigned char Adr)
{
  #ifdef SUPPORT_MODBUS_BUND_ADR67 //֧�ֵ�ַ����λ��Ϊ������ʱ,ǿ����У��8����λ
     ModbusRtuMng_SetCommCfg(_Adr67ToBaud[(Adr >> 2) & 0x30]); 
     Adr &= ~0xC0;  //ȥ������λ  
  #endif 

  ModbusRtuMng.Info.Adr = Adr;
  #ifdef SUPPORT_MODBUS_SW_ADR
    Eeprom_Wr(ModbusRtuMng_GetInfoBase() + 
                struct_offset(struct _ModbusRtuMngInfo, Adr),
              &ModbusRtuMng.Info.Adr, 1);
  #endif
}

//--------------------------ͨѶ�������ú���-----------------------------
void ModbusRtuMng_SetCommCfg(unsigned char CommCfg)
{
  ModbusRtuMng.Info.CommCfg = CommCfg;
  ModbusRtuMng.Count = _Baud2Ov[_GetBuadCfg()];
  Eeprom_Wr(ModbusRtuMng_GetInfoBase() + 
            struct_offset(struct _ModbusRtuMngInfo, CommCfg),
            &ModbusRtuMng.Info.CommCfg, 1);
  UsartTiny_CfgHw(ModbusRtuMng.Info.CommCfg);//�����õײ�Ӳ��
  UsartTiny_Init();//��ʼ���ײ�
}

//--------------------------���ݴ�����-------------------------------
//���������ݺ���ô˺���
//�˺���������UsartTiny.Data�������������,��������������
//����255��ʾ����;  0:������ȷ�������������� 
//����:�������ݸ���
unsigned char ModbusRtuMng_Data(void)
{
  //�ȼ���ַ�Ƿ���ȷ
  unsigned char CurAdr = UsartTiny.Data[0];
  //��ַ����,254ΪĬ�ϵ�ַ,��������ͨѶַ�벨���ʵ�
  if((CurAdr != 0) && (CurAdr != ModbusRtuMng.Info.Adr)&& (CurAdr != 254))
    return -1;
    
  //��鳤���Ƿ����,��ַ1+������1+����[0~n]+У����2 >=4
  if(UsartTiny.Index < 4) return -1; //��������
  
  //CRCУ��
  unsigned char Len = UsartTiny.Index - 2;//CRC����λ
  unsigned short CRC;
  CRC = (unsigned short)(UsartTiny.Data[Len] << 8) | 
         (unsigned short)(UsartTiny.Data[Len + 1]); 
  if(CRC != ModbusRtuMng_GetCRC16(UsartTiny.Data,Len)) 
    return (unsigned char)-3;   //CRC����У�������
      
  //������ȷ,����Ӧ�ò㴦��,������UsartTiny.Data��,0Ϊ��ַ,LenΪ����
  Len = ModbusRtuMng_cbDataNotify(UsartTiny.Data, Len);
  #ifdef SUPPORT_MODBUS_DATA_REMAIN//����ʶ�������ʱ����
  if(Len == 255)
     Len = ModbusRtuMng_cbDataRemain(UsartTiny.Data, Len);
  #endif
  if((Len == 0) || (Len == 255)) return Len;//������Ҫ����
  if(CurAdr == 0) return 0;//�㲥��ַ�����ݷ���
  //�������跢����
  //��CRCУ����
  CRC = ModbusRtuMng_GetCRC16(UsartTiny.Data, Len);
  UsartTiny.Data[Len] = (unsigned char)(CRC >> 8);//CRC��λ
  UsartTiny.Data[Len + 1] = (unsigned char)(CRC & 0xff);//CRC��λ
  return Len + 2; //�����跢�͵����ݸ���CRC16ռ2λ
}
//-------------------------------��ͨ��ѯ����----------------------------
//���˺�������ϵͳ1ms������
void ModbusRtuMng_Task(void)
{
  #ifdef SUPPORT_MODBUS_PRE //����ʱ��ͣ����
    if(ModbusRtuMng.Flag & MODBUS_RTU_MNG_SUSPEND) return;
  #endif  
  
  //����״̬��������
  if(UsartTiny.eState == UsartTiny_eState_Idie){
    UsartTiny_RcvStart();
    //ֹͣ�����շ�����
    ModbusRtuMng.Flag &= ~(MODBUS_RTU_MNG_RCV_DOING | MODBUS_RTU_MNG_SEND_DOING);
    return;
  }
  //û�ڷ�������չ�����,//��ִ��
  if(!(ModbusRtuMng.Flag & (MODBUS_RTU_MNG_RCV_DOING | 
                          MODBUS_RTU_MNG_SEND_DOING))) return;
  //��������չ��̼�ʱ
  if(ModbusRtuMng.Index) ModbusRtuMng.Index--;
  if(ModbusRtuMng.Index) return;//ʱ��δ��
  
  //���ݽ������
  if(ModbusRtuMng.Flag & MODBUS_RTU_MNG_RCV_DOING){
    UsartTiny_Stop(); //ǿ����ֹ����
    ModbusRtuMng.Flag &= ~MODBUS_RTU_MNG_RCV_DOING;//ֹͣ���ݽ���
     unsigned char Resume;
    #ifdef SUPPORT_MODBUS_PRE 
      Resume = ModbusRtuMng_cbRcvPreDataPro();
      if(Resume == 254){//��ͣ����
        UsartTiny_Stop();
        return;
      }
      if(Resume == 255) Resume = ModbusRtuMng_Data();
    #else
      Resume = ModbusRtuMng_Data();
    #endif
     if((Resume != 0) && (Resume != 255)){ //������ȷ,��������
       UsartTiny_SendStart(Resume);
       ModbusRtuMng.Flag |= MODBUS_RTU_MNG_SEND_DOING;//��������
       ModbusRtuMng.Index = ModbusRtuMng.Count;//���㷢�ͳ�ʱ
     }
  }
  else{//���ͳ�ʱ�����
    UsartTiny_Stop();
  }

  /*/====================-����0x55���Բ�����===================
  if(ModbusRtuMng.Index) ModbusRtuMng.Index--;
  if(ModbusRtuMng.Index) return;//ʱ��δ��
  ModbusRtuMng.Index = 10;
  UsartTiny.Data[0] = 0x55;;
  UsartTiny_SendStart(1); */
  //485����A,B����ӦΪ��������(8λ����λ��1λֹͣλ)
  //�������ʵ��Ϊ��(ռ�ձ�50%,����ʱΪ�м��ƽ):
  //       �� b0:1 0   1   0   1   0   1 b7:0ͣ
  //   ����  ������  ������  ������  ������  ��
  // ������  ��  ��  ��  ��  ��  ��  ��  ��  ��������
  //     ������  ������  ������  ������  ������
  //������Ϊ9600ʱ������Ϊ200uS,����Ӧ1bitΪ100uS
}

/******************************************************************************
		                      ֧��Ԥ����ʱ���
******************************************************************************/ 
#ifdef SUPPORT_MODBUS_PRE

//-------------------------------������--------------------------------
//����󣬿ɲſ�ֱ��ʹ�û����������������ModbusRtuMng_PreInsertSend()���ܽ⿪
void ModbusRtuMng_Suspend(void)
{
  UsartTiny_Stop(); //ǿ����ֹ
  ModbusRtuMng.Flag &= ~(MODBUS_RTU_MNG_RCV_DOING |MODBUS_RTU_MNG_SEND_DOING);//ֹͣ����
  ModbusRtuMng.Flag |= MODBUS_RTU_MNG_SUSPEND;//����
}

//----------------------------ǿ�Ʋ��뷢�ͺ���--------------------------------
void ModbusRtuMng_InsertSend(unsigned char SendLen) //�������ݳ���
{
  ModbusRtuMng.Flag &= ~MODBUS_RTU_MNG_SUSPEND;//������ȡ������
  if(!SendLen) return; //������Ҫ����
  
  UsartTiny_Stop();//��ֹͣ
  UsartTiny_SendStart(SendLen);
  ModbusRtuMng.Flag |= MODBUS_RTU_MNG_SEND_DOING;//��������
  ModbusRtuMng.Index = ModbusRtuMng.Count;//���㷢�ͳ�ʱ
}

#endif //SUPPORT_MODBUS_PRE
