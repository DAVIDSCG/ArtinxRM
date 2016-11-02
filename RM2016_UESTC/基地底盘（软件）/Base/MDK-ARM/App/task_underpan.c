/**
  ******************************************************************************
  * @file    task_underpan.c
  * @author  
  * @version V1.1
  * @date    2016/01/22
  * @brief   ������������
  * 
  ******************************************************************************
  * @attention 
  *			
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "task_underpan.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "cmsis_os.h"
#include "bsp_uart.h"
#include "bsp_can.h"
#include "bsp_pid.h"
#include "task_bulletLoading.h"
#include "timer_canSender.h"
#include "task_detect.h"
#include "task_keyPress.h"
#include "judgement_libs.h"
#include "includes.h"
/* Defines -------------------------------------------------------------------*/
#define UNDERPAN_PERIOD     5         //����Ϊ5ms
#define MAX_WHEEL_SPEED     950       //��������ٶ�
#define AUTO_WHEEL_SPD_MAX  500       //�Զ��ƶ��ٶ�
#define ALPHA               45        //��Ƕ�
#define MATCH_RATIO         (1/5.8f)  //���ϵ��
#define UAV_RANGE           300       //���˻����Ƿ�Χ���߳���1/2
#define CUR_LMT             2600      //��ʼ������Ƶ��������Բ������ʣ����ܵ�ص����仯��

//װ�ױ��
#define ARMOUR_F      0
#define ARMOUR_L      1
#define ARMOUR_B      2
#define ARMOUR_R      3

#define MAX_ACC   3.00f //�����ٶ�
//����һ������Сֵ�ͳ���ȷ���������
#define CreateRandNum(min, len) (min+len*(HAL_RNG_GetRandomNumber(&hrng))/4294967296.f)
#define RNG_GETNUM()  ((HAL_RNG_GetRandomNumber(&hrng))/4294967296.f)   //[0,1]
#define SQUARE(x)   (x)*(x)   //ƽ��
#define GET_HPTNS(x1,y1,x2,y2)  sqrtf(SQUARE(x1-x2)+SQUARE(y1-y2)) //hypotenuse(��б�߳���)
/* Variables -----------------------------------------------------------------*/
osThreadId underpanTaskHandle;

portTickType xLastWakeTime;
UnderpanDataTypeDef g_tBaseUnderpan;
PowerManageTypeDef g_tPowerManage;
PID_TypeDef tPID_Angle;       //���̽Ƕ�pid
PID_TypeDef tPID_VX;          //X�����ϵ��ٶ�
PID_TypeDef tPID_VY;          //Y�����ϵ��ٶ�
AtckMngTypeDef tBaseAM;       //����Ѫ���仯����

int16_t s16_CalP[4][2] = {{0,0},{400,500},{600,300},{200,100}};  //���ڼ���·���ĳ�ʼ��
float   fXminPoint[2], fXmaxPoint[2];

float   fMoveP[3][2];   //�ƶ�Ҫ������·���㣨Move point��
float   t = 0;          //RatioRunned   �ܹ��ı���

////test......................................
float fRC_Sensitivity = 1.4f;             //ң����������
float testXminP[2]={800,-800},testXmaxP[2]={800,800};
//float testTemp1;
//float testTemp2;

extern RNG_HandleTypeDef hrng;
extern CAN_HandleTypeDef hcan1;
extern RC_TypeDef tRC_Data;
extern MixLocDataTypeDef tBaseMixLocData;
extern UAV_DataTypeDef tUAV_Data;
extern MotorDataTypeDef tMotorData[4];
extern int16_t debugPos[2];
extern GyroDataTypeDef      tBaseGyroData;
extern LoadingStatusTypeDef  tBaseLdSt;
extern __IO float fFixedMovePoint[][2];
extern JudgementDataTypedef tJData;
extern int8_t s8_LoadingStuckFlg;
extern KEYENCE_StateTypeDef  KS_List[KEYENCE_LIST_LEN];
extern uint8_t u8_KS_Symbol;
extern UpperDataTyperDef     tUpperData;
extern GameStartTypeDef      tGameStart; //������ʼ


//Debug
//static void MoveDebugPID(void);

static void MoveModeSelect(void);
static void UnderpanMoveControl(void);
static void CalculateWheelSpeed(float vx, float vy, float omega, float radian, int16_t maxspeed);
//static void MoveAlongTracePoint(uint16_t range);
static void CalVxVy(float* point);
//static void ResetGyro(void);
//static void ResetRadar(void);
static void Underpan_PID_Configuration(void);
static void CircleRun(int16_t r);
static void CalWheelSpdUnderPowerManage(float vx, float vy, float omega, float tarAngle);
//static void RepellentMode(void);
static void AllAutoMove(void);
static void RandomSpin(void);
//static void MoveInLineSegment(float* pfXmin, float* pfXmax);
//static void ArmourInjuredRun(AtckMngTypeDef* p_tBaseAM, uint16_t u16_Range);
static void RunLikeDancing(int16_t tranSpd, int16_t rttSpd, uint8_t Mode);


/**
  * @brief  �����˶�����
  * @param  void const * argument
  * @retval None
  */
void UnderpanTask(void const * argument)
{
  osDelay(100);
  Underpan_PID_Configuration();
  g_tPowerManage.s16_CurLimit = CUR_LMT;
  xLastWakeTime = xTaskGetTickCount();

  for(;;)
  {
    osDelayUntil(&xLastWakeTime, UNDERPAN_PERIOD);
    MoveModeSelect();
    UnderpanMoveControl();
  }
}


/**
  * @brief  �����˶�ģʽѡ��
  * @param  None
  * @retval None
  */
static void MoveModeSelect(void)
{
  int8_t nowLeftSW, nowRightSW;
  static int8_t lastLeftSW = -1, lastRightSW = -1;
  
  nowLeftSW = tRC_Data.switch_left;
  nowRightSW = tRC_Data.switch_right;
  
  if(lastLeftSW!=nowLeftSW \
    || lastRightSW!=nowRightSW)
  {
    switch(nowLeftSW)
    {
      case Switch_Up:
        switch(nowRightSW)
        {
          case Switch_Up:
            g_tBaseUnderpan.emBaseMoveMode = MoveUnderRC;
            break;
          
          case Switch_Middle:
            g_tBaseUnderpan.emBaseMoveMode = StopMoveLock;
            break;
          
          case Switch_Down:
//            ResetGyro();
//            osDelay(2500);
//            tBaseMixLocData.fAngleErrEf = 0;
//            xLastWakeTime = xTaskGetTickCount();
//            CHANGE_MIAXDATASRC2(M_AND_R);
//            g_tBaseUnderpan.emBaseMoveMode = MoveAuto;
            g_tBaseUnderpan.emBaseMoveMode = MoveFullAuto;
            break;
        }
        break;
        
      case Switch_Middle:
        switch(nowRightSW)
        {
          case Switch_Up:
            g_tBaseUnderpan.emBaseMoveMode = MoveCircle;
            break;
          
          case Switch_Middle:
            g_tBaseUnderpan.emBaseMoveMode = PID_Debug;
            break;
          
          case Switch_Down:
            g_tBaseUnderpan.emBaseMoveMode = MovePlay;
            break;
        }
        break;
        
      case Switch_Down:
        switch(nowRightSW)
        {
          case Switch_Up:
            g_tBaseUnderpan.emBaseMoveMode = UpperComputerDebug;
            break;
          
          case Switch_Middle:
            g_tBaseUnderpan.emBaseMoveMode = StopMoveNotLock;
            break;
          
          case Switch_Down:
            g_tBaseUnderpan.emBaseMoveMode = MoveFullAuto;
            break;
        }
        break;
        
      case No_Data:
        switch(nowRightSW)
        {
          case No_Data:
            if(tRC_Data.tSelfCheck.dataState == OFFLINE)
            {
              g_tBaseUnderpan.emBaseMoveMode = MoveFullAuto;
//              tBaseGyroData.fInitAngle = 90;
            }
            break;
          default:
            break;
        }
        break;
    }
    //����ֵ����
    lastLeftSW  = nowLeftSW;
    lastRightSW = nowRightSW;
  }
}


/**
  * @brief  �����˶�����
  * @param  None
  * @retval None
  */
static void UnderpanMoveControl(void)
{
  switch(g_tBaseUnderpan.emBaseMoveMode)
  {
    case StopMoveLock:
      g_tBaseUnderpan.fSpeedX = 0;
      g_tBaseUnderpan.fSpeedY = 0;
      g_tBaseUnderpan.fOmega  = 0;
      break;
    
    case MoveUnderRC:
      g_tBaseUnderpan.fTarDrct = tBaseMixLocData.fAngleMix;
      g_tBaseUnderpan.fSpeedX = tRC_Data.ch1*fRC_Sensitivity;
      g_tBaseUnderpan.fSpeedY = tRC_Data.ch2*fRC_Sensitivity;
      g_tBaseUnderpan.fOmega  = tRC_Data.ch3*fRC_Sensitivity;
      g_tBaseUnderpan.s16_WheelSpdLmt = MAX_WHEEL_SPEED;
      break;
    
    case MoveAuto:
//      g_tBaseUnderpan.fTarDrct = 45;
//      MoveAlongTracePoint(1550);
//      RepellentMode();
//      ArmourInjuredRun(tBaseAM, &tBaseAM,700);
//      MoveInLineSegment(testXminP, testXmaxP);
      RunLikeDancing(500,500,0);
//      g_tBaseUnderpan.s16_WheelSpdLmt = AUTO_WHEEL_SPD_MAX;
      break;
    
    case PID_Debug:
//      MoveDebugPID();
      break;
    
    case MovePlay:
      g_tBaseUnderpan.s16_WheelSpdLmt = MAX_WHEEL_SPEED;
      RandomSpin();
      break;
    
    case MoveCircle:
      CircleRun(650);
      CalVxVy(fMoveP[2]);
      break;
    
    case UpperComputerDebug:
      CalVxVy((float *)debugPos);
      break;
    
    case MoveFullAuto:
      g_tBaseUnderpan.s16_WheelSpdLmt = MAX_WHEEL_SPEED;
      AllAutoMove();
      break;
    
    default:
      break;
  }
  CalWheelSpdUnderPowerManage(g_tBaseUnderpan.fSpeedX, \
                              g_tBaseUnderpan.fSpeedY, \
                              g_tBaseUnderpan.fOmega, \
                              g_tBaseUnderpan.fTarDrct);
}


/**
  * @brief  ����3���ٶ�ֵ�������ӵ��ٶ�
  * @param  float vx X�����ϵ��ٶ�
  * @param  float vy Y�����ϵ��ٶ�
  * @param  float omega ��ת���ٶ�
  * @param  float angle ��ʱ���������ĽǶȣ�˳ʱ��Ϊ����
  * @param  int16_t maxspped  ����ٶ�
  * @retval None
  */
static void CalculateWheelSpeed(float vx, float vy, float omega, float radian, int16_t maxspeed)
{
  float   fMaxSpd = 0;
  float   fWheelSpd[4];
  int16_t __packed s16_WheelSpd[4];
  fWheelSpd[W_Right] = -vy*cos(radian)-vx*sin(radian)+omega;
  fWheelSpd[W_Front] = -vx*cos(radian)+vy*sin(radian)-omega;
  fWheelSpd[W_Left] = -(-vy*cos(radian)-vx*sin(radian)-omega);
  fWheelSpd[W_Behind] = -(-vx*cos(radian)+vy*sin(radian)+omega);
  
  fMaxSpd = abs(fWheelSpd[W_Right]);
  if(abs(fWheelSpd[W_Front]) > fMaxSpd)
		fMaxSpd = abs(fWheelSpd[W_Front]);
	if(abs(fWheelSpd[W_Left]) > fMaxSpd)
		fMaxSpd = fWheelSpd[W_Left];
	if(abs(fWheelSpd[W_Behind]) > fMaxSpd)
		fMaxSpd = fWheelSpd[W_Behind];
  
  //�����������ٶ�
  if(fMaxSpd > maxspeed)
  {
		s16_WheelSpd[W_Right]   = (int16_t)(fWheelSpd[W_Right]*maxspeed/fMaxSpd);
		s16_WheelSpd[W_Front]   = (int16_t)(fWheelSpd[W_Front]*maxspeed/fMaxSpd);
		s16_WheelSpd[W_Left]    = (int16_t)(fWheelSpd[W_Left]*maxspeed/fMaxSpd);
		s16_WheelSpd[W_Behind]  = (int16_t)(fWheelSpd[W_Behind]*maxspeed/fMaxSpd);
	}
  else
  {
    s16_WheelSpd[W_Right]   = (int16_t)fWheelSpd[W_Right];
		s16_WheelSpd[W_Front]   = (int16_t)fWheelSpd[W_Front];
		s16_WheelSpd[W_Left]    = (int16_t)fWheelSpd[W_Left];
		s16_WheelSpd[W_Behind]  = (int16_t)fWheelSpd[W_Behind];
  }
  //ǿ���������ظ���
  memcpy((void*)g_tBaseUnderpan.s16_WheelSpd, (void*)s16_WheelSpd, 8);
}


/**
  * @brief  Create a new point
  * @param  uint16_t range(��λ:mm)
  * @retval None
  */
//static void CreateNewPoint(uint16_t range)
//{ 
//  //memcpy ����ͬ��
//  memcpy(s16_CalP,s16_CalP+1,sizeof(uint16_t [3][3]));
//  
//  float fAngle  = atan2((s16_CalP[1][Y_Drct]-s16_CalP[2][Y_Drct]),(s16_CalP[1][X_Drct]-s16_CalP[2][X_Drct]));
//  float fK1     = tan(fAngle - ALPHA / 180.f * 3.14f);
//  float fK2     = tan(fAngle + ALPHA / 180.f * 3.14f);
//  float tmp1 = fK1*s16_CalP[1][X_Drct] - fK1*s16_CalP[2][X_Drct] + s16_CalP[2][Y_Drct] - s16_CalP[1][Y_Drct];
//  float tmp2 = fK2*s16_CalP[1][X_Drct] - fK2*s16_CalP[2][X_Drct] + s16_CalP[2][Y_Drct] - s16_CalP[1][Y_Drct];
//  
//  while(1)
//  {
//    s16_CalP[3][X_Drct] = (RNG_GETNUM()-0.5f) * range;
//    s16_CalP[3][Y_Drct] = (RNG_GETNUM()-0.5f) * range;
//    
//    float tmp3 = fK1*s16_CalP[3][0] - fK1*s16_CalP[2][0] + s16_CalP[2][1] - s16_CalP[3][1];
//    float tmp4 = fK2*s16_CalP[3][0] - fK2*s16_CalP[2][0] + s16_CalP[2][1] - s16_CalP[3][1];
//    if (//(tmp3*tmp1 < 0 || tmp4*tmp2 < 0) && 
//    (GET_HPTNS(s16_CalP[3][X_Drct],s16_CalP[3][Y_Drct],s16_CalP[2][X_Drct],s16_CalP[2][Y_Drct])>700))
//    {
//      t = 0;
//      break;
//    }
//  }
//}


/**
  * @brief  Update Cureve Trace //�������߹켣�õ�Ҫ�ƶ���Ŀ���
  * @param  uint16_t range(��λ:mm)
  * @retval None
  */
//static void UpdateCurveTrace(uint16_t range)
//{
//  fMoveP[0][X_Drct] = fMoveP[1][X_Drct];
//  fMoveP[1][X_Drct] = fMoveP[2][X_Drct];
//  fMoveP[0][Y_Drct] = fMoveP[1][Y_Drct];
//  fMoveP[1][Y_Drct] = fMoveP[2][Y_Drct];
//  
//  fMoveP[2][X_Drct] = s16_CalP[0][X_Drct]*MATCH_RATIO*(-t*t*t+3*t*t-3*t+1)+ \
//                      s16_CalP[1][X_Drct]*MATCH_RATIO*(3*t*t*t-6*t*t+4)+ \
//                      s16_CalP[2][X_Drct]*MATCH_RATIO*(-3*t*t*t+3*t*t+3*t+1)+ \
//                      s16_CalP[3][X_Drct]*MATCH_RATIO*t*t*t;
//  
//  fMoveP[2][Y_Drct] = s16_CalP[0][Y_Drct]*MATCH_RATIO*(-t*t*t+3*t*t-3*t+1)+ \
//                      s16_CalP[1][Y_Drct]*MATCH_RATIO*(3*t*t*t-6*t*t+4)+ \
//                      s16_CalP[2][Y_Drct]*MATCH_RATIO*(-3*t*t*t+3*t*t+3*t+1)+ \
//                      s16_CalP[3][Y_Drct]*MATCH_RATIO*t*t*t;
//  
//  t+=0.0043f;//0.0025f;
//  
//  if(t>=1.0f)
//  {
//    CreateNewPoint(range);
//  }
//}


/**
  * @brief  ����������㳵�ƶ�������������ٶ�
  * @param  float* point  �ƶ���Ŀ���
  * @retval None
  */
static void CalVxVy(float* point)
{
  g_tBaseUnderpan.fSpeedX = tPID_VX.f_cal_pid(&tPID_VX, *(point+0), tBaseMixLocData.s16_xMix);
  g_tBaseUnderpan.fSpeedY = tPID_VY.f_cal_pid(&tPID_VY, *(point+1), tBaseMixLocData.s16_yMix);
  g_tBaseUnderpan.fOmega  = tPID_Angle.f_cal_pid(&tPID_Angle, g_tBaseUnderpan.fTarAngle, tBaseMixLocData.fAngleMix);
}


///**
//  * @brief  ����������ɵ�(����������)��
//  * @param  uint16_t range(��λ��mm)
//  * @retval None
//  */
//static void MoveAlongTracePoint(uint16_t range)
//{
//  UpdateCurveTrace(range);
//  CalVxVy(fMoveP[2]);
//}


//float fAdvanceV = 3.6f;
/**
  * @brief  �ų�ģʽ
  * @param  None
  * @retval None
  */
//uint16_t Circle2BaseLocLen, Origin2BaseLocLen, UAV2BaseLocLen;
//float fResultantCircle2Base, fResultantUAV2Base, fResultantAll2Base;
//float fComponentF_Circle2Base[2], fComponentF_UAV2Base[2], fComponentF_All2Base[2];

//#define CIRCLE_RADIUS   800

//static void RepellentMode(void)
//{
////  uint16_t Circle2BaseLocLen, Origin2BaseLocLen, UAV2BaseLocLen;
//  //������С
////  float fResultantCircle2Base, fResultantUAV2Base, fResultantAll2Base;
//  //�ֽ���
////  float fComponentF_Circle2Base[2], fComponentF_UAV2Base[2], fComponentF_All2Base[2];
//  static int16_t testThing = 0;
//  tUAV_Data.s16_UAV_AbsoluX = tRC_Data.ch1;
//  tUAV_Data.s16_UAV_AbsoluY = tRC_Data.ch2;
//  
//  if(testThing == 0)
//  {
//    testThing ++;
//    fMoveP[1][X_Drct] = tBaseMixLocData.s16_xMix;
//    fMoveP[1][Y_Drct] = tBaseMixLocData.s16_yMix;
//    fMoveP[2][X_Drct] = tBaseMixLocData.s16_xMix;
//    fMoveP[2][Y_Drct] = tBaseMixLocData.s16_yMix;
//  }

//  tUAV_Data.s16_UAV_RelaX = tUAV_Data.s16_UAV_AbsoluX-tBaseMixLocData.s16_xMix;
//  tUAV_Data.s16_UAV_RelaY = tUAV_Data.s16_UAV_AbsoluY-tBaseMixLocData.s16_yMix;
///******************************************************************************/
//  
////  tBaseMixLocData.s16_xMix = fMoveP[1][X_Drct];
////  tBaseMixLocData.s16_yMix = fMoveP[1][Y_Drct];
//  
//  fMoveP[1][X_Drct] = fMoveP[2][X_Drct];
//  fMoveP[1][Y_Drct] = fMoveP[2][Y_Drct];
//  
////  fMoveP[1][X_Drct] = tBaseMixLocData.s16_xMix;
////  fMoveP[1][Y_Drct] = tBaseMixLocData.s16_yMix;
//  
////  Origin2BaseLocLen = GET_HPTNS((float)tBaseMixLocData.s16_xMix, (float)tBaseMixLocData.s16_yMix, 0.0f, 0.0f)+0.001f;
//  Origin2BaseLocLen = GET_HPTNS((float)fMoveP[1][X_Drct], (float)fMoveP[1][Y_Drct], 0.0f, 0.0f)+0.001f;
//  UAV2BaseLocLen = GET_HPTNS((float)tUAV_Data.s16_UAV_RelaX, (float)tUAV_Data.s16_UAV_RelaY, 0.0f, 0.0f)+0.001f;
//  
//  if(Origin2BaseLocLen < CIRCLE_RADIUS)
//  {
//    Circle2BaseLocLen = CIRCLE_RADIUS - Origin2BaseLocLen;
//    
//    //һ�η���
////    fResultantCircle2Base = 100.0f/(float)Circle2BaseLocLen;
////    fResultantUAV2Base  = 500.0f/(float)UAV2BaseLocLen;
//    
//    //�����˻�����㹻Զ����ڿ��Ժܽ�����������ϵ����50���Ĺ�ϵ
//    fResultantCircle2Base = 20000.0f/((float)Circle2BaseLocLen*(float)Circle2BaseLocLen);
//    fResultantUAV2Base  = 1000000.0f/((float)UAV2BaseLocLen*(float)UAV2BaseLocLen);
//    //����Բ��ڶԻ��ص�XY����
////    fComponentF_Circle2Base[X_Drct] = -tBaseMixLocData.s16_xMix/(float)Origin2BaseLocLen*fResultantCircle2Base;
////    fComponentF_Circle2Base[Y_Drct] = -tBaseMixLocData.s16_yMix/(float)Origin2BaseLocLen*fResultantCircle2Base;
//    fComponentF_Circle2Base[X_Drct] = -fMoveP[1][X_Drct]/(float)Origin2BaseLocLen*fResultantCircle2Base;
//    fComponentF_Circle2Base[Y_Drct] = -fMoveP[1][Y_Drct]/(float)Origin2BaseLocLen*fResultantCircle2Base;

//    //���˻��Ի��ص�XY����
//    fComponentF_UAV2Base[X_Drct] = -tUAV_Data.s16_UAV_RelaX/(float)UAV2BaseLocLen*fResultantUAV2Base;
//    fComponentF_UAV2Base[Y_Drct] = -tUAV_Data.s16_UAV_RelaY/(float)UAV2BaseLocLen*fResultantUAV2Base;
//    //�����Ի��ص�XY����
//    fComponentF_All2Base[X_Drct] = fComponentF_Circle2Base[X_Drct]+ fComponentF_UAV2Base[X_Drct];
//    fComponentF_All2Base[Y_Drct] = fComponentF_Circle2Base[Y_Drct]+ fComponentF_UAV2Base[Y_Drct];
//    
//    //��һ�����������ƶ����򣬼���õ��ƶ�����λ��
//    fResultantAll2Base = GET_HPTNS(fComponentF_All2Base[X_Drct], fComponentF_All2Base[Y_Drct], 0, 0)+0.00001f;
//    fMoveP[2][X_Drct] = fMoveP[1][X_Drct] + fComponentF_All2Base[X_Drct]/fResultantAll2Base*fAdvanceV;
//    fMoveP[2][Y_Drct] = fMoveP[1][Y_Drct] + fComponentF_All2Base[Y_Drct]/fResultantAll2Base*fAdvanceV;
//  }
//  else
//  {
//    fMoveP[2][X_Drct] = 0;
//    fMoveP[2][Y_Drct] = 0;
//  }
//  
//  CalVxVy(fMoveP[2]);
//}


/**
  * @brief  ԲȦ�����˻������ã�
  * @param  r �뾶
  * @retval None
  */
static void CircleRun(int16_t r)
{
  static float sfAngle = 0;
  static uint16_t ss16_Period = 2500;
  static int16_t ss16_Num = 0;
  
  fMoveP[2][X_Drct] = cos(sfAngle)*r;
  fMoveP[2][Y_Drct] = sin(sfAngle)*r;
  
  if(tRC_Data.ch2 > 100)
  {
    if(ss16_Period>1000)
      ss16_Period--;
  }
  else if(tRC_Data.ch2 < -100)
  {
    if(ss16_Period<5000)
      ss16_Period++;
  }
  
  ss16_Num = ss16_Num%ss16_Period;
  ss16_Num++;
  sfAngle = (float)ss16_Num/((float)ss16_Period/2)*PI;
}


/**
  * @brief  ʵʱѪ���ı䴦��
  * @param  None
  * @retval None
  */
uint32_t u32_JudgeTimRcrd = 0;
uint32_t u32_BldChngInOneSec = 0;
void RealBloodChangedHandler(void)
{
  switch(tJData.realBloodChangedData.way)
  {
    case 0x0:
      switch(tJData.realBloodChangedData.weakId)
      {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
          if(tJData.realBloodChangedData.value == 500)//�ܵ���Ӣ�۹���
          {
            tBaseAM.u8_EnemyHeroAppr = 1;
            tBaseAM.tAtckRcrd[HERO_RCRD].u32_TimRcrd = HAL_GetTick();
          }
          else//Ӧ���Ǳ�С�ӵ�����
          {
            tBaseAM.u8_EnemyInfAppr = 1;
            tBaseAM.tAtckRcrd[INF_RCRD].u32_TimRcrd = HAL_GetTick();
            tBaseAM.tAtckRcrd[INF_RCRD].s8_WeakId = tJData.realBloodChangedData.weakId;
            tBaseAM.tAtckRcrd[INF_RCRD].fAngRcrd = tBaseMixLocData.fAngleMix;
          }
          break;
        case 0x04:
        case 0x05:
          tBaseAM.u8_EnemyUAVAppr = 1;
          tBaseAM.tAtckRcrd[UAV_RCRD].u32_TimRcrd = HAL_GetTick();
          break;
      }
      break;
    case 0x1:
    case 0x2:
    case 0x3:
    case 0x4:
    case 0x6:
    case 0xa:
      break;
  }
  
  //�Ƿ�Ϊͬһ��
  if(tJData.gameInfo.remainTime != u32_JudgeTimRcrd)
  {
    u32_BldChngInOneSec = 0;
    u32_JudgeTimRcrd = tJData.gameInfo.remainTime;
  }
  
  //����Ѫ��
  switch(tJData.realBloodChangedData.weakId)
  {
    case 0x00:
    case 0x01:
    case 0x02:
    case 0x03:
      u32_BldChngInOneSec += tJData.realBloodChangedData.value;
      break;
    default:
      break;
  }
  
  //�ж��Ƿ������
  if(u32_BldChngInOneSec >= 400)
  {
    tBaseAM.u8_TooManyEnemy = 1;
    tBaseAM.tAtckRcrd[MANY_RCRD].u32_TimRcrd = HAL_GetTick();
  }
}


/**
  * @brief  ��ת����
  * @param  tranSpd��ƽ�Ʒ����ٶ�
  * @param  rttSpd����ת�����ٶȣ�ֻ����ȫ��ת���ã�
  * @param  Mode��0.��ȫ��ת��1.�̶�����ɨ��
  * @retval None
  */
#define CrtRandNumOnBase(base, range) (base+range*(HAL_RNG_GetRandomNumber(&hrng)-2147483648.f)/2147483648.f)
#define ANGLE_CHANGEPROTECT   75
float fDancingAng = 45;       //��������ת����������Ȼ��ƽ��(����������ƽ�ư�)�ĽǶ�
float fDancingAngBase = 45;   //zhuangbi��ƽ�ƽǶȣ�ֻ�������״̬�Ƶ����ᴥ���ĵط�
float fDancingRadian;         //����
int16_t s16_Gyro360 = 0;
int8_t s8_RoughlyDrct;        //���³���:0Ϊ��ǰ��1Ϊ�Һ�2Ϊ���3Ϊ��ǰ
int16_t s16_ScnRttDrct = 1;   //ɨ����ת����
//int8_t s8_CollideWall = -1;
static void RunLikeDancing(int16_t tranSpd, int16_t rttSpd, uint8_t Mode)
{
  static uint8_t  u8_KS_LastSymbol = 10;     //��һ�μ�⵽�ı�־
  static int8_t   s8_Drc = 1;               //��ת����
  static int8_t   s8_PrtctSymbol = 0;       //�Ƿ��ڼ�Ᵽ��
  static float    fAngRcrdCollideWall;      //ײǽ֮��ı�ƽ�Ʒ���Ƕȼ�¼
  static int16_t  s16_AngChange;            //�Ƕȱ仯���ٱ仯���ٽǶȺ��ٴο����Ƕȼ�Ᵽ��������ͬһ��ǽ���������
  
  /*
  �ı��˶���������
  1.�õ��ı�־�Ƿ�ı�
  2.�Ƿ���״̬�仯�ļ�Ᵽ���У��ڱ����������⵽��־��Ϊ0 �����Դ˴α仯��
  */
  if((u8_KS_LastSymbol != u8_KS_Symbol) && (s8_PrtctSymbol == 0 || u8_KS_Symbol != 0))
  {
    s16_Gyro360 = ((int32_t)tBaseMixLocData.fAngleMix)%360;
    s16_Gyro360 = (s16_Gyro360<0)?(s16_Gyro360+360):s16_Gyro360;
    s8_RoughlyDrct = s16_Gyro360/90;
    fAngRcrdCollideWall = tBaseMixLocData.fAngleMix;
    
    //��ȫ�䣬֮������ʲô��û�м�⵽�����Ϊû�н�����
    s8_PrtctSymbol = 1;
    
    switch(u8_KS_Symbol)
    {
      case (1<<KEYENCE_FR):
        fDancingAngBase = (540 - s8_RoughlyDrct*90)%360;
        fDancingAng = fDancingAngBase;
        break;
      case (1<<KEYENCE_RB):
        fDancingAngBase = (450 - s8_RoughlyDrct*90)%360;
        fDancingAng = fDancingAngBase;
        break;
      case (1<<KEYENCE_BL):
        fDancingAngBase = (360 - s8_RoughlyDrct*90)%360;
        fDancingAng = fDancingAngBase;
        break;
      case (1<<KEYENCE_LF):
        fDancingAngBase = (270- s8_RoughlyDrct*90)%360;
        fDancingAng = fDancingAngBase;
        break;
      
      //���濼��һ���ǵ����
      case (1<<KEYENCE_FR|1<<KEYENCE_RB):
        fDancingAngBase = (495 - s8_RoughlyDrct*90)%360;
        fDancingAng = fDancingAngBase;
        break;
      case (1<<KEYENCE_RB|1<<KEYENCE_BL):
        fDancingAngBase = (405 - s8_RoughlyDrct*90)%360;
        fDancingAng = fDancingAngBase;
        break;
      case (1<<KEYENCE_BL|1<<KEYENCE_LF):
        fDancingAngBase = (315 - s8_RoughlyDrct*90)%360;
        fDancingAng = fDancingAngBase;
        break;
      case (1<<KEYENCE_LF|1<<KEYENCE_FR):
        fDancingAngBase = (585 - s8_RoughlyDrct*90)%360;
        fDancingAng = fDancingAngBase;
        break;
      
      //����ֻ����һ���ǵ������Ӧ�ú��ٺ��٣���Ҳ��֪����û�У���д����˵��
      case (1<<KEYENCE_LF|1<<KEYENCE_FR|1<<KEYENCE_RB):
      case (1<<KEYENCE_FR|1<<KEYENCE_RB|1<<KEYENCE_BL):
      case (1<<KEYENCE_RB|1<<KEYENCE_BL|1<<KEYENCE_LF):
      case (1<<KEYENCE_BL|1<<KEYENCE_LF|1<<KEYENCE_FR):
        s8_PrtctSymbol = 0;
        break;
      
      case 0:
        s8_PrtctSymbol = 0;
        fDancingAng = CrtRandNumOnBase(fDancingAngBase,45);
        s8_Drc = -s8_Drc;
        break;
      
      default:
        s8_PrtctSymbol = 0;
        break;
    }
    u8_KS_LastSymbol = u8_KS_Symbol;
    fDancingRadian = Ang2Rad(fDancingAng);
  }
  else if(s8_PrtctSymbol == 1)
  {
    s16_AngChange = tBaseMixLocData.fAngleMix - fAngRcrdCollideWall;
    
    if(xABS(s16_AngChange) > ANGLE_CHANGEPROTECT)
    {
      s8_PrtctSymbol = 0;
    }
  }
  g_tBaseUnderpan.fSpeedX = tranSpd * cos(fDancingRadian);
  g_tBaseUnderpan.fSpeedY = tranSpd * sin(fDancingRadian);
  
  if(Mode == 1)
  {
    if(tBaseMixLocData.fAngleMix>=60)
      s16_ScnRttDrct = -1;
    else if(tBaseMixLocData.fAngleMix<=-125)
      s16_ScnRttDrct = 1;
    
    g_tBaseUnderpan.fTarAngle = tBaseMixLocData.fAngleMix + 36.0f * s16_ScnRttDrct;
    g_tBaseUnderpan.fOmega = tPID_Angle.f_cal_pid(&tPID_Angle, g_tBaseUnderpan.fTarAngle, tBaseMixLocData.fAngleMix);;
  }
  else
    g_tBaseUnderpan.fOmega = s8_Drc * rttSpd;
  g_tBaseUnderpan.fTarDrct = tBaseMixLocData.fAngleMix;
}




/**
  * @brief  ȫ�Զ�
  * @param  None
  * @retval None
  */
//���ͷ��ȴ�������ʼʱ�� ��λS;
#define IMPATIENT_TIMOUT 40
uint32_t u32_StartTimRcrd;      //��ʼ��ʱ����ʱʱ���¼
int32_t s32_ImpatientWaiting = IMPATIENT_TIMOUT;  //��ʱ�ȴ�ʣ��ʱ��
uint32_t u32_InsJudgeCnt = 0;   //ָ���ж�ʱ�䣬��ֹ�����õĽǶ�
uint32_t u32_AngStbCntDwn = 0;  //�Ƕ��ȶ�����
extern SW_StatusTypeDef emStart_CSW;

//test
uint32_t testEnterInsCnt = 0;
static void AllAutoMove(void)
{
  //�жϱ�����ʼ�źţ��յ�������ʼ�źţ�
  if((tGameStart.s8_StartFlg == 1 || tGameStart.s8_TimoutFlg == 1) && tGameStart.s8_EndFlg != 1)
  {//�����������Ƿ�����
    if(tBaseGyroData.tSelfCheck.dataState == DATANORMAL)
    {//����ϵͳ�����Ƿ����� & ʣ��Ѫ���Ƿ����2500
      if(tJData.tSelfCheck.dataState == DATANORMAL && tJData.gameInfo.remainLifeValue>=2500)
      {
        if(tBaseAM.u8_TooManyEnemy == 1)
        {
          if(HAL_GetTick() - tBaseAM.tAtckRcrd[MANY_RCRD].u32_TimRcrd > 20000)
          {
            tBaseAM.u8_TooManyEnemy = 0;
          }
          RunLikeDancing(500,600,0);
        }
        //Ӣ���Ƿ����
        else if(tBaseAM.u8_EnemyHeroAppr == 1)
        {
          if(HAL_GetTick() - tBaseAM.tAtckRcrd[HERO_RCRD].u32_TimRcrd > 20000)
          {
            tBaseAM.u8_EnemyHeroAppr = 0;
          }
          RunLikeDancing(500,600,0);
        }
        else
        {
          //���˻��Ƿ����
          if(tBaseAM.u8_EnemyUAVAppr == 1)
          {
            if(HAL_GetTick() - tBaseAM.tAtckRcrd[UAV_RCRD].u32_TimRcrd > 25000)
            {
              tBaseAM.u8_EnemyUAVAppr = 0;
            }
            RunLikeDancing(550,550,0);
          }
          else
          {
            if(tUpperData.tSelfCheck.dataState == DATANORMAL)
            {
              switch(tUpperData.cmd)
              {
                case STAY_CALM:
                case OBEY_INS:
                  u32_InsJudgeCnt = 200;
                  if((u8_KS_Symbol&(1<<KEYENCE_FR))||(u8_KS_Symbol&(1<<KEYENCE_LF)))
                  {
                    g_tBaseUnderpan.fSpeedX = 0;
                    g_tBaseUnderpan.fSpeedY = 0;
                    g_tBaseUnderpan.fTarAngle = tUpperData.ang;
                    g_tBaseUnderpan.fTarDrct = 0;
                    g_tBaseUnderpan.fOmega  = tPID_Angle.f_cal_pid(&tPID_Angle, g_tBaseUnderpan.fTarAngle, tBaseMixLocData.fAngleMix);
                  }
                  else
                  {
                    g_tBaseUnderpan.fTarAngle = tUpperData.ang;
                    g_tBaseUnderpan.fTarDrct = 0;
                    g_tBaseUnderpan.fOmega  = tPID_Angle.f_cal_pid(&tPID_Angle, g_tBaseUnderpan.fTarAngle, tBaseMixLocData.fAngleMix);
                    g_tBaseUnderpan.fSpeedX = 0;
                    if(xABS(g_tBaseUnderpan.fTarAngle-tBaseMixLocData.fAngleMix) <4)
                    {
                      g_tBaseUnderpan.fSpeedY = 350;
                      u32_AngStbCntDwn++;
                    }
                    else
                    {
                      g_tBaseUnderpan.fSpeedY = 0;
                    }
                  }
                  break;
                case BE_CRAZY:
                  if(u32_InsJudgeCnt>0)
                  {
                    u32_InsJudgeCnt--;
                    g_tBaseUnderpan.fSpeedX = 0;
                    g_tBaseUnderpan.fSpeedY = 0;
                    g_tBaseUnderpan.fTarAngle = tUpperData.ang;
                    g_tBaseUnderpan.fTarDrct = 0;
                    g_tBaseUnderpan.fOmega  = tPID_Angle.f_cal_pid(&tPID_Angle, g_tBaseUnderpan.fTarAngle, tBaseMixLocData.fAngleMix);
                  }
                  else
                  {
                    if(tBaseAM.u8_EnemyInfAppr == 1)
                    {
                      //Ŀ��Ƕȡ�����
                      switch(tBaseAM.tAtckRcrd[INF_RCRD].s8_WeakId)
                      {
                        case 0:
                          g_tBaseUnderpan.fTarAngle = tBaseAM.tAtckRcrd[INF_RCRD].fAngRcrd+135;
                          break;
                        case 1:
                          g_tBaseUnderpan.fTarAngle = tBaseAM.tAtckRcrd[INF_RCRD].fAngRcrd+55;
                          break;
                        case 2:
                          g_tBaseUnderpan.fTarAngle = tBaseAM.tAtckRcrd[INF_RCRD].fAngRcrd-55;
                          break;
                        case 3:
                          g_tBaseUnderpan.fTarAngle = tBaseAM.tAtckRcrd[INF_RCRD].fAngRcrd-135;
                          break;
                      }
                      if((HAL_GetTick() - tBaseAM.tAtckRcrd[INF_RCRD].u32_TimRcrd>5000) || \
                        (xABS(tBaseMixLocData.fAngleMix-g_tBaseUnderpan.fTarAngle)<4))
                      {
                        tBaseAM.u8_EnemyInfAppr = 0;
                      }
                      else
                      {
                        g_tBaseUnderpan.fSpeedX = 0;
                        g_tBaseUnderpan.fSpeedY = 0;
                        g_tBaseUnderpan.fOmega  = tPID_Angle.f_cal_pid(&tPID_Angle, g_tBaseUnderpan.fTarAngle, tBaseMixLocData.fAngleMix);
                      }
                    }
                    else  //����ɨ��
                    {
                      RunLikeDancing(500,0,1);
                    }
                  }
                  break;
                case CANT_STOP:
                  RunLikeDancing(550,550,0);
                  break;
              }
            }
            else
            {
              RunLikeDancing(550,550,0);
            }
          }
        }
      }
      else
      {
        RunLikeDancing(550,550,0);
      }
    }
    else
    {
      RandomSpin();
    }
  }
  else if(tGameStart.s8_EndFlg != 1)//(δ�յ�������ʼ�ź� �ұ���δ����)
  {
    //�����Ƿ�Ϊ��ʼ����ʱ (ֻ���ڲ���ϵͳ���߲ſ�ʼ����ʱ)
    if((emStart_CSW == SW_LOOSEN) && (tJData.tSelfCheck.dataState != DATANORMAL))
    {
      if(s32_ImpatientWaiting <= 0)
      {
        tGameStart.s8_TimoutFlg = 1;
      }
      else
      {
        s32_ImpatientWaiting = IMPATIENT_TIMOUT - ((HAL_GetTick()-u32_StartTimRcrd)/1000);
      }
    }
    else if(emStart_CSW == SW_PRESS || tJData.tSelfCheck.dataState == DATANORMAL)
    {
      u32_StartTimRcrd = HAL_GetTick();
    }
    g_tBaseUnderpan.fSpeedX = 0;
    g_tBaseUnderpan.fSpeedY = 0;
    g_tBaseUnderpan.fOmega  = 0;
  }
  else
  {
    g_tBaseUnderpan.fSpeedX = 0;
    g_tBaseUnderpan.fSpeedY = 0;
    g_tBaseUnderpan.fOmega  = 0;
  }
}


/**
  * @brief  �������,�ű������ǹ���Ҫ�ã�����������û��
  * @param  None
  * @retval None
  */
static void RandomSpin(void)
{
  static uint16_t u16_timCnt1 = 0, u16_timCnt2 = 0;
  static int8_t  u8_Dir = 1;
  g_tBaseUnderpan.fSpeedX = 0;
  g_tBaseUnderpan.fSpeedY = 0;
  
  
  if(u16_timCnt1>(1000/UNDERPAN_PERIOD))
  {
    g_tBaseUnderpan.fOmega = u8_Dir*CreateRandNum(250.0f, 600.0f);
    u16_timCnt1 = 0;
  }
  else
  {
    u16_timCnt1++;
  }
  
  if(u16_timCnt2>(6000/UNDERPAN_PERIOD))
  {
    u8_Dir = (HAL_RNG_GetRandomNumber(&hrng)>2147483648)?1:-1;
    u16_timCnt2 = 0;
  }
  else
  {
    u16_timCnt2++;
  }
}




/**
  * @brief  �߶��˶����ı�MoveP��ֵӰ��켣��һ��ʱ������ı䷽��
  * @param  xֵ��С���Ǹ���
  * @param  xֵ�ϴ���Ǹ���
  * @retval None
  */
//#define UINT_LEN    3     //��λǰ������
//#define C_DRCT_TIM  900   //�ı䷽���ʱ��(���ȿ��ܲ�������)
//float fK, fB, fAng, fRad;
//float fDx = 0, fDy = 0;

//static void MoveInLineSegment(float* pfXmin, float* pfXmax)
//{
////  static float fK, fB, fAng, fRad;
//  static int8_t s8_Drct = 1;
//  static ArmAtckMoveModeTypedef tLastLineMode;
//  static uint16_t u16_TimCnt = 0;
////  float fDx, fDy;
//  if(tBaseAM.emAtackMoveMode != MOVE_NO_ATTACK)
//  {
//    fDx = *(pfXmax+X_Drct) - *(pfXmin+X_Drct);
//    fDy = *(pfXmax+Y_Drct) - *(pfXmin+Y_Drct);
//      
//    //����ı䷽��ʱ��
//    if(u16_TimCnt<C_DRCT_TIM/UNDERPAN_PERIOD)
//    {
//      u16_TimCnt++;
//    }
//    else
//    {
//      s8_Drct = RNG_GETNUM()>0.5f?-1:1;
//      u16_TimCnt = 0;
//    }
//  
//    if(tLastLineMode != tBaseAM.emAtackMoveMode)
//    {
//      fMoveP[2][X_Drct] = CreateRandNum(*(pfXmin+X_Drct),fDx);
//      if(fDx!=0)
//      {
//        fK = fDy/fDx;
//        fB = *(pfXmax+Y_Drct) -fK*(*(pfXmax+X_Drct));
//        fMoveP[2][Y_Drct] = fK*fMoveP[2][X_Drct]+fB;
//      }
//      else
//      {
//        fMoveP[2][Y_Drct] = CreateRandNum(*(pfXmin+Y_Drct),fDy);
//      }
//    }
//    else
//    {
//      fRad = atan2(fDy, fDx);
//      fMoveP[2][X_Drct] = fMoveP[1][X_Drct]+s8_Drct*cos(fRad)*UINT_LEN;
//      fMoveP[2][Y_Drct] = fMoveP[1][Y_Drct]+s8_Drct*sin(fRad)*UINT_LEN;
//      if(fMoveP[2][Y_Drct]>((*(pfXmin+Y_Drct))>(*(pfXmax+Y_Drct))?(*(pfXmin+Y_Drct)):(*(pfXmax+Y_Drct))) || \
//        fMoveP[2][Y_Drct]<((*(pfXmin+Y_Drct))<(*(pfXmax+Y_Drct))?(*(pfXmin+Y_Drct)):(*(pfXmax+Y_Drct))) || \
//        fMoveP[2][X_Drct]>*(pfXmax+X_Drct)||fMoveP[2][X_Drct]<*(pfXmin+X_Drct))
//      {
//        s8_Drct = -s8_Drct;
//        fMoveP[2][X_Drct] += s8_Drct*cos(fRad)*UINT_LEN*2;
//        fMoveP[2][Y_Drct] += s8_Drct*sin(fRad)*UINT_LEN*2;
//      }
//      
//    }
//    fMoveP[1][X_Drct] = fMoveP[2][X_Drct];
//    fMoveP[1][Y_Drct] = fMoveP[2][Y_Drct];
//  }
//  tLastLineMode = tBaseAM.emAtackMoveMode;
//}


/**
  * @brief  �̶�ֱ�߹켣�ܣ�����װ�װ��ܵ��˺������
  * @param  AtckMngTypeDef AtckMng      //������������Ż��ˡ���������û�У���ָ��û����
  * @param  AtckMngTypeDef* p_tBaseAM
  * @param  uint16_t u16_Range
  * @retval None
  */
//static void ArmourInjuredRun(AtckMngTypeDef* p_tBaseAM, uint16_t u16_Range)
//{
//  int16_t s16_Cnt = 0;             //��ͼ���
//  float fMaxNum = 0;

//  //4.5���ж��Ƿ��ܵ�����
//  if(HAL_GetTick() - p_tBaseAM->tAtckRcrd[p_tBaseAM->u16_AtckRcrdCnt].u32_TimRcrd >4500)
//  {
//    p_tBaseAM->u8_EffFlg = 0; 
//    tBaseAM.emAtackMoveMode = MOVE_NO_ATTACK;
//  }
//  else
//  {
//    p_tBaseAM->u8_EffFlg = 1; 
//  }
//  //���ٱ������˰�
//  if(1==p_tBaseAM->u8_EffFlg && p_tBaseAM->u16_EffRcrdCnt >= 1)
//  {
//    //��������,�����ٷֱ���������
//    p_tBaseAM->u16_WeakBldSum = 0;
//    p_tBaseAM->fAvrWeakId = 0;
//    p_tBaseAM->fEachIdWeakPercent[ARMOUR_F] = 0;
//    p_tBaseAM->fEachIdWeakPercent[ARMOUR_L] = 0;
//    p_tBaseAM->fEachIdWeakPercent[ARMOUR_B] = 0;
//    p_tBaseAM->fEachIdWeakPercent[ARMOUR_R] = 0;
//    
//    //��Ѫ�������,����װ���ܵ��������
//    for(int16_t i=0;i<p_tBaseAM->u16_EffRcrdCnt;i++)
//    {
//      s16_Cnt = p_tBaseAM->u16_AtckRcrdCnt - i;
//      if(s16_Cnt < 0)
//      {
//        s16_Cnt += ATTACKRCRD_NUM;
//      }
//      p_tBaseAM->u16_WeakBldSum += p_tBaseAM->tAtckRcrd[s16_Cnt].u16_Val;
//    }
//    
//    //����
//    s16_Cnt = 0;
//    //ƽ����ѪID,����IDȨ�ؼ���
//    for(int16_t i=0;i<p_tBaseAM->u16_EffRcrdCnt;i++)
//    {
//      s16_Cnt = p_tBaseAM->u16_AtckRcrdCnt - i;
//      if(s16_Cnt < 0)
//      {
//        s16_Cnt += ATTACKRCRD_NUM;
//      }
//      p_tBaseAM->fAvrWeakId += p_tBaseAM->tAtckRcrd[s16_Cnt].u16_Val / \
//            ((float)p_tBaseAM->u16_WeakBldSum) * p_tBaseAM->tAtckRcrd[s16_Cnt].s8_WeakId;
//      p_tBaseAM->fEachIdWeakPercent[p_tBaseAM->tAtckRcrd[s16_Cnt].s8_WeakId] += p_tBaseAM->tAtckRcrd[s16_Cnt].u16_Val / \
//            (float)p_tBaseAM->u16_WeakBldSum;
//    }        
//    //������Ȩ��ID
//    p_tBaseAM->s8_MaxWeakId = -1;
//    for(int16_t i=0;i<4;i++)
//    {
//      if(p_tBaseAM->fEachIdWeakPercent[i] > fMaxNum)
//      {
//        fMaxNum = p_tBaseAM->fEachIdWeakPercent[i];
//        p_tBaseAM->s8_MaxWeakId = i;
//      }
//    }
//    p_tBaseAM->u16_WeakBldSum = p_tBaseAM->u16_WeakBldSum;
//    p_tBaseAM->fAvrWeakId = p_tBaseAM->fAvrWeakId;
//    p_tBaseAM->s8_MaxWeakId = p_tBaseAM->s8_MaxWeakId;
//    
//    p_tBaseAM->fEachIdWeakPercent[0] = p_tBaseAM->fEachIdWeakPercent[0];
//    p_tBaseAM->fEachIdWeakPercent[1] = p_tBaseAM->fEachIdWeakPercent[1];
//    p_tBaseAM->fEachIdWeakPercent[2] = p_tBaseAM->fEachIdWeakPercent[2];
//    p_tBaseAM->fEachIdWeakPercent[3] = p_tBaseAM->fEachIdWeakPercent[3];
//    
//    //�ָ�Ĭ������
//    p_tBaseAM->s8_WeakIdSort[0] = 0;
//    p_tBaseAM->s8_WeakIdSort[1] = 1;
//    p_tBaseAM->s8_WeakIdSort[2] = 2;
//    p_tBaseAM->s8_WeakIdSort[3] = 3;
//    
//    float fTmp;
//    //ð������,��0��3���Ӵ�С
//    for(int16_t i=4;i>0;i--)
//    {
//      for(uint8_t j=1;j<i;j++)
//      {
//        if(p_tBaseAM->fEachIdWeakPercent[j-1] < p_tBaseAM->fEachIdWeakPercent[j])
//        {
//          fTmp = p_tBaseAM->fEachIdWeakPercent[j];
//          p_tBaseAM->fEachIdWeakPercent[j] = p_tBaseAM->fEachIdWeakPercent[j-1];
//          p_tBaseAM->fEachIdWeakPercent[j-1] = fTmp;
//          fTmp = p_tBaseAM->s8_WeakIdSort[j];
//          p_tBaseAM->s8_WeakIdSort[j] = p_tBaseAM->s8_WeakIdSort[j-1];
//          p_tBaseAM->s8_WeakIdSort[j-1] = (int8_t)fTmp;
//        }
//      }
//    }
//    
//    //ѡ����װ�װ��ܵ��˺�Ҫ�ܶ��ķ�ʽ
//    switch(p_tBaseAM->s8_WeakIdSort[0])
//    {
//      case ARMOUR_F:
//        if(p_tBaseAM->fEachIdWeakPercent[p_tBaseAM->s8_WeakIdSort[0]] - \
//          p_tBaseAM->fEachIdWeakPercent[p_tBaseAM->s8_WeakIdSort[1]]<=0.1f)
//        {
//          switch(p_tBaseAM->s8_WeakIdSort[1])
//          {
//            case ARMOUR_L:
//              p_tBaseAM->emAtackMoveMode = MOVE_FL_ATTACK;
//              break;
//            case ARMOUR_R:
//              p_tBaseAM->emAtackMoveMode = MOVE_FR_ATTACK;
//              break;
//          }
//        }
//        else
//        {
//          p_tBaseAM->emAtackMoveMode = MOVE_F_ATTACK;
//        }
//        break;
//      case ARMOUR_L:
//        p_tBaseAM->emAtackMoveMode = MOVE_FL_ATTACK;
//        break;
//      case ARMOUR_R:
//        p_tBaseAM->emAtackMoveMode = MOVE_FR_ATTACK;
//        break;
//      default:
//        break;
//    }
//    switch(p_tBaseAM->emAtackMoveMode)
//    {
//      case MOVE_F_ATTACK:
//        fXminPoint[X_Drct] = -u16_Range;
//        fXminPoint[Y_Drct] = u16_Range;
//        fXmaxPoint[X_Drct] = u16_Range;
//        fXmaxPoint[Y_Drct] = -u16_Range;
//        break;
//      case MOVE_FL_ATTACK:
//        fXminPoint[X_Drct] = -u16_Range;
//        fXminPoint[Y_Drct] = -u16_Range;
//        fXmaxPoint[X_Drct] = u16_Range;
//        fXmaxPoint[Y_Drct] = -u16_Range;
//        break;
//      case MOVE_FR_ATTACK:
//        fXminPoint[X_Drct] = -u16_Range;
//        fXminPoint[Y_Drct] = -u16_Range;
//        fXmaxPoint[X_Drct] = -u16_Range;
//        fXmaxPoint[Y_Drct] = u16_Range;
//        break;
//      default:
//        break;
//    }
//  }
//  MoveInLineSegment(fXminPoint, fXmaxPoint); //������ֻ�ǽ�ȥ����һ��tLastLineMode
//}


/**
  * @brief  ���ƹ����ٶ��������������
  * @param  ����x,y�����ٶ�,���ٶ�omega��Ŀ��Ƕ�ֵ
  * @retval None
  */
static void CalWheelSpdUnderPowerManage(float vx, float vy, float omega, float tarAngle)
{
  CalculateWheelSpeed(vx, vy, omega, Ang2Rad(tarAngle), AUTO_WHEEL_SPD_MAX); //������ƶ��ٶ�
  
  //����ʵ��ʹ���ܵ���ֵ
  g_tPowerManage.s16_AllWheelCur = tMotorData[W_Right].s16_RealCur + \
                                  tMotorData[W_Front].s16_RealCur + \
                                  tMotorData[W_Left].s16_RealCur + \
                                  tMotorData[W_Behind].s16_RealCur;
  //����ʣ�����
  g_tPowerManage.s16_ResidueCur = g_tPowerManage.s16_CurLimit - g_tPowerManage.fAllPower;
  //�����������ֵ
  int16_t s16_AllNeededCur = tMotorData[W_Right].s16_NeededCur + \
                            tMotorData[W_Front].s16_NeededCur + \
                            tMotorData[W_Left].s16_NeededCur + \
                            tMotorData[W_Behind].s16_NeededCur;
  
  if(0 == s16_AllNeededCur)
  {
    g_tPowerManage.s16_GivenMaxCur[W_Right] = g_tPowerManage.s16_CurLimit/4;
    g_tPowerManage.s16_GivenMaxCur[W_Front] = g_tPowerManage.s16_CurLimit/4;
    g_tPowerManage.s16_GivenMaxCur[W_Left] = g_tPowerManage.s16_CurLimit/4;
    g_tPowerManage.s16_GivenMaxCur[W_Behind] = g_tPowerManage.s16_CurLimit/4;
  }
  else
  {
    g_tPowerManage.s16_GivenMaxCur[W_Right] = (int16_t)((float)tMotorData[W_Right].s16_NeededCur* \
                                                  (float)g_tPowerManage.s16_CurLimit/ \
                                                  (float)s16_AllNeededCur);
    g_tPowerManage.s16_GivenMaxCur[W_Front] = (int16_t)((float)tMotorData[W_Front].s16_NeededCur* \
                                                  (float)g_tPowerManage.s16_CurLimit/ \
                                                  (float)s16_AllNeededCur);
    g_tPowerManage.s16_GivenMaxCur[W_Left] = (int16_t)((float)tMotorData[W_Left].s16_NeededCur* \
                                                  (float)g_tPowerManage.s16_CurLimit/ \
                                                  (float)s16_AllNeededCur);
    g_tPowerManage.s16_GivenMaxCur[W_Behind] = (int16_t)((float)tMotorData[W_Behind].s16_NeededCur* \
                                                  (float)g_tPowerManage.s16_CurLimit/ \
                                                  (float)s16_AllNeededCur);
  }
  
  //���͵�can����
  sendMessageToCANQueue(CAN_FourMotor_ID, g_tBaseUnderpan.s16_WheelSpd);
  sendMessageToCANQueue(CAN_FourCur_ID, g_tPowerManage.s16_GivenMaxCur);
}


/**
  * @brief  ����PIDģʽ
  * @param  None
  * @retval None
  */
//static void MoveDebugPID(void)
//{
//}


/**
  * @brief  ��λ������
  * @param  None
  * @retval None
  */
//static void ResetGyro(void)
//{
//  sendMessageToCANQueue(CAN_GyroReset_ID, (void*)0);
//}


/**
  * @brief  ��λ�״�
  * @param  None
  * @retval None
  */
//static void ResetRadar(void)
//{
//  sendMessageToCANQueue(CAN_RadarReset_ID, (void*)0);
//}

/**
  * @brief  ����PID����
  * @param  None
  * @retval None
  */
static void Underpan_PID_Configuration(void)
{
  PID_Configuration(
    &tPID_Angle,  //	PID_TypeDef* pid,
    PID_ANGLE,    //	PID_ID   id,
    2000,        //	uint16_t maxout,
    65535,        //	uint16_t intergral_limit,
    1,            //	uint16_t deadband,
    10,           //	uint16_t period,
    0,            //	int16_t  max_err,
    0,            //	float    target,

    10,           //	float 	kp,
    0,            //	float 	ki,
    100,          //	float 	kd
    DB_OUTPUT_ZERO);  //�������ѡ��
  
  PID_Configuration(
    &tPID_VX,     //	PID_TypeDef* pid,
    PID_VX,       //	PID_ID   id,
    2000,         //	uint16_t maxout,
    65535,        //	uint16_t intergral_limit,
    0,            //	uint16_t deadband,
    10,           //	uint16_t period,
    0,            //	int16_t  max_err,
    0,            //	float    target,

    2.5,            //	float 	kp,
    0,            //	float 	ki,
    21,            //	float 	kd
    DB_OUTPUT_LAST);  //�������ѡ��
    
  PID_Configuration(
    &tPID_VY,     //	PID_TypeDef* pid,
    PID_VY,       //	PID_ID   id,
    2000,         //	uint16_t maxout,
    65535,        //	uint16_t intergral_limit,
    0,            //	uint16_t deadband,
    10,           //	uint16_t period,
    0,            //	int16_t  max_err,
    0,            //	float    target,

    2.5,            //	float 	kp,
    0,            //	float 	ki,
    21,            //	float 	kd
    DB_OUTPUT_LAST);  //�������ѡ��
}


/**
  * @brief  Create the UnderpanTask threads
  * @param  osPriority taskPriority
  * @retval None
  */
void underpanTaskThreadCreate(osPriority taskPriority)
{
	osThreadDef(underpanTask, UnderpanTask, taskPriority, 0, 512);
  underpanTaskHandle = osThreadCreate(osThread(underpanTask), NULL);
}
