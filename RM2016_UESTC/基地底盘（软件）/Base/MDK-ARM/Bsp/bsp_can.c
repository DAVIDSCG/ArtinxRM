/**
  ******************************************************************************
  * @file    bsp_can.c
  * @author  
  * @version V1.2
  * @date    2016/05/16
  * @brief   
  * 
  ******************************************************************************
  * @attention
  *   7/26���״��ѷϣ�������ʿ
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "bsp_can.h"
#include "math.h"
#include "string.h"
#include "task_check.h"
/* Defines -------------------------------------------------------------------*/
#define WHEEL_PERIMETER 	162.19818f//162.4961717374034f  //(PI*WHEEL_DIAMETER)��λ���ܳ�����λ:mm 
#define Gyro_Ratio        1.0f        //�����Ǳ���ϵ��
#define RADAR2CARCENTER   332         //�״ﵽ�������ľ���

/* Variables -----------------------------------------------------------------*/
DataConvertTypeDef angle_union;
DataConvertTypeDef mapanXY_union;
DataConvertTypeDef data_union;

RadarDataTypeDef      tBaseRadarData;
GyroDataTypeDef       tBaseGyroData;
MixLocDataTypeDef     tBaseMixLocData;
UAV_DataTypeDef       tUAV_Data;
MotorDataTypeDef      tMotorData[4];
GimbalMeasureTypeDef  tEggMotorMeasure;
GimbalCheckTypeDef    tGimbalPitch;
GimbalCheckTypeDef    tGimbalYaw;
UpperDataTyperDef     tUpperData;
//uint8_t u8_KS_Symbol = 0;


/**
  * @brief  ��can.c��HAL_CAN_MspInit�е��ã�can���˲���������
  * @param  CAN_HandleTypeDef* hcan
  * @retval None
  */
void My_CAN_FilterConfig(CAN_HandleTypeDef* _hcan)
{
	CAN_FilterConfTypeDef		CAN_FilterConfigStructure;
	static CanTxMsgTypeDef		TxMessage;
  static CanRxMsgTypeDef 		RxMessage;
	
	CAN_FilterConfigStructure.FilterNumber = 0;
	CAN_FilterConfigStructure.FilterMode = CAN_FILTERMODE_IDMASK;
	CAN_FilterConfigStructure.FilterScale = CAN_FILTERSCALE_32BIT;
	CAN_FilterConfigStructure.FilterIdHigh = 0x0000;
	CAN_FilterConfigStructure.FilterIdLow = 0x0000;
	CAN_FilterConfigStructure.FilterMaskIdHigh = 0x0000;
	CAN_FilterConfigStructure.FilterMaskIdLow = 0x0000;
	CAN_FilterConfigStructure.FilterFIFOAssignment = CAN_FilterFIFO0;
	CAN_FilterConfigStructure.BankNumber = 14;
	CAN_FilterConfigStructure.FilterActivation = ENABLE;
	
	//��ʼ�����ɹ�������ѭ����̫�ã���Ȼ����һ�㲻����ʲô���⣩
	if(HAL_CAN_ConfigFilter(_hcan, &CAN_FilterConfigStructure) != HAL_OK)
	{
		while(1);
	}

	_hcan->pTxMsg = &TxMessage;
	_hcan->pRxMsg = &RxMessage;
}


/**
  * @brief  CAN�жϵĻص�����
  * @param  CAN_HandleTypeDef* _hcan
  * @retval None
  */
void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* _hcan)
{
  if(_hcan == &hcan1)
  {
    switch(_hcan->pRxMsg->StdId)
    {
      case CAN_GyroRecAg_ID:  /*���������ǽǶ�ֵ����*/
        angle_union.u8_form[0] = _hcan->pRxMsg->Data[0];
        angle_union.u8_form[1] = _hcan->pRxMsg->Data[1];
        angle_union.u8_form[2] = _hcan->pRxMsg->Data[2];
        angle_union.u8_form[3] = _hcan->pRxMsg->Data[3];
        memcpy((void *)&tBaseGyroData.fGyroAngle,&angle_union.float_form, 4);
        tBaseGyroData.fGyroAngle*=Gyro_Ratio;
        tBaseGyroData.fGyroAngleTmp = tBaseGyroData.fInitAngle-tBaseGyroData.fGyroAngle;
        tBaseMixLocData.fAngleMix = tBaseGyroData.fGyroAngleTmp - tBaseMixLocData.fAngleErrEf;
        tBaseMixLocData.fRadianMix = tBaseMixLocData.fAngleMix/180.0f*3.1415926f*1.0f;
        UPDATE_CHECKTIME(&tBaseGyroData);
        break;
      
      case CAN_GyroRecXY_ID:
        break;
        
      case CAN_Radar_ID:
        switch(_hcan->pRxMsg->Data[0])
        {
          case ONE_ANG:
          case TWO_ANG:
          case THR_ANG:
          case BREAK_P:
            tBaseRadarData.u8_DataType = _hcan->pRxMsg->Data[0];
            tBaseRadarData.s16_RcvX = (int16_t)((_hcan->pRxMsg->Data[1]<<8) +
                                                    (_hcan->pRxMsg->Data[2]));
            tBaseRadarData.s16_RcvY = (int16_t)((_hcan->pRxMsg->Data[3]<<8) +
                                                    (_hcan->pRxMsg->Data[4]));
            tBaseRadarData.s16_RcvO = (int16_t)((_hcan->pRxMsg->Data[5]<<8) +
                                                    (_hcan->pRxMsg->Data[6]));
            tBaseRadarData.s8_RcvAngle = _hcan->pRxMsg->Data[7];
            break;
          
          case JUNK_DATA:
            break;
        }
        if(tBaseMixLocData.emMixSrc != MAPAN_ONLY&& \
              tBaseRadarData.u8_DataType == ONE_ANG)
        {
          tBaseRadarData.s16_RadarX = tBaseRadarData.s16_RcvY - 965;
          tBaseRadarData.s16_RadarY = tBaseRadarData.s16_RcvX - 965;
          tBaseRadarData.s8_RadarAngle = tBaseRadarData.s8_RcvAngle;
          
          tBaseMixLocData.s16_xErr = tBaseRadarData.s16_RadarX - tBaseGyroData.fGyroMapanX;
          tBaseMixLocData.s16_yErr = tBaseRadarData.s16_RadarY - tBaseGyroData.fGyroMapanY;
          
          //�Ƕȼ�¼
          #if 0 //���״���У׼�Ƕ�
          tBaseMixLocData.AngleErrRcd.fAngleErr[tBaseMixLocData.AngleErrRcd.AngleErrCT] = 
                                      (tBaseGyroData.fGyroAngleTmp - tBaseRadarData.s8_RadarAngle);
          if(++tBaseMixLocData.AngleErrRcd.AngleErrCT >= ANGLE_CT_LEN)
          {
            float angleSum = 0;
            for(int i=0;i<ANGLE_CT_LEN;i++)
            {
              angleSum += tBaseMixLocData.AngleErrRcd.fAngleErr[i];
            }
            tBaseMixLocData.fAngleErrEf += angleSum/(float)ANGLE_CT_LEN;
          }
          tBaseMixLocData.AngleErrRcd.AngleErrCT = tBaseMixLocData.AngleErrRcd.AngleErrCT%ANGLE_CT_LEN;
          #endif
          
          #if 1 //���״��Ҳ�У׼�Ƕ�
          tBaseMixLocData.fAngleErrEf = 0;
          #endif
          
          if(xABS(tBaseRadarData.s16_RadarX)<500 && xABS(tBaseRadarData.s16_RadarY)<500)
          {
            tBaseMixLocData.s16_xErrEf = tBaseMixLocData.s16_xErr;
            tBaseMixLocData.s16_yErrEf = tBaseMixLocData.s16_yErr;
           
          }
        }
        UPDATE_CHECKTIME(&tBaseRadarData);
        break;

      case CAN_UAV_ID:
        if((0x55==_hcan->pRxMsg->Data[0]) && (0xff == _hcan->pRxMsg->Data[7]))
        {
          if(1 == _hcan->pRxMsg->Data[1])//��ʾ�յ���Ч
          {
            tUAV_Data.s16_UAV_RelaX = (int16_t)((_hcan->pRxMsg->Data[2] << 8) + \
                                                  (_hcan->pRxMsg->Data[3]));
            tUAV_Data.s16_UAV_RelaY = (int16_t)((_hcan->pRxMsg->Data[4] << 8) + \
                                                  (_hcan->pRxMsg->Data[5]));
            //Ԥ����
            tUAV_Data.s16_UAV_RelaY = -tUAV_Data.s16_UAV_RelaY;
            
            tUAV_Data.s16_UAV_AbsoluX = tUAV_Data.s16_UAV_RelaX + tBaseMixLocData.s16_xMix;
            tUAV_Data.s16_UAV_AbsoluY = tUAV_Data.s16_UAV_RelaY + tBaseMixLocData.s16_yMix;
            tUAV_Data.em_UAV_State = UAV_FOUND;
          }
          else
          {
            tUAV_Data.em_UAV_State = UAV_LOST;
          }
          UPDATE_CHECKTIME(&tUAV_Data);
        }
        break;
        
      case CAN_MotorR_ID:
      case CAN_MotorF_ID:
      case CAN_MotorL_ID:
      case CAN_MotorB_ID:
        tMotorData[_hcan->pRxMsg->StdId%CAN_MotorR_ID].s16_NeededCur
              = (int16_t)((_hcan->pRxMsg->Data[0])|(_hcan->pRxMsg->Data[1]<<8));
        tMotorData[_hcan->pRxMsg->StdId%CAN_MotorR_ID].s16_RevSpd
              = (int16_t)((_hcan->pRxMsg->Data[2])|(_hcan->pRxMsg->Data[3]<<8));
        tMotorData[_hcan->pRxMsg->StdId%CAN_MotorR_ID].s16_RealCur
              = (int16_t)((_hcan->pRxMsg->Data[4])|(_hcan->pRxMsg->Data[5]<<8));
        tMotorData[_hcan->pRxMsg->StdId%CAN_MotorR_ID].ESR.BYTE
              = (int16_t)((_hcan->pRxMsg->Data[6])|(_hcan->pRxMsg->Data[7]<<8));
      
        UPDATE_CHECKTIME(tMotorData+_hcan->pRxMsg->StdId%CAN_MotorR_ID);
        break;
      
      case CAN_UPPER_ID:
        tUpperData.cmd = (upperCmdemTypeDef)_hcan->pRxMsg->Data[0];
        switch(tUpperData.cmd)
        {
          case STAY_CALM:
          case BE_CRAZY:
          case CANT_STOP:
            break;
          case OBEY_INS:
            data_union.u8_form[0] = _hcan->pRxMsg->Data[1];
            data_union.u8_form[1] = _hcan->pRxMsg->Data[2];
            data_union.u8_form[2] = _hcan->pRxMsg->Data[3];
            data_union.u8_form[3] = _hcan->pRxMsg->Data[4];
            tUpperData.ang = -data_union.float_form;
            break;
        }
        UPDATE_CHECKTIME(&tUpperData);
        break;
        
      case CAN_24V_PITCH_ID:
        UPDATE_CHECKTIME(&tGimbalPitch);
        break;
      
      case CAN_24V_YAW_ID:
        UPDATE_CHECKTIME(&tGimbalYaw);
        break;

      default:
        break;
    }
  }
	__HAL_CAN_ENABLE_IT(_hcan, CAN_IT_FMP0);
}


/**
  * @brief  ����can����Ϣ
  * @param  CAN_HandleTypeDef* _hcan
  * @param  CAN_Message_ID _id
  * @param	int16_t* _message
  * @param	uint8_t* _pBuff
  * @retval None
  */
HAL_StatusTypeDef CAN_Send_Message(CAN_HandleTypeDef* _hcan, CAN_Message_ID _id, int16_t* _message, uint8_t* _pBuff)
{
	int16_t ii; 
  HAL_StatusTypeDef halStatus;
	
  _hcan->pTxMsg->RTR = CAN_RTR_DATA;
  _hcan->pTxMsg->IDE = CAN_ID_STD;
  _hcan->pTxMsg->StdId = _id;

	switch(_id)
	{
		case CAN_GyroReset_ID:		//�����Ǹ�λ
			_hcan->pTxMsg->DLC = 2;
			_hcan->pTxMsg->Data[0] = 0x55;
			_hcan->pTxMsg->Data[1] = 0xff;
			break;
    
    case CAN_RadarReset_ID:   //�״︴λ
      _hcan->pTxMsg->DLC = 2;
			_hcan->pTxMsg->Data[0] = 0x55;
			_hcan->pTxMsg->Data[1] = 0xaa;
      break;
		
		case CAN_FourMotor_ID:		//����ٶ�
    case CAN_FourCur_ID:
			_hcan->pTxMsg->DLC = 8;
			for(ii=0;ii<4;ii++)
			{
				_hcan->pTxMsg->Data[ii*2] = (uint8_t)(*(_message+ii));
				_hcan->pTxMsg->Data[ii*2+1] = (uint8_t)(*(_message+ii)>>8);
			}
			break;
      
    case CAN_6623CTRL_ID:
      _hcan->pTxMsg->DLC = 8;
      _hcan->pTxMsg->Data[0] = (uint8_t)((*_message)>>8);
      _hcan->pTxMsg->Data[1] = (uint8_t)((*_message)>>0);
      _hcan->pTxMsg->Data[2] = (uint8_t)((*_message)>>8);
      _hcan->pTxMsg->Data[3] = (uint8_t)((*_message)>>0);
      _hcan->pTxMsg->Data[4] = 0;
      _hcan->pTxMsg->Data[5] = 0;
      _hcan->pTxMsg->Data[6] = 0;
      _hcan->pTxMsg->Data[7] = 0;
      break;
    
		default:
      break;
	}
	//ʱ�䲻��̫С����Ȼ��timeout
	halStatus = HAL_CAN_Transmit(_hcan, 0x24);
  
  return(halStatus);
}
