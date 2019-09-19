/***********************************************************************

		             IPv4Cfg��QMenu�еĲ����޸Ľӿ�
		             
***********************************************************************/

#ifndef __MENU_IPV4_CFG_H
#define __MENU_IPV4_CFG_H


/*******************************************************************************
		                       Set��Get�����ӿ�
*******************************************************************************/

//-------------------------------IPv4��1��-------------------------------------
void Get_IPv4Cfg0IpPos0(unsigned char Type);// {_Get_IPv4CfgIpPos(0,0);}
void Set_IPv4Cfg0IpPos0(unsigned char Type);// {_Set_IPv4CfgIpPos(0,0);}
void Get_IPv4Cfg0IpPos1(unsigned char Type);// {_Get_IPv4CfgIpPos(0,1);}
void Set_IPv4Cfg0IpPos1(unsigned char Type);// {_Set_IPv4CfgIpPos(0,1);}
void Get_IPv4Cfg0IpPos2(unsigned char Type);// {_Get_IPv4CfgIpPos(0,2);}
void Set_IPv4Cfg0IpPos2(unsigned char Type);// {_Set_IPv4CfgIpPos(0,2);}
void Get_IPv4Cfg0IpPos3(unsigned char Type);// {_Get_IPv4CfgIpPos(0,3);}
void Set_IPv4Cfg0IpPos3(unsigned char Type);// {_Set_IPv4CfgIpPos(0,3);}
void Get_IPv4Cfg0PortMax(unsigned char Type);//  {_Get_IPv4CfgPortMax(0);}
void Set_IPv4Cfg0PortMax(unsigned char Type);//  {_Set_IPv4CfgPortMax(0);}
void Get_IPv4Cfg0PortLow4(unsigned char Type);// {_Get_IPv4CfgPortLow4(0);}
void Set_IPv4Cfg0PortLow4(unsigned char Type);// {_Set_IPv4CfgPortLow4(0);}

//-------------------------------IPv4��2��-------------------------------------
#if IPV4_CFG_COUNT >= 2
void Get_IPv4Cfg1IpPos0(unsigned char Type);// {_Get_IPv4CfgIpPos(1,0);}
void Set_IPv4Cfg1IpPos0(unsigned char Type);// {_Set_IPv4CfgIpPos(1,0);}
void Get_IPv4Cfg1IpPos1(unsigned char Type);// {_Get_IPv4CfgIpPos(1,1);}
void Set_IPv4Cfg1IpPos1(unsigned char Type);// {_Set_IPv4CfgIpPos(1,1);}
void Get_IPv4Cfg1IpPos2(unsigned char Type);// {_Get_IPv4CfgIpPos(1,2);}
void Set_IPv4Cfg1IpPos2(unsigned char Type);// {_Set_IPv4CfgIpPos(1,2);}
void Get_IPv4Cfg1IpPos3(unsigned char Type);// {_Get_IPv4CfgIpPos(1,3);}
void Set_IPv4Cfg1IpPos3(unsigned char Type);// {_Set_IPv4CfgIpPos(1,3);}
void Get_IPv4Cfg1PortMax(unsigned char Type);//  {_Get_IPv4CfgPortMax(1);}
void Set_IPv4Cfg1PortMax(unsigned char Type);//  {_Set_IPv4CfgPortMax(1);}
void Get_IPv4Cfg1PortLow4(unsigned char Type);// {_Get_IPv4CfgPortLow4(1);}
void Set_IPv4Cfg1PortLow4(unsigned char Type);// {_Set_IPv4CfgPortLow4(1);}
#endif

//-------------------------------IPv4��3��-------------------------------------
#if IPV4_CFG_COUNT >= 3
void Get_IPv4Cfg2IpPos0(unsigned char Type);// {_Get_IPv4CfgIpPos(2,0);}
void Set_IPv4Cfg2IpPos0(unsigned char Type);// {_Set_IPv4CfgIpPos(2,0);}
void Get_IPv4Cfg2IpPos1(unsigned char Type);// {_Get_IPv4CfgIpPos(2,1);}
void Set_IPv4Cfg2IpPos1(unsigned char Type);// {_Set_IPv4CfgIpPos(2,1);}
void Get_IPv4Cfg2IpPos2(unsigned char Type);// {_Get_IPv4CfgIpPos(2,2);}
void Set_IPv4Cfg2IpPos2(unsigned char Type);// {_Set_IPv4CfgIpPos(2,2);}
void Get_IPv4Cfg2IpPos3(unsigned char Type);// {_Get_IPv4CfgIpPos(2,3);}
void Set_IPv4Cfg2IpPos3(unsigned char Type);// {_Set_IPv4CfgIpPos(2,3);}
void Get_IPv4Cfg2PortMax(unsigned char Type);//  {_Get_IPv4CfgPortMax(2);}
void Set_IPv4Cfg2PortMax(unsigned char Type);//  {_Set_IPv4CfgPortMax(2);}
void Get_IPv4Cfg2PortLow4(unsigned char Type);// {_Get_IPv4CfgPortLow4(2);}
void Set_IPv4Cfg2PortLow4(unsigned char Type);// {_Set_IPv4CfgPortLow4(2);}
#endif

/*******************************************************************************
		                       �ص�����
*******************************************************************************/
//-------------------------------���ݶ�дȨ������Cfg---------------------------
void Menu_IPv4Cfg_cbSetCfg(unsigned char Id, //IP id��
                           unsigned char Pos,//�޸�λ��,4Ϊport��λ��5port���λ
                           unsigned char Cfg);//��ǰ��Ҫ������


#endif //__MENU_IPV4_CFG_H


