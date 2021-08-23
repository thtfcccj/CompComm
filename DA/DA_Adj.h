/******************************************************************************
                            DA�������ģ��
//��ģ�����ϵͳ��DAģ��֮ǰ��������й���:
//1.ͨ����������棬 ���Ӧ����AD������ת����
//2.����������DAֵ
//3.������������궨����

//����ת����ʽΪ:  ����ֵ * ת��ϵ��Gain + ���ֵ
//��ģ�������Ӧ����Ӳ��
******************************************************************************/
  
#ifndef __DA_ADJ_H
#define __DA_ADJ_H

#include "DA.h"
/******************************************************************************
                                 �������
******************************************************************************/

//������Ӧ����ת��ΪDAֵ��ת��ϵ����Qֵ
#ifndef DA_ADJ_GAIN_Q
  #define DA_ADJ_GAIN_Q          10
#endif

//�����Զ��˳�ʱ��,Task��������Ϊ��λ
#ifndef DA_ADJ_QUIT_OV
  #define  DA_ADJ_QUIT_OV         40
#endif

//����Ĭ�����Ĭ��ֵ
#ifndef DA_ADJ_ZERO_DEFAULT
  #define DA_ADJ_ZERO_DEFAULT	  0  //Ĭ��DAһһ��Ӧ
#endif

//����Ĭ��ת��ϵ��ֵ,���������,DA_ADJ_GAIN_Qֵ
#ifndef DA_ADJ_GAIN_DEFAULT
  #define DA_ADJ_GAIN_DEFAULT	  (1 << DA_ADJ_GAIN_Q)  //Ĭ��DAһһ��Ӧ
#endif

//�����û���ֵ����ֵ
#ifndef DA_ADJ_USER_FULL
  #define DA_ADJ_USER_FULL      DA_FULL //Ĭ��DA��Ӧ
#endif

//���������ʱ��DAֵ(��4mA��Ӧ��ֵ)
#ifndef DA_ADJ_OUT_ZERO
  #define DA_ADJ_OUT_ZERO      0 //Ĭ����С
#endif

//�������������ʱ��DAֵ(��20mA��Ӧ��ֵ)
#ifndef DA_ADJ_OUT_FULL
  #define DA_ADJ_OUT_FULL      4000
#endif

//���������ʱ��DAֵ
#ifndef DA_ADJ_OUT_MAX
  #define DA_ADJ_OUT_MAX      DA_FULL //Ĭ�����
#endif

/******************************************************************************
                                ��ؽṹ
******************************************************************************/
struct _DA_AdjInfo{
  unsigned short Zero;    //���DAֵ
  unsigned short Gain;    //ת��ΪDAֵ��ת��ϵ��,Qֵ
};

struct _DA_Adj{
  struct _DA_AdjInfo Info;
  unsigned char Timer;  //��ʱ��,ֱ�������λ��ʾ��Уģʽ
};

extern struct _DA_Adj DA_Adj;  //ֱ��ʵ����

/******************************************************************************
                                 ��غ���
******************************************************************************/

//-------------------------------��ʼ������------------------------------
void DA_Adj_Init(signed char Inited);

//-------------------------------������------------------------------
//����512mS������
void DA_Adj_Task(void);

//----------------------------����Ӧ��������------------------------------
void DA_Adj_UpdateVol(unsigned short Vol);

//----------------------------����Ӧ��Ϊ��������--------------------------
//���0������ֵ��VolԽ���0��Խ��
void DA_Adj_UpdateNegaVol(unsigned short Vol);

//----------------------------����Ϊ���������------------------------------
void DA_Adj_UpdateZero(void);

//----------------------------����Ϊ���ֵ�������------------------------------
void DA_Adj_UpdateMax(void);

//------------------------------������غ���--------------------------------
//��������ģʽ
void DA_Adj_SetZeroMode(void);
//void DA_Adj_ClrZeroMode(void);
#define DA_Adj_ClrZeroMode() do{DA_Adj.Timer = 0;}while(0)
//������DAֵ
//unsigned short DA_Adj_GetZero(void);
#define DA_Adj_GetZero() (DA_Adj.Info.Zero)
//�����DAֵ(������EERPOM)
void DA_Adj_SetZero(unsigned short Zero);
//���浱ǰ���DAֵ
void DA_Adj_SaveZero(void);

//------------------------------�궨��غ���--------------------------------
//���� ģʽ
void DA_Adj_SetGainMode(void);
//void DA_Adj_ClrGainMode(void);
#define DA_Adj_ClrGainMode() do{DA_Adj.Timer = 0;}while(0)
//���Gainֵ
//unsigned short DA_Adj_GetGain(void);
#define DA_Adj_GetGain()   (DA_Adj.Info.Gain)
//��Gainֵ(������EERPOM)
void DA_Adj_SetGain(unsigned short Gain);
//���浱ǰGainֵ
void DA_Adj_SaveGain(void);


#endif //#define __DA_ADJ_H












