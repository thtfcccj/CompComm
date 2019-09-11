/***********************************************************************

                  �ַ������ģ��ʵ��

***********************************************************************/

#include "Code.h"
#include "GB2312_UCS2.h"
#include <string.h>


/****************************************************************************
		                      ��ؽṹ������-SUPPORT_CODE_TASK���
****************************************************************************/
#ifdef SUPPORT_CODE_TASK
struct _Code Code;

//-----------------------------��ʼ������------------------------------------
//����ʱ����
void Code_Init(void)
{
  memset(&Code, 0, sizeof(struct _Code));
}

//----------���ÿ���ʱ�䴦��Ucs2���������GB2312������������---------------------
//MSB��ʽ(��λ��ǰ)����, �����Ƿ���ճɹ�!
signed char Code_Ucs2ToGB2312Start(const unsigned char *pUcs2,  //MSB��ʽ
                                   unsigned char *pGB2312,       //MSB��ʽ
                                   unsigned short Len,          //Unicode���ֽ�������
                                   void(*FinalNotify)(unsigned short)) //�������ͨ������
{
  if(Code.pUcs2 != NULL) return -1; //æ��
  //��ס�ֳ�
  Code.pUcs2 = pUcs2;
  Code.pGB2312 = pGB2312; 
  Code.FinalNotify = FinalNotify;
  Code.UnicodeLen = Len; 
  Code.UnicodePos = 0;   
  Code.GB2312Pos = 0; 
  return 0;
}

//-----------------------------����������------------------------------------
//����ϵͳ���н�����ɨ��
void Code_FastTask(void)
{
  unsigned char CurLen;
  do{
    CurLen = Code_Ucs2ToGB2312(Code.pUcs2 + Code.UnicodePos,
                               Code.pGB2312 + Code.GB2312Pos,
                               2);
    Code.GB2312Pos += CurLen;
    Code.UnicodePos += 2;
    if(Code.UnicodePos >= Code.UnicodeLen){//������
      Code.FinalNotify(Code.GB2312Pos);
      break;
    }
  }while(CurLen <= 1); //ASC��ʱ�ظ�ִ��
}

#endif //#ifdef SUPPORT_CODE_TASK



/****************************************************************************
		                      ��ؽṹ-��������
****************************************************************************/

//-------------------���GB2312����ת����Ucs2��������------------------------
//����ת������ַ�����
unsigned short Code_GB2312ToUcs(const char *pGB2312, //MSB��ʽ
                                unsigned char *pUcs2,         //MSB��ʽ
                                unsigned short Len)           //�ַ�����
{
  const char *pGB2312End = pGB2312 + Len;
  const unsigned char *pUcs2Start = pUcs2;  
  for( ;pGB2312 < pGB2312End; pGB2312++, pUcs2++){
    unsigned char H = *pGB2312;
    if(H < 0x80){ //ASCIIֱ��ת��
      *pUcs2++= 0x00;
      *pUcs2 = H;
      continue;
    }
    if((H >= 0xA1) || (H <= 0xF7)){//GB2312�������1��
      pGB2312++;
      unsigned char L = *pGB2312;
      if((L >= 0xA1) || (L <= 0xF7)){//��λ�жϣ�GB2312ǿ��ת��
        unsigned short Unicode = GB2312ToUcs2(((unsigned short)H << 8) + L);
        *pUcs2++= Unicode >> 8;
        *pUcs2 = Unicode & 0xFF;
        continue;
      }
    }
    //�쳣���0
    *pUcs2++= 0x00;
    *pUcs2 = 0x00;
  }
  return pUcs2 - pUcs2Start;
}

//--------------------------Ucs2���������GB2312��������---------------------
//MSB��ʽ(��λ��ǰ)����, ����ת������ַ�����
//ע��˺�����Ҫ�Ĵ���ʱ��ϳ�����
unsigned short Code_Ucs2ToGB2312(const unsigned char *pUcs2,  //MSB��ʽ
                                 char *pGB2312,       //MSB��ʽ
                                 unsigned short Len)        //Unicode���ֽ�������     
{
  const unsigned char *pUcs2End = pUcs2 + Len;
  const char *pGB2312Start = pGB2312;    
  for( ;pUcs2 < pUcs2End; pUcs2 += 2, pGB2312++){  
    unsigned short Unicode = ((unsigned short)(*pUcs2) << 8) + *(pUcs2 + 1);
    if(Unicode < 0x0080){//ASCIIֱ��ת��
      *pGB2312 = Unicode;
    }
    else{ //Unicode��ת��ΪGB2312
      Unicode = Ucs2ToGB2312(Unicode);
      *pGB2312++= Unicode >> 8;
      *pGB2312 = Unicode & 0xFF;
    }
  }
  return pGB2312 - pGB2312Start;
}






 