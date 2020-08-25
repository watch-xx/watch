#ifndef _GLOBDEFS_H
#define _GLOBDEFS_H

#include <signal.h>
#include <stdint.h>
#include <time.h>


#define		SEC_IN_HOUR			3600

/* ��������� ��� 504 ��������, �� 506!!!!  */
#ifndef _WIN32			/* Embedded platform */
#include <adi_types.h>
#include <services_types.h>
#include <sys/exception.h>
#include <cdefBF504f.h>
#include <defBF504f.h>
#include <ccblkfn.h>
#include <float.h>
#include "config.h"


typedef  	char c8;


#else				/* windows   */
#include <windows.h>
#include <tchar.h>

#ifndef u8
#define u8 unsigned char
#endif

#ifndef s8
#define s8 char
#endif

#ifndef c8
#define c8 char
#endif

#ifndef u16
#define u16 unsigned short
#endif


#ifndef s16
#define s16 short
#endif

#ifndef i32
#define i32  int
#endif


#ifndef u32
#define u32 unsigned long
#endif


#ifndef s32
#define s32 long
#endif


#ifndef u64
#define u64 uint64_t
#endif


#ifndef s64
#define s64 int64_t
#endif


/* ������� ����� */
#ifndef	time64
#define time64	int64_t
#endif

#endif



#ifndef u64
#define u64 uint64_t
#endif

#ifndef s64
#define s64 int64_t
#endif

/* ������� ����� */
#ifndef	time64
#define time64	int64_t
#endif


#ifndef _WIN32			/* Embedded platform */

/* long double �� ������������� -  � ������� ����� ������ �� ����! */
#ifndef f32
#define f32 float32_t
#endif

#ifndef bool
#define bool u8
#endif


#ifdef true
#define true 1
#endif

#ifdef false
#define false 0
#endif

#else
/* long double �� �������������  */
#ifndef f32
#define f32 float
#endif

#endif



#ifndef IDEF
#define IDEF static inline
#endif

/* �� ���� ����� ����� �������� ���� ���������� */
#define 	BROADCAST_ADDR	0xff

/* ������� ������� � �����  */
#define 	NUM_ADS1282_PACK	20

#define 	ADC_CHAN			4	/* ����� �������  */
#define		MAGIC				0x4b495245


/*******************************************************************************
 * ��������� ��������� ��� �������� State machine
 *******************************************************************************/
typedef enum {
    DEV_POWER_ON_STATE = 0,
    DEV_CHOOSE_MODE_STATE = 1,
    DEV_INIT_STATE,
    DEV_TUNE_WITHOUT_GPS_STATE,	/*������ ������� ��� GPS */
    DEV_GET_3DFIX_STATE,
    DEV_TUNE_Q19_STATE,		/* ��������� ��������� ������ 19 ��� */
    DEV_SLEEP_AND_DIVE_STATE,	/* ���������� � ��� ����� ������� ������ */
    DEV_WAKEUP_STATE,
    DEV_REG_STATE,
    DEV_FINISH_REG_STATE,
    DEV_WAIT_GPS_STATE,
    DEV_HALT_STATE,
    DEV_COMMAND_MODE_STATE,
    DEV_POWER_OFF_STATE,
    DEV_EMERGENCY_WAIT_STATE,	/* ��������� �������� */            
    DEV_ERROR_STATE = -1	/* ������ */
} DEV_STATE_ENUM;


/*******************************************************************************
 * ���� ����������� �������
 *******************************************************************************/
typedef enum {
    GNS110_NOT_MODEM = 0,
    GNS110_MODEM_OLD,		//1
    GNS110_MODEM_AM3,		//2
    GNS110_MODEM_BENTHOS,	//3
} GNS110_MODEM_TYPE;


/**
 * ��������� ������� ������� 
 * � ���� ���������� ������������ ��������� ��������
 * ������������� ����� ������:
 * 16.10.12 17.15.22	// ����� ������ �����������
 * 17.10.12 08.15.20	// ����� ��������� �����������
 * 18.10.12 11.17.00	// ����� ��������, �������� 5 ���. ������� ��������
 * ....
 */
#pragma pack(4)
typedef struct {
    /* ����� ����������� */
    u32 gns110_pos;		/* ������� ��������� */

    char gns110_dir_name[18];	/* �������� ���������� ��� ���� ������ */
    u8 gns110_file_len;		/* ������ ����� ������ � ����� */
    bool gns110_const_reg_flag;	/* ���������� ����������� */


    /* ����� */
    s32 gns110_modem_rtc_time;	/* ����� ����� ������ */
    s32 gns110_modem_alarm_time;	/* ��������� ����� �������� �� ������   */
    s32 gns110_modem_type;	/* ��� ������ */
    u16 gns110_modem_num;	/* ����� ������ */
    u16 gns110_modem_burn_len_sec;	/* ������������ �������� ��������� � �������� �� ������ */
    u8 gns110_modem_h0_time;
    u8 gns110_modem_m0_time;
    u8 gns110_modem_h1_time;
    u8 gns110_modem_m1_time;

    /* ������� */
    s32 gns110_wakeup_time;	/* ����� ������ ���������� ����� ������������ */
    s32 gns110_start_time;	/* ����� ������ �����������  */
    s32 gns110_finish_time;	/* ����� ��������� ����������� */
    s32 gns110_burn_on_time;	/* ����� ������ �������� ��������  */
    s32 gns110_burn_off_time;	/* ����� ��������� �������� ��������  */
    s32 gns110_gps_time;	/* ����� ��������� gps ����� ������� �������� */

    /* ��� */
    f32 gns110_adc_flt_freq;	/* ������� ����� ������� HPF */

    u16 gns110_adc_freq;	/* ������� ���  */
    u8 gns110_adc_pga;		/* �������� ���  */
    u8 gns110_adc_bitmap;	/* ����� ������ ��� ������������ */
    u8 gns110_adc_consum;	/* ����������������� ��� ���� */
    u8 gns110_adc_bytes;	/* ����� ���� � ����� ������  */
    u8 rsvd[2];
} GNS110_PARAM_STRUCT;


/* ���������, ����������� �����, �������� c8 �� u8 - ����� �� �.�. ������������� */
#pragma pack(1)
typedef struct {
    u8 sec;
    u8 min;
    u8 hour;
    u8 week;			/* ���� ������...�� ����������� */
    u8 day;
    u8 month;			/* 1 - 12 */
#define mon 	month
    u16 year;
} TIME_DATE;
#define TIME_LEN  8		/* ���� */


/**
 * ���� ��������� ���� � ����������   
 */
#pragma pack(4)
struct timespec {
    time_t tv_sec;		/* �������  */
    long tv_nsec;		/* ����������� */
};



/**
 * � ��� �������� ������� �������� ��������� ��� ������ �� SD ����� (pack �� 1 �����!)
 */
#if defined   ENABLE_NEW_SIVY
/* � ����� ���� �������� �� 4 ����� */
#pragma pack(4)
typedef struct {
    c8 DataHeader[12];		/* ��������� ������ SeismicDat0\0  */

    c8 HeaderSize;		/* ������ ��������� - 80 ���� */
    u8 ConfigWord;		/* ������������ */
    c8 ChannelBitMap;		/* ���������� ������: 1 ����� ������� */
    c8 SampleBytes;		/* ����� ���� ����� ������ */

    u32 BlockSamples;		/* ������ 1 ��������� ����� � ������ */

    s64 SampleTime;		/* ����� ��������� ������: ����������� ������� UNIX */
    s64 GPSTime;		/* ����� �������������: ����������� ������� UNIX  */
    s64 Drift;			/* ����� �� ������ ����� GPS: �����������  */

    u32 rsvd0;			/* ������. �� ������������ */

    /* ��������� �����: �����������, ���������� � �� */
    u16 u_pow;			/* ���������� �������, U mv */
    u16 i_pow;			/* ��� �������, U ma */
    u16 u_mod;			/* ���������� ������, U mv */
    u16 i_mod;			/* ��� ������, U ma */

    s16 t_reg;			/* ����������� ������������, ������� ���� �������: 245 ~ 24.5 */
    s16 t_sp;			/* ����������� ������� �����, ������� ���� �������: 278 ~ 27.8  */
    u32 p_reg;			/* �������� ������ ����� ��� */

    /* ��������� ������� � ������������ (�� ������) */
    s32 lat;			/* ������ (+ ~N, - S):   55417872 ~ 5541.7872N */
    s32 lon;			/* �������(+ ~E, - W): -37213760 ~ 3721.3760W */

    s16 pitch;			/* ����, ������� ���� �������: 12 ~ 1.2 */
    s16 roll;			/* ������, ������� ���� ������� 2 ~ 0.2 */

    s16 head;			/* ������, ������� ���� ������� 2487 ~ 248.7 */
    u16 rsvd1;			/* ������ ��� ������������ */
} ADC_HEADER;
#else

#pragma pack(1)
typedef struct {
    c8 DataHeader[12];		/* ��������� ������ SeismicData\0  */

    c8 HeaderSize;		/* ������ ��������� - 80 ���� */
    c8 ConfigWord;
    c8 ChannelBitMap;
    u16 BlockSamples;
    c8 SampleBytes;

    TIME_DATE SampleTime;	/* �������� ����� ������ */

    u16 Bat;
    u16 Temp;
    u8 Rev;			/* ������� = 2 ������  */
    u16 Board;

    u8 NumberSV;
    s16 Drift;

    TIME_DATE SedisTime;	/* �� ������������ ����  */
    TIME_DATE GPSTime;		/* ����� ������������� */

    union {
	struct {
	    u8 rsvd0;
	    bool comp;		/* ������ */
	    c8 pos[23];		/* ������� (����������) */
	    c8 rsvd1[3];
	} coord;		/* ���������� */

	u32 dword[7];		/* ������ */
    } params;
} ADC_HEADER;
#endif

//////////////////////////////////////////////////////////////////////////////////////////
//                        �� �����������
//////////////////////////////////////////////////////////////////////////////////////////

/**
 * ����� ������ �� ����������� - 20 ��������� ������������� -  240 ����, �������� �� 1!!!
 */
#pragma pack(1)
typedef struct {
    u8 len;			/* ����� ������ ��� ����������� ����� */
    u8 adc;			/* ����� ���������� ��� - ������ ���� ������ */
    u16 msec;			/* ������������ ������� ��������� */
    u32 sec;			/* UNIX TIME ������� ��������� */

    struct {			/* 3-� ������� ����� (* 4) */
	unsigned x:24;
	unsigned y:24;
	unsigned z:24;
	unsigned h:24;
    } data[NUM_ADS1282_PACK];

    u16 rsvd;			/* ������������ */
    u16 crc16;			/* ����������� ����� ������  */
} ADS1282_PACK_STRUCT;



/** 
 *  ������ � ������ ��������� �� ����������� 64 ����. �������� ������ ���������� ��� ������ (������ 2 ����� + 1 len + 2 CRC16)
 */
#pragma pack(4)
typedef struct {
    u8 len;			/* 1....����� ������ ��� ����������� ����� */
    u8 st_main;			/* 2....�������: ��� �������, ��� ��������, ������ � �������, ��� �����, ����������, ������������ ������, ����������, ����� ������ */
    u8 st_test0;		/* 3....���������������� � ������: 0 - �����, 1 - ������ T&P, 2 - �������/ ������, 3 - �����, 4 - GPS, 5 - EEPROM, 6 - ����� SD, 7 - flash */
    u8 st_test1;		/* 4....��������� ��������� */

    u8 st_reset;		/* 5....������� ������ */
    u8 st_adc;			/* 6....������ - ����� ����� ������ ��� */
    s16 temper0;		/* 7-8..����������� �� �������� ����� * 10 */

    s32 st_tim4[2];		/* 9-16..�������� ������� 4 �� ����� ����� �����������. Min � ���. ����� ����� ����� � ����� */
    s32 st_tim3[2];		/* 17-24..�������� ������� 3 �� ����� ����� �����������. Min � ��� */

#define gns_rtc      st_tim4[0]
#define gns_time     st_tim4[1]
#define gns_drift    st_tim3[0]

    u32 eeprom;			/* 25-28..������ EEPROM - ������ �� ������ ���� */

    s16 am_power_volt;		/* 29-30..�� 1 ����-����� 4 ������ ���������� */
    s16 burn_ext_volt;          /* 31-32 */

    s16 burn_volt;              /* 33-34 */
    s16 regpwr_volt;            /* 35-36 */

    s16 iam_sense;		/* 37-38.. 3 ������ ���� */
    s16 ireg_sense;             /* 39-40 */

    s16 iburn_sense;            /* 41-42 */
    s16 temper1;		/* 43-44..����������� * 10 */

    u32 press;			/* 45-48..�������� ��� */

    s16 pitch;			/* 49-50..���� */
    s16 roll;			/* 51-52..������ */

    s16 head;			/* 53-54.. ������ */
    u16 ports;                  /* 55-56 ..���������� ���� � ����� */

    u32 quartz;                 /* 57-60 ������ - 4 ����� */
    u8  cclk;   		/* 61-62..������� � MHz - 2 ����� */
    u8  sclk;

    u16 crc16;			/* 63-64..����������� ����� ������  */
} DEV_STATUS_STRUCT;



/**
 * ����� ������ ������� �� �����������
 */
#pragma pack(1)
typedef struct {
    u8 len;			/* ����� ����  */
    u32 time_work;		/* ����� ������ � ������ ����� ������ */
    u32 time_cmd;		/* ����� ������ � ��������� ������ */
    u32 time_modem;		/* ����� ������ ������ */
    u16 crc16;			/* CRC16  */
} DEV_WORK_TIME_STRUCT;


/* ��� � ����� */
#pragma pack(4)
typedef struct {
    u8 len;			/* ����� */
    u8 ver;			/* ������ ��: 1, 2, 3, 4 ��� */
    u16 rev;			/* ������� �� 0, 1, 2 ��� */
    u32 time;			/* ����� ���������� */
    u16 addr;			/* ����� ����� */
    u16 crc16;
} DEV_ADDR_STRUCT;


/**
 * �������� ������� �� ����
 */
typedef struct {
    int rx_pack;		/* �������� ������ */
    int tx_pack;		/* ���������� ������ */
    int rx_cmd_err;		/* ������ � ������� */
    int rx_stat_err;		/* ������ ���������, ����� (��� ������) � �� */
    int rx_crc_err;		/* ������ ����������� ����� */
} DEV_UART_COUNTS;


/* ��������� ����� �������� ������� ����� � ������ ������ */
typedef struct {
    u32 cmd_error;
    u32 read_timeout;
    u32 write_timeout;
    u32 any_error;
} SD_CARD_ERROR_STRUCT;


/**
 * ���������� �������
 */
typedef struct {
    int baud;			/*�������� ������ ����� */
    void (*rx_call_back_func) (u8);	/* ��������� �� ������� ������ */
    void (*tx_call_back_func) (void);	/* ��������� �� ������� ������ */
    bool is_open;		/* ���� ������ */
} DEV_UART_STRUCT;

#define 	MODEM_BUF_LEN			64	/* ������ ���������� */

/**
 * ������� ������� ������ � UART
 */
#pragma pack (4)
typedef struct {
    u8 cmd;
    u8 len;
    u8 rsvd0[2];

    union {
	c8 cPar[MODEM_BUF_LEN];
	u16 wPar[MODEM_BUF_LEN/2];
	u32 lPar[MODEM_BUF_LEN/4];
	f32 fPar[MODEM_BUF_LEN/4];
    } u;
} DEV_UART_CMD;

/* ������ � ��� �����  */
#define   INFO_NUM_STR  	0
#define   POS_NUM_STR		1
#define   BEGIN_REG_NUM_STR	2
#define   END_REG_NUM_STR	3
#define   BEGIN_BURN_NUM_STR	4
#define   BURN_LEN_NUM_STR	5
#define   POPUP_LEN_NUM_STR	6
#define   MODEM_NUM_NUM_STR	7
#define   ALARM_TIME_NUM_STR	8
#define   DAY_TIME_NUM_STR	9
#define   ADC_FREQ_NUM_STR	10
#define   ADC_CONSUM_NUM_STR	11
#define   ADC_PGA_NUM_STR	12
#define   MODEM_TYPE_NUM_STR	13
#define   ADC_BITMAP_NUM_STR	14
#define   FILE_LEN_NUM_STR	15
#define   FLT_FREQ_NUM_STR	16
#define   CONST_REG_NUM_STR	17


#define   RELE_GPS_BIT			(1 << 0)
#define   RELE_MODEM_BIT		(1 << 1)
#define   RELE_BURN_BIT			(1 << 2)
#define   RELE_ANALOG_POWER_BIT		(1 << 3)
#define   RELE_DEBUG_MODULE_BIT		(1 << 4)
#define   RELE_MODEM_MODULE_BIT		(1 << 5)
#define   RELE_USB_BIT			(1 << 6)
#define   RELE_WUSB_BIT			(1 << 7)



#endif				/* globdefs.h */
