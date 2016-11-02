/**
  ******************************************************************************
  * @file    task_keyPress.c
  * @author  EC_Dog
  * @version V0.0
  * @date    2016/07/013
  * @brief   
  * 
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "task_keyPress.h"
/* Defines -------------------------------------------------------------------*/
//#define TRIG_SW   GPIOB, GPIO_PIN_11          //��������
//#define CHOSE_SW  GPIOB, GPIO_PIN_10          //ѡ�񿪹�
#define START_SW  GPIOC, GPIO_PIN_12           //��ʼ����
/* Variables -----------------------------------------------------------------*/
osThreadId keyPressTaskHandle;

//SW_StatusTypeDef emLoadChose_CSW;         //ѡ��Ӣ�ۻ��ǲ�������
//SW_StatusTypeDef emLbrLdCmplt_TSW;        //�˹��������״̬�ı䴥��
SW_StatusTypeDef emStart_CSW;


/**
  * @brief  �����������
  * @param  void const * argument
  * @retval None
  */
void KeyPressTask(void const * argument)
{
  portTickType xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  for(;;)
  {
    osDelayUntil(&xLastWakeTime, 10);
    //������������
//    switch(HAL_GPIO_ReadPin(TRIG_SW))
//    {
//      case GPIO_PIN_RESET:
//        switch(emLbrLdCmplt_TSW)
//        {
//          case SW_LOOSEN:
//            emLbrLdCmplt_TSW = SW_MAYPRESS;
//            break;
//          case SW_MAYPRESS:
//            emLbrLdCmplt_TSW = SW_PRESS;
//            break;
//          case SW_MAYLOOSEN:
//            emLbrLdCmplt_TSW = SW_LOOSEN;
//            break;
//          case SW_PRESS:
//            emLbrLdCmplt_TSW = SW_PRESS;
//            break;
//        }
//        break;
//      case GPIO_PIN_SET:
//        switch(emLbrLdCmplt_TSW)
//        {
//          case SW_LOOSEN:
//            emLbrLdCmplt_TSW = SW_LOOSEN;
//            break;
//          case SW_MAYPRESS:
//            emLbrLdCmplt_TSW = SW_PRESS;
//            break;
//          case SW_MAYLOOSEN:
//            emLbrLdCmplt_TSW = SW_LOOSEN;
//            break;
//          case SW_PRESS:
//            emLbrLdCmplt_TSW = SW_MAYLOOSEN;
//            break;
//        }
//        break;
//    }
    
    //ѡ�񿪹�
    switch(HAL_GPIO_ReadPin(START_SW))
    {
      case GPIO_PIN_RESET:
        switch(emStart_CSW)
        {
          case SW_LOOSEN:
            emStart_CSW = SW_MAYPRESS;
            break;
          case SW_MAYPRESS:
            emStart_CSW = SW_PRESS;
            break;
          case SW_MAYLOOSEN:
            emStart_CSW = SW_LOOSEN;
            break;
          case SW_PRESS:
            emStart_CSW = SW_PRESS;
            break;
        }
        break;
      case GPIO_PIN_SET:
        switch(emStart_CSW)
        {
          case SW_LOOSEN:
            emStart_CSW = SW_LOOSEN;
            break;
          case SW_MAYPRESS:
            emStart_CSW = SW_PRESS;
            break;
          case SW_MAYLOOSEN:
            emStart_CSW = SW_LOOSEN;
            break;
          case SW_PRESS:
            emStart_CSW = SW_MAYLOOSEN;
            break;
        }
        break;
    }
  }
}

/**
  * @brief  Create the BeepTask threads
  * @param  taskPriority ���ȼ�
  * @retval None
  */
void keyPressTaskThreadCreate(osPriority taskPriority)
{
	osThreadDef(keyPressTask, KeyPressTask, taskPriority, 0, 128);
  keyPressTaskHandle = osThreadCreate(osThread(keyPressTask), NULL);
}

