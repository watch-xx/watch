#ifndef _TIMER3_H
#define _TIMER3_H

#include "globdefs.h"
#include "ads1282.h"

#define 	TIM3_DELAY_SEC			5	/* 5 ������ */

#if QUARTZ_CLK_FREQ==(19200000)
bool TIMER3_init(void);
bool TIMER3_is_run(void);
bool TIMER3_wait_for_irq(void);
void TIMER3_shift_phase(int);
bool TIMER3_is_shift_ok(void);
u32 TIMER3_get_counter(void);
s32 TIMER3_get_period(void);
void TIMER3_ISR(void);
s32 TIMER3_get_freq_err(void);
s32 TIMER3_get_phase_err(void);
bool TIMER3_is_tuned_ok(void);
s32 TIMER3_get_sec_ticks(void);

/* ��������� ������3 */
#pragma always_inline
IDEF void TIMER3_enable(void)
{
	*pTIMER_ENABLE = TIMEN3;
	ssync();
}

#else

/* ��������� ������3 */
#pragma always_inline
IDEF void TIMER3_enable(void)
{
	/* ��������� ���� PF13 �� ����  */
	*pPORTF_FER |= PF13;
	*pPORTFIO_DIR |= PF13;

	/* �������� 2 ������� ��� TMR3 �� PF13  01b - ���� �������3 */
	*pPORTF_MUX |= (1 << 12);	/* 1 �� 12 */
	*pPORTF_MUX &= ~(1 << 13);	/* 0 �� 13 */
	ssync();

	/* ������� ����� + IRQ */
	*pTIMER3_CONFIG = PERIOD_CNT | PULSE_HI | PWM_OUT;

	/* ������� �� ����� �������� */
	*pTIMER3_PERIOD = 6;
	*pTIMER3_WIDTH = 2;

	/* ����� ���������  */
	*pTIMER_ENABLE = TIMEN3;
	ssync();
}

/**
 * �������� ������� ������������ ��� 
 */
IDEF void TIMER3_change_freq(ADS1282_FreqEn sps)
{
	*pTIMER_DISABLE = TIMEN3;
	*pTIMER3_CONFIG = PERIOD_CNT | PULSE_HI | PWM_OUT;

	if (sps == SPS125) {
		*pTIMER3_PERIOD = 12;
		*pTIMER3_WIDTH = 4;
	} else if (sps == SPS62) {
		*pTIMER3_PERIOD = 24;
		*pTIMER3_WIDTH = 8;
	}
	*pTIMER_ENABLE = TIMEN3;
	ssync();
}


#endif


/* ��������� ������ 3 */
#pragma always_inline
IDEF void TIMER3_disable(void)
{
	*pTIMER_DISABLE = TIMEN3;
	ssync();
}



#endif				/* timer3.h */
