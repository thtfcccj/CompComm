/***********************************************************************

//		               LRC校验程序集合

***********************************************************************/


//----------------------得到十六位LRC校验码----------------------------
//输出没有取反
unsigned short LRC16_Get(const unsigned char *pData, 
                         unsigned short Len)
{
  unsigned short LRC16 = 0;
  for(; Len > 0; Len--, pData++){  
    LRC16 += *pData;
  }  
  return LRC16;
}

//----------------------得到8位LRC校验码----------------------------
//输出没有取反
unsigned char LRC8_Get(const unsigned char *pData, 
                       unsigned short Len)
{
  unsigned char LRC = 0;
  for(; Len > 0; Len--, pData++){  
    LRC += *pData;
  }  
  return LRC;                    
}

