/***************************************************************************
 * ������ 3 ������� ����� ��������� ������� SCLK ����� 2-�� ������������
 * ������� 1 (���� PPS)
 * ���� ������ ����� ��������� �� FLASH
 **************************************************************************/
#include <string.h>
#include "timer3.h"
#include "timer1.h"
#include "pll.h"
#include "irq.h"


#if QUARTZ_CLK_FREQ==(19200000)

/* ������� ����� ������� 4.096 ��� ����� � ���� �����? 
 * �������� ��������� �������� ��� SCLK 24000000 � 48000000
 */
#define QUARTZ_F4MHZ_FREQ    	(4096000)
#define	TIM3_MAX_ERROR		(50)
#define	TIM3_COUNT_MUL  	((long)QUARTZ_F4MHZ_FREQ / 64000)
#define TIM3_COUNT_DIV          ((long)TIMER_PERIOD / 64000)


/************************************************************************
 * 	����������� ����������
 ************************************************************************/
/* ������� ����������  */
static struct {
    s32 timer_sec;		/* ������� */
    s32 curr_err;		/* ������ � ����� ������  */
    s32 last_err;
    s32 last_count;
    s32 curr_count;
    s32 phase_err;
    s32 timer_per;
    u8   sync_num;
    bool sync_req;
    bool sync_ok;
    bool tune_ok;		/* ������� � ���� ����������! */
} timer_sync_struct;


/************************************************************************
 * 	����������� ������� - ����� ���� �����
 ************************************************************************/

/**
 * ������ ������ ������� ������� 4 ��� � ������ EXT_CLK 
 */
#pragma section("FLASH_code")
bool TIMER3_init(void)
{
    /* ��������� ���� PF13 �� ����  */
    *pPORTF_FER |= PF13;
    *pPORTFIO_DIR &= ~PF13;


    /* �������� 2 ������� ��� TMR3 �� PF13  01b - ���� �������3 */
    *pPORTF_MUX |= (1 << 12);	/* 1 �� 12 */
    *pPORTF_MUX &= ~(1 << 13);	/* 0 �� 13 */
    ssync();

    /* ������� ����� + IRQ */
    *pTIMER3_CONFIG = IRQ_ENA | PULSE_HI | EXT_CLK;


    /* ������� �� ����� �������� */
    *pTIMER3_PERIOD = QUARTZ_F4MHZ_FREQ;

    /* �� �� ��������, ���� �� ��������� */
    memset(&timer_sync_struct, 0, sizeof(timer_sync_struct));

    /* ����������������� TIMER3 �� �� 7 ��������� ������ � �������� 1. ����� ����������� ����� ���������� ������ 1 */
    *pSIC_IMASK1 |= IRQ_TIMER3;
    *pSIC_IAR4 &= 0xFFFF0FFF;
    *pSIC_IAR4 |= 0x00000000;

    /* ����� ���������  */
    *pTIMER_ENABLE = TIMEN3;
    ssync();
    return true;
}

/**
 * TIMER3 ISR ������ ��������� ��� ������ 4096000 �����
 * � ������ ������� ������ ������ ���� ��� ������������ �� ��������
 * ������� ������� 1, ����� ��� �������� ���������� ������
 */
section("L1_code")
void TIMER3_ISR(void)
{
   register volatile u32 count;

    /* ���������� ���������� - ����� ����� ���� ���������� ������ */
    *pTIMER_STATUS = TIMIL3;
    ssync();

   /* ������� ����� - ������� ������1 ��� �������� */
    count = (TIMER1_is_run()? *pTIMER1_COUNTER : *pTIMER2_COUNTER);

    /* ���� ���� ������ ���� ������ ����������! 
     * ����� ���������� ���� ������ �� �������� 1-�� ������� */
    if (timer_sync_struct.sync_req) {
	int period; 

	do {
	    if (timer_sync_struct.sync_num == 0) {

		/* �� ������� � ������ ������������ */
		if (count > ((int)TIMER_PERIOD - TIM3_MAX_ERROR * 2))
		    break;	/* ��������� ��� ��� */

		/* �� ������� ��������� ������ �����, ����� ���� �� �������� ������� 1
		 * �.�. �� 24000000 
		 * ���� diff > 0, - ��������� ������ ������� 3, ���� diff < 0 - ���������
		 * ������� ���������� ����, ��� 'timer_sec' ��� ��������!    
		 */
/*		period = (((long)TIMER_HALF_PERIOD - (long) count) * 32 / 375) + QUARTZ_F4MHZ_FREQ;*/ // 48���
/*		period = (((long)TIMER_HALF_PERIOD - (long) count) * 64 / 375) + QUARTZ_F4MHZ_FREQ;*/ // 24���

		period = (((long)TIMER_HALF_PERIOD - (long) count) * TIM3_COUNT_MUL / TIM3_COUNT_DIV) + QUARTZ_F4MHZ_FREQ;

		/* diff �� ����� ���� > �������� ������� ������� 3! */
		*pTIMER3_PERIOD = period;
		ssync();
	    }
	    timer_sync_struct.sync_num++;

	    /* ���������� ������ ������� */
	    if (timer_sync_struct.sync_num == 2) {
		*pTIMER3_PERIOD = QUARTZ_F4MHZ_FREQ;
		ssync();
	    }

	    if (timer_sync_struct.sync_num >= 4) {
		timer_sync_struct.sync_num = 0;
		timer_sync_struct.sync_req = 0;
		timer_sync_struct.sync_ok = true;	/* ���������� */
	    }
	} while (0);
    }

    timer_sync_struct.last_count = timer_sync_struct.curr_count;
    timer_sync_struct.curr_count = count;

    /* ���� ������ error < 0 - ������ 3 ���� ������� ��� ������ 1, �.� �� ������� ������� �����. ���� ������ error > 0 - ������ 3 ���� ��������� ��� ������ 1 */
    timer_sync_struct.last_err = timer_sync_struct.curr_err;
    timer_sync_struct.curr_err = count - timer_sync_struct.last_count;
    timer_sync_struct.timer_per = SCLK_VALUE + timer_sync_struct.curr_err;	/* ������ */

    /* ���� ���� > 0, �� ������ 3 ��������� �� ���� �������� ������� 1. ���� ���� < 0, �� ������ 3 ������� �� ���� �� �������� ����� ������� 1 */
    timer_sync_struct.phase_err = SCLK_VALUE / 2 - count;

    /* ������ ������� � ���� ������ ���� ������ 50 ��� ������� ��� ������� � if() ������ �����! */
    if (!timer_sync_struct.tune_ok && (abs(timer_sync_struct.curr_err) < TIM3_MAX_ERROR) && 
				      (abs(timer_sync_struct.last_err) < TIM3_MAX_ERROR) && 
				      (abs(timer_sync_struct.phase_err) < TIM3_MAX_ERROR)) {
	timer_sync_struct.tune_ok = true;
    }

    /* ������ � ��������� - ������ ������� */
    timer_sync_struct.timer_sec++;
}


/* ��� ��������� OK - ��� ���������� */
#pragma section("FLASH_code")
bool TIMER3_is_shift_ok(void)
{
    return timer_sync_struct.sync_ok;
}

/**
 * ������ ����, ������� � ���������������? 
 */
#pragma section("FLASH_code")
bool TIMER3_is_run(void)
{
    bool res;
    res = (*pTIMER_STATUS & TRUN3) ? true : false;
    return res;
}


/* �������� ���� ������� 3 �� ��������� � ������� 1 */
section("L1_code")
void TIMER3_shift_phase(int sec)
{
    /* ����� �������� */
    timer_sync_struct.sync_req = true;	/* ����� ������ */
    timer_sync_struct.sync_ok = false;	/* �� ���������� ��� */
    timer_sync_struct.timer_sec = sec;  /* �������� ����� � ���� ������ */
}


/**
 * ����� ������������ ������� 3, ���� �� ���������� ������!
 */
section("L1_code")
u32 TIMER3_get_counter(void)
{
    return *pTIMER3_COUNTER;
}

/**
 * ����������� ������ �������!
 */
section("L1_code")
s32 TIMER3_get_period(void)
{
    return timer_sync_struct.timer_per;
}


section("L1_code")
s32 TIMER3_get_freq_err(void)
{
    return timer_sync_struct.curr_err;
}

section("L1_code")
s32 TIMER3_get_phase_err(void)
{
    return timer_sync_struct.phase_err;
}


section("L1_code")
bool TIMER3_is_tuned_ok(void)
{
 return timer_sync_struct.tune_ok;
}

section("L1_code")
s32 TIMER3_get_sec_ticks(void)
{
    return timer_sync_struct.timer_sec;
}


#endif /* QUARTZ_CLK_FREQ==(19200000) - ��� ���������� 19.2 ��� */

