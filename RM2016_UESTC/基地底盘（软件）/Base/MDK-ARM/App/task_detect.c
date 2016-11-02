/**
  ******************************************************************************
  * @file    task_detect.c
  * @author  EC_Dog
  * @version V0.0
  * @date    2016/08/11
  * @brief   
  *   
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "task_detect.h"
/* Defines -------------------------------------------------------------------*/
#define KEYENCE_DISSHAKETIM 50  //����ʿ����ʱ��
#define DETECT_PERIOD       5   //��������
/* Variables -----------------------------------------------------------------*/
osThreadId detectTaskHandle;

KEYENCE_DataTypeDef KD_List[KEYENCE_LIST_LEN];

uint8_t u8_KS_Symbol = 0;


static void KEYENCE_IO_Init(void);
static void KEYENCE_Detection(void);


/**
  * @brief  ����ʿ�������
  * @param  void const * argument
  * @retval None
  */
void DetectTask(void const * argument)
{
  KEYENCE_IO_Init();
  portTickType xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  for(;;)
  {
    osDelayUntil(&xLastWakeTime,DETECT_PERIOD);
    KEYENCE_Detection();
  }
}

/**
  * @brief  KEYENCE IO Init 
  * @param  None
  * @retval None
  */
static void KEYENCE_IO_Init(void)
{
  KD_List[KEYENCE_FR].tpK_IO = GPIOA;
  KD_List[KEYENCE_FR].K_PIN = GPIO_PIN_4;
  KD_List[KEYENCE_RB].tpK_IO = GPIOC;
  KD_List[KEYENCE_RB].K_PIN = GPIO_PIN_4;
  KD_List[KEYENCE_BL].tpK_IO = GPIOA;
  KD_List[KEYENCE_BL].K_PIN = GPIO_PIN_6;
  KD_List[KEYENCE_LF].tpK_IO = GPIOA;
  KD_List[KEYENCE_LF].K_PIN = GPIO_PIN_7;
}


/**
  * @brief  KEYENCE detection (ÿһ������һ����ã����룩)
  * @param  None
  * @retval None
  */
static void KEYENCE_Detection(void)
{
  GPIO_PinState PinState_Tmp;
  for(uint8_t i=0;i<KEYENCE_LIST_LEN;i++)
  {
    if(KD_List[i].tpK_IO != 0)
    {
      PinState_Tmp = HAL_GPIO_ReadPin(KD_List[i].tpK_IO, KD_List[i].K_PIN);
      //IO��ƽ�ͱ�ʾ����⵽
      if(PinState_Tmp == GPIO_PIN_RESET)
      {
        switch(KD_List[i].emK_State)
        {
          case NOT_DETECTED:
            KD_List[i].emK_State = MAY_DETECTED;
            KD_List[i].u16_VerifyCnt++;
            break;
          case MAY_DETECTED:
            KD_List[i].u16_VerifyCnt++;
            if(KD_List[i].u16_VerifyCnt >= KEYENCE_DISSHAKETIM/DETECT_PERIOD)
            {
              KD_List[i].u16_VerifyCnt = 0;
              KD_List[i].emK_State = DETECTED;
            }
            break;
          case DETECTED:
            break;
        }
      }
      else
      {
        switch(KD_List[i].emK_State)
        {
          case NOT_DETECTED:
            break;
          case MAY_DETECTED:
            KD_List[i].u16_VerifyCnt = 0;
            KD_List[i].emK_State = NOT_DETECTED;
            break;
          case DETECTED:
            KD_List[i].emK_State = NOT_DETECTED;
            break;
        }
      }
    }
    
    if(KD_List[i].emK_State == DETECTED)
    {
      u8_KS_Symbol |= (1<<i);
    }
    else
    {
      u8_KS_Symbol &= (0xFF - (1<<i));
    }
  }
}

/**
  * @brief  Create the DetectTask threads
  * @param  taskPriority ���ȼ�
  * @retval None
  */
void detectTaskThreadCreate(osPriority taskPriority)
{
	osThreadDef(detectTask, DetectTask, taskPriority, 0, 128);
  detectTaskHandle = osThreadCreate(osThread(detectTask), NULL);
}
