#ifndef __JUDGEMENT_LIBS_H
#define __JUDGEMENT_LIBS_H

#include "stm32f4xx_hal.h"
#include "includes.h"

#define JUDGEMENT_BUFLEN 60


/** 
  * @brief  ������ID
  */
typedef enum{
  GameInfoId              = 0x0001, //����������Ϣ
  RealBloodChangedDataId  = 0x0002, //ʵʱѪ���仯����
  RealShootDataId         = 0x0003, //ʵʱ�������
}comIdType;


/** 
  * @brief  eBuffType enum definition,�ĸ�С����״̬
  */
typedef enum{
  BUFF_TYPE_NONE,           //��Ч
  BUFF_TYPE_ARMOR   = 0x01, //������
  BUFF_TYPE_SUPPLY  = 0x04, //��Ѫ��
  BUFF_TYPE_BULLFTS = 0x08, //�ӵ���
}eBuffType;


/** 
  * @brief  FrameHeader structure definition,֡ͷ�ṹ��
  */
typedef __packed struct
{
  uint8_t   sOF;
  uint16_t  dataLenth;
  uint8_t   cRC8;
}tFrameHeader;


/** 
  * @brief  GPS state structures definition,GPS״̬�ṹ��
  */
typedef __packed struct
{
  uint8_t flag; //0 ��Ч�� 1 ��Ч
  uint32_t x;
  uint32_t y;
  uint32_t z;
  uint32_t compass;
}tGpsData;


/** 
  * @brief  Game information structures definition,����������Ϣ�� 0x0001��
  *         �����ݰ��ķ���Ƶ��Ϊ 50Hz
  */
typedef __packed struct
{
  uint32_t remainTime;        /*����ʣ��ʱ�䣨�ӵ���ʱ�����ӿ�ʼ���㣬��λ s��*/
  uint16_t remainLifeValue;   /*������ʣ��Ѫ��*/
  float realChassisOutV;      /*ʵʱ���������ѹ����λ V��*/
  float realChassisOutA;      /*ʵʱ���������������λ A��*/
  
  eBuffType runeStatus[4];    /*�ĸ�С����״̬�������ǣ�(�Ⲩ�����ܡ��ҷ���)
                                0x00 = ��Ч��
                                0x01 = ��������
                                0x04 = ��Ѫ����
                                0x08 = �ӵ���*/
  
  uint8_t bigRune0Status;     /*����� 1 ״̬��
                                0x00 = ��Ч��
                                0x01 = ��Ч���޻�����ռ�죻
                                0x02 = ����������ռ�죻
                                0x03 = ��ռ��*/
  uint8_t bigRune1status;     /*����� 2. ��ֵ����ͬ����� 1*/
  
  uint8_t conveyorBelts0:2;   /*���ʹ��Լ�ͣ��ƺ״̬��0�� 1bits: 0 �Ŵ��ʹ�״̬��
                                0x00 = ���ʹ�ֹͣ
                                0x01 = ���ʹ���ת
                                0x02 = ���ʹ���ת*/
  uint8_t conveyorBelts1:2;   /*2�� 3bits: 1 �Ŵ��ʹ�״̬����ֵ����ͬ���ʹ� 0*/
  
  uint8_t parkingApron0:1;    /*4bits: 0 ��ͣ��ƺ״̬��
                                0�� δ��⵽������;
                                1�� ��⵽������*/
  uint8_t parkingApron1:1;    /*5bits: 1 ��ͣ��ƺ״̬*/
  uint8_t parkingApron2:1;    /*6bits: 2 ��ͣ��ƺ״̬*/
  uint8_t parkingApron3:1;    /*7bits: 3 ��ͣ��ƺ״̬*/
  
  tGpsData gpsData;           /*GPS ״̬�� �� tGpsData �ṹ�嶨��*/
}tGameInfo;


/** 
  * @brief  ʵʱѪ���仯��Ϣ(0x0002)
  */
typedef __packed struct
{
  uint8_t weakId:4;    /*0-3bits: ���仯����Ϊװ���˺�ʱ����ʶװ�� ID
                        0x00: 0 ��װ���� ��ǰ��
                        0x01�� 1 ��װ���� ����
                        0x02�� 2 ��װ���� ����
                        0x03�� 3 ��װ���� �� �ң�
                        0x04: 4 ��װ���� ���� 1��
                        0x05: 5 ��װ���棨 �� 2��*/
  
  uint8_t way:4;       /*4-7bits: Ѫ���仯����
                        0x0: װ���˺����ܵ�������
                        0x1���ӵ����ٿ�Ѫ
                        0x2: �ӵ���Ƶ��Ѫ
                        0x3: ���ʳ���
                        0x4: ģ�����߿�Ѫ
                        0x6: ��ͨ�����Ѫ
                        0xa: ��ȡ��Ѫ���*/
  
  uint16_t value;     /*Ѫ���仯ֵ*/
}tRealBloodChangedData;


/** 
  * @brief  ʵʱ�����Ϣ(0x0003)
  */
typedef __packed struct
{
  float realBulletShootSpeed; //�ӵ�ʵʱ���٣� m/s��
  float realBulletShootFreq;  //�ӵ�ʵʱ��Ƶ�� ��/s��
  float realGolfShootSpeed;   //�߶���ʵʱ����(m/s Ӣ�ۻ�����)
  float realGolfShootFreq;    //�߶���ʵʱ��Ƶ(��/s Ӣ�ۻ�����)
}tRealShootData;


/** 
  * @brief  ����ϵͳ��Ϣ����
  */
typedef struct
{
  tFrameHeader          frameHeader;
  uint16_t              rxCmdId;
  tGameInfo             gameInfo;
  tRealBloodChangedData realBloodChangedData;
  tRealShootData        realShootData;
  SelfCheckTypeDef      tSelfCheck;
}JudgementDataTypedef;


void judgementDataHandler(void);

#endif
