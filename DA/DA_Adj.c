/******************************************************************************

                            DA�������ģ��ʵ��

******************************************************************************/

#include "DA_Adj.h"
#include <string.h>

#include "InfoBase.h"
#include "Eeprom.h"

struct _DA_Adj DA_Adj;  //ֱ��ʵ����

/******************************************************************************
                               ��غ���
******************************************************************************/

//-------------------------------��ʼ������------------------------------
void DA_Adj_Init(signed char Inited)
{
  memset(&DA_Adj, 0,  sizeof(struct _DA_Adj));
  if(!Inited){
    DA_Adj.Info.Zero = DA_ADJ_ZERO_DEFAULT;
    DA_Adj.Info.Gain = DA_ADJ_GAIN_DEFAULT;
    Eeprom_Wr(DA_Adj_GetInfoBase(),
              &DA_Adj.Info,
              sizeof(struct _DA_AdjInfo));
  }
  else{
    Eeprom_Rd(DA_Adj_GetInfoBase(),
              &DA_Adj.Info,
              sizeof(struct _DA_AdjInfo));  
  }
  DA_Init();//DA��ʼ��
  DA_Adj_UpdateVol(0);//����������ֵ
}

//-------------------------------������------------------------------
//����512mS������
void DA_Adj_Task(void)
{
  if(!DA_Adj.Timer) return;
  DA_Adj.Timer--;
  if(DA_Adj.Timer == 0x80) DA_Adj.Timer = 0;
}

//-------------------------��Ũ��ֵ����DAֵ����------------------------------
//���ض�Ӧ��DAֵ
static void _UpdateVol(unsigned short Vol)
{
  unsigned long Data = (unsigned long)Vol;
  Data *= DA_Adj.Info.Gain;
  Data >>= DA_ADJ_GAIN_Q;
  Data += DA_Adj.Info.Zero;
  if(Data >= 65535) DA_SetDA(65535);
  else DA_SetDA(Data);
}

//----------------------------����Ũ��ֵ����------------------------------
void DA_Adj_UpdateVol(unsigned short Vol)
{
  if(DA_Adj.Timer) return; //�궨�в��ܸ���
  _UpdateVol(Vol);
}

//----------------------------����Ӧ��Ϊ��������--------------------------
//���0������ֵ��VolԽ���0��Խ��
void DA_Adj_UpdateNegaVol(unsigned short Vol)
{
  if(DA_Adj.Timer) return; //�궨�в��ܸ���  
  unsigned long Data = (unsigned long)Vol;
  Data *= DA_Adj.Info.Gain;
  Vol = Data >> DA_ADJ_GAIN_Q;
  
  if(DA_Adj.Info.Zero > Vol) DA_SetDA(Vol);
  else DA_SetDA(0);//�쳣
}

//----------------------------����Ϊ���������------------------------------
void DA_Adj_UpdateZero(void)
{
  if(DA_Adj.Timer) return; //�궨�в��ܸ���
  DA_SetDA(DA_ADJ_OUT_ZERO);  
}

//----------------------------����Ϊ���ֵ�������------------------------------
void DA_Adj_UpdateMax(void)
{
  DA_SetDA(DA_ADJ_OUT_MAX);
}

//------------------------------������غ���--------------------------------
//��������ģʽ
void DA_Adj_SetZeroMode(void)
{
  DA_Adj.Timer = DA_ADJ_QUIT_OV;
  DA_SetDA(DA_Adj.Info.Zero);
}

//�����DAֵ(������EERPOM)
void DA_Adj_SetZero(unsigned short Zero)
{
  DA_Adj.Timer = DA_ADJ_QUIT_OV;
  DA_Adj.Info.Zero = Zero;
  DA_SetDA(Zero);
}
//���浱ǰ���DAֵ
void DA_Adj_SaveZero(void)
{
	Eeprom_Wr(DA_Adj_GetInfoBase() + struct_offset(struct _DA_AdjInfo,Zero),
			     &DA_Adj.Info.Zero,
           2);
  DA_Adj.Timer = 0;
}

//------------------------------�궨��غ���--------------------------------
//���� ģʽ
void DA_Adj_SetGainMode(void)
{
  DA_Adj.Timer = 0x80 | DA_ADJ_QUIT_OV;
  _UpdateVol(DA_ADJ_OUT_FULL);
}
//��Gainֵ(������EERPOM)
void DA_Adj_SetGain(unsigned short Gain)
{
  DA_Adj.Timer = 0x80 | DA_ADJ_QUIT_OV;
  DA_Adj.Info.Gain = Gain;
  _UpdateVol(DA_ADJ_OUT_FULL);
}
//���浱ǰGainֵ
void DA_Adj_SaveGain(void)
{
  Eeprom_Wr(DA_Adj_GetInfoBase() + struct_offset(struct _DA_AdjInfo,Gain),
			   &DA_Adj.Info.Gain,
               2); 
  DA_Adj.Timer = 0;
}





