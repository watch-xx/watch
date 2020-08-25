#include <string.h>
#include <stdio.h>
#include "am0.h"
#include "log.h"
#include "led.h"
#include "uart1.h"
#include "utils.h"
#include "ports.h"

#define OLD_MODEM_BUF_LEN	64	/* ������ ���������� */
#define MODEM_RX_DELAY		350	/* �������� ����� ��������� ��� ������  */
#define MODEM_TX_DELAY		100	/* �������� ����� ��������� ��� ������  */

/************************************************************************
 * 	����������� ����������
 ************************************************************************/
/* ����� � ������� - ��������� */
static struct MODEM_XCHG_STRUCT {
    u8 rx_buf[OLD_MODEM_BUF_LEN];
    u8 rx_beg;			/* ������ ������ */
    u8 rx_cnt;			/* ������� ��������� */
    u8 rx_fin;			/* ����� ������ */
    u8 tx_len;			/* ������� ����������  */
} *modem_xchg_buf;

static int  set_oldmodem_time(TIME_DATE *, TIME_DATE *);
static int  get_oldmodem_time(TIME_DATE *, TIME_DATE *);
static void oldmodem_read_ISR(u8);

#pragma section("FLASH_code")
int am0_init(void)
{
    DEV_UART_STRUCT com_par;
    int res = -1;

    select_modem_module();	/* ����������� �������� �� ���������� �����  */
    delay_ms(25);

    select_analog_power();
    delay_ms(25);

    // �������� �����
    modem_on();
    delay_ms(25);


    /* ��� ������������ ������ ������� ����� �� ����� */
    if (modem_xchg_buf == NULL) {
	modem_xchg_buf = calloc(1, sizeof(struct MODEM_XCHG_STRUCT));
	if (modem_xchg_buf == NULL) {
	    log_write_log_file("ERROR: can't alloc buf for am3\n");
	    return -2;
	}
    } else {
	log_write_log_file("WARN: am0 buf dev already exists\n");
    }

    log_write_log_file("INFO: alloc buf for am0 OK\n");

    /* �������� UART1 init */
    com_par.baud = UART_OLDMODEM_SPEED;
    com_par.rx_call_back_func = oldmodem_read_ISR;
    com_par.tx_call_back_func = NULL;	/* ���  */

    if (UART1_init(&com_par) == true)
	res = 0;

    return res;
}


/**
 * ������������� � ��������� ������� ������
 */
#pragma section("FLASH_code")
int am0_prg_modem(void *par)
{
    char str0[64];
    char str1[64];
    char buf[128];
    long t0, s0, s1;
    int i, res = -1;		// ����� �������� ������. ���� ��� ���������� ��-������ 0
    int j;
    TIME_DATE td, ta;
    long alarm;			// ��������� �����
    DEV_UART_STRUCT com_par;


    /* ���� -1 ��� ���� �����. ����� - ����� �� ��������  */
    if (par == NULL)
	return -1;

    log_write_log_file(">>>>>>>>>>>>>>>>>>>> Init modem AM0 <<<<<<<<<<<<<<<<<<<\n");
    alarm = ((GNS110_PARAM_STRUCT *) par)->gns110_modem_alarm_time;	/* ���������� ����� */

    select_analog_power();
    delay_ms(50);

    // �������� �����
    modem_on();
    delay_ms(50);

    /* ��������� ������������� ������ - ������ ���������� ������� */
    if (am0_init() < 0) {
	return res;
    }

    for (j = 0; j < 3; j++) {

	old_modem_reset();

	/* ���� ~6 ������ ����� ��������� */
	t0 = get_sec_ticks();
	while (get_sec_ticks() - t0 < 6) {
	    LED_toggle(LED_GREEN);
	    delay_ms(50);
	}

	i = 5;
	do {
	    LED_toggle(LED_GREEN);
	    if (get_oldmodem_time(&td, &ta) == 0) {
		break;
	    }
	    delay_ms(250);
	} while (i--);


	/* ���� ~2 ������� */
	t0 = get_sec_ticks();
	while (get_sec_ticks() - t0 < 2) {
	    LED_toggle(LED_GREEN);
	    delay_ms(250);
	}


	i = 5;
	do {
	    LED_toggle(LED_GREEN);

	    t0 = get_sec_ticks();
	    sec_to_td(t0, &td);
	    sec_to_td(alarm, &ta);

	    if (set_oldmodem_time(&td, &ta) == 0) {
		break;
	    }
	    delay_ms(250);
	} while (i--);


	// ������ �����
	old_modem_reset();
	t0 = get_sec_ticks();

	/* ���� ~8 ������ ����� ������ */
	while (get_sec_ticks() - t0 < 8) {
	    LED_toggle(LED_GREEN);
	    delay_ms(250);
	}

	/* ��������� �������!  */
	i = 5;
	res = -1;
	do {
	    LED_toggle(LED_GREEN);
	    if (get_oldmodem_time(&td, &ta) == 0) {
		s0 = td_to_sec(&td);
		s1 = td_to_sec(&ta);

		log_write_log_file("INFO: modem time: %02d-%02d-%04d - %02d:%02d:%02d\n", td.day, td.mon, td.year, td.hour, td.min, td.sec);
		log_write_log_file("INFO: alarm time: %02d-%02d-%04d - %02d:%02d:%02d\n", ta.day, ta.mon, ta.year, ta.hour, ta.min, ta.sec);

		// ���� ������� � 2 ������ �������� - ���������
		if (abs(s0 - get_sec_ticks()) < 120 && s1 == alarm) {
		    res = 0;
		    break;
		}
	    }
	    delay_ms(250);
	} while (i--);

    }

    log_write_log_file("%s: set modem with %d attempts\n", res == 0 ? "SUCCESS" : "FAIL", j + 1);

    am0_close();
    return res;
}

/************************************************************************************************
 * ������������� ���������� �� ������� ������
 *************************************************************************************************/
#pragma section("FLASH_code")
void am0_close(void)
{
    UART1_close();

    if (modem_xchg_buf) {
	free(modem_xchg_buf);	/* ����������� �����  */
	modem_xchg_buf = NULL;
    }
}



/************************************************************************************************
 * ������������� ���������� �� ������� ������
 *************************************************************************************************/
#pragma section("FLASH_code")
static void oldmodem_read_ISR(u8 rx_byte)
{
    modem_xchg_buf->rx_buf[modem_xchg_buf->rx_cnt++ % OLD_MODEM_BUF_LEN] = rx_byte;
}


#pragma section("FLASH_code")
static int oldmodem_write(char *buf, int len)
{
    return UART1_write_str(buf, len);
}


/**
 * ������ ������ ������� �������� ������ ����� (�� 'R')
 */
#pragma section("FLASH_code")
static int get_oldmodem_time(TIME_DATE * st, TIME_DATE * sa)
{
    c8 buf[128];
    int i, pos = 39, res = -1;

    modem_xchg_buf->rx_cnt = 0;
    memset(modem_xchg_buf->rx_buf, 0, OLD_MODEM_BUF_LEN);
    do {
	// 3 ��� ��������
	for (i = 0; i < pos; i++) {
	    oldmodem_write("R", 1);
	    LED_toggle(LED_GREEN);
	    delay_ms(52);
	}

	// ��� ������
	if (modem_xchg_buf->rx_cnt == 0)
	    break;

/*	
	for(i = 0; i < 40; i++) {
         log_write_log_file("    buf[%2d] = %02d\n", i, modem_xchg_buf->rx_buf[i]);
        }

*/
	// ������ ������ ��� ����� ��� �������-����� � ������ R
	memcpy(buf, modem_xchg_buf->rx_buf, pos);
	for (i = 5; i < pos - 5; i++) {
	    // ���� �����
	    if (buf[i] == 'R') {
		pos = i;
		break;
	    }
	}


	// ����� ������
	st->sec = buf[pos + 1];	// �������
	st->min = buf[pos + 2];	// ������
	st->hour = buf[pos + 3];	// ����, ����� �����������
	st->day = buf[pos + 5];	// �����
	st->mon = buf[pos + 6];	// �����
	st->year = buf[pos + 7] + 2000;	// ���


	// ��������� �����
	sa->min = buf[pos + 8];	// ������
	sa->hour = buf[pos + 9];	// ����
	sa->day = buf[pos + 10];	// �����
	sa->mon = buf[pos + 11];	// �����
	sa->year = buf[pos + 12] + 2000;	// ���
/*
	log_write_log_file("Modem time: %02d-%02d-%04d - %02d:%02d:%02d\n", st->day, st->mon, st->year, st->hour, st->min, st->sec);
	log_write_log_file("Alarm time: %02d-%02d-%04d - %02d:%02d:%02d\n", sa->day, sa->mon, sa->year, sa->hour, sa->min, sa->sec);
*/

	if (st->min > 60 || st->hour > 24 || st->day > 31 || st->mon > 12)
	    break;

	res = 0;
    } while (0);
    return res;
}

#pragma section("FLASH_code")
static int set_oldmodem_time(TIME_DATE * st, TIME_DATE * at)
{
    c8 c;
    char buf[32];
    long sec;
    int res = -1;

    modem_xchg_buf->rx_cnt = 0;
    memset(modem_xchg_buf->rx_buf, 0, OLD_MODEM_BUF_LEN);

    oldmodem_write("S", 1);
    delay_ms(52);

    // ���� � ������ ������ S
    for (sec = 0; sec < 10; sec++)
	if (modem_xchg_buf->rx_buf[sec] == 'S') {

	    // ������ ������� �����
	    c = st->min;
	    oldmodem_write(&c, 1);
	    delay_ms(1);

	    c = st->hour;
	    oldmodem_write(&c, 1);
	    delay_ms(1);

	    c = 1;
	    oldmodem_write(&c, 1);
	    delay_ms(1);

	    c = st->day;
	    oldmodem_write(&c, 1);
	    delay_ms(1);

	    c = st->mon;
	    oldmodem_write(&c, 1);
	    delay_ms(1);

	    c = st->year - 2000;
	    oldmodem_write(&c, 1);
	    delay_ms(1);

	    c = 'S';		// ����������
	    oldmodem_write(&c, 1);
	    delay_ms(1);

/////////////////       

	    // Alarm time
	    c = 'A';
	    oldmodem_write(&c, 1);
	    delay_ms(1);

	    c = at->min;
	    oldmodem_write(&c, 1);
	    delay_ms(1);

	    c = at->hour;
	    oldmodem_write(&c, 1);
	    delay_ms(1);

	    c = at->day;
	    oldmodem_write(&c, 1);
	    delay_ms(1);

	    c = at->mon;
	    oldmodem_write(&c, 1);
	    delay_ms(1);

	    c = at->year - 2000;
	    oldmodem_write(&c, 1);
	    delay_ms(1);

	    // Alarm time
	    c = 'E';
	    oldmodem_write(&c, 1);
	    delay_ms(52);
	    res = 0;
	    log_write_log_file("Set modem OK\n");
	    break;
	}
    return res;
}
