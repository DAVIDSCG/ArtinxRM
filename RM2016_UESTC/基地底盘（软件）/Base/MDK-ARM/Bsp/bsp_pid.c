/**
  ******************************************************************************
  * @file    bsp_pid.c
  * @author  X
  * @version V1.0
  * @date    2016/01/17
  * @brief   pid�ļ����о��е㸴����
  * 					Max_err ��Ϊ���Թرգ�
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "bsp_pid.h"
/* Defines -------------------------------------------------------------------*/
/* Variables -----------------------------------------------------------------*/

/*������ʼ��--------------------------------------------------------------*/
static void pid_param_init(
	PID_TypeDef * pid, 
	PID_ID   id,
	uint16_t maxout,
	uint16_t intergral_limit,
	uint16_t deadband,
	uint16_t period,
	int16_t  max_err,
	float  target,

	float 	kp, 
	float 	ki, 
	float 	kd,
    
  uint8_t deadbandoptype)
{
	pid->id = id;
	
	pid->ControlPeriod = period;
	pid->DeadBand = deadband;
	pid->IntegralLimit = intergral_limit;
	pid->MaxOutput = maxout;
	pid->Max_Err = max_err;
	pid->target = target;
	
	pid->kp = kp;
	pid->ki = ki;
	pid->kd = kd;
  
  pid->DeadBandOpType = deadbandoptype;
	
	pid->output = 0;
}

/*��;���Ĳ����趨(����)------------------------------------------------------------*/
static void pid_reset(PID_TypeDef * pid, float kp, float ki, float kd)
{
	pid->kp = kp;
	pid->ki = ki;
	pid->kd = kd;
}

/*pid����--------------------------------------------------------------------------*/
static int16_t pid_calculate(PID_TypeDef* pid, float target, float measure)
{
	pid->measure = measure;
	pid->target = target;
	pid->last_err  = pid->err;
	pid->last_output = pid->output;
	
	pid->err = pid->target - pid->measure;
	
	//�Ƿ��������
	if(ABS(pid->err) >= pid->DeadBand)
	{
		pid->pout = pid->kp * pid->err;
//    //�޸Ļ������,�����෴�����0
//    if(pid->err * pid->last_err <= 0)
//      pid->iout = 0;
//		pid->iout += pid->ki * pid->err;
		pid->iout = pid->iout+pid->ki * pid->err;
		pid->dout =  pid->kd * (pid->err - pid->last_err);
		
		//�����Ƿ񳬳�����
		if(pid->iout > pid->IntegralLimit)
			pid->iout = pid->IntegralLimit;
		if(pid->iout < - pid->IntegralLimit)
			pid->iout = - pid->IntegralLimit;
		
		//pid�����
		pid->output = pid->pout + pid->iout + pid->dout;
//		pid->output = pid->output*0.8 + pid->last_output*0.2;  //�����˲�(�ò��ã�)
		
		//�Ƿ񳬳�������
		if(pid->output > pid->MaxOutput)
		{
			pid->output = pid->MaxOutput;
		}
		if(pid->output < -(pid->MaxOutput))
		{
			pid->output = -(pid->MaxOutput);
		}
		//���̫��˴����Ϊ0,Max_ErrΪ0ʱ������������
		if((pid->Max_Err) != 0 && \
			 ((pid->err > 0 && pid->err > pid->Max_Err) || \
       (pid->err < 0 && pid->err < -pid->Max_Err)))
		{
			pid->output = pid->last_output;
		}
	}
	else if(pid->DeadBandOpType == DB_OUTPUT_LAST)
	{
		pid->output = pid->last_output;
	}
  else if(pid->DeadBandOpType == DB_OUTPUT_ZERO)
  {
    pid->output = 0;
  }
	
	return pid->output;
}

/*pid�����ʼ��-----------------------------------------------------------------*/
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
    
  uint8_t deadbandOpType)
{
	pid->f_param_init = pid_param_init;
	pid->f_pid_reset = pid_reset;
	pid->f_cal_pid = pid_calculate;
	
	pid->f_param_init(pid, id, maxout, intergral_limit, deadband, period, max_err, target, kp, ki, kd, deadbandOpType);
}
