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
                             ��ؽṹ
*******************************************************************************/

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
extern struct _HuffmanTree HuffmanTree[2]; //��̬�������ṹ

/*******************************************************************************
                             ��غ���
*******************************************************************************/

#include "bReader.h"

//----------------------------���¶�̬�������ṹ-------------------------------
//ԭgetTreeInflateDynamic, ����ǰ����,���ط�0����
//�˺�������λ�����¶�̬�ṹ
signed short HuffmanTree_UpdateDync(bReader_t *reader);

//---------------------------------����������-----------------------------
//ԭhuffmanDecodeSymbol
unsigned short HuffmanTree_DecodeSymbol(bReader_t *reader, 
                                         const HuffmanTree_t* codetree);

#endif //_HAFFMAN_TREE_H


