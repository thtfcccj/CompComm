/***********************************************************************

//		               LRCУ����򼯺�

***********************************************************************/
#ifndef _LRC_H
#define _LRC_H



//----------------------�õ�ʮ��λLRCУ����----------------------------
//���û��ȡ��
unsigned short LRC16_Get(const unsigned char *pData, 
                         unsigned short Len);

//----------------------�õ�8λLRCУ����----------------------------
//���û��ȡ��
unsigned char LRC8_Get(const unsigned char *pData, 
                       unsigned short Len);

#endif


