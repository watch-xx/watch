#ifndef _VERSION_H
#define _VERSION_H

#include "globdefs.h"

/* ������ ������ ����� */
#define __MY_VER__	1
#define __MY_REV__	116

#ifndef _WIN32			/* Embedded platform */
	section("FLASH_data") static const unsigned char  prog_ver = __MY_VER__; /* ������ ��������� */
	section("FLASH_data") static const unsigned short prog_rev = __MY_REV__; /* ������� ��������� */
#else
	const unsigned char  prog_ver = __MY_VER__; /* ������ ��������� */
	const unsigned short prog_rev = __MY_REV__; /* ������� ��������� == */
#endif


/**
 *  ���������� ������ ������
 */
IDEF unsigned char get_version(void)
{
    return prog_ver; 	/* ������ ��������� */
}



/**
 *  ���������� ������� ������
 */
IDEF unsigned short get_revision(void)
{
    return prog_rev; 	/* ������ ��������� */
}



#endif /* version.h */

