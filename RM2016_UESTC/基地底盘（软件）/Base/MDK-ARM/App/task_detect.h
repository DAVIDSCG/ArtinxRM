#ifndef __TASK_DETECT_H
#define __TASK_DETECT_H

#include "stm32f4xx.h"
#include "cmsis_os.h"

/** 
  * @brief  KEYENCE Location enum definition,����8��
  */
enum
{
  KEYENCE_F   = 0,    //ǰ
  KEYENCE_FR,         //ǰ��
  KEYENCE_R,          //��
  KEYENCE_RB,         //�Һ�
  KEYENCE_B,          //��
  KEYENCE_BL,         //����
  KEYENCE_L,          //��
  KEYENCE_LF,         //��ǰ
  KEYENCE_LIST_LEN,
};

/** 
  * @brief  KEYENCE State enum definition
  */
typedef enum
{
  NOT_DETECTED  = 0,
  MAY_DETECTED  = 1,
  DETECTED      = 2,
}KEYENCE_StateTypeDef;

/** 
  * @brief  KEYENCE State structures definition
  */
typedef struct
{
  GPIO_TypeDef* tpK_IO;
  uint32_t  K_PIN;
  uint16_t  u16_VerifyCnt;
  KEYENCE_StateTypeDef emK_State;
}KEYENCE_DataTypeDef;


void detectTaskThreadCreate(osPriority taskPriority);


#endif
