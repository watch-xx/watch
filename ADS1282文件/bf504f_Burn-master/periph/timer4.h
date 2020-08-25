#ifndef _TIMER4_H
#define _TIMER4_H

#include "globdefs.h"
#include "pll.h"

#define 	TIM4_BUF_SIZE			16	/* ������ �� 16 ��������, � ������� ����� �������� ���� */
#define 	TIM4_TICKS_FOR_16_SEC	 	(SCLK_VALUE * TIM4_BUF_SIZE)	/* �� ��� ����� �������� ������ ������ ��� TIM4 */

void TIMER4_config(void);
bool TIMER4_is_run(void);
void TIMER4_init_vector(void);
void TIMER4_del_vector(void);
void TIMER4_ISR(void);
void TIMER4_time_pps_signal(u64 *, u64 *);
void TIMER4_get_tick_buf(u32 *, int);
u32 TIMER4_get_ticks_buf_sum(void);
s64 TIMER4_get_long_time(void);

/* ������ �������� ������� */
#pragma always_inline
IDEF u32 TIMER4_get_period(void)
{
    return *pTIMER4_PERIOD;
}


/* ��������� ������ 4 */
#pragma always_inline
IDEF void TIMER4_enable(void)
{
    *pTIMER_ENABLE = TIMEN4;
    ssync();
}

/* ��������� ������ 4  */
#pragma always_inline
IDEF void TIMER4_disable(void)
{
    *pTIMER_DISABLE = TIMEN4;
    ssync();
}


/* enable Timer4 interrupt */
IDEF void TIMER4_enable_irq(void)
{
    /* ����������������� TIMER4 �� ������ ��������� �� 12 */
    *pSIC_IMASK1 |= IRQ_TIMER4;
    *pSIC_IAR4 &= 0xFFF0FFFF;
    *pSIC_IAR4 |= 0x00050000;	/* TIMER4 IRQ: IVG12 */
    ssync();
}

/* disable Timer4 interrupt */
#pragma always_inline
IDEF void TIMER4_disable_irq(void)
{
    /* ������� TIMER4 */
    *pSIC_IMASK1 &= ~IRQ_TIMER4;
    ssync();
}


/**
 * ����� ������������ ������� 4, ���� �� �������� ����������
 * �������� ������� ���� � ��� ������ 
 */
#pragma inline
IDEF bool TIMER4_wait_for_irq(void)
{
    if (*pTIMER_STATUS & TIMIL4) {
	*pTIMER_STATUS |= TIMIL4;
	ssync();
	return true;		/* ���������! */
    }
    return false;
}


#endif				/* timer4.h */
