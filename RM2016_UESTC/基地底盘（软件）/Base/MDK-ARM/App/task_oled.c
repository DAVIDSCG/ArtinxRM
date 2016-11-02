/**
  ******************************************************************************
  * @file    task_oled.c
  * @author  EC_Dog
  * @version V0.0
  * @date    2016/07/14
  * @brief   
  *   
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "task_oled.h"
#include "task_check.h"
#include "task_detect.h"
#include "task_keyPress.h"
#include "bsp_oled.h"
#include "bsp_can.h"
#include "drv_gui.h"
#include "judgement_libs.h"
#include "task_bulletLoading.h"
#include "includes.h"
/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
osThreadId oledTaskHandle;

//����ͼƬ
extern const uint8_t Bmp_EC_DOG[40][16];
BmpInfoStructTypedDef tTestBmpInfo = {.pBmp = (const uint8_t*)Bmp_EC_DOG,.width = 128,.height = 39};
//ͼƬ�ؼ�
WidgetImageHandle ecdogIH;
//�ĸ�����ʿ���
WidgetCheckBoxHandle dtctCBH[4];
//������
//WidgetCheckBoxHandle colorCBH[2];
//��ʱ����ʱ
WidgetTextHandle  ovrCntDwnTH;
//ʣ��ʱ��label
WidgetTextHandle  rtLabelTH;
//ʣ��ʱ�������
WidgetProgBarHandle rtPBH;
//������Ϣ�ı���չʾ
WidgetTextHandle  infoTH;
//�����ǽǶ��ı���ʾ
WidgetTextHandle  gyroTH;


extern SelfCheckTypeDef*    selfCheckList[DATACHECK_LEN];
extern JudgementDataTypedef tJData;
extern GyroDataTypeDef      tBaseGyroData;
extern int32_t              s32_ImpatientWaiting;
extern MixLocDataTypeDef    tBaseMixLocData;
extern uint8_t u8_KS_Symbol;
extern GameStartTypeDef      tGameStart; //������ʼ

//debug
extern UpperDataTyperDef     tUpperData;
extern uint32_t testEnterInsCnt;
extern SW_StatusTypeDef emStart_CSW;


/**
  * @brief  oled task
  * @param  void const * argument
  * @retval None
  */
void OledTask(void const * argument)
{
  OLED_Init();
  
  ecdogIH = GUI_WidgetImage_Create(0,15,128,40);
  
  dtctCBH[LF] = GUI_WidgetCheckBox_Create(0,0,26,FALSE,"LF");
  dtctCBH[FR] = GUI_WidgetCheckBox_Create(28,0,26,FALSE,"FR");
  dtctCBH[BL] = GUI_WidgetCheckBox_Create(0,12,26,FALSE,"BL");
  dtctCBH[RB] = GUI_WidgetCheckBox_Create(28,12,26,FALSE,"RB");
  
  ovrCntDwnTH = GUI_WidgetText_Create(0,52,48,12);
  GUI_WidgetText_SetRim(ovrCntDwnTH,IS);
  
//  colorCBH[0] = GUI_WidgetCheckBox_Create(0,53,26,FALSE,"R");
//  colorCBH[1] = GUI_WidgetCheckBox_Create(22,53,26,FALSE,"B");
  
  rtLabelTH = GUI_WidgetText_Create(0,27,50,12);
//  GUI_WidgetText_SetRim(rtLabelTH,IS);
  GUI_WidgetText_SetText(rtLabelTH,"J_TimRm:");
  
  rtPBH = GUI_WidgetProgBar_Create(0,36,0,660,53,660,1,IS);
  
  infoTH = GUI_WidgetText_Create(62,0,66,51);
  GUI_WidgetText_SetRim(infoTH,IS);
  
  gyroTH = GUI_WidgetText_Create(48,52,80,12);
  GUI_WidgetText_SetRim(gyroTH,IS);
  
  WidgetTypeDef tWidget[10] = 
  {
    {.handle=(uint32_t)ecdogIH,       .type = WIDGET_IMAGE},
    {.handle=(uint32_t)dtctCBH[LF],   .type = WIDGET_CHECKBOX},
    {.handle=(uint32_t)dtctCBH[FR],   .type = WIDGET_CHECKBOX},
    {.handle=(uint32_t)dtctCBH[BL],   .type = WIDGET_CHECKBOX},
    {.handle=(uint32_t)dtctCBH[RB],   .type = WIDGET_CHECKBOX},
//    {.handle=(uint32_t)colorCBH[Red], .type = WIDGET_CHECKBOX},
//    {.handle=(uint32_t)colorCBH[Blu], .type = WIDGET_CHECKBOX},
    {.handle=(uint32_t)ovrCntDwnTH,   .type = WIDGET_TEXT},
    {.handle=(uint32_t)rtLabelTH,     .type = WIDGET_TEXT},
    {.handle=(uint32_t)rtPBH,         .type = WIDGET_PROGBAR},
    {.handle=(uint32_t)infoTH,        .type = WIDGET_TEXT},
    {.handle=(uint32_t)gyroTH,        .type = WIDGET_TEXT},
  };
  
  //����ͼƬ
  GUI_WidgetImage_SetBmp(ecdogIH, &tTestBmpInfo);
  GUI_WidgetImage_Show(ecdogIH,IS,IS);
  osDelay(500);
  GUI_WidgetImage_Show(ecdogIH,NOT,IS);
  GUI_WidgetDestroy(&tWidget[0]);
  
  //�ı�ZB
  GUI_WidgetText_SetText(infoTH, "OLED\nStarting");
//  for(uint8_t i=0;i<6;i++)
//  {
//    GUI_WidgetText_AddText(infoTH, ".");
//    GUI_WidgetText_Show(infoTH,IS,IS);
//  }
  GUI_WidgetText_AddText(infoTH, "OK!");
  GUI_WidgetText_Show(infoTH,IS,IS);
  osDelay(100);
  
  uint16_t cnt;
  
  for(;;)
  {
    //����ʿ״̬
    for(cnt=KEYENCE_FR;cnt<8;cnt+=2)
    {
      if(u8_KS_Symbol&(1<<cnt))
        GUI_WidgetCheckBox_SetVal(dtctCBH[cnt/2],TURE);
      else
        GUI_WidgetCheckBox_SetVal(dtctCBH[cnt/2],FALSE);
    }
    //����ϵͳʣ��ʱ���������ʾ
    GUI_WidgetProgBar_SetVal(rtPBH, tJData.gameInfo.remainTime);
    //����ʱ�ı���ʾ
    if(tGameStart.s8_StartFlg == 1)
    {
      GUI_WidgetText_SetText(ovrCntDwnTH, "Start!", s32_ImpatientWaiting);
    }
    else if(tGameStart.s8_TimoutFlg == 1)
    {
      GUI_WidgetText_SetText(ovrCntDwnTH, "Timout!", s32_ImpatientWaiting);
    }
    else
    {
      GUI_WidgetText_SetText(ovrCntDwnTH, "CntD:%d", s32_ImpatientWaiting);
    }
    //������Ϣ�ı���
//    //debug
//    GUI_WidgetText_SetText(infoTH, "%f\n",tUpperData.ang);
//    GUI_WidgetText_AddText(infoTH, "%d\n",testEnterInsCnt);
//    //debug
    GUI_WidgetText_SetText(infoTH, "OFFLINE:\n");
    if(selfCheckList[GYRO_DATACHECK]->dataState != DATANORMAL)
    {
      GUI_WidgetText_AddText(infoTH, "Gyro ");
    }
    if(selfCheckList[JUDGE_DATACHECK]->dataState != DATANORMAL)
    {
      GUI_WidgetText_AddText(infoTH, "Judg ");
    }
    if(selfCheckList[MOTOR_L_DATACHECK]->dataState != DATANORMAL)
    {
      GUI_WidgetText_AddText(infoTH, "M_L  ");
    }
    if(selfCheckList[MOTOR_R_DATACHECK]->dataState != DATANORMAL)
    {
      GUI_WidgetText_AddText(infoTH, "M_R  ");
    }
    if(selfCheckList[MOTOR_F_DATACHECK]->dataState != DATANORMAL)
    {
      GUI_WidgetText_AddText(infoTH, "M_F  ");
    }
    if(selfCheckList[MOTOR_B_DATACHECK]->dataState != DATANORMAL)
    {
      GUI_WidgetText_AddText(infoTH, "M_B  ");
    }
    if(selfCheckList[PIT_DATACHECK]->dataState != DATANORMAL)
    {
      GUI_WidgetText_AddText(infoTH, "PIT  ");
    }
    if(selfCheckList[YAW_DATACHECK]->dataState != DATANORMAL)
    {
      GUI_WidgetText_AddText(infoTH, "YAW  ");
    }
    if(selfCheckList[UPPER_DATACHECK]->dataState != DATANORMAL)
    {
      GUI_WidgetText_AddText(infoTH, "UPP  ");
    }
    
    //����״̬��ʾ
    if(emStart_CSW == SW_LOOSEN)
    {
      GUI_WidgetText_AddText(infoTH, "\nCDK_ON!!!");
    }
    else if(emStart_CSW == SW_PRESS)
    {
      GUI_WidgetText_AddText(infoTH, "\nCDK_OFF..");
    }
    
//    GUI_WidgetText_AddText(infoTH, "\n%d's",s32_ImpatientWaiting);
    
    //Gyro
    GUI_WidgetText_SetText(gyroTH,"Gyro:%.4f",tBaseMixLocData.fAngleMix);
    
    GUI_WidgetFresh(tWidget,10,NOT);
    OLED_RefreshGram();
    osDelay(4);
  }
}

/**
  * @brief  Create the Oled Task threads
  * @param  None
  * @retval None
  */
void oledTaskThreadCreate(osPriority taskPriority)
{
	osThreadDef(oledTask, OledTask, taskPriority, 0, 512);
  oledTaskHandle = osThreadCreate(osThread(oledTask), NULL);
}
