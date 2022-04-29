/***********************************************************************

                        ��Ϣ������
�˽ṹ��Ҫ����ϵͳ���̻����罻��������
***********************************************************************/
#ifndef __MSG_PACKET_H
#define	__MSG_PACKET_H

/***********************************************************************
		                          ��Ϣ�ṹ����
***********************************************************************/

//�˰�����Ϊ��������ݶ���(Object)��Id��������ĳ������(Fun)�Ķ�,д�Ȳ���
struct _MsgPacket{
  unsigned char  Object;       //���û���������ݶ���
  unsigned char  TypeFun;      //�����빦���룬���嶨��
  unsigned short Id;           //����ID��
};

//��������Ӧ�ò����������λ����Ϊ��д������:
#define MSG_PACKET_RD_RW  0x00  //����д������,���EEPROM
#define MSG_PACKET_RD_RO  0x40  //��ֻ��������,���״̬��Ϣ
#define MSG_PACKET_WR_RW  0x80  //д��д������,��дEEPROM
#define MSG_PACKET_WR_WO  0xC0  //дֻд������,��ָ��
#define MSG_PACKET_TYPE   0xC0 

/*******************************************************************************
		                         ��غ���
*******************************************************************************/

//--------------------------------��������------------------------------------- 
#define MsgPacket_GetObject(msgPacket) ((msgPacket)->Object)

//------------------------------����д������---------------------------------
#define MsgPacket_GetType(msgPacket) ((msgPacket)->TypeFun & MSG_PACKET_TYPE)
#define MsgPacket_IsRdRw(msgPacket) \
                       (MsgPacket_GetType(msgPacket) == MSG_PACKET_RD_RW)
#define MsgPacket_IsRdRo(msgPacket) \
                       (MsgPacket_GetType(msgPacket) == MSG_PACKET_RD_RO)
#define MsgPacket_IsWrRw(msgPacket) \
                       (MsgPacket_GetType(msgPacket) == MSG_PACKET_WR_RW)                         
#define MsgPacket_IsWrWo(msgPacket) \
                       (MsgPacket_GetType(msgPacket) == MSG_PACKET_WR_WO) 
//�����ж�
#define MsgPacket_IsWr(msgPacket) ((msgPacket)->TypeFun &  0x80)                         
#define MsgPacket_IsRd(msgPacket) (!MsgPacket_IsWr(msgPacket))
                         
//----------------------------------������------------------------------------- 
#define MsgPacket_GetFun(msgPacket) ((msgPacket)->TypeFun & 0x3F)
                         
//----------------------------------ID------------------------------------- 
#define MsgPacket_GetId(msgPacket) ((msgPacket)->Id)                         


#endif //#define	__MSG_PACKET_H

