/*******************************************************************************

//		                 ZLIB解码解码实现

*******************************************************************************/

#include "ZlibDecompress.h"
#include "math_3.h"
#include "Adler32.h"

/*******************************************************************************
                             相关函数实现
*******************************************************************************/

//--------------------------------ZLib解压缩------------------------------------
//原lodepng_zlib_decompressv
signed char ZlibDecompress(const unsigned char *in, //压缩数据包
                           brsize_t insize,           //idat区数据个数
                           winWriter_t *out)     //已准备好的接收数据缓冲
{
 unsigned error = 0;
  unsigned CM, CINFO, FDICT;

  if(insize < 2) return 53; /*error, size of zlib data too small*/
  /*read information from zlib header*/
  if((in[0] * 256 + in[1]) % 31 != 0) {
    /*error: 256 * in[0] + in[1] must be a multiple of 31, the FCHECK value is supposed to be made that way*/
    return 24;
  }

  CM = in[0] & 15;
  CINFO = (in[0] >> 4) & 15;
  /*FCHECK = in[1] & 31;*/ /*FCHECK is already tested above*/
  FDICT = (in[1] >> 5) & 1;
  /*FLEVEL = (in[1] >> 6) & 3;*/ /*FLEVEL is not used here*/

  if(CM != 8 || CINFO > 7) {
    /*error: only compression method 8: inflate with sliding window of 32k is supported by the PNG spec*/
    return 25;
  }
  if(FDICT != 0) {
    /*error: the specification of PNG says about the zlib stream:
      "The additional flags shall not specify a preset dictionary."*/
    return 26;
  }

  error = DeflateNano_Decoder(in + 2, insize - 2, out); //去除数据头了
  if(error) return error;

  //校验数据完整性
  if(!out->Cfg & WIN_WRITER_EN_CHECK){
    unsigned ADLER32 = MsbFull2L(&in[insize - 4]);//字节转u32,校验压缩数据是否正确
    unsigned checksum = Adler32_Get(out->Checksum,
                                    out->data, 
                                    (unsigned)(out->start));
    if(checksum != ADLER32) return 58; /*error, adler checksum not correct, data must be corrupted*/
  }

  return 0; /*no error*/
}


