/***********************************************************************

		             IPv4Cfg��QMenu�еĲ����޸�ʵ��
		             
***********************************************************************/

#include "Menu.h"
#include "Menu_IPv4Cfg.h"
#include "IPv4Cfg.h"

/*******************************************************************************
		                       UserPara�ṹ 
*******************************************************************************/

//-------------------------------IP��ַ��Ӧλ-----------------------------------
//�˵���ʼ��
static void _Get_IPv4CfgIpPos(unsigned char Id, unsigned char Pos)
{
  Menu_IPv4Cfg_cbSetCfg(Id,Pos, MENU_CFG_ADJ_BIT);
  MenuPara.Min = 0;
  MenuPara.Max = 255;
  MenuPara.Adj = IPv4Cfg_GetIpPos(Id, Pos);
}

//��������
static void _Set_IPv4CfgIpPos(unsigned char Id, unsigned char Pos)
{
  IPv4Cfg_SetIpPos(Id, Pos, MenuPara.Adj);
}

//---------------------------------�˿ں����λ---------------------------------
//�˵���ʼ��
static void _Get_IPv4CfgPortMax(unsigned char Id)
{
  Menu_IPv4Cfg_cbSetCfg(Id, 4, MENU_CFG_ADJ_BIT);
  MenuPara.Min = 0;
  MenuPara.Max = 6;
  MenuPara.Adj = IPv4Cfg_GetPort(Id) / 10000;
}

//��������
static void _Set_IPv4CfgPortMax(unsigned char Id)
{
  unsigned short Port = MenuPara.Adj * 10000;
  Port += IPv4Cfg_GetPort(Id) % 10000;
  IPv4Cfg_SetPort(Id, Port);  
}

//---------------------------------�˿ںŵ�4λ---------------------------------
//�˵���ʼ��
static void _Get_IPv4CfgPortLow4(unsigned char Id)
{
  Menu_IPv4Cfg_cbSetCfg(Id, 5, MENU_CFG_ADJ_BIT);
  MenuPara.Min = 0;
  MenuPara.Max = 9999;
  MenuPara.Adj = IPv4Cfg_GetPort(Id) % 10000;
}

//��������
static void _Set_IPv4CfgPortLow4(unsigned char Id)
{
  unsigned short Port = MenuPara.Adj;
  Port += (IPv4Cfg_GetPort(Id) / 10000) * 10000;
  IPv4Cfg_SetPort(Id, Port);  
}

/*******************************************************************************
		                        IPv4Cfg�ṹ 
//����ID��������
*******************************************************************************/

//-------------------------------IPv4��1��-------------------------------------
void Get_IPv4Cfg0IpPos0(unsigned char Type) {_Get_IPv4CfgIpPos(0,0);}
void Set_IPv4Cfg0IpPos0(unsigned char Type) {_Set_IPv4CfgIpPos(0,0);}
void Get_IPv4Cfg0IpPos1(unsigned char Type) {_Get_IPv4CfgIpPos(0,1);}
void Set_IPv4Cfg0IpPos1(unsigned char Type) {_Set_IPv4CfgIpPos(0,1);}
void Get_IPv4Cfg0IpPos2(unsigned char Type) {_Get_IPv4CfgIpPos(0,2);}
void Set_IPv4Cfg0IpPos2(unsigned char Type) {_Set_IPv4CfgIpPos(0,2);}
void Get_IPv4Cfg0IpPos3(unsigned char Type) {_Get_IPv4CfgIpPos(0,3);}
void Set_IPv4Cfg0IpPos3(unsigned char Type) {_Set_IPv4CfgIpPos(0,3);}
void Get_IPv4Cfg0PortMax(unsigned char Type)  {_Get_IPv4CfgPortMax(0);}
void Set_IPv4Cfg0PortMax(unsigned char Type)  {_Set_IPv4CfgPortMax(0);}
void Get_IPv4Cfg0PortLow4(unsigned char Type) {_Get_IPv4CfgPortLow4(0);}
void Set_IPv4Cfg0PortLow4(unsigned char Type) {_Set_IPv4CfgPortLow4(0);}

//-------------------------------IPv4��2��-------------------------------------
#if IPV4_CFG_COUNT >= 2
void Get_IPv4Cfg1IpPos0(unsigned char Type) {_Get_IPv4CfgIpPos(1,0);}
void Set_IPv4Cfg1IpPos0(unsigned char Type) {_Set_IPv4CfgIpPos(1,0);}
void Get_IPv4Cfg1IpPos1(unsigned char Type) {_Get_IPv4CfgIpPos(1,1);}
void Set_IPv4Cfg1IpPos1(unsigned char Type) {_Set_IPv4CfgIpPos(1,1);}
void Get_IPv4Cfg1IpPos2(unsigned char Type) {_Get_IPv4CfgIpPos(1,2);}
void Set_IPv4Cfg1IpPos2(unsigned char Type) {_Set_IPv4CfgIpPos(1,2);}
void Get_IPv4Cfg1IpPos3(unsigned char Type) {_Get_IPv4CfgIpPos(1,3);}
void Set_IPv4Cfg1IpPos3(unsigned char Type) {_Set_IPv4CfgIpPos(1,3);}
void Get_IPv4Cfg1PortMax(unsigned char Type)  {_Get_IPv4CfgPortMax(1);}
void Set_IPv4Cfg1PortMax(unsigned char Type)  {_Set_IPv4CfgPortMax(1);}
void Get_IPv4Cfg1PortLow4(unsigned char Type) {_Get_IPv4CfgPortLow4(1);}
void Set_IPv4Cfg1PortLow4(unsigned char Type) {_Set_IPv4CfgPortLow4(1);}
#endif

//-------------------------------IPv4��3��-------------------------------------
#if IPV4_CFG_COUNT >= 3
void Get_IPv4Cfg2IpPos0(unsigned char Type) {_Get_IPv4CfgIpPos(2,0);}
void Set_IPv4Cfg2IpPos0(unsigned char Type) {_Set_IPv4CfgIpPos(2,0);}
void Get_IPv4Cfg2IpPos1(unsigned char Type) {_Get_IPv4CfgIpPos(2,1);}
void Set_IPv4Cfg2IpPos1(unsigned char Type) {_Set_IPv4CfgIpPos(2,1);}
void Get_IPv4Cfg2IpPos2(unsigned char Type) {_Get_IPv4CfgIpPos(2,2);}
void Set_IPv4Cfg2IpPos2(unsigned char Type) {_Set_IPv4CfgIpPos(2,2);}
void Get_IPv4Cfg2IpPos3(unsigned char Type) {_Get_IPv4CfgIpPos(2,3);}
void Set_IPv4Cfg2IpPos3(unsigned char Type) {_Set_IPv4CfgIpPos(2,3);}
void Get_IPv4Cfg2PortMax(unsigned char Type)  {_Get_IPv4CfgPortMax(2);}
void Set_IPv4Cfg2PortMax(unsigned char Type)  {_Set_IPv4CfgPortMax(2);}
void Get_IPv4Cfg2PortLow4(unsigned char Type) {_Get_IPv4CfgPortLow4(2);}
void Set_IPv4Cfg2PortLow4(unsigned char Type) {_Set_IPv4CfgPortLow4(2);}
#endif


