/******************************************************************************** 
 * SPORT0 ������������ ��� ����� � ������� - �������� � ������ 
 * ������������� � ���������� SPI
 ********************************************************************************/
#include "sport0.h"
#include "utils.h"
#include "log.h"
#include "pll.h"

/* ������� ������ ����  ��� ����� � ������� - � ����� config.h */
#define SPORT0_SCLK 		ATMEGA_BF_XCHG_SPEED


/* ����. SPORT0 ��� � ������ SPI */
#pragma section("FLASH_code")
void SPORT0_config(void)
{
	u16 divider;

	/* 1. ����������� ����� SPORT0: 1-� ������� �� ����� F,
	 * PORTF_MUX �� 1-� �������  ���� - ~(3F)
	 * ��������� �� PF3 �������� TFS0 
	 */
	*pPORTF_FER |= (PF0 | PF1 | PF2 | PF3 | PF4 | PF5);
	*pPORTF_MUX &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5));


	/* 2. ����������� ������� ��������, ���� 0 ���������� 0  
	 * TCKFE = 0 
	 * ITCLK = 1 - Internal transmit clock select
	 * LTFS  = 1 - FrameSYNC (CS) Active Low
	 * DTYPE = 0 - ���������� ��������
	 * TLSBIT= 0 - Transmit Bit Order, MSB ������
	 * TFSR  = 1 - �������� ������� �������� ������������� ��� CS
	 * TCKFE = 0 - Clock Falling Edge Select ���� CPOL = 0
	 * TSPEN  ������� �����! 
	 */
	*pSPORT0_TCR1 = TCKFE | LATFS | LTFS | TFSR | ITFS | ITCLK;
	*pSPORT0_TCR2 = 7;	/* ����� �������� ����� 1 */

	/* 3. ����������� ������� �����, ���� 0 ���������� 0  */
	*pSPORT0_RCR1 = RCKFE | LARFS | LRFS | RFSR;	/* RSPEN ������� �����! */
	*pSPORT0_RCR2 = 7;	/* 32 - 1; ����� ��������� 32 ���� - � ��� ����� ��� */
	ssync();

	/* ��������� �������� � ������� ��� � �������� �������� ������� ������ � �������� */
	divider = (u16) (SCLK_VALUE / (SPORT0_SCLK * 2) - 1);

	/* 4. �������� � ��� �������� */
	*pSPORT0_TCLKDIV = divider;
	*pSPORT0_RCLKDIV = divider;

	/* 5. �������� ��� �������� ������������� ??? */
	*pSPORT0_TFSDIV = 7;
	 ssync();
}


/* ������ � ������ � ����� ������� */
#pragma section("L1_code")
u8 SPORT0_write_read(u8 data)
{
	u8 byte;
	s64 t1, t2;

	/* ��������� SPORT */
	SPORT0_enable();

	*pSPORT0_TX16 = (u8) data;
	ssync();	

	t1 = get_msec_ticks();

	/* ���� ������ */
	while (!(*pSPORT0_STAT & RXNE))	{
		t2 = get_msec_ticks();
		if((t2 - t1) > 100) { /* �� 100 ����������� �� ����������� :( */
		  break; // return 0; 
		}		
	}
	
	byte = (u8) * pSPORT0_RX16;

	SPORT0_disable();	/* ��������� SPORT */
	return byte;
}
