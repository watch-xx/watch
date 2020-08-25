#ifndef _ADS1282_H
#define _ADS1282_H

#include "globdefs.h"
#include "utils.h"


/************************************************************************
 * ��������� ��� ��� �� ��������
 ************************************************************************/
#define _1282_SYNC	PG14
#define _1282_OWFL	PF8
#define _1282_DRDY	PF9
#define  CH_SEL_A	PF14
#define  CH_SEL_B	PF15

/* ������ � ������� - ������ � ����� ���������� ����� �������� ���   
 * ������������� ConfigWord � ��������� */
typedef enum {
    SPS62 = 0,
    SPS125,
    SPS250,
    SPS500,
    SPS1K,
    SPS2K,
    SPS4K,
} ADS1282_FreqEn;


/* ����� ������ ��� */
typedef enum {
    TEST_MODE = 0,
    WORK_MODE,
    CMD_MODE
} ADS1282_ModeEn;

/* �������� ������ - ������ � ����� ���������� ����� �������� ���   */
typedef enum {
    PGA1 = 0,
    PGA2,
    PGA4,
    PGA8,
    PGA16,
    PGA32,
    PGA64
} ADS1282_PgaEn;


/* ��������� ������ ��������� ������� ��� */
typedef struct {
    ADS1282_FreqEn sps;  /* �������  */
    ADS1282_PgaEn  pga;  /* �������� */
    ADS1282_ModeEn mode; /* ����� ������ */
    u16     hpf;	 /* ������ */
    u8	    res;	 /* Hi ��� Lo */
    u8      chop;	 /* choping */
    u8      bitmap;	 /* ���������� ������: 1 ����� �������, 0 - �������� */
    u8      file_len;    /* ����� ����� ������ */
} ADS1282_Params;


/**
 * ��������� ��� ��� ���� 4-� �������.
 * �������� 0 � �������� 
 */
typedef struct {
    u32 magic;			/* �����. ����� */
    struct {
	u32 offset;		/* ����������� 1 - �������� */
	u32 gain;		/* ����������� 2 - �������� */
    } chan[ADC_CHAN];			/* 4 ������ */
} ADS1282_Regs;

/**
 * ������ ��� ����� ����
 */
#pragma pack(4)
typedef struct {
    s64 time_now;		/* �����, ���� ���������� ������ - ������ � ����� ����� */
    s64 time_last;
    s32 sample_miss;		/* ������ ������ �� ������� */
    s32 block_timeout;		/* ���� �� ����� ������� ���������� */
    u32 test_counter;		/* ������� �������� ������� */

    struct {
	u8 cfg0_wr;
	u8 cfg0_rd;
	u8 cfg1_wr;
	u8 cfg1_rd;
    } adc[ADC_CHAN];
} ADS1282_ERROR_STRUCT;


/**
 * ������ ��� ������ ��� � ���� ���������
 */
#pragma pack(4)
typedef struct {
   ADS1282_FreqEn  sample_freq; // �������

   u8   num_bytes;   // ����� ���� � ����� ������ ���������
   u8   time_ping;   // ����� ������ ������ ping
   u16  samples_in_ping; // ����� �������� � ����� ping

   u16  ping_pong_size_in_byte; 
   u16  period_us;	// ������ ������ ������ � ���

   u16  sig_in_min; 
   u16  sig_in_time;
} ADS1282_WORK_STRUCT;


/*****************************************************************************************
 * ��������� �������. �c� ������� � ���������� void ADS1282_ ���������� 
 * �� ������ ������ � ����� ������ �� ��� ������������ ���
 ******************************************************************************************/
bool ADS1282_config(void *);
void ADS1282_start(void);	/* ������ ��� */
void ADS1282_stop(void);	/* ���� ��� � POWERDOWN ��� ���� */
bool ADS1282_is_run(void);
bool ADS1282_get_pack(void *);
void ADS1282_get_error_count(ADS1282_ERROR_STRUCT*);
void ADS1282_standby(void);
int ADS1282_get_irq_count(void);
bool ADS1282_ofscal(void);
bool ADS1282_gancal(void);
bool ADS1282_get_adc_const(u8, u32*, u32*);
bool ADS1282_set_adc_const(u8, u32, u32);
bool ADS1282_clear_adc_buf(void);

void ADS1282_ISR(void);		/* ���������� �� ���������� */
bool ADS1282_get_handler_flag(void);	/* ��� ������ � ��������� ����� ���� ���! */
void ADS1282_reset_handler_flag(void);
void ADS1282_stop_irq(void);

/*****************************************************************************************
 * ����� ��� - ������ �������� ����������! 
 ****************************************************************************************/
IDEF void ADS1282_start_irq(void)
{
    *pPORTFIO_CLEAR = _1282_DRDY;
    *pILAT |= EVT_IVG8;		/* clear pending IVG8 interrupts */
    *pSIC_IMASK0 |= IRQ_PFA_PORTF;
    asm("ssync;");
}



#endif				/* ads1282.h */
