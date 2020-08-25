/* � ���� ����� ����� ��������� */
#ifndef _CONFIG_H 
#define _CONFIG_H


/* ������ GNS110 ��� ���?  ����� ���� ������ ��� ����������� ��� ��� Geonode */
#define		GNS110_PROJECT	
//#undef		GNS110_PROJECT	

/* ����� ����� R2a ��� R2b */
//#define       GNS110_R2A_BOARD
//#define       GNS110_R2B_BOARD
#define 	GNS110_R2C_BOARD


#if defined GNS110_R2A_BOARD && defined GNS110_R2B_BOARD && defined GNS110_R2C_BOARD
#error "Please choose only 1 variant GNS110_R2A_BOARD or GNS110_R2B_BOARD or GNS110_R2C_BOARD"
#endif

/* ������� ���������� ��� ������� ��������� ������� - 8.5 V */
#define 	POWER_DROP_MIN			8500

/* 
 * ������ �������� ��������� ����� �������
 */
#define		ENABLE_NEW_SIVY
#undef		ENABLE_NEW_SIVY


/* �������� �������� �� ������  � ��� */
#define		ENABLE_DEBUG_MODEM
//#undef                ENABLE_DEBUG_MODEM


/* ��������� ���������� ������� 2  */
#define		DISABLE_TIM2_OFF
#undef		DISABLE_TIM2_OFF


/* ������� ��� � ������� �������� ������ ��� - ��� � 600 ������  � ��. */
#define 	ADC_DATA_PRINT_COEF		60

#define		TIME_ONE_DAY			86400	

#define		TIME_ONE_WEEK			604800	

#define		TIME_ONE_MONTH			(TIME_ONE_WEEK * 8)			

/* ���������� ������ ��������� ��� ������� 1  */
#define		TIM1_GRANT_ERROR  		3


/* ������� ����� ������ ATMEGA ��� ������  */
#define		WAIT_START_ATMEGA	500

/* ���������� ������ ��������� ��� ������� 3 - 4 ��� ��������, ��� ������� ������� � ���! */
#define		TIM3_GRANT_ERROR  		100

/* ����� �������� ����������� ������ �� WUSB ��� ������  */
#define		MODEM_POWERON_TIME_SEC			(12)


/* ����� ������, � ������� ������� ����� ������� �������� ������� � PC - ������� */
#define 	WAIT_PC_TIME			120
#define 	WAIT_PC_COMMAND			WAIT_PC_TIME

#define 	WAIT_PC_CMD_MS			(WAIT_PC_TIME * 1000)

/* ����� ������, � ������� ������� �� ����� ����� ������������� */
#define		WAIT_TIM3_SYNC			10

/* ����� �������� 3DFIX - 1 ��� */
#define 	WAIT_3DFIX_TIME			3600

/* �������� ������ ����� ���������� */
#define 	TIME_START_AFTER_WAKEUP		120

/* ����������� ����� ������ - 5 �����  */
#define 	RECORD_TIME			300

/* �������� �������� ����� ���������  ����������� - 2 ������� */
#define 	TIME_BURN_AFTER_FINISH		2


/* ����� ����������� �������� - ��� ����� ����� ������������ �  ������� ���������  */
#define 	RELEBURN_TIME			1

/* ����� �������� - ��� ����� ���������� � ������� �������� */
#define 	POPUP_DURATION		60


/* ����� �������� �������� � ��������� ������, ���� ��� ����� - �����������. 10 ����� */
#define 	POWEROFF_NO_LINK_TIME		600



/**   
 * ������������ ������ ��� ������������ ���������
 */

/* �������� ������ ����� ����������� � ������� - ������ �������� �� 8 ���   */
#define		ATMEGA_BF_XCHG_SPEED		1600000

/* �������� SPI ��� ������ � ��� ADS1282 - �������� �� 4.096 ��� */
#define		ADS1282_SPI_SPEED		2000000

/* �������� SPI ��� ������ � �������� ��� */
#define		AD5640_SPEED			1000000

/* ���� ��� ������ ������ NMEA */
#define		UART_SPEED_9600			9600
#define		UART_SPEED_115200		115200



/* ������ ����� �� 4800 */
#define		UART_OLDMODEM_SPEED		4800


/* ����� teledyne �� 9600 */
#define		UART_TELEDYNE_SPEED		9600



#endif				/* config.h  */
