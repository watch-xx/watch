/**
 * ����� ������ ����� ��� ������ � ������
 */
#include <math.h>
#include "flash.h"
#include "pll.h"

#define ERASE_TIMEOUT			50000000
#define PROGRAM_TIMEOUT			1000000



/**
 * ��� ������ � flash � �������� �� ����!
 */
static int PollToggleBit(u32, u32);
static bool IsStatusReady(u32);
static int WriteFlash(u32, u16);
static int ReadFlash(u32, u16 *);

/**
 * Init Flash. This function initializes parallel flash.
 * ��� SCLK = 49 ��� tsclk = 20.35 ns - ��������� 4.096
 * BOST = roundUP(20 ns / tsclk) = 20 / 20.35 = 1
 * BORAT = roundUP(60 ns  / tsclk) = 60 / 20.35 = 3
 * BOWAT = roundUP(45 ns  / tsclk) = 45 / 20.35 = 3
 * BOHT = roundUP(10 ns  / tsclk) = 1
 * BOTT = 1
 **********************************
 * ��� SCLK = 24 ��� tsclk = 41.7 ns - ��������� 19.2
 * BOST = roundUP(20 ns / tsclk) = 20 / 41 = 1
 * BORAT = roundUP(60 ns  / tsclk) = 60 / 41 = 2
 * BOWAT = roundUP(45 ns  / tsclk) = 45 / 41 = 2
 * BOHT = roundUP(10 ns  / tsclk) = 1
 * BOTT = 1
 **********************************
 * ��� SCLK = 24.576 ��� tsclk = 40.7 ns - ��������� 8.192
 * BOST = roundUP(20 ns / tsclk) = 20 / 40.7 = 1
 * BORAT = roundUP(60 ns  / tsclk) = 60 / 40.7 = 2
 * BOWAT = roundUP(45 ns  / tsclk) = 45 / 40.7 = 2
 * BOHT = roundUP(10 ns  / tsclk) = 1
 * BOTT = 1
 */
section("L1_code")
int FLASH_init(void)
{
    *pEBIU_MODE = 0x0001;	// ����������� ����� flash ������
    ssync();

    *pFLASH_CONTROL_CLEAR = FLASH_ENABLE;	/* Reset the flash */
    asm("ssync;nop;nop;nop;nop;nop;nop;nop;");
    asm("ssync;nop;nop;nop;nop;nop;nop;nop;");
    asm("ssync;nop;nop;nop;nop;nop;nop;nop;");
    asm("ssync;nop;nop;nop;nop;nop;nop;nop;");

    *pFLASH_CONTROL_SET = FLASH_ENABLE;	/* Release flash from reset state */
    asm("ssync;nop;nop;nop;nop;nop;nop;nop;");

    *pFLASH_CONTROL_SET = FLASH_UNPROTECT;	/* Unprotect the flash to enable program/erase */
    asm("ssync;nop;nop;nop;nop;nop;nop;nop;");

    /* ���������� ������� ��� ������ � �� �������! */
#if QUARTZ_CLK_FREQ==(4096000)
	*pEBIU_AMBCTL = 0x3356;
#else
 // ��� 19200000	/* ��� ���������� 19.2 ��� - ������ 41.7 �� */
 // ��� 8192000
	*pEBIU_AMBCTL = 0x2256;
#endif

#if 0
    // ���������� �������� ��� 48 ���! 3 ����� �� ������, 3 ����� �� ������, 1 ���� �� ������
    // 0 ������ �� HOLD TIME, 1 ���� �� setup time, 1 ���� ���. transition time
    if (SCLK_PERIOD_NS < 20) {	// > 48 ���
	*pEBIU_AMBCTL = 0xFFC2;	// ������� �������  
    } else if (SCLK_PERIOD_NS >= 20 && SCLK_PERIOD_NS < 40) {	// 48 ���
	*pEBIU_AMBCTL = 0x3316;
    } else if (SCLK_PERIOD_NS >= 40 && SCLK_PERIOD_NS < 50) {	// 24 ���
	*pEBIU_AMBCTL = 0x2116;
    } else if (SCLK_PERIOD_NS >= 50 && SCLK_PERIOD_NS < 55) {	// 20 ���
	*pEBIU_AMBCTL = 0x1116;
    } else if (SCLK_PERIOD_NS >= 55 && SCLK_PERIOD_NS < 60) {	// 18 ���
	*pEBIU_AMBCTL = 0x1116;
    } else {			// 12 ��� � ����
	*pEBIU_AMBCTL = 0x1116;
    }
#endif

    asm("ssync;nop;nop;nop;nop;nop;nop;nop;");
    return 0;
}


/**
 *   ������� ������, ��������� ����� ������� ��� ��������
 */
section("L1_code")
int FLASH_erase_page(u32 addr)
{
    int ErrorCode = 0;		// tells us if there was an error erasing flash

    // unlock this block
    WriteFlash(addr, 0x60);
    WriteFlash(addr, 0xD0);
    ErrorCode = PollToggleBit(addr, ERASE_TIMEOUT);

    // erase block
    WriteFlash(addr, 0x20);
    WriteFlash(addr, 0xD0);
    ErrorCode = PollToggleBit(addr, ERASE_TIMEOUT);
    WriteFlash(addr, 0xFF);

    return ErrorCode;
}

/**
 * �������� 16-�� ������ �����
 */
section("L1_code")
int FLASH_program_half_word(u32 addr, u16 data)
{
    int res;

    // send the unlock command to the flash
    WriteFlash(addr, 0x60);
    WriteFlash(addr, 0xD0);
    res = PollToggleBit(addr, PROGRAM_TIMEOUT);
    WriteFlash(addr, 0x40);
    WriteFlash(addr, data);	// program our actual value now

    // make sure the write was successful
    res = PollToggleBit(addr, PROGRAM_TIMEOUT);
    return res;
}


/**
 *  	Write a value to an address in flash.
 */
section("L1_code")
static int WriteFlash(u32 ulAddr, u16 usValue)
{
    u16 *pFlashAddr = (u16 *) (ulAddr);	// set the address
    *pFlashAddr = usValue;
    return 0;			// ok
}




/**
 *  	Reads a value from an address in flash.
 */
section("L1_code")
static int ReadFlash(u32 ulAddr, u16 * pusValue)
{
    u16 *pFlashAddr = (u16 *) (ulAddr);	// set our flash address to where we want to read
    *pusValue = (u16) * pFlashAddr;	// read the value
    return 0;			// ok
}

section("L1_code")
static bool IsStatusReady(u32 ulOffset)
{
    u16 status = 0;

    WriteFlash(ulOffset, 0x0070);
    ReadFlash(ulOffset, &status);

    if ((status & 0x80) == 0x80) {
	WriteFlash(ulOffset, 0x0050);
	return true;
    } else
	return false;
}

/**
 * Polls the toggle bit in the flash to see when the operation
 */
section("L1_code")
static int PollToggleBit(u32 ulOffset, u32 timeout)
{
    while (timeout) {
	if (IsStatusReady(ulOffset)) {
	    WriteFlash(ulOffset, 0xFF);
	    return 0;
	}
	timeout--;
    }
    return -1;
}
