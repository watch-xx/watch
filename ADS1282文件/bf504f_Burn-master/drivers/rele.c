#include "xpander.h"
#include "utils.h"
#include "rele.h"

#define RELE_PORT	PORTD

/**
 * ����-burn2 � "0" ����! 
 */

static struct {
     bool rele_pow;
     bool rele_am;
     bool rele_burn;
     u8   rsvd;
} rele_status_struct;

/* ������������� ���� */
#pragma section("FLASH_code")
void RELE_init(void)
{
	/* ������ ����� D �� ����� */
	pin_set(RELEBURN_CS_PORT, RELEBURN_CS_PIN);
	pin_set(RELEAM_CS_PORT,   RELEAM_CS_PIN);
	pin_set(RELEPOW_CS_PORT,  RELEPOW_CS_PIN);
	pin_clr(RELE_SET_PORT,RELE_SET_PIN);	
	pin_clr(RELE_CLR_PORT,RELE_CLR_PIN);

	rele_status_struct.rele_pow = false;
	rele_status_struct.rele_am = false;
	rele_status_struct.rele_burn = false;
}


/**
 * ��������� ����, ������ ���� ��� ������� ������2 � ��������������� XPANDER !
 * ��������: ����� ����
 * �������:  ����� ��� ���
 */
#pragma section("FLASH_code")
bool RELE_on(RELE_TYPE_ENUM rele)
{
	u8 cs;

	switch (rele) {
	case RELEPOW:
		cs = RELEPOW_CS_PIN;
		rele_status_struct.rele_pow = true;
		break;
	case RELEAM:
		cs = RELEAM_CS_PIN;
		rele_status_struct.rele_am = true;
		break;
	case RELEBURN:
		cs = RELEBURN_CS_PIN;
		rele_status_struct.rele_burn = true;
		break;
	default:
		return false;
	}

	pin_clr(RELE_SET_PORT, RELE_SET_PIN);	/* ������ SET � 0  */
	pin_set(RELE_CLR_PORT, RELE_CLR_PIN);	/* ������ CLR � 1  */
	pin_clr(RELE_PORT, cs);			/* CS ����, ��� �� ����� D  */
	delay_ms(15);				/* �������� ��� ������������ ����� ���� */                              	
	pin_set(RELE_PORT, cs); 		/* CS �����, ��� �� ����� D  */
	pin_set(RELE_SET_PORT, RELE_SET_PIN);	/* Set � 1  */
	pin_set(RELE_CLR_PORT, RELE_CLR_PIN);	/* ������ CLR � 1  */
	return true;
}


/**
 * ���������� ����, ������ ���� ��� ������� ������2 � ��������������� XPANDER !
 * ��������: ����� ����
 * �������:  ����� ��� ���
 */
#pragma section("FLASH_code")
bool RELE_off(RELE_TYPE_ENUM rele)
{
	u8 cs;
return 1;
	switch (rele) {
	case RELEPOW:
		cs = RELEPOW_CS_PIN;
		rele_status_struct.rele_pow = false;
		break;
	case RELEAM:
		cs = RELEAM_CS_PIN;
		rele_status_struct.rele_am = false;
		break;
	case RELEBURN:
		cs = RELEBURN_CS_PIN;
		rele_status_struct.rele_burn = false;
		break;
	default:
		return false;
	}

	pin_set(RELE_SET_PORT, RELE_SET_PIN);	/* Set � 1  */
	pin_clr(RELE_CLR_PORT, RELE_CLR_PIN);	/* ������ CLR � 0  */
	pin_clr(RELE_PORT, cs);			/* CS ���� */
	delay_ms(15);				/* �������� ��� ������������ ����� ���� */
	pin_set(RELE_PORT, cs);			/* CS ����� */
	pin_set(RELE_CLR_PORT, RELE_CLR_PIN);	/* ������ CLR � 1  */
	pin_set(RELE_SET_PORT, RELE_SET_PIN);	/* Set � 1  */


	return true;
}


/**
 * ���� �������� ��������?
 */
#pragma section("FLASH_code")
bool rele_burn_is_on(void)
{
   return rele_status_struct.rele_burn;
}

/**
 * ���� ������ ��������?
 */
#pragma section("FLASH_code")
bool rele_modem_is_on(void)
{
   return rele_status_struct.rele_am;
}
