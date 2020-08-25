#ifndef _DAC_H
#define _DAC_H

#include "globdefs.h"


// ��� 12 �� ���������� ����� FFF !!!!! + ����� �� 2 ���� �����
// ��� 14 �� ���������� ����� 3FFF !!!!!
#define		DAC19_MAX_DATA			0x3fff
#define		DAC19_MIN_DATA			0
#define 	DAC19_INIT_DATA	 		(DAC19_MAX_DATA / 2)	/* ��������� ����������� ��� 19 ��� - �������� ��������� */

#define 	DAC4_INIT_DATA	 		11450	/* ��������� ����������� ��� 4��� */
#define		DAC4_MAX_DIFF			2500


/**************************************************************
 * 	��������� � ���� ���, ���. ������������ �� �����
 *************************************************************/


/* ���� ���, �� ���. ������ �������  */
typedef enum {
  DAC_19MHZ = 0,
  DAC_4MHZ,
  DAC_TEST,
} DAC_TYPE_ENUM;


/* ��� �������� �� ������ ��� - ����������� � flash */
typedef struct {
	u16 dac19_data;  /* �� ��� 19.2  */
	u16 dac19_coef;  /* ������ (�����������) */
	u16 dac4_data;   /* �� ��� 4  */
	u16 data_4_coef;  /* ������ (�����������) */
} DEV_DAC_STRUCT;

/* ������ � ����� ��� */
void DAC_write(DAC_TYPE_ENUM, u16);
void DAC_init(void);

#endif /* dac.h */

