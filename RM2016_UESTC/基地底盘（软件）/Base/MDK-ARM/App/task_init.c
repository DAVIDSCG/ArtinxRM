/**
  ******************************************************************************
  * @file    task_init.c
  * @author  EC_Dog
  * @version V1.0
  * @date    2016/06/01
  * @brief   
  * 
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "cmsis_os.h"
#include "bsp_uart.h"
#include "bsp_can.h"
#include "bsp_oled.h"
#include "junru_libs.h"
#include "judgement_libs.h"
#include "task_init.h"
#include "task_underpan.h"
#include "task_check.h"
#include "task_debug.h"
#include "timer_canSender.h"
#include "task_bulletLoading.h"
#include "task_keyPress.h"
#include "task_oled.h"
#include "task_detect.h"
/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
osThreadId initTaskHandle;

extern uint8_t arrRC_Buf[];
extern uint8_t judgementBuf[];
extern uint8_t arrBT_RxBuf[];


void InitTask(void const * argument)
{
  //Ħ���ֵ����ʼ��
  HAL_TIM_PWM_Start(&FWHEEL_TIM, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&FWHEEL_TIM, TIM_CHANNEL_2);
	FWHEEL_TIM.Instance->CCR1=1000;
	FWHEEL_TIM.Instance->CCR2=1000;
  //���׳�ʼ��
  HAL_GPIO_WritePin(BASE_JJ,GPIO_PIN_SET);

  osDelay(500);
  //can�����ж�
  HAL_CAN_Receive_IT(&hcan1, CAN_FIFO0);
  //ң�������ڿ����ж�
  Junru_UART_Receive_IT(&DBUS_HUART, arrRC_Buf, RC_BUF_LEN);
  //����ϵͳ���ڿ����ж�
  Junru_UART_Receive_IT(&JUDGE_HUART, judgementBuf, JUDGEMENT_BUFLEN);
  //�������ڿ����ж�
  Junru_UART_Receive_IT(&BLUET_HUART, arrBT_RxBuf, BT_RXBUF_LEN);
  
  //������������
  underpanTaskThreadCreate(osPriorityAboveNormal);
//  //������������
//  bulletLoadingThreadCreate(osPriorityAboveNormal);
  //�����Լ�����
  checkTaskThreadCreate(osPriorityBelowNormal);
  //������������
  debugTaskThreadCreate(osPriorityNormal);
  //�����������
  keyPressTaskThreadCreate(osPriorityBelowNormal);
  //can��ʱ������
  canSenderTimerCreate();
  //����ʿ�������
  detectTaskThreadCreate(osPriorityAboveNormal);
  //oled����
  oledTaskThreadCreate(osPriorityLow);

  
  for(;;)
  {
    vTaskDelete(initTaskHandle);
  }
}


/**
  * @brief  Create the UnderpanTask threads
  * @param  None
  * @retval None
  */
void initTaskThreadCreate(osPriority taskPriority)
{
	osThreadDef(initTask, InitTask, taskPriority, 0, 256);
  initTaskHandle = osThreadCreate(osThread(initTask), NULL);
}
