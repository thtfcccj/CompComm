/***********************************************************************

//		               LRCУ����򼯺�

***********************************************************************/


//----------------------�õ�ʮ��λLRCУ����----------------------------
//���û��ȡ��
unsigned short LRC16_Get(const unsigned char *pData, 
                         unsigned short Len)
{
  unsigned short LRC16 = 0;
  for(; Len > 0; Len--, pData++){  
    LRC16 += *pData;
  }  
  return LRC16;
}
