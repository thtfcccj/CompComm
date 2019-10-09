/***********************************************************************

                  SHA256算法实现-非标实现
此算法移值至https://www.jianshu.com/p/0251bb005d70
//原代码直接移值，比较与改写后结果一致，但与在线校验不一致！！！
**********************************************************************/
#include "SHA256.h"
#include <string.h>

//--------------------------循环右移函数函数实现--------------------------------
#define rightrotate(w, n) ((w >> n) | (w) << (32-(n)))

//--------------------------copy_uint32函数实现--------------------------------
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
void copy_uint32(unsigned char *p, unsigned long val)
{
  *p++ = val >> 24;
  *p++ = val >> 16;
  *p++ = val >> 8;
  *p = val;
}
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
void copy_uint32(unsigned char *p, unsigned long val)
{
  *p++ = val;
  *p++ = val >> 8;
  *p++ = val >> 16;
  *p = val >> 24;
}
#else
  #error "Unsupported target architecture endianess!"
#endif

static const unsigned long k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

//-----------------------由数据流得到SHA256值相关变量-------------------------
#ifndef SUPPORT_SHA256_REEENTRANT
  //将局部变量提出为全局变量(不支持)
  static unsigned long w[64];   //原w
  static unsigned long h[8];    //原h
  static unsigned long a_h[8];  //对应原a-h
  static unsigned char buf[512 / 8 + 8];//申请append8长度空间以计算
#endif

static const unsigned long hInitVol[8] = {//h初始化值
  0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 
  0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

//---------------------由数据流得到SHA256值----------------------------------
//此函数不支持重入
void SHA256(const unsigned char *pData, //数据
            size_t Len,           //数据长度
            unsigned char *pResume)    //结果,长度为256bit/8=32
{
    #ifdef SUPPORT_SHA256_REEENTRANT
      unsigned long w[64];   //原w
      unsigned long h[8];    //原h
      unsigned long a_h[8];  //对应原a-h
      unsigned char buf[448 + 8];//申请append8长度空间以计算
    #endif
  
    //计算append大小,buf附加在pData+Len后
    unsigned short r = ((unsigned long)Len * 8 % 512);
    unsigned short append = ((r < 448) ? (448 - r) : (448 + 512 - r)) / 8;
    //初始化append区域
    memset(buf, 0, append);
    buf[0] = 0x80;
    //初始化append后的8个数据区域
    unsigned long long bits_Len = Len * 8;
    for(unsigned char i = 0; i < 8; i++){
      buf[append + i] = (bits_Len >> ((7 - i) * 8)) & 0xff;
    }
    memcpy(h, hInitVol, sizeof(hInitVol)); //使用前初始化h
    memset(w, 0, sizeof(w)); //初始化w
    unsigned long new_Len = Len + append + 8; //新的组合字符长度    
    unsigned long chunk_Len = new_Len / 64;
    for (unsigned long idx = 0; idx < chunk_Len; idx++) {
        unsigned long val = 0;
        for (int i = 0; i < 64; i++) {
            unsigned long DataPos = idx * 64 + i;
            unsigned long CurData;
            if(DataPos >= Len) CurData = *(buf + (DataPos - Len));//在附加里
            else CurData = *(pData + DataPos); //在数据区
            val =  val | (CurData << (8 * (3 - i)));
            if (i % 4 == 3) {
                w[i / 4] = val;
                val = 0;
            }
        }
        for (int i = 16; i < 64; i++) {
            unsigned long s0 = rightrotate(w[i - 15], 7) ^ rightrotate(w[i - 15], 18) ^ (w[i - 15] >> 3);
            unsigned long s1 = rightrotate(w[i - 2], 17) ^ rightrotate(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }
        
        memcpy(a_h, h ,sizeof(a_h));//初始化a~h为h对应值
        for (int i = 0; i < 64; i++) {
            unsigned long s_1 = rightrotate(a_h[4], 6) ^ rightrotate(a_h[4], 11) ^ rightrotate(a_h[4], 25);
            unsigned long ch = (a_h[4] & a_h[5]) ^ (~a_h[4] & a_h[6]);
            unsigned long temp1 = a_h[7] + s_1 + ch + k[i] + w[i];
            unsigned long s_0 = rightrotate(a_h[0], 2) ^ rightrotate(a_h[0], 13) ^ rightrotate(a_h[0], 22);
            unsigned long maj = (a_h[0] & a_h[1]) ^ (a_h[0] & a_h[2]) ^ (a_h[1] & a_h[2]);
            unsigned long temp2 = s_0 + maj;
            a_h[7] = a_h[6];
            a_h[6] = a_h[5];
            a_h[5] = a_h[4];
            a_h[4] = a_h[3] + temp1;
            a_h[3] = a_h[2];
            a_h[2] = a_h[1];
            a_h[1] = a_h[0];
            a_h[0] = temp1 + temp2;
        }
        h[0] += a_h[0];
        h[1] += a_h[1];
        h[2] += a_h[2];
        h[3] += a_h[3];
        h[4] += a_h[4];
        h[5] += a_h[5];
        h[6] += a_h[6];
        h[7] += a_h[7];
    }
    //各位结果copy
    copy_uint32(pResume + 0, h[0]);
    copy_uint32(pResume + 4, h[1]);
    copy_uint32(pResume + 8, h[2]);
    copy_uint32(pResume + 12, h[3]);
    copy_uint32(pResume + 16, h[4]);
    copy_uint32(pResume + 20, h[5]);
    copy_uint32(pResume + 24, h[6]);
    copy_uint32(pResume + 28, h[7]);
}

//原代码直接移值，比较与改写后结果一致，但与在线校验不一致！！！
/*void SHA256(const unsigned char *data, size_t len, unsigned char *out) {
    unsigned long h0 = 0x6a09e667;
    unsigned long h1 = 0xbb67ae85;
    unsigned long h2 = 0x3c6ef372;
    unsigned long h3 = 0xa54ff53a;
    unsigned long h4 = 0x510e527f;
    unsigned long h5 = 0x9b05688c;
    unsigned long h6 = 0x1f83d9ab;
    unsigned long h7 = 0x5be0cd19;
    int r = (int)(len * 8 % 512);
    int append = ((r < 448) ? (448 - r) : (448 + 512 - r)) / 8;
    size_t new_len = len + append + 8;
    unsigned char buf[1024];
    memset(buf + len, 0, append);
    if (len > 0) {
        memcpy(buf, data, len);
    }
    buf[len] = (unsigned char)0x80;
    unsigned long long bits_len = len * 8;
    for (int i = 0; i < 8; i++) {
        buf[len + append + i] = (bits_len >> ((7 - i) * 8)) & 0xff;
    }
    unsigned long w[64];
    memset(w, 0, sizeof(w));
    size_t chunk_len = new_len / 64;
    for (int idx = 0; idx < chunk_len; idx++) {
        unsigned long val = 0;
        for (int i = 0; i < 64; i++) {
            val =  val | (*(buf + idx * 64 + i) << (8 * (3 - i)));
            if (i % 4 == 3) {
                w[i / 4] = val;
                val = 0;
            }
        }
        for (int i = 16; i < 64; i++) {
            unsigned long s0 = rightrotate(w[i - 15], 7) ^ rightrotate(w[i - 15], 18) ^ (w[i - 15] >> 3);
            unsigned long s1 = rightrotate(w[i - 2], 17) ^ rightrotate(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }
        
        unsigned long a = h0;
        unsigned long b = h1;
        unsigned long c = h2;
        unsigned long d = h3;
        unsigned long e = h4;
        unsigned long f = h5;
        unsigned long g = h6;
        unsigned long h = h7;
        for (int i = 0; i < 64; i++) {
            unsigned long s_1 = rightrotate(e, 6) ^ rightrotate(e, 11) ^ rightrotate(e, 25);
            unsigned long ch = (e & f) ^ (~e & g);
            unsigned long temp1 = h + s_1 + ch + k[i] + w[i];
            unsigned long s_0 = rightrotate(a, 2) ^ rightrotate(a, 13) ^ rightrotate(a, 22);
            unsigned long maj = (a & b) ^ (a & c) ^ (b & c);
            unsigned long temp2 = s_0 + maj;
            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }
        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
        h5 += f;
        h6 += g;
        h7 += h;
    }
    copy_uint32(out, h0);
    copy_uint32(out + 4, h1);
    copy_uint32(out + 8, h2);
    copy_uint32(out + 12, h3);
    copy_uint32(out + 16, h4);
    copy_uint32(out + 20, h5);
    copy_uint32(out + 24, h6);
    copy_uint32(out + 28, h7);
}*/


