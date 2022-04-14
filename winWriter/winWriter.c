/*******************************************************************************

//		              ��������д���ݽṹ ���ʵ��

*******************************************************************************/

#include "winWriter.h"
#include <string.h>


/*******************************************************************************
                             ��غ���ʵ��
*******************************************************************************/

//--------------------------����������괦����---------------------------------
//������������ʱ�����ô˺����ͳ����ݣ�����0�������,�����쳣
signed char winWriter_OutData(winWriter_t *out)
{
  if(out->LaterPro == NULL) return -1;//�쳣
  brsize_t used = out->LaterPro(out); //�����û�����
  if((used > out->start) || (used > out->capability))
    return  -2;//�����쳣 
  out->start -= used; //δ�����������
  memcpy(out->data, out->data + used, out->start);//��������������
  return 0;
}

//-------------------------------copy����--------------------------------
//��copy���ǵ�copy��start�����������������ˣ���Ҫ�ͳ���������
signed char winWriter_Copy(winWriter_t *out,
                           const unsigned char *pData,//����
                           brsize_t DataLen,    //���ݳ���
                           brsize_t LeavedSize)//�����µ����ݿռ�
{
  signed char error = 0;
  while(DataLen){
    brsize_t start = out->start;
    brsize_t CurLen = out->capability - start;
    if(DataLen <= CurLen) CurLen = DataLen;
    memcpy(out->data + start, pData, CurLen);
    start += CurLen;    
    out->start = start;    
    pData += CurLen;
    DataLen -= CurLen; 

    //��������
    if((out->capability - start) < LeavedSize){
      error = winWriter_OutData(out);
      if(error) break;
    }
  };
  return error;
}

//--------------------------�ӵ�ǰλ����ǰcopy����------------------------------
//��copy���ڴ����������,copy��������ǰstartλ��,����0�������,�����쳣
//��copy���ǵ�copy��start�����������������ˣ���Ҫ�ͳ���������
signed char winWriter_CopyBackward(winWriter_t *out,
                                    brsize_t backward,//��ǰλ��(�ɵ�ǰstart��ʼ)
                                    brsize_t distance,//copy����
                                    brsize_t LeavedSize)//�����µ����ݿռ�
{
  signed char error = 0;
  while(distance){
    brsize_t start = out->start;
    brsize_t CurLen = out->capability - start;
    if(distance <= CurLen) CurLen = distance;
    memcpy(out->data + start, out->data + backward, CurLen);
    start += CurLen;  
    backward += CurLen;
    distance -= CurLen; 
    //��������
    if((out->capability - start) < LeavedSize){
      error = winWriter_OutData(out);
      if(error) break;
      //�������ݱ������ˣ���ǰ����Ҫ����
      backward =- (start - out->start);
    }
  };
  return error;
}