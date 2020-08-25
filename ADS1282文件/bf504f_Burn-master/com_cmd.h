#ifndef _COM_CMD_H
#define _COM_CMD_H

#include "globdefs.h"


/*******************************************************************
 *  �������� �������
 *******************************************************************/
#define		UART_CMD_NONE			0x00	/* ��� ������� */	

/* ����������� �������. ������ �������� ������� ��� ������ ���-������ ��� �� ����������� */
#define 	UART_CMD_COMMAND_PC		0x01	/* ������� � ����� ������ � PC ��� �������� Who Are you? */
#define 	UART_CMD_GET_COUNTS		0x02    /* ����� �������� ������� */
#define		UART_CMD_GET_MODEM_REPLY	0x03	/* �������� ����� ������ - �� ������ ������ */
#define		UART_CMD_DSP_RESET		0x04	/* ����� DSP */
#define 	UART_CMD_DEV_STOP	        0x05	/* ���� ���������  */
#define		UART_CMD_INIT_TEST		0x06	/* ������ ������������ */
#define		UART_CMD_CLR_BUF		0x07	/* �������� ����� � ������� */
#define		UART_CMD_GET_RTC_CLOCK		0x08	/* �������� ����� ������� �� RTC */
#define 	UART_CMD_GET_WORK_TIME		0x09	/* �������� ����� ������ DSP */
#define		UART_CMD_ZERO_ALL_EEPROM	0x0A	/* �������� eeprom */

#define		UART_CMD_GET_DSP_STATUS		0x0C	/* �������� ������: ������ � ������� � ����������� � ��������� */
#define 	UART_CMD_GET_DATA	        0x10	/* ������ ����� ������ */

#define		UART_CMD_POWER_OFF		0x12    /* ���������� ��������: ���������� */
#define		UART_CMD_GET_ADC_CONST		0x13	/* �������� ��������� */
#define		UART_CMD_MODEM_ON		0x14	/* ���������� ����: ����� ON */
#define		UART_CMD_MODEM_OFF		0x15	/* ���������� ����: ����� OFF */
#define 	UART_CMD_BURN_ON		0x16    /* ���������� ����: burn ON */
#define 	UART_CMD_BURN_OFF		0x17    /* ���������� ����: burn OFF */

#define		UART_CMD_GPS_ON			0x18	/* ���������� GPS ��� */
#define		UART_CMD_GPS_OFF		0x19	/* ���������� GPS ���� */
#define		UART_CMD_NMEA_GET		0x1a	/* ������ ������ GPS NMEA */

#define		UART_CMD_MODEM_REQUEST		0x23    /* ������� ������� ������  */
#define		UART_CMD_SET_DSP_ADDR		0x24	/* ���������� ����� ������� */
#define		UART_CMD_SYNC_RTC_CLOCK		0x48	/* ���������� ����� �������. ������������� RTC */
#define		UART_CMD_SET_WORK_TIME		0xC9    /* ���������� ��� ������� ����� ������ */
#define 	UART_CMD_DEV_START	        0x85	/* ����� ��������� */
#define		UART_CMD_SET_ADC_CONST		0x93	/* ���������� / �������� � EEPROM ��������� */

#endif /* _COM_CMD_H */
