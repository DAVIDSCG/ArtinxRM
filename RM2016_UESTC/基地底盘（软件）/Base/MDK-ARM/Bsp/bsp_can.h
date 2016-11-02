#ifndef __BSP_CAN_H
#define __BSP_CAN_H

#include "stm32f4xx_hal.h"
#include "can.h"
#include "includes.h"


#define CHANGE_MIAXDATASRC2(n)    do{tBaseMixLocData.emMixSrc = n;}while(0)


/** 
  * @brief  CAN���ͻ��ǽ��յ�ID
  */
typedef enum
{
  CAN_6623CTRL_ID   = 0x1FF,  //6623����ID
  CAN_24V_YAW_ID    = 0x205,  //6623YAW��ID
  CAN_24V_PITCH_ID  = 0x206,  //6623Pitch��ID
	CAN_FourMotor_ID  = 0x046,	//����
	CAN_GyroRecAg_ID	= 0x011,	//�����ǽ��սǶ�
	CAN_GyroRecXY_ID	=	0x012,	//�����ǽ���x,y����
  CAN_GyroReset_ID  = 0x013,	//�����Ǹ�λ
  CAN_RadarReset_ID = 0x014,  //�״︴λ
  CAN_Radar_ID      = 0x058,  //�״����
  CAN_UAV_ID        = 0x02B,  //�ɻ�λ��
  CAN_MotorR_ID     = 0x041,  //�Ҳ���
  CAN_MotorF_ID     = 0x042,  //ǰ�����
  CAN_MotorL_ID     = 0x043,  //�����
  CAN_MotorB_ID     = 0x044,  //�󷽵��
  CAN_FourCur_ID    = 0x040,  //���͸��ĸ����ӵ��ĸ�����
  CAN_KDetect_ID    = 0x015,  //����ʿ��⵽��״̬��ID
  CAN_UPPER_ID      = 0x303,  //��̨���ַ�������ID
}CAN_Message_ID;


/** 
  * @brief ����ת��������
  */
typedef union
{
 uint8_t  u8_form[4];
 int32_t  s32_form;
 float    float_form;
}DataConvertTypeDef;


/** 
  * @brief �״�״̬ö��
  */
typedef enum
{
  HAVENOTRECEIVE  = 0x00, //δ���յ�
  INITiALIZING    = 0x01, //��ʼ��״̬
  BELIEVABLE      = 0x02, //���ŵ�
  UNBELIEVABLE    = 0x03, //�����ŵ�
}RadarStatusTypedef;


/** 
  * @brief �״����ݽṹ��
  */
typedef struct
{
  RadarStatusTypedef emRadarStatus;
  uint8_t u8_DataType;
  int8_t  s8_RcvAngle;
  int16_t s16_RcvX;
  int16_t s16_RcvY;
  int16_t s16_RcvO;
  int8_t  s8_RadarAngle;
  int16_t s16_RadarX;
  int16_t s16_RadarY;
  SelfCheckTypeDef  tSelfCheck;
}RadarDataTypeDef;


/** 
  * @brief ���������ݽṹ��
  */
typedef struct
{
  float fInitAngle;           //��ʼ�Ƕ�
  float fGyroAngle;           //���յ��������ǽǶ�ֵ
  float fGyroAngleTmp;
  float fGyroMapanX;
  float fGyroMapanY;
  float fGyroMapanX_cnt;
  float fGyroMapanY_cnt;
  int32_t s32_EncX_cnt;       //���յ���X���������ֵ
  int32_t s32_EncY_cnt;       //���յ���Y���������ֵ
  int32_t s32_EncDx_cnt;
  int32_t s32_EncDy_cnt;
  SelfCheckTypeDef  tSelfCheck;
}GyroDataTypeDef;


/** 
  * @brief �ں�����������Դѡ��ö��
  */
typedef enum
{
  MAPAN_ONLY = 0x00,
  RADAR_ONLY = 0x01,
  M_AND_R    = 0x02,
}DataSrcChooseTypeDef;

/** 
  * @brief �ں��������ݽṹ��
  */
typedef struct
{
  #define ANGLE_CT_LEN 10
  DataSrcChooseTypeDef  emMixSrc;
  float   fAngleMix;
  float   fRadianMix;   //����ֵ
  struct
  {
    float   fAngleErr[ANGLE_CT_LEN];
    uint8_t AngleErrCT;
  }AngleErrRcd;         //�Ƕ����ݼ�¼
  float   fAngleErrEf;  //��Ч�Ƕ����
  int16_t s16_xMix;
  int16_t s16_yMix;
  int16_t s16_xErr;
  int16_t s16_yErr;
  int16_t s16_xErrEf;   //x��Ч���
  int16_t s16_yErrEf;   //y��Ч���
}MixLocDataTypeDef;


/** 
  * @brief  ���˻�λ��״̬
  */
typedef enum
{
  UAV_LOST    = 1,
  UAV_FOUND   = 2,
}UAV_StateTypeDef;


/** 
  * @brief ���˻�״̬������
  */
typedef struct
{
  int16_t   s16_UAV_RelaX;        //����ڳ���X����
  int16_t   s16_UAV_RelaY;        //����ڳ���Y����
  int16_t   s16_UAV_AbsoluX;      //���Ե�X����
  int16_t   s16_UAV_AbsoluY;      //���Ե�Y����
  UAV_StateTypeDef  em_UAV_State; //���˻�״̬
  SelfCheckTypeDef  tSelfCheck;
}UAV_DataTypeDef;


/** 
  * @brief  ����λ��
  */
enum
{
  W_Right   = 0,
  W_Front   = 1,
  W_Left    = 2,
  W_Behind  = 3,
};


/** 
  * @brief  �״ﴫ��������Ϣ
  */
enum
{
  ONE_ANG = 11,
  TWO_ANG = 12,
  THR_ANG = 13,
  BREAK_P = 2,      //�жϵ㣨��ѧ��˵���ٳ���,Ȼ�������кࣩܶ
  JUNK_DATA = 0,    //�������ݣ����ô���
};


/** 
  * @brief  ���̵��״̬���ݽṹ����������Ҧѧ��������
  */
typedef struct
{
  int16_t s16_NeededCur;    //�������ֵ
  int16_t s16_RevSpd;       //����ת��
  int16_t s16_RealCur;      //ʵ�ʵ���ֵ
#pragma pack(push)
#pragma pack(1)
	union{
		struct{
			unsigned can:1;
			unsigned encoder:1;
			unsigned adc:1;
			unsigned usart:1;
			unsigned :4;
		}bit;
		uint8_t BYTE;
	}ESR;
#pragma pack(pop)
  SelfCheckTypeDef  tSelfCheck;
}MotorDataTypeDef;


/** 
  * @brief  ��̨������ݽṹ��
  */
typedef struct
{
  uint8_t rcvFlag;              //�Ƿ��յ��������Ϣ
  uint16_t u16_LastAngleTmp;    //��һ���յ��ĽǶ�
  uint16_t u16_AngleTmp;        //��һ���յ��ĽǶ�
  int32_t s32_AngleBase;        //��׼ֵ
  int32_t s32_AngleAfCal;       //�����ĽǶ�
  int32_t s32_AngleAfCalLast;   //��һ�μ���ĽǶ�
  int16_t s16_OmgAfCal;         //����õ��Ľ��ٶ�
  int16_t s16_RealCurrent;      //ʵ�ʵ���ֵ
  int16_t s16_GivenCurrent;     //��������ֵ
  uint16_t u16_CurBydCnt;       //��������Χ����
  uint8_t  u8_CurBydFlg;        //��������Χ�ˣ�Ӧ���ǿ���
  uint16_t u16_CurBydFlgCnt;    //�������˼���
  SelfCheckTypeDef  tSelfCheck;
}GimbalMeasureTypeDef;


/** 
  * @brief  ��̨������can�Ƿ����߽ṹ��
  */
typedef struct
{
  SelfCheckTypeDef  tSelfCheck;
}GimbalCheckTypeDef;

/** 
  * @brief  ��̨��������
  */
typedef enum
{
  STAY_CALM = 0,  //��ֹ����
  BE_CRAZY  = 1,  //�Ҷ�
  OBEY_INS  = 2,  //����ָ��
  CANT_STOP = 3,  //����ͣ������
}upperCmdemTypeDef;

/** 
  * @brief  ��̨��������
  */
typedef struct
{
  upperCmdemTypeDef cmd;
  float ang;
  SelfCheckTypeDef  tSelfCheck;
}UpperDataTyperDef;



void My_CAN_FilterConfig(CAN_HandleTypeDef* _hcan);
HAL_StatusTypeDef CAN_Send_Message(CAN_HandleTypeDef* _hcan, CAN_Message_ID, int16_t* _message, uint8_t* _pBuff);

#endif
