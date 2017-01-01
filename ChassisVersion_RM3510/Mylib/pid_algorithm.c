/************************************************************************************
  File Name     :  pid_algorithm.c 
  cpu           :  STM32F405RGT6
  Create Date   :  2016/6/29
  Author        :  yf
  Description   :  ��̨pitch��206����yaw��205�����ٶȻ���λ�û���pid�㷨��
									 RM3510���820R����ĵ��ٶȻ���λ�û���pid�㷨
									 

-------------------------------Revision Histroy-----------------------------------
No   Version    Date     Revised By       Item       Description   
1     1.1       6/28       yf   			  pid�㷨	
2     1.2       6/29       gyf 
3     1.3       6/29       yf 					  ע��	
4			1.4       7/1				 yf     Velocity_Control_Shoot  ��������ٶȻ�PID          
************************************************************************************/
#include "main.h"

#define GAP 0.0

/********************************************************************************
                           820R��������ٶȻ�����
                      ���� 820R�ᵱǰ�ٶ� 820R��Ŀ���ٶ�
*********************************************************************************/
float Velocity_Control_820R(float current_velocity_820R,float target_velocity_820R)
{
    const float l_p = ESC_820R_VEL_P;//7.0
    const float l_i = ESC_820R_VEL_I;//0.5
    const float l_d = ESC_820R_VEL_D;

    static float error_l[2] = {0.0,0.0};
    static float output = 0;
    static float inte = 0;
    
    error_l[0] = error_l[1];
    error_l[1] = target_velocity_820R - current_velocity_820R;
    inte += error_l[1]; 
    
    output = error_l[1] * l_p 
            + inte * l_i 
            + (error_l[1] - error_l[0]) * l_d;
    
    if(output > ESC_MAX)
    {
        output = ESC_MAX;
    }
    
    if(output < -ESC_MAX)
    {
        output = -ESC_MAX;
    }
    		
    return output;
}
/********************************************************************************
                           820R������λ�û�����
                      ���� 820R�ᵱǰλ�� 820R��Ŀ��λ��
*********************************************************************************/
float Position_Control_820R(float current_position_820R,float target_position_820R)
{
    const float l_p = ESC_820R_POS_P;
		const float l_i = ESC_820R_POS_I;
		const float l_d = ESC_820R_POS_D;
    static float error_l[2] = {0.0,0.0};
    static float output = 0;
    static float inte = 0;
    
    error_l[0] = error_l[1];
    error_l[1] = target_position_820R - current_position_820R;
    inte += error_l[1]; 
    
    output = error_l[1] * l_p 
            + inte * l_i 
            + (error_l[1] - error_l[0]) * l_d;
    
    if(output > ESC_MAX)
    {
        output = ESC_MAX;
    }
    
    if(output < -ESC_MAX)
    {
        output = -ESC_MAX;
    }
    		
    return output;
}


