/*******************************************************************************

//		               ��������ģ��
��ģ����Ҫ���ڵĹ���������(����֧�ֱ���)���Խ�ѹѹ����ʽ�ļ���ͼƬ
��ģ���޸ĳ���"lodepng->HuffmanTree", ��л���ߣ���������
��ģ��Ϊ����������ѧϰ����������Ϊ����ӡ�񣬲������ֱ�������Է���ʹ��
��ģ�����Ƕ��ʽϵͳ�����Ż�,�̶���������Ϊ��������ά������̬�ṹ�Ҳ�������ڴ�

��ģ�鲻֧�ֶ��̵߳��ã�
*******************************************************************************/
#ifndef _HAFFMAN_TREE_H
#define _HAFFMAN_TREE_H


/*******************************************************************************
                             �궨��
*******************************************************************************/
/* amount of bits for first huffman table lookup (aka root bits), 
   see HuffmanTree_makeTable and HuffmanTree_DecodeSymbol.*/
/* values 8u and 9u work the fastest */
#define HAFFMAN_TREE_FIRSTBITS     9u

#define HAFFMAN_TREE_INVALIDSYMBOL 65535u

#define HAFFMAN_TREE_FIRST_LENGTH_CODE_INDEX   257
#define HAFFMAN_TREE_LAST_LENGTH_CODE_INDEX    285
/*256 literals, the end code, some length codes, and 2 unused codes*/
#define HAFFMAN_TREE_NUM_DEFLATE_CODE_SYMBOLS   288
/*the distance codes have their own symbols, 30 used, 2 unused*/
#define HAFFMAN_TREE_NUM_DISTANCE_SYMBOLS       32
/*the code length codes. 0-15: code lengths, 16: copy previous 3-6 times, 17: 3-10 zeros, 18: 11-138 zeros*/
#define HAFFMAN_TREE_NUM_CODE_LENGTH_CODES      19


/*******************************************************************************
                             ��ؽṹ
*******************************************************************************/


//--------------------------�����������ṹ-----------------------------------
typedef struct _HuffmanTree{
  //������ұ���Ҫ������(��֧�ֱ���)
  unsigned long *codes; //huffman codes, ��С��numcodes
  unsigned long *lengths; //huffman codes lengths,��С��numcodes
  unsigned short maxbitlen; //single code maximum number
  unsigned short numcodes; //number of codes
  
  //���������ɵĲ��ұ�����ʱֱ�Ӳ����
  unsigned char *table_len;//ֵ�ĳ��Ȳ��ұ�,��С512
  unsigned short *table_value; //ֵ���ұ�,��С512
}HuffmanTree_t;

extern const struct _HuffmanTree HuffmanTree_FixD;//����õĹ̶�����������ṹ
extern const struct _HuffmanTree HuffmanTree_FixLL;//����õĹ̶�ֵ�볤�ȹ������ṹ

#define HAFFMAN_TREE_LL  0  //��̬����������ṹ
#define HAFFMAN_TREE_D   1  //��ֵ̬�볤�ȹ������ṹ

//--------------------------��̬������������-----------------------------------
struct _HuffmanTreeMng{
  //����Ķ�̬�������ṹ
  struct _HuffmanTree HuffmanTree[2]; 
  //�����Ҫ�ģ���̬�������ṹ���Ȳ��ұ�, _makeTable()�в���
  unsigned char table_len[2][1u << HAFFMAN_TREE_FIRSTBITS]; 
  //�����Ҫ�ģ���̬�������ṹֵ���ұ�, _makeTable()�в���
  unsigned short table_value[2][1u << HAFFMAN_TREE_FIRSTBITS]; 
};


//�˹�������ʹ�ö�̬��������ʱ����һֱ�������ݵ�������
extern struct _HuffmanTreeMng *pHuffmanTreeMng;

//---------------------------��̬�������ڲ��ڴ滺����---------------------------
//��HuffmanTree_UpdateDync()��ʹ�ã��˳������ͷ�
struct _HuffmanTreeBuf{
  struct{//HuffmanTree_UpdateDync()��ʹ�õĻ��壬�豣�����˳�
    //HuffmanTree_UpdateDync�в���,makeTable()��Ҫ
    unsigned long bitlen_ll[HAFFMAN_TREE_NUM_DEFLATE_CODE_SYMBOLS];
    unsigned long bitlen_d[HAFFMAN_TREE_NUM_DISTANCE_SYMBOLS];
    //_makeFromLengths2()��makeTable()�д���
    unsigned long codes[HAFFMAN_TREE_NUM_DEFLATE_CODE_SYMBOLS];
  }m; 
  
  union {//��������ʹ�õĻ���
    struct{//_makeFromLengths2()��ʹ��, ��������_codes
      unsigned long blcount[16];
      unsigned long nextcode[16];
    }makecodes;
    struct{//makeTable()��ʹ�ã�
      unsigned long maxlens[1u << HAFFMAN_TREE_FIRSTBITS];
    }makeTables;
  }u;
};

/*******************************************************************************
                             ��غ���
*******************************************************************************/

#include "bReader.h"

//----------------------------���¶�̬�������ṹ-------------------------------
//ԭgetTreeInflateDynamic, ����ǰ����,���ط�0����
//�˺�������λ�����¶�̬�ṹ
signed short HuffmanTree_UpdateDync(struct _HuffmanTreeMng *mng,//������ڴ��ٴ���
                                     struct _HuffmanTreeBuf *buf,//������ڴ��ٴ���
                                     bReader_t *reader);

//---------------------------------����������-----------------------------
//ԭhuffmanDecodeSymbol
unsigned short HuffmanTree_DecodeSymbol(bReader_t *reader, 
                                         const HuffmanTree_t* codetree);

#endif //_HAFFMAN_TREE_H


