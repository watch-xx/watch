/* ��������� �������: ������� ����, ���������� �������, ������ �� SD ����� */
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "utils.h"
#include "main.h"
#include "timer1.h"
#include "timer2.h"
#include "ads1282.h"
#include "ports.h"
#include "gps.h"
#include "log.h"
#include "dac.h"
#include "irq.h"
#include "led.h"
#include "rsi.h"
#include "uart1.h"		/* ����� ��� �� SD ����� ��� � UART */
#include "pll.h"
#include "eeprom.h"
#include "ff.h"
#include "version.h"

#define   	MAX_FILE_SIZE		1024
#define   	MAX_START_NUMBER	100	/* ������������ ����� �������� */
#define	  	MAX_TIME_STRLEN		26	/* ����� ������ �� ��������  */
#define   	MAX_LOG_FILE_LEN	134217728	/* 128 ����� */
#define   	MAX_FILE_NAME_LEN	31	/* ����� ����� ������� ��� ���������� � '\0' */
#define		LOCK_FILE_NAME		"lock.fil"
#define		PARAM_FILE_NAME		"recparam.cfg"
#define 	ERROR_LOG_NAME		"error.log"
/*************************************************************************************
 *     ��� ���������� �� ������ � ������ ������ 
 *************************************************************************************/
static FATFS fatfs;		/* File system object - ����� ������ �� global? ���! */
static DIR dir;			/* ���������� ��� ������� ��� ����� - ����� ������ �� global? */
static FIL log_file;		/* File object */
static FIL adc_file;		/* File object ��� ��� */
static FIL env_file;		/* File object ��� ���������� �����: ����������� � �� */
static ADC_HEADER adc_hdr;	/* ��������� ����� ������� ��� */
static int num_log = 0;		/* ����� ��� �����, ���� ����� �� �����  */

/**
 * ��� ������ ������� � ��� - �������� �����, ���� ������1 �� �������, �� �� RTC
 */
#pragma section("FLASH_code")
static void time_to_str(char *str)
{
	TIME_DATE t;
	u64 msec;
	char sym;
	int stat;

	msec = get_msec_ticks();	/* �������� ����� �� ������� 1 */

	stat = get_clock_status();
	if (stat == CLOCK_RTC_TIME)
		sym = 'R';
	else if (stat == CLOCK_NO_GPS_TIME)
		sym = 'G';
	else if (stat == CLOCK_PREC_TIME)
		sym = 'P';
	else
		sym = 'N';

	/* ���������� ���� � �������: P: 09-07-2013 - 13:11:39.871  */
	if (sec_to_td(msec / TIMER_MS_DIVIDER, &t) != -1) {
		sprintf(str, "%c %02d-%02d-%04d %02d:%02d:%02d.%03d ", sym, t.day, t.mon, t.year, t.hour, t.min, t.sec,
			(u32) (msec % TIMER_MS_DIVIDER));
	} else {
		sprintf(str, "set time error ");
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  ������ ��� LOG ������ 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * ������������� �������� �������
 * ��� ������������ ������ ������ � FLASH
 */
#pragma section("FLASH_code")
int log_mount_fs(void)
{
	int status;
	FRESULT res;		/* Result code */

	/* SD ����� ���������� � BF */
	select_sdcard_to_bf();
	ssync();

	/* ��������� �� */
	asm("nop;");
	if ((res = f_mount(&fatfs, "", 1))) {
		return RES_MOUNT_ERR;
	}

	return RES_NO_ERROR;
}

/**
 * ����� ����������� ��� ���?
 */
#pragma section("FLASH_code")
bool log_check_mounted(void)
{
	return fatfs.fs_type;
}


/**
 * �������� ������� ���-����� �� SD - ��� �����������-����� �� ����� WUSB 2 ������
 */
#pragma section("FLASH_code")
int log_check_lock_file(void)
{
	FIL lock_file;
	FRESULT res;		/* Result code */

	/* ��������� ������������ ���� */
	if (f_open(&lock_file, LOCK_FILE_NAME, FA_READ | FA_OPEN_EXISTING)) {
		return RES_NO_LOCK_FILE;	/* ���� ���� �� ����������! */
	}

	/* ���� ���� ���������� - ������ ��� */
	res = f_unlink(LOCK_FILE_NAME);
	if (res)
		return RES_DEL_LOCK_ERR;	/* �� ������� */

	return RES_NO_ERROR;	/* ���� ���� ���������� */
}



/**
 * �������� ������� ����� ����������� �� SD
 */
#pragma section("FLASH_code")
int log_check_reg_file(void)
{
	FIL file;
	FRESULT res;		/* Result code */

	/* ��������� ������������ ���� */
	if (f_open(&file, PARAM_FILE_NAME, FA_READ | FA_OPEN_EXISTING)) {
		return RES_OPEN_PARAM_ERR;
	}

	/* ������� ���� */
	if (f_close(&file)) {
		return RES_CLOSE_PARAM_ERR;
	}

	return RES_NO_ERROR;	/* ���� ���� ���������� */
}



/**
 * �������� ������-����� ���� ������ ���� UART �� ������
 */
#pragma section("FLASH_code")
int log_write_log_to_uart(char *fmt, ...)
{
	int r;
	char str[256];
	va_list p_vargs;	/* return value from vsnprintf  */

	va_start(p_vargs, fmt);
	r = vsnprintf(str, sizeof(str), fmt, p_vargs);
	va_end(p_vargs);
	if (r < 0)		/* formatting error?            */
		return RES_FORMAT_ERR;

	if (UART1_write_str(str, strlen(str)) == strlen(str))
		return RES_NO_ERROR;
	else
		return RES_WRITE_UART_ERR;
}



/**
 * ������ ������ ���������� � ����
 * ����� �� 10 �����?
 */
#pragma section("FLASH_code")
int log_write_env_data_to_file(void *p)
{
	DEV_STATUS_STRUCT *status;
	unsigned bw;		/* ��������� ��� �������� ����  */
	FRESULT res;		/* Result code */
	int t0;
	char str0[256];
	char str1[32];

	if (p == NULL)
		return RES_WRITE_LOG_ERR;;

	status = (DEV_STATUS_STRUCT *) p;

	t0 = get_sec_ticks();
	sec_to_str(t0, str1);

	sprintf(str0, "%s \t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%3.1f\t%3.1f\t%d\t%c%3.1f\t%c%3.1f\t%c%3.1f\n",
		str1, status->regpwr_volt, status->ireg_sense, status->burn_ext_volt, status->burn_volt,
		status->iburn_sense, status->am_power_volt, status->iam_sense,
		status->temper0 / 10.0, status->temper1 / 10.0, status->press,
		status->pitch > 0 ? ' ' : '-', abs(status->pitch) / 10.0,
		status->roll > 0 ? ' ' : '-', abs(status->roll) / 10.0, status->head > 0 ? ' ' : '-',
		abs(status->head) / 10.0);


	*pIMASK &= ~EVT_IVG14;	/* ��������� handler 14, ����� �� ����� � log */
	ssync();

	res = f_write(&env_file, str0, strlen(str0), &bw);
	if (res) {
		return RES_WRITE_LOG_ERR;
	}

	/* ����������� �������! */
	res = f_sync(&env_file);
	if (res) {
		return RES_SYNC_LOG_ERR;
	}

	/* �������� handler 14  */
	*pIMASK |= EVT_IVG14;
	ssync();

	return 0;
}



/**
 * ������ ������ � ��� ����, ���������� ������� ��������. � �������� ������!
 * ����� �� 10 �����?
 */
#pragma section("FLASH_code")
int log_write_log_file(char *fmt, ...)
{
	char str[256];
	FRESULT res;		/* Result code */
	unsigned bw;		/* ��������� ��� �������� ����  */
	int i, ret = 0;
	va_list p_vargs;	/* return value from vsnprintf  */

	/* �� ����������� (��� �� - �������� �� PC), ��� � ��������� �������� */
	if (log_file.obj.fs == NULL) {
		return RES_MOUNT_ERR;
	}

	/* �������� ������� ����� - MAX_TIME_STRLEN �������� � �������� - ������ ����� */
	time_to_str(str);	// �������� ����� ������
	va_start(p_vargs, fmt);
	i = vsnprintf(str + MAX_TIME_STRLEN, sizeof(str), fmt, p_vargs);

	va_end(p_vargs);
	if (i < 0)		/* formatting error?            */
		return RES_FORMAT_ERR;


	// ������� �������� ������ �� UNIX (�� � ������!)
	for (i = MAX_TIME_STRLEN + 4; i < sizeof(str) - 3; i++) {
		if (str[i] == 0x0d || str[i] == 0x0a) {
			str[i] = 0x0d;	// ������� ������
			str[i + 1] = 0x0a;	// Windows
			str[i + 2] = 0;
			break;
		}
	}

	*pIMASK &= ~EVT_IVG14;	/* ��������� handler 14, ����� �� ����� � log */
	ssync();

	res = f_write(&log_file, str, strlen(str), &bw);
	if (res) {
		return RES_WRITE_LOG_ERR;
	}

	/* ����������� �������! */
	res = f_sync(&log_file);
	if (res) {
		return RES_SYNC_LOG_ERR;
	}
	/* ���� ������ 128 ����� - ����� � ������� ����. ����. */
	if (f_size(&log_file) > MAX_LOG_FILE_LEN) {
		GNS110_PARAM_STRUCT gns110_param;
		get_gns110_start_params(&gns110_param);	/* �������� ���������  */

		res = f_close(&log_file);
		if (res) {
			return RES_CLOSE_LOG_ERR;
		}

		/* ������� ���� ���� � ���� ���������� ������, ������� ���� ���!  */
		sprintf(str, "%s/gns110.l%02d", gns110_param.gns110_dir_name, num_log++ % 100);
		if ((res = f_open(&log_file, str, FA_WRITE | FA_OPEN_ALWAYS))) {
			ret = RES_CREATE_LOG_ERR;
		}
	}

	/* �������� handler 14  */
	*pIMASK |= EVT_IVG14;
	ssync();

	return ret;
}

/**
 * ������ ������ ������ � ��� ����
 */
#pragma section("FLASH_code")
int log_write_error_file(char *fmt, ...)
{
	char str[256];
	FIL error_log;		/* File object */
	FRESULT res;		/* Result code */
	unsigned bw;		/* ��������� ��� �������� ����  */
	int i, ret = 0;
	va_list p_vargs;	/* return value from vsnprintf  */

	/* �� ����������� */
	if (fatfs.fs_type == 0) {
		return RES_MOUNT_ERR;
	}

	va_start(p_vargs, fmt);
	i = vsnprintf(str, sizeof(str), fmt, p_vargs);
	va_end(p_vargs);
	if (i < 0)		/* formatting error?            */
		return RES_FORMAT_ERR;

	*pIMASK &= ~EVT_IVG14;	/* ��������� handler 14, ����� �� ����� � log */
	ssync();

	/* ���� ��� �� - ������� error_log */
	res = f_open(&error_log, ERROR_LOG_NAME, FA_WRITE | FA_READ | FA_OPEN_ALWAYS);
	if (res) {
		return RES_WRITE_LOG_ERR;
	}

	/* ��������� ������ � ����� ���������� */
	i = f_size(&error_log);
	if (i > 0x7F000000) {
		f_truncate(&error_log);
		i = 0;
	}
	/* ���������� ��������� ����� */
	f_lseek(&error_log, i);


	res = f_write(&error_log, str, strlen(str), &bw);
	if (res) {
		return RES_WRITE_LOG_ERR;
	}

	/* ����������� �������! */
	res = f_close(&error_log);
	if (res) {
		return RES_CLOSE_LOG_ERR;
	}

	/* �������� handler 14  */
	*pIMASK |= EVT_IVG14;
	ssync();

	return res;
}



/**
 * ������ ������ � ��� ����, ��� ������� � ���������
 */
#pragma section("FLASH_code")
int log_write_debug_str(char *fmt, ...)
{
	char str[256];
	FRESULT res;		/* Result code */
	unsigned bw;		/* ��������� ��� �������� ����  */
	int i, ret = 0;
	va_list p_vargs;	/* return value from vsnprintf  */

	/* �� ����������� (��� �� - �������� �� PC), ��� � ��������� �������� */
	if (log_file.obj.fs == NULL) {
		return RES_MOUNT_ERR;
	}

	/* �������� ������� ����� - MAX_TIME_STRLEN �������� � �������� - ������ ����� */
	time_to_str(str);	// �������� ����� ������
	va_start(p_vargs, fmt);
	i = vsnprintf(str + MAX_TIME_STRLEN, sizeof(str), fmt, p_vargs);

	va_end(p_vargs);
	if (i < 0)		/* formatting error?            */
		return RES_FORMAT_ERR;


	*pIMASK &= ~EVT_IVG14;	/* ��������� handler 14, ����� �� ����� � log */
	ssync();

	res = f_write(&log_file, str, strlen(str), &bw);
	if (res) {
		return RES_WRITE_LOG_ERR;
	}

	/* ����������� �������! */
	res = f_sync(&log_file);
	if (res) {
		return RES_SYNC_LOG_ERR;
	}

	/* �������� handler 14  */
	*pIMASK |= EVT_IVG14;
	ssync();

	return ret;
}

/**
 * ������� ���-����
 */
#pragma section("FLASH_code")
int log_close_log_file(void)
{
	FRESULT res;		/* Result code */

	res = f_close(&log_file);
	if (res) {
		return RES_CLOSE_LOG_ERR;
	}

	/* ����� �� ������� � ���� ��� ����� � ������  */
	res = f_close(&env_file);
	if (res) {
		return RES_CLOSE_LOG_ERR;
	}


	rsi_power_off();	/* ��������� RSI */
	return RES_NO_ERROR;
}


/**
 * ������� ��������� - �� ����� �������������� ��� ����� ������ 
 * ���������� �� ����� main
 * ��������� ����� ������������!!!
 */
#pragma section("FLASH_code")
void log_create_adc_header(s64 gps_time, s64 drift, s32 lat, s32 lon)
{
	char str[MAX_TIME_STRLEN];	// 26 ����
#if defined		ENABLE_NEW_SIVY
	strncpy(adc_hdr.DataHeader, "SeismicDat1\0", 12);	/* ��������� ������ SeismicDat1\0 - ����� Sivy */
	adc_hdr.HeaderSize = sizeof(ADC_HEADER);	/* ������ ���������     */
	adc_hdr.GPSTime = gps_time;	/* ����� �������������: ����������� */
	adc_hdr.Drift = drift;	/* ����� �� ������ ����� GPS: �����������  */
	adc_hdr.lat = lat;	/* ������: 55417872 = 5541.7872N */
	adc_hdr.lon = lon;	/* �������:37213760 = 3721.3760E */
#else
	TIME_DATE data;
	long t0;
	u8 ind;

	t0 = gps_time / TIMER_NS_DIVIDER;
	sec_to_td(t0, &data);	/* ����� ������������� ���� */
	strncpy(adc_hdr.DataHeader, "SeismicData\0", 12);	/* ���� ID  */
	adc_hdr.HeaderSize = sizeof(ADC_HEADER);	/* ������ ���������     */
	adc_hdr.Drift = drift * 32768;	/* ����� ��������� - ������� � ������������� �� ������� */

	memcpy(&adc_hdr.GPSTime, &data, sizeof(TIME_DATE));	/* ����� ��������� ������ �� GPS */
	adc_hdr.NumberSV = 3;	/* ����� ���������: ����� ����� 3 */
	adc_hdr.params.coord.comp = true;	/* ������ �������  */

	/* ���������� ����� - ��� �� ��������� */
	if (lat == lon == 0)
		ind = 90;
	else
		ind = 12;	// +554177+03721340000009

	/* ���������� �������� ����������� */
	snprintf(str, sizeof(str), "%c%06d%c%07d%06d%02d", (lat >= 0) ? '+' : '-', abs(lat / 100),
		 (lon >= 0) ? '+' : '-', abs(lon / 100), 0, ind);
	memcpy(adc_hdr.params.coord.pos, str, sizeof(adc_hdr.params.coord.pos));

#endif
	log_write_log_file("INFO: Create ADS1282 header OK\n");
}



/**
 * ������� ���������. ���������� �� ����� ADS1282
 */
#pragma section("FLASH_code")
void log_fill_adc_header(char sps, u8 bitmap, u8 size)
{
	u32 block;

	/* ������������ � ������� ������� ����� �� 1 ������ */
	adc_hdr.ConfigWord = sps;
	switch (sps) {
	case SPS4K:
		block = SPS4K_PER_MIN;
		break;

	case SPS2K:
		block = SPS2K_PER_MIN;
		break;

	case SPS1K:
		block = SPS1K_PER_MIN;
		break;

	case SPS500:
		block = SPS500_PER_MIN;
		break;

	case SPS250:
		block = SPS250_PER_MIN;
		break;

	case SPS125:
		block = SPS125_PER_MIN;
		break;

	default:
		block = SPS62_PER_MIN;
		break;
	}

	adc_hdr.ChannelBitMap = bitmap;	/* ����� 1.2.3.4 ������ */
	adc_hdr.SampleBytes = size;	/* ������ 1 ������ �� ���� ���������� ��� ����� */


#if defined		ENABLE_NEW_SIVY
	/* � ����� Sivy ������ �����  - 4 ���� */
	adc_hdr.BlockSamples = block;
#else

	adc_hdr.BlockSamples = block & 0xffff;
	adc_hdr.params.coord.rsvd0 = (block >> 16) & 0xff;
	adc_hdr.Board = read_mod_id_from_eeprom();	/* ����� ����� ������ ���� */
	adc_hdr.Rev = 2;	/* ����� ������� �.�. 2 */
#endif
}


/**
 * ��� ��������� ���������. ���������� ���������� � ����������.
 * ����������� ��� � ������
 */
#pragma section("FLASH_code")
void log_change_adc_header(void *p)
{
	DEV_STATUS_STRUCT *status;
	int temp;

	if (p != NULL) {
		status = (DEV_STATUS_STRUCT *) p;

#if defined		ENABLE_NEW_SIVY
		adc_hdr.u_pow = status->regpwr_volt;	/* ���������� �������, U mv */
		adc_hdr.i_pow = status->ireg_sense;	/* ��� �������, U ma */
		adc_hdr.u_mod = status->am_power_volt;	/* ���������� ������, U mv */
		adc_hdr.i_mod = status->iam_sense;	/* ��� ������, U ma */

		adc_hdr.t_reg = status->temper0;	/* ����������� ������������, ������� ���� ������� */
		adc_hdr.t_sp = status->temper1;	/* ����������� ������� �����, ������� ���� ������� */
		adc_hdr.p_reg = status->press;	/* �������� ������ ����� */
		adc_hdr.pitch = status->pitch;
		adc_hdr.roll = status->roll;
		adc_hdr.head = status->head;

#else
		/* ������������� �� �������� DataFormatProposal */
		adc_hdr.Bat = (int) status->regpwr_volt * 1024 / 50000;	/* ����� ������� ��� ������� 12 ����� */

		/* ���� ���� ������� */
		if (status->temper1 == 0 && status->press == 0) {
			temp = status->temper0;
		} else {
			temp = status->temper1;
		}
		adc_hdr.Temp = ((temp + 600) * 1024 / 5000);	/* ����� ������� ��� ����������� * 10 */

#endif
	}
}


/**
 * ��������� ��������� ������ ���, ����� - � ��� ����� ���������� ������ �����
 * ����� �� ������� ���� ��� ������ �������� ���������� � ���
 *
 * ������������� ��� �������!!!!! 
 */
section("L1_code")
int log_create_hour_data_file(u64 ns)
{
	char name[32];		/* ��� ����� */
	TIME_DATE date;
	FRESULT res;		/* Result code */
	u32 sec = ns / TIMER_NS_DIVIDER;

	/* ���� ��� ���� ������� ���� - ������� ��� � �������� �����  */
	if (adc_file.obj.fs) {
		if ((res = f_close(&adc_file)) != 0) {
			return RES_CLOSE_DATA_ERR;
		}
	}

	/* �������� ����� � ����� ������� - ��� ����� ��� �������� */
	if (sec_to_td(sec, &date) != -1) {
		GNS110_PARAM_STRUCT gns110_param;
		get_gns110_start_params(&gns110_param);	/* �������� ���������  */


		/* �������� ����� �� �������: ���-�����-����-����.������ */
		snprintf(name, MAX_FILE_NAME_LEN, "%s/%02d%02d%02d%02d.%02d",
			 &gns110_param.gns110_dir_name[0], ((date.year - 2000) > 0) ? (date.year - 2000) : 0,
			 date.month, date.day, date.hour, date.min);

		/* ������� ����� */
		res = f_open(&adc_file, name, FA_WRITE | FA_CREATE_ALWAYS);
		if (res) {
			return RES_OPEN_DATA_ERR;
		}
		return RES_NO_ERROR;	/* ��� OK */
	} else {
		return RES_FORMAT_TIME_ERR;
	}
}


/**
 * ����������� � ���������� ������ ������ ��������� �� SD ����� �  ������ F_SYNC!
 * � ��������� ������ ������ ����� 
 * ������������� ��� �������!!!!! 
 */
section("L1_code")
int log_write_adc_header_to_file(u64 ns)
{
	TIME_DATE date;
	unsigned bw;		/* ��������� ��� �������� ����  */
	FRESULT res;		/* Result code */

#if defined		ENABLE_NEW_SIVY
	adc_hdr.SampleTime = ns;	/* ����� ������, ����������� */
#else
	u64 msec = ns / TIMER_US_DIVIDER;
	memcpy(&adc_hdr.SedisTime, &msec, 8);	/* ����� ������������ � ��������������� ����� */
	sec_to_td(msec / TIMER_MS_DIVIDER, &date);	/* �������� ������� � ����� � ����� ������� */
	memcpy(&adc_hdr.SampleTime, &date, sizeof(TIME_DATE));	/* �������� ����� ������ ������ ������ */
#endif

	/* ������ � ���� ��������� */
	res = f_write(&adc_file, &adc_hdr, sizeof(ADC_HEADER), &bw);
	if (res) {
		return RES_WRITE_HEADER_ERR;
	}

	/* ����������� �������! �� �������� ���� ���� �������� SD ����� */
	res = f_sync(&adc_file);
	if (res) {
		return RES_SYNC_HEADER_ERR;
	}

	return RES_NO_ERROR;
}

/**
 * ������ � ���� ������ ���: ������ � ������ � ������
 * Sync ������ ��� � ������!
 * ������������� ��� �������!!!!! 
 */
section("L1_code")
int log_write_adc_data_to_file(void *data, int len)
{
	unsigned bw;		/* ��������� ��� �������� ����  */
	FRESULT res;		/* Result code */

	res = f_write(&adc_file, (char *) data, len, &bw);
	if (res) {
		return RES_WRITE_DATA_ERR;
	}
	return RES_NO_ERROR;	/* �������� OK */
}

/**
 * ������� ���� ���-����� ���� ������� ������ �� ���� 
 */
section("L1_code")
int log_close_data_file(void)
{
	FRESULT res;		/* Result code */

	if (adc_file.obj.fs == NULL)	// ��� ����� ���
		return RES_CLOSE_DATA_ERR;

	/* ����������� ������� */
	res = f_sync(&adc_file);
	if (res) {
		return RES_CLOSE_DATA_ERR;
	}

	res = f_close(&adc_file);
	if (res) {
		return RES_CLOSE_DATA_ERR;
	}
	return FR_OK;		/* ��� ���������! */
}


/***************************************************************************
 * ������� ���� �����������-���� ����������� ������,
 * � ����� ������ ���� ������ ����:
 ***************************************************************************************************
// ���� ���������� ��� �������� ������ �� �������������!
19.03.13 15:10:00  			// ����� ������ ����������� � UTC
20.03.13 05:50:00  			// ����� ��������� ����������� � UTC
20.03.13 05:55:00  			// ����� ������ �������� � UTC
30   			// ������������ �������� ��������� � ��������
2   			// ��������� ������������ �������� � �������
-1   			// ����� ������, (0)-�� �����, (-1) �� ���������
20.03.13 15:11:00  			// ����� ���������� �������� �� ������ � UTC
06.00-18.00  			// ������� ����� ����� (����-������)
500  			// ������� ��������� ���
Hi  			// �����������������  ��� (High ��� Low)
2  			// PGA ���
3  			// ����� ���� � ����� ������
1111  			// ���������� ������ ->(1-�, 2-�, 3-�, 4-�) ����� �������
4  			// ������ ����� ������ (1 ���, 4 ���� ��� �����)
 ***************************************************************************************************
 * ��������: ��������� �� �����
 * �������:  ����� (0) ��� ��� (-1)
 * ���������� ����������� ��������� ������
 ****************************************************************************/
#pragma section("FLASH_code")
int log_read_reg_file(void *param)
{
	u32 sec;
	int j, ret = RES_NO_ERROR, x, y, m, d;	/* ret - ��������� ���������� */
	unsigned bw;		/* ��������� ��� �������� ����  */
	FIL reg_file;		/* File object ��� ������� ������ ����������� */
	FRESULT res;		/* Result code */
	char home_dir[8];	/* �������� �������� ���������� */
	char buf[32];
	char *str = NULL;	/* ��������� � ���� */
	TIME_DATE time, dir_time;	/* �������� ��� ���������� */
	GNS110_PARAM_STRUCT *reg;	/* ��������� �������  */


	do {
		/* �������� �� �������� */
		if (param == NULL) {
			ret = RES_REG_PARAM_ERR;
			break;
		}

		reg = (GNS110_PARAM_STRUCT *) param;	/* ��� ��� ���� */



		/* �������� ������ ��� �������� ����� */
		str = (char *) malloc(MAX_FILE_SIZE);
		if (str == NULL) {
			ret = RES_MALLOC_PARAM_ERR;
			break;
		}


		/* ��������� ���� � ����������� */
		if (f_open(&reg_file, PARAM_FILE_NAME, FA_READ | FA_OPEN_EXISTING)) {
			ret = RES_OPEN_PARAM_ERR;
			break;
		}


		/* ������ ������ �� ����� ��������� */
		if (f_read(&reg_file, str, MAX_FILE_SIZE - 1, &bw)) {
			ret = RES_READ_PARAM_ERR;
			break;
		}
		str[bw] = 0;

		/* ������� ���� */
		if (f_close(&reg_file)) {
			ret = RES_CLOSE_PARAM_ERR;
			break;
		}

///////////////////////////////////////////////////////////////////////////////////////////////
// 0...������ ����� ������ ������, ��� ������ ���������� � "//"
///////////////////////////////////////////////////////////////////////////////////////////////

		/* ��������� - ������ ������ ������� ������ */
		if (str[0] == '/' && str[1] == '/') {
			for (j = 2; j < 80; j++) {
				if (str[j] == '\n') {	// ������� ������ DOS
					break;
				}
			}
		}
///////////////////////////////////////////////////////////////////////////////////////////////
// 1...��������� ������ - ������� ���������
///////////////////////////////////////////////////////////////////////////////////////////////
		x = 0;
		for (j = 0; j < bw - 1; j++) {
			if (str[j] == '\n') {	// ������� ������ DOS
				x++;
				if (x == POS_NUM_STR) {
					break;
				}
			}
		}


		/* 5 ����� �� ����� */
		strncpy(buf, str + j + 1, 6);
		buf[5] = 0;
		reg->gns110_pos = atoi(buf);

///////////////////////////////////////////////////////////////////////////////////////////////
// 2...��������� ������ ������� ������ ����������� ����� 1-�� ��������
///////////////////////////////////////////////////////////////////////////////////////////////
		x = 0;
		for (j = 0; j < bw - 1; j++) {
			if (str[j] == '\n') {	// ������� ������ DOS
				x++;
				if (x == BEGIN_REG_NUM_STR) {
					break;
				}
			}
		}

		buf[2] = 0;

		/* ������� 2 ����� �� ����� */
		strncpy(buf, str + j + 1, 2);
		dir_time.day = atoi(buf);

		/* ����� */
		strncpy(buf, str + j + 4, 2);
		dir_time.mon = atoi(buf);

		/* ��� */
		strncpy(buf, str + j + 7, 2);
		dir_time.year = atoi(buf) + 2000;

		/* ���� */
		strncpy(buf, str + j + 10, 2);
		dir_time.hour = atoi(buf);

		/* ������ */
		strncpy(buf, str + j + 13, 2);
		dir_time.min = atoi(buf);

		/* ������� */
		strncpy(buf, str + j + 16, 2);
		dir_time.sec = atoi(buf);

		/* �������� �� ������? */


		/* 1. ��������� �� ����� ������ ����������� */
		reg->gns110_start_time = td_to_sec(&dir_time);

		/* ����� ������ ���������� (����������) ����� �� 10 ����� �� �����  */
		reg->gns110_wakeup_time = (int) (reg->gns110_start_time) - TIME_START_AFTER_WAKEUP;


///////////////////////////////////////////////////////////////////////////////////////////////
// ���� 3-� ������� ������ -  ����� ��������� ����������� 
///////////////////////////////////////////////////////////////////////////////////////////////
		x = 0;
		for (j = 0; j < bw - 1; j++) {
			if (str[j] == '\n') {	// ������� ������ DOS
				x++;
				if (x == END_REG_NUM_STR) {
					break;
				}
			}
		}

		/* 3...��������� ������ ������� ��������� ����������� */
		buf[2] = 0;

		/* ������� 2 ����� �� ����� */
		strncpy(buf, str + j + 1, 2);
		time.day = atoi(buf);

		/* ����� */
		strncpy(buf, str + j + 4, 2);
		time.mon = atoi(buf);

		/* ��� */
		strncpy(buf, str + j + 7, 2);
		time.year = atoi(buf) + 2000;

		/* ���� */
		strncpy(buf, str + j + 10, 2);
		time.hour = atoi(buf);

		/* ������ */
		strncpy(buf, str + j + 13, 2);
		time.min = atoi(buf);

		/* ������� */
		strncpy(buf, str + j + 16, 2);
		time.sec = atoi(buf);


		/* 3. ����� ��������� ����������� */
		reg->gns110_finish_time = td_to_sec(&time);


//////////////////////////////////////////////////////////////////////////////////////////
//���� 4-� ������� ������ 
//////////////////////////////////////////////////////////////////////////////////////////
		x = 0;
		for (j = 0; j < bw - 1; j++) {
			if (str[j] == '\n') {	/* ������� ������ (DOS ��� UNIX ?) */
				x++;
				if (x == BEGIN_BURN_NUM_STR) {
					break;
				}
			}
		}

		/* 3... ��������� ������ ������� �������� (������ ��������) */
		buf[2] = 0;

		/* ������� 2 ����� �� ����� */
		strncpy(buf, str + j + 1, 2);
		time.day = atoi(buf);

		/* ����� */
		strncpy(buf, str + j + 4, 2);
		time.mon = atoi(buf);

		/* ��� */
		strncpy(buf, str + j + 7, 2);
		time.year = atoi(buf) + 2000;

		/* ���� */
		strncpy(buf, str + j + 10, 2);
		time.hour = atoi(buf);

		/* ������ */
		strncpy(buf, str + j + 13, 2);
		time.min = atoi(buf);

		/* ������� */
		strncpy(buf, str + j + 16, 2);
		time.sec = atoi(buf);

		/* 3. ��������� � ������� ������� ������ �������� */
		reg->gns110_burn_on_time = td_to_sec(&time);

/////////////////////////////////////////////////////////////////////////////////////////
// ���� 5-� ������� ������ ������������ ��������
/////////////////////////////////////////////////////////////////////////////////////////
		x = 0;
		for (j = 0; j < bw - 1; j++) {
			if (str[j] == '\n') {	/* ������� ������ (DOS ��� UNIX ?) */
				x++;
				if (x == BURN_LEN_NUM_STR) {
					break;
				}
			}
		}

		strncpy(buf, str + j + 1, 4);	/* ������������ �������� - 3 ����� */
		reg->gns110_modem_burn_len_sec = atoi(buf);	/* ������ ������ �������� ��������� ����� �������� � +10 ������ �� ������ */
		reg->gns110_burn_off_time = reg->gns110_burn_on_time + reg->gns110_modem_burn_len_sec;

/////////////////////////////////////////////////////////////////////////////////////////
// ���� 6-� ������� ������ ������������ ��������
/////////////////////////////////////////////////////////////////////////////////////////
		x = 0;
		for (j = 0; j < bw - 1; j++) {
			if (str[j] == '\n') {	/* ������� ������ (DOS ��� UNIX ?) */
				x++;
				if (x == POPUP_LEN_NUM_STR) {
					break;
				}
			}
		}

		strncpy(buf, str + j + 1, 4);	/* ������������ ��������  � ������� - 3 ����� */
		reg->gns110_gps_time = reg->gns110_burn_off_time + atoi(buf) * 60;	/* ����� ��������� GPS = ������ �������� + ������������ �������� */


/////////////////////////////////////////////////////////////////////////////////////////
// ���� 7-� ������� ������. ����� ������������� ������ 
/////////////////////////////////////////////////////////////////////////////////////////
		x = 0;
		for (j = 0; j < bw - 1; j++) {
			if (str[j] == '\n') {	/* ������� ������ (DOS ��� UNIX ?) */
				x++;
				if (x == MODEM_NUM_NUM_STR) {
					break;
				}
			}
		}

		/* ����� ������������� ������ - 5 ���� �� ��� �����! */
		strncpy(buf, str + j + 1, 6);
		reg->gns110_modem_num = atoi(buf);

/////////////////////////////////////////////////////////////////////////////////////////
// ���� 8-� ������� ������. ����� ���������� �������� �� ������
/////////////////////////////////////////////////////////////////////////////////////////
		x = 0;
		for (j = 0; j < bw - 1; j++) {
			if (str[j] == '\n') {	/* ������� ������ (DOS ��� UNIX ?) */
				x++;
				if (x == ALARM_TIME_NUM_STR) {
					break;
				}
			}
		}

		/* ����� ���������� �������� �� ������ */

		/* 6... ��������� ������ ������� �������� */
		buf[2] = 0;

		/* ������� 2 ����� �� ����� */
		strncpy(buf, str + j + 1, 2);
		time.day = atoi(buf);

		/* ����� */
		strncpy(buf, str + j + 4, 2);
		time.mon = atoi(buf);

		/* ��� */
		strncpy(buf, str + j + 7, 2);
		time.year = atoi(buf) + 2000;

		/* ���� */
		strncpy(buf, str + j + 10, 2);
		time.hour = atoi(buf);

		/* ������ */
		strncpy(buf, str + j + 13, 2);
		time.min = atoi(buf);

		/* ������� */
		strncpy(buf, str + j + 16, 2);
		time.sec = atoi(buf);

		/* 6. ��������� � ������� UTC */
		reg->gns110_modem_alarm_time = td_to_sec(&time);



/////////////////////////////////////////////////////////////////////////////////////////
// ���� 9-� ������� ������. ����� �������� ������� �����
/////////////////////////////////////////////////////////////////////////////////////////
		x = 0;
		for (j = 0; j < bw - 1; j++) {
			if (str[j] == '\n') {	/* ������� ������ (DOS ��� UNIX ?) */
				x++;
				if (x == DAY_TIME_NUM_STR) {
					break;
				}
			}
		}

		/* ����� �������� ������� ����� */
		buf[2] = 0;

		/* ������� 2 ����� �� ��� */
		strncpy(buf, str + j + 1, 2);
		reg->gns110_modem_h0_time = atoi(buf);

		/* 2 ����� �� ������ */
		strncpy(buf, str + j + 4, 2);
		reg->gns110_modem_m0_time = atoi(buf);

		/* ��� 2 ����� �� ��� */
		strncpy(buf, str + j + 7, 2);
		reg->gns110_modem_h1_time = atoi(buf);

		/* 2 ����� �� ������ */
		strncpy(buf, str + j + 10, 2);
		reg->gns110_modem_m1_time = atoi(buf);


/////////////////////////////////////////////////////////////////////////////////////////
// ���� 10-� ������� ������. ������� ���������.
/////////////////////////////////////////////////////////////////////////////////////////
		x = 0;
		for (j = 0; j < bw - 1; j++) {
			if (str[j] == '\n') {	/* ������� ������ (DOS ��� UNIX ?) */
				x++;
				if (x == ADC_FREQ_NUM_STR) {
					break;
				}
			}
		}
		/* ��������� ����� 62 - ��� ������� 62.5 */
		/* 4 ����� �� ����� 62, 125, 250, 500, 1000, 2000, 4000 */
		strncpy(buf, str + j + 1, 5);
		reg->gns110_adc_freq = atoi(buf);

		// ������ � ������� �������
		if ((reg->gns110_adc_freq != 62) && (reg->gns110_adc_freq != 125) &&
		    (reg->gns110_adc_freq != 250) && (reg->gns110_adc_freq != 500) &&
		    (reg->gns110_adc_freq != 1000) && (reg->gns110_adc_freq != 2000) &&
		    (reg->gns110_adc_freq != 4000)) {
			ret = RES_FREQ_PARAM_ERR;
			break;
		}
/////////////////////////////////////////////////////////////////////////////////////////
// ���� 11-� ������� ������. ����������������� ���
/////////////////////////////////////////////////////////////////////////////////////////
		x = 0;
		for (j = 0; j < bw - 1; j++) {
			if (str[j] == '\n') {	/* ������� ������ (DOS ��� UNIX ?) */
				x++;
				if (x == ADC_CONSUM_NUM_STR) {
					break;
				}
			}
		}

		/* 3 ������� hi ��� lo */
		strncpy(buf, str + j + 1, 3);

		/* � ������� ������� */
		str_to_cap(buf, 2);

		/* ��� � ��� ��������? */
		if (strncmp(buf, "HI", 2) == 0) {
			reg->gns110_adc_consum = 1;
		} else if (strncmp(buf, "LO", 2) == 0) {
			reg->gns110_adc_consum = 0;
		} else {
			ret = RES_CONSUMP_PARAM_ERR;
			break;
		}


/////////////////////////////////////////////////////////////////////////////////////////
// ���� 12-� ������� ������. �������� PGA
/////////////////////////////////////////////////////////////////////////////////////////
		x = 0;
		for (j = 0; j < bw - 1; j++) {
			if (str[j] == '\n') {	/* ������� ������ (DOS ��� UNIX ?) */
				x++;
				if (x == ADC_PGA_NUM_STR) {
					break;
				}
			}
		}

		/* 2 ����� �� ����� PGA */
		strncpy(buf, str + j + 1, 3);
		reg->gns110_adc_pga = atoi(buf);

		// ������ � ������� ��������
		if ((reg->gns110_adc_pga != 1) && (reg->gns110_adc_pga != 2) && (reg->gns110_adc_pga != 4)
		    && (reg->gns110_adc_pga != 8)
		    && (reg->gns110_adc_pga != 16) && (reg->gns110_adc_pga != 32) && (reg->gns110_adc_pga != 64)) {
			ret = RES_PGA_PARAM_ERR;
			break;
		}
/////////////////////////////////////////////////////////////////////////////////////////
// ���� 13-� ������� ������. ��� ������
/////////////////////////////////////////////////////////////////////////////////////////
		x = 0;
		for (j = 0; j < bw - 1; j++) {
			if (str[j] == '\n') {	/* ������� ������ (DOS ��� UNIX ?) */
				x++;
				if (x == MODEM_TYPE_NUM_STR) {
					break;
				}
			}
		}

		/* 2 ����� �� �����: 0, 1, 2, 3 */
		strncpy(buf, str + j + 1, 2);
		x = atoi(buf);
		if (x > 3 || x < 0) {
			ret = RES_MODEM_TYPE_ERR;
			break;
		}
		reg->gns110_modem_type = (GNS110_MODEM_TYPE) x;
/////////////////////////////////////////////////////////////////////////////////////////
// ���� 14-� ������� ������. ������������ ������ ���. ������ ������
/////////////////////////////////////////////////////////////////////////////////////////
		x = 0;
		for (j = 0; j < bw - 1; j++) {
			if (str[j] == '\n') {	/* ������� ������ (DOS ��� UNIX ?) */
				x++;
				if (x == ADC_BITMAP_NUM_STR) {
					break;
				}
			}
		}

		/* 4 ����� �� ����� */
		reg->gns110_adc_bitmap = 0;
		strncpy(buf, str + j + 1, 5);

		// ������� �� ��������� 
		if (buf[0] != '0')
			reg->gns110_adc_bitmap |= 8;
		if (buf[1] != '0')
			reg->gns110_adc_bitmap |= 4;
		if (buf[2] != '0')
			reg->gns110_adc_bitmap |= 2;
		if (buf[3] != '0')
			reg->gns110_adc_bitmap |= 1;


/////////////////////////////////////////////////////////////////////////////////////////
// ���� 15-� ������� ������. ������� ����� ������ ���� ������
/////////////////////////////////////////////////////////////////////////////////////////
		x = 0;
		for (j = 0; j < bw - 1; j++) {
			if (str[j] == '\n') {	/* ������� ������ (DOS ��� UNIX ?) */
				x++;
				if (x == FILE_LEN_NUM_STR) {
					break;
				}
			}
		}

		/* 2 ����� �� ����� */
		strncpy(buf, str + j + 1, 3);
		j = atoi(buf);

		/* � ���� �������� - �� ���� �� ����� */
		reg->gns110_file_len = (j > 0 && j < 25) ? j : 1;

/////////////////////////////////////////////////////////////////////////////////////////
// ���� 16-� ������� ������. ������� ����� �������
/////////////////////////////////////////////////////////////////////////////////////////
		x = 0;
		for (j = 0; j < bw - 1; j++) {
			if (str[j] == '\n') {	/* ������� ������ (DOS ��� UNIX ?) */
				x++;
				if (x == FLT_FREQ_NUM_STR) {
					break;
				}
			}
		}

		/* 7 ���� �� ����� � ���. ����� */
		strncpy(buf, str + j + 1, 7);
		reg->gns110_adc_flt_freq = atof(buf);	/* � ���� �������� - �� 0 �� ??? */


/////////////////////////////////////////////////////////////////////////////////////////
// ���� 17-� ������� ������. ���������� �����������
/////////////////////////////////////////////////////////////////////////////////////////
		x = 0;
		for (j = 0; j < bw - 1; j++) {
			if (str[j] == '\n') {	/* ������� ������ (DOS ��� UNIX ?) */
				x++;
				if (x == CONST_REG_NUM_STR) {
					break;
				}
			}
		}

		/* 2 ����� �� ����� */
		strncpy(buf, str + j + 1, 2);
		reg->gns110_const_reg_flag = ((atoi(buf) == 0) ? false : true);	/* � ���� �������� - �� 0 �� ??? */


/////////////////////////////////////////////////////////////////////////////////////////
// ��������� ������ ����� ����� � EEPROM
/////////////////////////////////////////////////////////////////////////////////////////
		x = read_mod_id_from_eeprom();
		sprintf(home_dir, "GNS%04d", x % 9999);	/* ������� ������ ����������, ���� ��� ����������� - ������ ���������� */

		/* Open the directory */
		if (f_opendir(&dir, home_dir)) {

			/* �� ������� �������! */
			if (f_mkdir(home_dir)) {
				ret = RES_MKDIR_PARAM_ERR;
				break;
			}
		}

/////////////////////////////////////////////////////////////////////////////////////////
// ������� ����� � ��������� ������� �������
/////////////////////////////////////////////////////////////////////////////////////////
		y = dir_time.year - 2000;
		m = dir_time.mon;
		d = dir_time.day;

		if (read_reset_cause_from_eeprom() == CAUSE_WDT_RESET) {	// ������� ������ - WDT
			/* ������� ������ ���������� ����, ���� ��� ����������� - ������ ���������� */
			for (x = MAX_START_NUMBER - 1; x >= 0; x--) {
				sprintf(reg->gns110_dir_name, "%s/%02d%02d%02d%02d",
					home_dir, (y > 0) ? y % MAX_START_NUMBER : 0, m % MAX_START_NUMBER,
					d % MAX_START_NUMBER, x % MAX_START_NUMBER);

				/* Open the directory - ����� ��������� */
				res = f_opendir(&dir, reg->gns110_dir_name);
				if (res == FR_OK) {
					break;
				}
			}
		} else {

			/* ������� ������ ���������� ���� � 99 �� 0,
			 * ���� ��� ����������� - ������ ���������� */
			for (x = MAX_START_NUMBER - 1; x >= 0; x--) {
				sprintf(reg->gns110_dir_name, "%s/%02d%02d%02d%02d",
					home_dir, (y > 0) ? y % MAX_START_NUMBER : 0, m % MAX_START_NUMBER,
					d % MAX_START_NUMBER, x % MAX_START_NUMBER);

				/* ���������� ������� */
				res = f_opendir(&dir, reg->gns110_dir_name);

				/* ���� ����� ���������� - ��� ����� �������� */
				if (res == FR_OK && x < (MAX_START_NUMBER - 1)) {
					x += 1;

					/* ����� �������� �������� �����, ������� ������� */
					sprintf(reg->gns110_dir_name, "%s/%02d%02d%02d%02d",
						home_dir, (y > 0) ? y % MAX_START_NUMBER : 0, m % MAX_START_NUMBER,
						d % MAX_START_NUMBER, x % MAX_START_NUMBER);
					break;
				}
			}

			/* ����� ������� ������� �� ��������, �� �� ������ ������ */
			if (x > (MAX_START_NUMBER - 1)) {
				ret = RES_MAX_RUN_ERR;
				break;
			}

			/* ������� ����� � ���������: ����� � ����� ������� */
			if ((res = f_mkdir(reg->gns110_dir_name))) {
				ret = RES_DIR_ALREADY_EXIST;
				break;
			}
		}


		/* ������� ���� ���� � ���� ���������� ������, ������� ���� ���!  */
		sprintf(str, "%s/gns110.log", reg->gns110_dir_name);
		if ((res = f_open(&log_file, str, FA_WRITE | FA_READ | FA_OPEN_ALWAYS))) {
			ret = RES_CREATE_LOG_ERR;
			break;
		}

		/* ��������� ������     */
		x = f_size(&log_file);
		if (x >= MAX_LOG_FILE_LEN)
			f_truncate(&log_file);

		/* ���������� ��������� ����� */
		f_lseek(&log_file, x);


		/*vvvvv: �������� � ���� ������  */

		/* ����� �� ������� ����, ���� ����� ��������� ���������� ��������� �����  */
		sprintf(str, "%s/gns110.env", reg->gns110_dir_name);
		if ((res = f_open(&env_file, str, FA_WRITE | FA_READ | FA_OPEN_ALWAYS))) {
			ret = RES_CREATE_ENV_ERR;
			break;
		}

		/* ��������� ������ */
		x = f_size(&env_file);
		if (x == 0) {
			/* ������� �������� �������� */
			strcpy(str,
			       "Time\tREGpwr\tREGcur\tBURNext\tBURNpwr\tBURNcur\tMODEMpwr\tMODEMcur\tTemp0\tTemp1\tPress\tPitch\tRoll\tHead\n");
			res = f_write(&env_file, str, strlen(str), &bw);
			if (res) {
				return RES_WRITE_LOG_ERR;
			}
		} else {
			/* ���������� ��������� ����� */
			f_lseek(&env_file, x);
		}

	} while (0);

	/* ������� ���������� ����� */
	if (str != NULL) {
		free(str);
		str = NULL;
	}

	return ret;		/* ��� �������� - ����� ��� �������? */
}


/* �������� ��������� ����� �� ������� */
#pragma section("FLASH_code")
void log_get_free_space(void)
{
    FRESULT res;		/* Result code */
    DWORD fre_clust, fre_sect, tot_sect;
    FATFS *fs = &fatfs;

/* Get volume information and free clusters of drive 1 */
	res = f_getfree("1:", &fre_clust, &fs);
	if (res) {
		/* Get total sectors and free sectors */
		tot_sect = (fs->n_fatent - 2) * fs->csize;
		fre_sect = fre_clust * fs->csize;

		/* Print the free space (assuming 512 bytes/sector) */
		log_write_log_file("%10lu KiB total drive space.\n%10lu KiB available.\n", tot_sect / 2, fre_sect / 2);
	} else {
		log_write_log_file("can't get free space\r\n");
	}
}
