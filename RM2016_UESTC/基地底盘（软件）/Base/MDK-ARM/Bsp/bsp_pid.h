#ifndef __BSP_PID_H
#define __BSP_PID_H

#include "stm32f4xx_hal.h"

//ȡ����ֵ
#define ABS(x)		((x>0)?x:-x)

//PID��ID��ö��
typedef enum
{
  PID_ANGLE = 0,
  PID_VX,
  PID_VY,
  PID_EGGROTATE_ANG,
  PID_EGGROTATE_OMG,
//	Gimbal_PID_Pitch = 0,
//	Gimbal_PID_Yaw,
//	Gimbal_PID_PitchSpeed,
//	Gimbal_PID_YawSpeed,
//	
//	UnderPan_PID_Yaw,
}PID_ID;

enum
{
  DB_OUTPUT_LAST = 0,
  DB_OUTPUT_ZERO = 1,
};


//PID�ṹ��
typedef struct _PID_TypeDef
{
	PID_ID id;
	
	float target;							//Ŀ��ֵ
	
	float kp;
	float ki;
	float kd;
	
	float   measure;					//����ֵ
	float   err;							//���
	float   last_err;      		//�ϴ����
	
	float   pout;							//p���
	float   iout;							//i���
	float   dout;							//d���
	
	float   output;						//�������
	float   last_output;			//�ϴ����
	
	uint16_t MaxOutput;				//����޷�
	uint16_t IntegralLimit;		//�����޷�
	uint16_t DeadBand;			  //����������ֵ��
	uint16_t ControlPeriod;		//��������
	int16_t  Max_Err;					//������(�ⶫ�����˺ö��ˣ�
  
  uint8_t  DeadBandOpType;  //������ʱ���������
	
	void (*f_param_init)(struct _PID_TypeDef *pid,  //PID������ʼ��
					PID_ID id,
					uint16_t maxOutput,
				  uint16_t integralLimit,
					uint16_t deadband,
					uint16_t controlPeriod,
					int16_t	 max_err,     
					float    target,
				   
					float kp,
					float ki,
					float kd,
            
          uint8_t deadbandOpType);
				   
	void (*f_pid_reset)(struct _PID_TypeDef *pid, float kp,float ki, float kd);		//pid���������޸�
	int16_t (*f_cal_pid)(struct _PID_TypeDef *pid, float target, float measure);  //pid����
}PID_TypeDef;

//��������
void PID_Configuration(
	PID_TypeDef* pid,
	PID_ID   id,
	uint16_t maxout,
	uint16_t intergral_limit,
	uint16_t deadband,
	uint16_t period,
	int16_t  max_err,
	float    target,

	float 	kp, 
	float 	ki, 
	float 	kd,
    
  uint8_t deadbandOpType);
#endif
