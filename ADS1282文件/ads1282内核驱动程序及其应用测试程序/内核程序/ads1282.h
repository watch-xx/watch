/*
 * ads1282.h
 *
 *  Created on: Mar 24, 2018
 *      Author: root
 */

#ifndef ADS1282_H_
#define ADS1282_H_

/* ADC command definition */

#define ADC_WAKEUP		0x00	/* Wake-up from Standby mode, 00h or 01h, TYPE: Control */
#define ADC_STANDBY		0x02	/* Enter Standby mode, 02h or 03h, TYPE: control */
#define ADC_SYNC		0x04	/* Synchronize the A/D conversion, 04h or 05h, TYPE: Control */
#define ADC_RESET		0x06	/* Reset registers to default values, 06h or 07h, TYPE: Control */
#define ADC_RDATAC		0x10	/* Read data continuous, 10h, TYPE: Control */
#define ADC_SDATAC		0x11	/* Stop read data continuous, 11h, TYPE: Control */
#define ADC_RDATA		0x12	/* Read data by command, 12h, TYPE: Data */

#define ADC_RREG		0x20	/* Read nnnnn register(s) at address rrrr,
								    001r rrrr(20h + 000r rrrr),
							    	000n nnnn(00h + n nnnn), TYPE: Register */

#define ADC_WREG		0x40	/* Write nnnnn register(s) at address rrrr,
							    	010r rrrr(40h + 000r rrrr),
							   	  000n nnnn(00h + n nnnn), TYPE: Register */

#define ADC_OFSCAL		0x60	/* Offset calibration, 60h, TYPE: Calibration */
#define ADC_GANCAL		0x61	/* Gain calibration, 61h, TYPE: Calibration */

/* ADC internal register address definition */

#define ADC_ADDR_ID		0x00	/* Bit[7:4]: Factory-programmed identification
								bit(read-only)
								Bit[3:0]: Always write '0' */

#define ADC_ADDR_CFG0	0x01	/* CONFIG0 */
#define ADC_ADDR_CFG1	0x02	/* CONFIG1 */

#define ADC_ADDR_HPF0	0x03
#define ADC_ADDR_HPF1	0x04

#define ADC_ADDR_OFC0	0x05
#define ADC_ADDR_OFC1	0x06
#define ADC_ADDR_OFC2	0x07

#define ADC_ADDR_FSC0	0x08
#define ADC_ADDR_FSC1	0x09
#define ADC_ADDR_FSC2	0x0A

/* CONFIG0 definition */

/* Synchronization mode */
#define ADC_SYNC_P		0x7F	/* Pulse SYNC mode(default) */
#define ADC_SYNC_C		0xFF	/* Continuous SYNC mode */

/* MODE */
#define ADC_MODE_L		0xBF	/* Low-power mode */
#define ADC_MODE_H		0xFF	/* High-resolution mode(default) */

/* Data Rate Select */
#define ADC_RATE_250	0xC7	/* 250SPS */
#define ADC_RATE_500	0xCF	/* 500SPS */
#define ADC_RATE_1000	0xD7	/* 1000SPS(default) */
#define ADC_RATE_2000	0xDF	/* 2000SPS */
#define ADC_RATE_4000	0xE7	/* 4000SPS */

/* FIR Phase Response */
#define ADC_FIR_LIN		0xFB	/* Linear phase(default) */
#define ADC_FIR_MIN		0xFF	/* Minimum phase */

/* Digital filter configuration */
#define ADC_FILTER_BYPASS		0xFC	/* On-chip filter bypassed, modulator output mode */
#define ADC_FILTER_SINC			0xFD	/* Sinc filter block only */
#define ADC_FILTER_SINC_LPF		0xFE	/* Sinc + LPF filter blocks (default) */
#define ADC_FILTER_SINC_LPF_HPF	0xFF	/* Sinc + LPF + HPF filter blocks */

/* CONFIG1 definition */

/* Multiplexer input select */
#define ADC_MUX_1		0x0F	/* AINP1 and AINN1(default) */
#define ADC_MUX_2		0x1F	/* AINP2 and AINN2 */
#define ADC_MUX_GND		0x2F	/* Internal short via 400R */
#define ADC_MUX_1_2		0x3F	/* AINP1 and AINN1 connected to AINP2 and AINN2 */
#define ADC_MUX_EX2		0x4F	/* External short to AINN2 */

/* PGA Chopping Enable */
#define ADC_CHOP_DIS	0x77/* PGA chopping disabled */
#define ADC_CHOP_EN		0x7F/* PGA chopping enabled(defaut) */

/* PGA Gain Select */
#define ADC_GAIN_1		0x78	/* G = 1(default) */
#define ADC_GAIN_2		0x79	/* G = 2 */
#define ADC_GAIN_4		0x7A	/* G = 4 */
#define ADC_GAIN_8		0x7B	/* G = 8 */
#define ADC_GAIN_16		0x7C	/* G = 16 */
#define ADC_GAIN_32		0x7D	/* G = 32 */
#define ADC_GAIN_64		0x7E	/* G = 64 */

#endif /* ADS1282_H_ */
