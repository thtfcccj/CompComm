/*******************************************************************************

                                   DA���ģ��ӿ�
//
*******************************************************************************/
#ifndef __DA_H
#define __DA_H

/*******************************************************************************             
                             �������
*******************************************************************************/

#define SUPPORT_DA      //��ģ���ⲿ���ý���ɱ�����


#ifndef DA_FULL     //������ֵ
  #define DA_FULL   65535   //Ĭ�����ֵ
#endif

/*******************************************************************************             
                             ��ؽṹ
*******************************************************************************/

extern unsigned short DA_Vol;  //����ĵ�ǰDAֵ,��Χ0~DA_FULL

/*******************************************************************************             
                             ��غ���
*******************************************************************************/

//-----------------------------��ʼ������--------------------------------
//ע:���ô�ģ���Ӧ��������DA_SetDA()ʵ��������
void DA_Init(void);

//----------------------------�����DAֵ-----------------------------------
void DA_SetDA(unsigned short DA);

//----------------------------�õ����DAֵ-----------------------------------
#define DA_GetDA() (DA_Vol)

#endif