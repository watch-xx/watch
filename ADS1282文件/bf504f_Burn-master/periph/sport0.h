#ifndef _SPORT_H
#define _SPORT_H

#include "globdefs.h"


/* ����. SPORT0 ��� � ������ SPI */
void SPORT0_config(void);
u8   SPORT0_write_read(u8);


/*********************************************************************************** 
 * ��������� �������� � ���������� - ���������� ������ ����� ��������� � ���������
 ***********************************************************************************/
#pragma always_inline
IDEF void SPORT0_enable(void)
{
  *pSPORT0_TCR1 |= TSPEN; /* ���������� */
  *pSPORT0_RCR1 |= RSPEN; /* �������� */
   asm("ssync;");
}


/*********************************************************************************** 
 * ��������� �������� � ���������� - ����������� ������ ����� �������� � ���������
 ***********************************************************************************/
#pragma always_inline
IDEF void SPORT0_disable(void)
{
  *pSPORT0_TCR1 &= ~TSPEN; /* ���������� */
  *pSPORT0_RCR1 &= ~RSPEN; /* �������� */
   asm("ssync;");
}


#endif /* sport.h */
