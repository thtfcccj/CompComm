/***********************************************************************

//		               Adler32校验模块
此模块主要用于Deflate压缩算法中校验数据的完整性用
***********************************************************************/


//----------------------得到Adler32校验码----------------------------
unsigned long Adler32_Get(unsigned long Adler,
                           const unsigned char *pData, 
                           unsigned long Len)
{
  unsigned long s1 = Adler & 0xffff;
  unsigned long s2 = (Adler >> 16) & 0xffff;

  while(Len != 0){
    unsigned long amount = Len > 5552 ? 5552 : Len;
    Len -= amount;
    for(unsigned long i = 0; i < amount; i++) {
      s1 += (*pData++);
      s2 += s1;
    }
    s1 %= 65521;
    s2 %= 65521;
  }

  return (s2 << 16) | s1;
}

//----------------------得到Adler=1的32校验码----------------------------
unsigned long Adler32_Get1(const unsigned char *pData, 
                            unsigned long Len)
{
  return Adler32_Get(1, pData, Len); 
}