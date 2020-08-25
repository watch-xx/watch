/*******************************************************************
 * ���� UART - ����� ������ ��������� UART1 
 * � ����� ���������� ������� CALLBACK
 *******************************************************************/
#include <string.h>
#include <stdio.h>
#include "uart1.h"
#include "utils.h"
#include "main.h"
#include "pll.h"		/* ������� SCLK ������� �����!  */
#include "log.h"
#include "irq.h"

#define 	WAIT_MODEM_TIME_MS			250	/* 200 �� - ������������ ����� �������� ������ �� ��������� */

/************************************************************************
 * 	����������� ����������
 ************************************************************************/
static DEV_UART_STRUCT uart1_xchg_struct;
static DEV_UART_COUNTS uart1_cnt;	/* �������� ������� - �� �������� */


/************************************************************************
 * 	����������� �������
 ************************************************************************/


/**
 * Function:    UART1_init
 * Description: �������������� UART �� ����� ������ � ������� ����� �������� �������� ����� �� ������ �� �������
 */
#pragma section("FLASH_code")
bool UART1_init(void *dev)
{
    volatile int temp;
    DEV_UART_STRUCT *comm;
    u16 divider;
    bool res = false;

    IRQ_unregister_vector(DEBUG_COM_VECTOR_NUM);	/* ������� ���������� ��� UART!!! */

    do {
	uart1_xchg_struct.is_open = false;

	if (dev == NULL)
	    break;

	comm = (DEV_UART_STRUCT *) dev;

	uart1_xchg_struct.baud = comm->baud;
	uart1_xchg_struct.rx_call_back_func = (void (*)(u8)) comm->rx_call_back_func;	/* ��������� �� ������� ������ */
	uart1_xchg_struct.tx_call_back_func = (void (*)(void)) comm->tx_call_back_func;	/* ��������� �� ������� ������ */


	if (uart1_xchg_struct.baud <= 0) {
	    log_write_log_file("ERROR: UART1 baud can't be zero\n");
	    break;
	}

	divider = SCLK_VALUE / uart1_xchg_struct.baud;

	*pPORTF_FER |= (PF6 | PF7);	/* ��������� ����� Rx � Tx - ��� ��������� �� PF7 � PF6 */
	*pPORTFIO_DIR |= PF6; 		/* ����� */
	*pPORTFIO_INEN |= PF7;		/* ���� */
	*pPORTF_MUX &= ~(PF6 | PF7);	/* ��������� ������� ����� - �� 0-� MUX. ������ ��� 0 �� 6 � 7-� */
	ssync();

	*pUART1_GCTL = EGLSI | UCEN | EDBO;	/* ���� UART clock + ��������-��������� �� 16 + rerout �� ������� ���������� + EDBO */
	ssync();

	/* �������� � ����������� �� ���������� */
	*pUART1_DLL = divider;
	*pUART1_DLH = divider >> 8;
	*pUART1_LCR = WLS_8;	/* 8N1 */

	/* �������, �� ������� ����� �����������: �������� ����� ����� */
	*pUART1_IER_CLEAR = 0xff;	/* ����������� ��������� - �������� �� ������ ��������� */
	*pUART1_IER_SET = ERBFI | ELSI;	/* �������� ����� ����� + ������ */
	ssync();

	temp = *pUART1_RBR;
	temp = *pUART1_LSR;
	ssync();

	/* ������ �� IVG10 ��� UART1 */
	*pSIC_IMASK0 |= IRQ_UART1_ERR;	/* ������ �� ������� */
	*pSIC_IAR0 &= 0xF0FFFFFF;
	*pSIC_IAR0 |= 0x03000000;	/* STATUS IRQ: IVG10 */
	ssync();
	uart1_xchg_struct.is_open = true;
	res = true;
    } while (0);

    IRQ_register_vector(DEBUG_COM_VECTOR_NUM);	/* ������ ���������� ��� UART �� �������. */
    return res;		/* ��� �� */
}

/* ������� UART  */
#pragma section("FLASH_code")
void UART1_close(void)
{
    *pUART1_GCTL = 0;
    ssync();
    uart1_xchg_struct.is_open = false;
}


/* ������� ������ ����� UART1 */
#pragma section("FLASH_code")
int UART1_write_str(char *str, int len)
{
    int i = -1;

    if (uart1_xchg_struct.is_open) {

	/*  ����� ����� �������. ����� ���������� ��� � ������� ��� � ������ */
	for (i = 0; i < len; i++) {
	    while (!(*pUART1_LSR & THRE));
	    *pUART1_THR = str[i];
	    ssync();
	    uart1_cnt.tx_pack++;
	}
    }
    return i;
}


/* �������� ������ ������ */
#pragma section("FLASH_code")
void UART1_get_count(DEV_UART_COUNTS * cnt)
{
    memcpy(cnt, &uart1_cnt, sizeof(DEV_UART_COUNTS));
}


/** 
 * ������������ ���������� - ��� ����� ��� ��������, �� ������������!
 * ������ ������� �� �����������!
 */
section("L1_code")
void UART1_STATUS_ISR(void)
{
    u16 stat, err;
    volatile u8 byte;		/* �������� ���� */

    /* ������� ������ */
    stat = *pUART1_LSR;
    ssync();

    /* ������� �������� ������ - �������� ��������� ��� ���� ������, ���� 1..4 */
    err = stat & 0x1E;
    if (err) {
	uart1_cnt.rx_stat_err++;
	*pUART1_LSR |= err;	/* ������� ������ (� "���" ?????) */
	byte = *pUART1_RBR;	/* ������ �� ������             */
	ssync();
    }

    if (stat & DR) {
	byte = *pUART1_RBR;
	ssync();

	/* �������� ������� callback */
	if (uart1_xchg_struct.rx_call_back_func != NULL) {
	    uart1_xchg_struct.rx_call_back_func(byte);
	    uart1_cnt.rx_pack++;
	}

    } else if (stat & THRE) {	/*  �������� ����� */
	if (uart1_xchg_struct.tx_call_back_func != NULL) {
	    uart1_xchg_struct.tx_call_back_func();
	    uart1_cnt.tx_pack++;
	}
    }
}
