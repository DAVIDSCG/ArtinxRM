/************************************************************************************
  File Name     :  sdbus.c 
  cpu           :  STM32F405RGT6
  Create Date   :  2016/6/29
  Author        :  yf
  Description   :  sdbus����ͽ��룬���õ��˽���
-------------------------------Revision Histroy-----------------------------------
No   Version    Date     Revised By       Item       Description   
1     1.6       7/8        yf   			    sdbus       �������	   
************************************************************************************/
#include "main.h"


SDBUS sdbus;
void SDBUS_Enc(const SDBUS* sdbus,unsigned char* sdbuf)//sdbus����
{
    sdbuf[0] = (sdbus->PitchAngle > 0)+'0';
    sdbuf[1] =  abs(sdbus->PitchAngle)/10+'0';
		sdbuf[2] = abs(sdbus->PitchAngle)%10+'0';
		sdbuf[3] = (sdbus->YawAngle > 0)+'0';
		sdbuf[4] = abs(sdbus->YawAngle)/10+'0';
		sdbuf[5] = abs(sdbus->YawAngle)%10+'0';
}

void SDBUS_Dec(SDBUS* sdbus,const unsigned char* sdbuf)//sdbus����
{
    sdbus->PitchAngle = (2*(sdbuf[0]-'0')-1)*((sdbuf[1]-'0')*10+(sdbuf[2]-'0'));
    sdbus->YawAngle= (2*(sdbuf[3]-'0')-1)*((sdbuf[4]-'0')*10+(sdbuf[5]-'0'));
}
