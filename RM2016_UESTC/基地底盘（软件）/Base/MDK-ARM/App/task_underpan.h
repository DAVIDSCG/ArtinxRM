#ifndef __UNDERPAN_TASK_H
#define __UNDERPAN_TASK_H

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"


/** 
  * @brief  �����˶�ģʽ
  */
typedef enum
{
  StopMoveLock    = 0x00,
  StopMoveNotLock,
  MoveUnderRC,              //ң��ģʽ
  MoveAuto,                 //Bezier����ģʽ
  PID_Debug,                //����PID
  MovePlay,                 //����ģʽ
  MoveCircle,               //ԲȦ
  MoveFullAuto,             //�ռ�ȫ�Զ�ģʽ
  UpperComputerDebug,       //��λ������
  
}MoveModeTypeDef;


/** 
  * @brief  X���� �� Y����
  */
enum
{
  X_Drct = 0,
  Y_Drct = 1,
};


/** 
  * @brief  �ڵ����˶������е�����
  */
typedef struct 
{
	float   fGlobalAngle;           //��������ϵ�е��̵ĽǶ�
	int16_t s16_GlobalX;            //��������ϵ�е�������X����
  int16_t s16_GlobalY;            //��������ϵ�е�������Y����
  float   fSpeedX;                //x�����ϵ��ٶ�
  float   fSpeedY;                //y�����ϵ��ٶ�
  float   fOmega;                 //��ת�Ľ��ٶ�
  float   fTarAngle;              //Ŀ��Ƕȣ�˳ʱ��Ϊ��
  float   fTarDrct;               //����Ŀ���˶��������׺�fTarAngle��죩
  int16_t s16_WheelSpd[4];        //���͸�������ٶ�
  int16_t s16_WheelSpdLmt;        //���������ٶ�
  MoveModeTypeDef emBaseMoveMode; //���ص�ģʽ
	/*new add*/
}UnderpanDataTypeDef;  //�������ݽṹ


/** 
  * @brief  Ѫ���仯ʵʱ��¼
  *       (�仯��ʽ���仯ֵ
  *       �仯װ�װ�ID���������װ�װ��Ѫ��Ϊ-1��)
  *       �仯ʱ�䣨important��
  *       ������ʱ��ͷ����
  */
typedef struct
{
  uint8_t   u8_Way;
  uint16_t  u16_Val;
  int8_t    s8_WeakId;
  uint32_t  u32_TimRcrd;
  float     fAngRcrd;
}BldChangeRcrdTypeDef;


/** 
  * @brief  �������ݼ�¼
  */
enum
{
  HERO_RCRD = 0,
  UAV_RCRD,
  INF_RCRD,
  MANY_RCRD,
  RCRD_LEN
};


/** 
  * @brief  �ܵ���������
  */
typedef struct
{
  uint8_t   u8_EnemyHeroAppr;      //Ӣ�۳���
  uint8_t   u8_EnemyUAVAppr;       //���˻�����
  uint8_t   u8_EnemyInfAppr;       //��������
  uint8_t   u8_TooManyEnemy;        //�ö����
  BldChangeRcrdTypeDef  tAtckRcrd[RCRD_LEN];  //�ܵ�����Ѫ���仯��¼
}AtckMngTypeDef;


/** 
  * @brief  ���ʹ���
  */
typedef struct {
	float fAllPower;              //�ܹ��� ��ʱ����
	int16_t s16_CurLimit;         //�ܵ������� mA
	int16_t s16_AllWheelCur;      //�ĸ����ӵ��ܵ��� mA
	int16_t s16_ResidueCur;       //ʣ�����
  int16_t s16_GivenMaxCur[4];   //����������ֵ
}PowerManageTypeDef;



void underpanTaskThreadCreate(osPriority taskPriority);

#endif


///**
//  * @brief  ���Ƴ��ƶ�����������ļ��ٶ�
//            ����һ��
//            1.X�ٶȷ���������ٶȷ���һ��(Ϊ��or��)����ʱY�ٶȷ���������ٶȷ���һ�»�һ��
//            2.X�ٶȷ���������ٶȷ���һ��(Ϊ��or��)����ʱY�ٶȷ���������ٶȷ���һ�»�һ��
//            �����ľ�������ң�����һ��Ҫ�����ܼ��ٶȣ�����һ�»������Բ����뿼��
//  * @param  None
//  * @retval None
//  */
//static void LimitAcc(void)
//{
//  
////  static float lastSpdX = 0,lastSpdY = 0;
////  float xSpdDiff,ySpdDiff;
////  
//  xSpdDiff = g_tBaseUnderpan.fSpeedX-lastSpdX;
//  ySpdDiff = g_tBaseUnderpan.fSpeedY-lastSpdY;
//  
//  //X�ٶȷ�������ٶȷ���һ��
//  if(xSpdDiff*g_tBaseUnderpan.fSpeedX>0)
//  {
//     //Y�ٶȷ�������ٶȷ���һ��
//    if(ySpdDiff*g_tBaseUnderpan.fSpeedY>0)
//    {
//      testAcc=sqrt(SQUARE(xSpdDiff)+SQUARE(ySpdDiff));

//      if(testAcc>MAX_ACC)
//      {
//        xSpdDiff = 1.5f/testAcc*xSpdDiff;
//        ySpdDiff = 1.5f/testAcc*ySpdDiff;
//      }
//    }
//    //Y�ٶȷ�������ٶȷ���һ��
//    else
//    {
//      if(xSpdDiff>MAX_ACC)
//      {
//        xSpdDiff = MAX_ACC;
//      }
//      else if(xSpdDiff<-MAX_ACC)
//      {
//         xSpdDiff = -MAX_ACC;
//      }
//    }
//  }
//  //X�ٶȷ�������ٶȷ���һ��
//  else
//  {
//    //Y�ٶȷ�������ٶȷ���һ��
//    if(ySpdDiff*g_tBaseUnderpan.fSpeedY>0)
//    {
//      //Y ������0
//      if(ySpdDiff>MAX_ACC)
//      {
//        ySpdDiff = MAX_ACC;
//      }
//      else if(ySpdDiff<-MAX_ACC)
//      {
//        ySpdDiff = -MAX_ACC;
//      }
//    }
//    //Y�ٶȷ�������ٶȷ���һ��
//    else
//    {
////      if(testAcc>5)
//      
////      {
////        xSpdDiff = MAX_ACC/testAcc*xSpdDiff;
////        ySpdDiff = MAX_ACC/testAcc*ySpdDiff;
////      }
//    }
//  }
//  g_tBaseUnderpan.fSpeedX=xSpdDiff+lastSpdX;
//  g_tBaseUnderpan.fSpeedY=ySpdDiff+lastSpdY;
//  
//  lastSpdX = g_tBaseUnderpan.fSpeedX;
//  lastSpdY = g_tBaseUnderpan.fSpeedY;
//}


///**
//  * @brief  �Զ��ƶ�_1��(����ģʽ),������
//  * @param  None
//  * @retval None
//  */
//#define GetRandomAngle(sA, eA)  (HAL_RNG_GetRandomNumber(&hrng)/4294967296.0f*(eA - sA)+sA)
//float fAdvanceV = 3.6f;
//static void AutoMove_No_1(void)
//{
//  static float fAngle = 0,fLastAngle = 0;  
////  tUAV_Data.s16_UAV_AbsoluX = testTemp1;
////  tUAV_Data.s16_UAV_AbsoluY = testTemp2;
////  tUAV_Data.s16_UAV_AbsoluX = tUAV_Data.s16_UAV_RelaX;
////  tUAV_Data.s16_UAV_AbsoluY = tUAV_Data.s16_UAV_RelaY;

//  if(fAngle == 0)
//  {
//    fAngle = GetRandomAngle(0.0f, 2.0f * PI);
//    //��ȡ��ǰλ��
//    fMoveP[2][X_Drct] = tBaseMixLocData.s16_xMix;
//    fMoveP[2][Y_Drct] = tBaseMixLocData.s16_yMix;
//  }
//  fMoveP[1][X_Drct] = fMoveP[2][X_Drct];
//  fMoveP[1][Y_Drct] = fMoveP[2][Y_Drct];
//  //������һλ��
//  fMoveP[2][X_Drct] = fMoveP[1][X_Drct]+cos(fAngle)*fAdvanceV;
//  fMoveP[2][Y_Drct] = fMoveP[1][Y_Drct]+sin(fAngle)*fAdvanceV;
//  
//  
//  //�����ж��Ƿ�ײ��
//  switch(((uint8_t)(fMoveP[2][X_Drct]>820)<<3)|
//          ((uint8_t)(fMoveP[2][X_Drct]<-820)<<2)|
//          ((uint8_t)(fMoveP[2][Y_Drct]>820)<<1)|
//          ((uint8_t)(fMoveP[2][Y_Drct]<-820)<<0))
//  {
//    case 0x08://1000 ��
//    case 0x0A://1010 ����
//      if(fMoveP[2][Y_Drct] > 600)
//      {
//        fAngle = GetRandomAngle(PI,1.4f*PI);
//      }
//      else if(fMoveP[2][Y_Drct] < -600)
//      {
//        fAngle = GetRandomAngle(0.6f*PI,PI);
//      }
//      else
//      {
//        fAngle = GetRandomAngle(0.6f*PI, 1.4f*PI);
//      }
//      fMoveP[2][X_Drct] = fMoveP[1][X_Drct]+cos(fAngle)*fAdvanceV;
//      fMoveP[2][Y_Drct] = fMoveP[1][Y_Drct]+sin(fAngle)*fAdvanceV;
//      break;
//    
//    case 0x02://0010 ��
//    case 0x06://0110 ����
//      if(fMoveP[2][X_Drct] > 600)
//      {
//        fAngle = GetRandomAngle(1.1f*PI,1.5f*PI);
//      }
//      else if(fMoveP[2][X_Drct] < -600)
//      {
//        fAngle = GetRandomAngle(1.5f*PI,1.9f*PI);
//      }
//      else
//      {
//        fAngle = GetRandomAngle(1.1f*PI,1.9f*PI);
//      }
//      fMoveP[2][X_Drct] = fMoveP[1][X_Drct]+cos(fAngle)*fAdvanceV;
//      fMoveP[2][Y_Drct] = fMoveP[1][Y_Drct]+sin(fAngle)*fAdvanceV;
//      break;
//    
//    case 0x04://0100 ��
//    case 0x05://0101 ����
//      if(fMoveP[2][Y_Drct] > 600)
//      {
//        fAngle = GetRandomAngle(1.6f*PI,2.0f*PI);
//      }
//      else if(fMoveP[2][Y_Drct] < -600)
//      {
//        fAngle = GetRandomAngle(2.0f*PI,2.4f*PI);
//      }
//      else
//      {
//        fAngle = GetRandomAngle(1.6f*PI,2.4f*PI);
//      }
//      fMoveP[2][X_Drct] = fMoveP[1][X_Drct]+cos(fAngle)*fAdvanceV;
//      fMoveP[2][Y_Drct] = fMoveP[1][Y_Drct]+sin(fAngle)*fAdvanceV;
//      break;
//    
//    case 0x01://0001 ��
//    case 0x09://1001 ����
//      if(fMoveP[2][X_Drct] > 600)
//      {
//        fAngle = GetRandomAngle(0.5f*PI,0.9f*PI);
//      }
//      else if(fMoveP[2][X_Drct] < -600)
//      {
//        fAngle = GetRandomAngle(0.1f*PI,0.5f*PI);
//      }
//      else
//      {
//        fAngle = GetRandomAngle(0.1f*PI,0.9f*PI);
//      }
//      fMoveP[2][X_Drct] = fMoveP[1][X_Drct]+cos(fAngle)*fAdvanceV;
//      fMoveP[2][Y_Drct] = fMoveP[1][Y_Drct]+sin(fAngle)*fAdvanceV;
//      break;
//    
//    default:
//      //�ж�λ��
//      switch(((uint8_t)(fMoveP[2][X_Drct]<tUAV_Data.s16_UAV_AbsoluX)<<1)|
//              ((uint8_t)(fMoveP[2][Y_Drct]<tUAV_Data.s16_UAV_AbsoluY)))
//      {
//        case 0x00:/*����*/
//          switch(((uint8_t)(fMoveP[2][X_Drct]<tUAV_Data.s16_UAV_AbsoluX+UAV_RANGE)<<1)|
//                  ((uint8_t)(fMoveP[2][Y_Drct]<tUAV_Data.s16_UAV_AbsoluY+UAV_RANGE)))
//          {
//            case 0x00:/*���ϲ��֣���ȫ���ཻ*/
//            case 0x01:
//            case 0x02:
//              break;
//            case 0x03:
//              if(abs(tUAV_Data.s16_UAV_AbsoluY-fMoveP[2][Y_Drct])>abs(tUAV_Data.s16_UAV_AbsoluX-fMoveP[2][X_Drct]))
//              {
//                fAngle = GetRandomAngle(PI*0.1f,PI*0.9f);
//              }
//              else
//              {
//                fAngle = GetRandomAngle(-PI*0.4f,PI*0.4f);
//              }
//              fMoveP[2][X_Drct] = fMoveP[1][X_Drct]+cos(fAngle)*fAdvanceV;
//              fMoveP[2][Y_Drct] = fMoveP[1][Y_Drct]+sin(fAngle)*fAdvanceV;
//              break;
//          }
//          break;
//        case 0x02:/*����*/
//          switch(((uint8_t)(fMoveP[2][X_Drct]>tUAV_Data.s16_UAV_AbsoluX-UAV_RANGE)<<1)|
//                  ((uint8_t)(fMoveP[2][Y_Drct]<tUAV_Data.s16_UAV_AbsoluY+UAV_RANGE)))
//          {
//            case 0x00:/*���ཻ*/
//            case 0x01:
//            case 0x02:
//              break;
//            case 0x03:
//              if(abs(tUAV_Data.s16_UAV_AbsoluY-fMoveP[2][Y_Drct])>abs(tUAV_Data.s16_UAV_AbsoluX-fMoveP[2][X_Drct]))
//              {
//                fAngle = GetRandomAngle(PI*0.1f,PI*0.9f);
//              }
//              else
//              {
//                fAngle = GetRandomAngle(PI*0.6f,PI*1.4f);
//              }
//              fMoveP[2][X_Drct] = fMoveP[1][X_Drct]+cos(fAngle)*fAdvanceV;
//              fMoveP[2][Y_Drct] = fMoveP[1][Y_Drct]+sin(fAngle)*fAdvanceV;
//              break;
//          }
//          break;
//        case 0x01:/*����*/
//          switch(((uint8_t)(fMoveP[2][X_Drct]<tUAV_Data.s16_UAV_AbsoluX+UAV_RANGE)<<1)|
//                  ((uint8_t)(fMoveP[2][Y_Drct]>tUAV_Data.s16_UAV_AbsoluY-UAV_RANGE)))
//          {
//            case 0x00:/*���ཻ*/
//            case 0x01:
//            case 0x02:
//              break;
//            case 0x03:
//              if(abs(tUAV_Data.s16_UAV_AbsoluY-fMoveP[2][Y_Drct])>abs(tUAV_Data.s16_UAV_AbsoluX-fMoveP[2][X_Drct]))
//              {
//                fAngle = GetRandomAngle(PI*1.1f,PI*1.9f);
//              }
//              else
//              {
//                fAngle = GetRandomAngle(-PI*0.4f,PI*0.4f);
//              }
//              fMoveP[2][X_Drct] = fMoveP[1][X_Drct]+cos(fAngle)*fAdvanceV;
//              fMoveP[2][Y_Drct] = fMoveP[1][Y_Drct]+sin(fAngle)*fAdvanceV;
//              break;
//          }
//          break;
//        case 0x03:/*����*/
//          switch(((uint8_t)(fMoveP[2][X_Drct]>tUAV_Data.s16_UAV_AbsoluX-UAV_RANGE)<<1)|
//                  ((uint8_t)(fMoveP[2][Y_Drct]>tUAV_Data.s16_UAV_AbsoluY-UAV_RANGE)))
//          {
//            case 0x00:/*���ཻ*/
//            case 0x01:
//            case 0x02:
//              break;
//            case 0x03:
//              if(abs(tUAV_Data.s16_UAV_AbsoluY-fMoveP[2][Y_Drct])>abs(tUAV_Data.s16_UAV_AbsoluX-fMoveP[2][X_Drct]))
//              {
//                fAngle = GetRandomAngle(PI*1.1f,PI*1.9f);
//              }
//              else
//              {
//                fAngle = GetRandomAngle(PI*0.6f,PI*1.4f);
//              }
//              fMoveP[2][X_Drct] = fMoveP[1][X_Drct]+cos(fAngle)*fAdvanceV;
//              fMoveP[2][Y_Drct] = fMoveP[1][Y_Drct]+sin(fAngle)*fAdvanceV;
//              break;
//          }
//          break;
//      }
//      break;
//  }
//  CalVxVy(fMoveP[2]);
////  LimitAcc();
//  //���£�
//  if(fAngle != fLastAngle)
//  {
//    fLastAngle = fAngle;
//    fAdvanceV = 2.4f;
//  }
//  else
//  {
//    if(fAdvanceV <= 3.6f)
//      fAdvanceV += 0.01f;
//  }
//}

