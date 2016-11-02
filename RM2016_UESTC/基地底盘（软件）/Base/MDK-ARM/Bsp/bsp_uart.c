/**
  ******************************************************************************
  * @file    bsp_uart.c
  * @author  
  * @version V2.0
  * @date    2016/05/23
  * @brief   
  * 
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stdarg.h"
#include "bsp_uart.h"
#include "judgement_libs.h"
#include "task_check.h"
/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
RC_TypeDef tRC_Data;

//To receive remote D-Bus Data
uint8_t arrRC_Buf[RC_BUF_LEN];
uint8_t arrBT_RxBuf[BT_RXBUF_LEN];
//uint8_t arrBT_TxBuf[BT_TXBUF_LEN] = {};


/**
  * @brief  ���ڻص��������ã�������յ�������
  * @param  RC_Type* rc �����򵥴�����ң��������������
  * @param  uint8_t* buff �������е�����
  * @retval None
  */
void Dma_Callback_RC_Handle(RC_TypeDef* rc, uint8_t* buff)
{
	rc->ch1 = (buff[0] | buff[1]<<8) & 0x07FF;
	rc->ch1 -= 1024;
	rc->ch2 = (buff[1]>>3 | buff[2]<<5 ) & 0x07FF;
	rc->ch2 -= 1024;
	rc->ch3 = (buff[2]>>6 | buff[3]<<2 | buff[4]<<10) & 0x07FF;
	rc->ch3 -= 1024;
	rc->ch4 = (buff[4]>>1 | buff[5]<<7) & 0x07FF;		
	rc->ch4 -= 1024;
	
	rc->switch_left = ( (buff[5] >> 4)& 0x000C ) >> 2;
	rc->switch_right =  (buff[5] >> 4)& 0x0003 ;
	
	rc->mouse.x = buff[6] | (buff[7] << 8);	//x axis
	rc->mouse.y = buff[8] | (buff[9] << 8);
	rc->mouse.z = buff[10]| (buff[11] << 8);
	
	rc->mouse.press_left 	= buff[12];
	rc->mouse.press_right = buff[13];
	
	rc->key_code = buff[14] | buff[15] << 8; //key borad code
	
  //�������ݴ����ǲ��ǲ�Ӧ�����ж���д��ô�ණ����������
//	dealKeyBoardData(rc);
}


/**
  * @brief  �������մ�����
  * @param  None
  * @retval None
  */
__weak void BT_RxDataHandler(void){}


/**
  * @brief  ���ڴ�ӡ����
  * @param  UART_HandleTypeDef *huart
  * @param   void* fmt, ...
  * @retval None
  */
void UART_Printf(UART_HandleTypeDef *huart, const char* fmt, ...)
{
  uint8_t buff[128] = {0};
	uint8_t *p = buff;
	va_list ap;

	va_start(ap, fmt);
	vsprintf((char *)buff, fmt, ap);
	
	uint8_t size=0;
	while(*p++)
  {
		size++;
	}
	
	HAL_UART_Transmit(huart, buff, size, 0xff);
	va_end(ap);
}


/**
  * @brief  ���ڿ����ж�DMA���ջص�����
  * @param  ����ͨ����ַ UART_HandleTypeDef *
  * @retval None
  */
void UART_IdleRxCallback(UART_HandleTypeDef *huart)
{
	if(huart == &DBUS_HUART)
	{
		Dma_Callback_RC_Handle(&tRC_Data, arrRC_Buf);
    UPDATE_CHECKTIME(&tRC_Data);
	}
  else if(huart == &JUDGE_HUART)
  {
    judgementDataHandler();
  }
  else if(huart == &BLUET_HUART)
  {
    BT_RxDataHandler();
  }
}


///**
//  * @brief  ���ڽ��ջص�����
//  * @param  ����ͨ����ַ UART_HandleTypeDef *
//  * @retval None
//  */
//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//	if(huart == &DBUS_HUART)
//	{
//		Dma_Callback_RC_Handle(&tRC_Data, arrRC_Buf);
//	}
//}
