/***************************************************************************
 *  ������ 4 ������� ����� ��������� ������� SCLK ����� 2-�� ���������� PPS
 **************************************************************************/
#include <string.h>
#include "timer4.h"
#include "timer1.h"
#include "gps.h"
#include "irq.h"


/************************************************************************
 * 	����������� ����������
 ************************************************************************/

/* �����  �� ������������� */
static struct {
    u32 sec;			/* ������� */
    u32 count;			/* ������� ���������� PPS */
    u32 tick_buf[TIM4_BUF_SIZE];	/* ����� �������� ���� ������� 4 */
    u64 t1;			/* ����� ������� PPS �� ������� ������� */
    u64 t4;			/* ����� ������� PPS �� ���� */
} timer_sync_struct;


/* 4-� ������ - ������� ��������� 1PPS */
#pragma section("FLASH_code")
void TIMER4_config(void)
{
    /* ��������� ���� �� ���� ��� PG2 */
    *pPORTG_FER |= PG2;
    *pPORTGIO_DIR &= ~PG2;

    /* �������� 2 ������� ��� TMR4 �� PG2 */
    *pPORTG_MUX &= ~(1 << 5);	/* 0 �� 5 */
    *pPORTG_MUX |= (1 << 4);	/* 1 �� 4 */

    memset(&timer_sync_struct, 0, sizeof(timer_sync_struct));

    /* ������� ������ ���������� ��������� �� ����� PG2 � �������� ���������� */
    *pTIMER4_CONFIG = WDTH_CAP | PULSE_HI | PERIOD_CNT | IRQ_ENA;
    ssync();

    *pTIMER_ENABLE = TIMEN4;
    ssync();
}


/* ������4 �������?  */
bool TIMER4_is_run(void)
{
    return (*pTIMER_STATUS & TRUN4) ? true : false;
}


/* ���������� ������� � ���������������� ������ */
#pragma section("FLASH_code")
void TIMER4_init_vector(void)
{
    IRQ_register_vector(TIM4_VECTOR_NUM);	/* ���������� �� 4 - �� �������  �� IVG12 */
    TIMER4_enable_irq();
}

/** 
 *  �������� ���������� �������4 � �������� ������� - ������ callback ������� 
 */
#pragma section("FLASH_code")
void TIMER4_del_vector(void)
{
    IRQ_unregister_vector(TIM4_VECTOR_NUM);
}

/**
 * ������� ���������� ��������� SCLK ����� ����� �������� 1PPS 
 * ����� ������ ����� ����� ������������ ������� �������� 1PPS.
 */
section("L1_code")
void TIMER4_ISR(void)
{
    register u32 cnt, per;

    *pTIMER_STATUS = TIMIL4;	/* ���������� ���������� */
    ssync();

    cnt = *pTIMER4_COUNTER;
    per = *pTIMER4_PERIOD;
    ssync();

    /* ����� ������� PPS - ��� ����� ������ (� ���������, ���� �� ����� ���)! */
    timer_sync_struct.t1 = TIMER1_get_long_time();


    timer_sync_struct.sec = gps_get_nmea_time() + 1;	/* ����� PPS + 1 c */
    timer_sync_struct.t4 = (u64) timer_sync_struct.sec * TIMER_NS_DIVIDER + 
                           (u64) cnt * TIMER_NS_DIVIDER / ((per == 0) ? TIMER_PERIOD : per);

    /* �������� �����. ������� ������������ �/� 2-�� ���������� */
    timer_sync_struct.tick_buf[timer_sync_struct.count++ % TIM4_BUF_SIZE] = per;
}

/**
 * �������� ����� � �������. �� ���������, ���� ����������
 */
#pragma section("FLASH_code")
void TIMER4_get_tick_buf(u32 * buf, int num)
{
    int i;

    for (i = 0; i < num; i++) {
	buf[i] = timer_sync_struct.tick_buf[i];	/* ��������� ������ */
	if (i >= TIM4_BUF_SIZE)
	    break;
    }
}


/**
 * �������� ����� �������� ������
 */
#pragma section("FLASH_code")
u32 TIMER4_get_ticks_buf_sum(void)
{
    int i;
    u32 sum = 0;

    for (i = 0; i < TIM4_BUF_SIZE; i++) {
	sum += timer_sync_struct.tick_buf[i];	/* ��������� ������ */
    }
  return sum;
}


/**
 * �������� ������� ����� �� ������� � ������������ 
 * return: s64.  PS: ��� ����� �� ������� 32 �������� �� �����
 * ���� ����� �������� � "���������" ������ - ������� 0
 */
section("L1_code")
s64 TIMER4_get_long_time(void)
{
    register u32 cnt0, sec, cnt1, per;

    cnt0 = *pTIMER4_COUNTER;
    ssync();

    sec = timer_sync_struct.sec;
    cnt1 = *pTIMER4_COUNTER;
    per = *pTIMER4_PERIOD;
    ssync();

    /* ���� ����� ����� � ����� ������ ������������ */
    if ((int) cnt1 < (int) cnt0) {
	sec = timer_sync_struct.sec;	// ��� ��� ���������!
    }

    if (per)
	return ((u64) sec * TIMER_NS_DIVIDER + (u64) cnt1 * TIMER_NS_DIVIDER / per);
    else
	return ((u64) sec * TIMER_NS_DIVIDER + (u64) cnt1 * TIMER_MS_DIVIDER / TIMER_FREQ_MHZ);
}



/**
 * ����� ������� PPS
 */
#pragma section("FLASH_code")
void TIMER4_time_pps_signal(u64 * t1, u64 * t4)
{
    if (t1 != NULL)
	*t1 = timer_sync_struct.t1;
    if (t4 != NULL)
	*t4 = timer_sync_struct.t4;
}
