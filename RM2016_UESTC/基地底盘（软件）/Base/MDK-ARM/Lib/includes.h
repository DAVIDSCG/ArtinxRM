//��Щ������ʵ���ǲ�֪��Ҫ��������ȽϺ�

#ifndef __INCLUDES_H
#define __INCLUDES_H


#ifndef UESTC__ONE_POINT_FIVE_S__NO_1
  #error "Cofidence!!!"
#endif

#if defined DEBUG_MODE
  #warning "You are debuging!!!"
#endif


#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

#define PI        3.141592653589793f
//����ֵ
#define xABS(m)  ((m)>=0?(m):-(m))
//�Ƕ�&���� �໥ת��
#define Ang2Rad(m)  (m/180.0f*PI)
#define Rad2Ang(n)  (n/PI*180.f)

#define UPDATE_CHECKTIME(n) do{ \
  (n)->tSelfCheck.lastUpdateTime = (n)->tSelfCheck.recentUpdateTime; \
  (n)->tSelfCheck.recentUpdateTime = HAL_GetTick(); \
  (n)->tSelfCheck.upDateTimeLen = (n)->tSelfCheck.recentUpdateTime - (n)->tSelfCheck.lastUpdateTime; \
  }while(0)


/** 
  * @brief  Data State enum definition
  */
typedef enum
{
  OFFLINE = 0x00,   //�豸����
  RESETING,         //�豸��λ
  DATANORMAL,       //����ֵ����
  DATAEXCEPTION,    //����ֵ�쳣
  DATATIMEOUT,      //����ʱ�䳬��
}DataStateTypeDef;


/** 
  * @brief  Selfcheck structures definition
  */
typedef struct
{
  TickType_t        lastUpdateTime;     //��һ�θ���ʱ��
  TickType_t        recentUpdateTime;   //�������ʱ��
  TickType_t        upDateTimeLen;      //����ʱ��
  DataStateTypeDef  dataState;          //����״̬
}SelfCheckTypeDef;

#endif
