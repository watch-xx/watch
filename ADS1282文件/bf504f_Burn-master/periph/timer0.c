#include "timer0.h"
#include "pll.h"


/* �������� - 1 ������������ */
#define DELAY_US_PERIOD   (SCLK_VALUE / TIMER_US_DIVIDER)
#define DELAY_MS_PERIOD   (SCLK_VALUE / TIMER_MS_DIVIDER)
#define DELAY_SEC_PERIOD  (SCLK_VALUE)

/* �������� �� ������������ ��� ������������ - �����������, ����������� �� 15 ������! */
section("L1_code")
void TIMER0_delay(u32 delay, DELAY_TYPE type)
{
    u32 div;

    if (type == DELAY_SEC) {
	div = DELAY_SEC_PERIOD;
    } else if (type == DELAY_MS) {
	div = DELAY_MS_PERIOD;
    } else {
	div = DELAY_US_PERIOD;
    }


    /* ������ �� PERIOD + ����������  */
    *pTIMER_STATUS = TIMIL0;
    *pTIMER0_CONFIG = PERIOD_CNT | IRQ_ENA | PWM_OUT | OUT_DIS;
    *pTIMER0_PERIOD = div * delay;	/* ������ �������� */
    *pTIMER0_WIDTH = 10;
    *pTIMER_ENABLE = TIMEN0;	/* ��������� */
    ssync();

    /* ���� ������������ ������� */
    while (!(*pTIMER_STATUS & TIMIL0)) {
	PLL_sleep(DEV_REG_STATE);	/* ���������: */
	asm("nop;");
	ssync();
    }
    *pTIMER_DISABLE = TIMEN0;	/* ��������� ������  */
    *pTIMER_STATUS = TIMIL0;

    *pTIMER0_CONFIG = 0;
    ssync();
}
