/*******************************************************************************

//		              滑动窗口写数据结构 相关实现

*******************************************************************************/

#include "winWriter.h"
#include <string.h>


/*******************************************************************************
                             相关函数实现
*******************************************************************************/

//--------------------------输出数据用完处理函数---------------------------------
//缓冲区即将满时，调用此函数送出数据，返回0正常输出,否则异常
signed char winWriter_OutData(winWriter_t *out)
{
  if(out->LaterPro == NULL) return -1;//异常
  brsize_t used = out->LaterPro(out); //交由用户处理
  if((used > out->start) || (used > out->capability))
    return  -2;//数据异常 
  out->start -= used; //未处理完的数据
  memcpy(out->data, out->data + used, out->start);//清掉处理掉的数据
  return 0;
}

//-------------------------------copy数据--------------------------------
//此copy考虑到copy到start后数据满情况，若如此，则要送出部分数据
signed char winWriter_Copy(winWriter_t *out,
                           const unsigned char *pData,//数据
                           brsize_t DataLen,    //数据长度
                           brsize_t LeavedSize)//可留下的数据空间
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

    //数据满了
    if((out->capability - start) < LeavedSize){
      error = winWriter_OutData(out);
      if(error) break;
    }
  };
  return error;
}

//--------------------------从当前位置往前copy数据------------------------------
//此copy用于从输出数据里,copy数据至当前start位置,返回0正常输出,否则异常
//此copy考虑到copy到start后数据满情况，若如此，则要送出部分数据
signed char winWriter_CopyBackward(winWriter_t *out,
                                    brsize_t backward,//往前位置(由当前start开始)
                                    brsize_t distance,//copy距离
                                    brsize_t LeavedSize)//可留下的数据空间
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
    //数据满了
    if((out->capability - start) < LeavedSize){
      error = winWriter_OutData(out);
      if(error) break;
      //部分数据被移走了，当前距离要休正
      backward =- (start - out->start);
    }
  };
  return error;
}