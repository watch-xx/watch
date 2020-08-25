/* ������ 1 ������ �� ����� ������� ���������������� ����������!
 * ����� ���������, ���� ������� ��������� ������ ������� GNS
 */
#include "timer2.h"
#include "irq.h"


/************************************************************************
 * 	����������� ����������
 ************************************************************************/

/* ������� ��������� + ����� ������������� */
static struct {
    u32 sec;			/* ������� */
    u32 work_time;
} timer_tick_struct;


/**
 * 2-� ��������� ������ ����� ���������, ������ �� �������� � ������ �����
 */
#pragma section("FLASH_code")
bool TIMER2_init(void)
{
	/* ������ �� PERIOD, ����������  */
	*pTIMER2_CONFIG = PERIOD_CNT | IRQ_ENA | PWM_OUT;
	*pTIMER2_PERIOD = TIMER_PERIOD; /* 1 ������� */
	*pTIMER2_WIDTH =  202;	/* �.�. TIMER_PERIOD / TIMER_US_DIVIDER * 4.225 --- 202 */


	/* ������� ��������� */
	timer_tick_struct.sec = 0;
	timer_tick_struct.work_time = 0;

	IRQ_register_vector(TIM2_VECTOR_NUM);	/* ������������ ���������� �� IVG11 */

	/* ����������������� TIMER2 �� ��������� IVG11 */
	*pSIC_IMASK1 |= IRQ_TIMER2;
	*pSIC_IAR4 &= 0xFFFFF0FF;
	*pSIC_IAR4 |= 0x00000400;

	/* ��������� */
	*pTIMER_ENABLE = TIMEN2;
	ssync();
        return true;			/* ��� ��  */
}


/**
 * Timer2 ISR �������� ������ �������!
 */
#pragma section("FLASH_code")
void TIMER2_ISR(void)
{
    /* ���������� ���������� - ����� ����� ���� ���������� ������ */
    *pTIMER_STATUS = TIMIL2;
    ssync();

    timer_tick_struct.sec++;		/* ������� ������ */
    timer_tick_struct.work_time++;	/* ������� �������� ������� - �� ��������� � ���� */
}



/**
 * ���������� ��������� �������, ���������� ������� ���� ������ �������,
 * ����� ��� ���� �� ��� ���������� 
 * ��� ��� �� �������� ��� ������ 1 �������!
 */
#pragma section("FLASH_code")
void TIMER2_set_sec(u32 sec)
{
    timer_tick_struct.sec = sec;
}


/**
 *  �������� ������� ������ �� ������ ������� 
 */
#pragma section("FLASH_code")
u32 TIMER2_get_sec(void)
{
    return timer_tick_struct.sec;
}


/**
 * �������� ������� ����� �� ������� sec + ��
 * return: u64.  PS: ��� ����� �� ������� 32 �������� �� �����
 */
#pragma section("FLASH_code")
s64 TIMER2_get_long_time(void)
{
    register u32 cnt0, sec, cnt1;

    cnt0 = *pTIMER2_COUNTER;
    sec = timer_tick_struct.sec;
    cnt1 = *pTIMER2_COUNTER;
    ssync();

    if (cnt1 < cnt0)
	sec++;

    return  (u64) sec * TIMER_NS_DIVIDER + (u64) cnt1 * TIMER_MS_DIVIDER / TIMER_FREQ_MHZ;
}
