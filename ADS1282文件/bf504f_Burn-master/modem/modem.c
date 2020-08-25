#include "modem.h"
#include "utils.h"
#include "am0.h"
#include "am3.h"
#include "amt.h"
#include "log.h"


/** 
 * ���������������� �����, � ����������� �� ����
 */
#pragma section("FLASH_code")
int modem_init_all_types(void *par)
{
    int res = -1;

    if (par != NULL) {

	switch (((GNS110_PARAM_STRUCT *) par)->gns110_modem_type) {

	    /* ������ ����� */
	case GNS110_MODEM_OLD:
	    res = am0_prg_modem(par);
	    break;

	    /* ����� am3 */
	case GNS110_MODEM_AM3:
	    res = am3_prg_modem(par);
	    break;

	    /* ����� ������ */
	case GNS110_MODEM_BENTHOS:
	    res = amt_prg_modem(par);
	    break;

	    /* ��� ������ */
	default:
	    res = 0;
	    break;
	}
    }
    return res;
}


/**
 * �������� ����� - ���� ������ ���� AM3
 */
#pragma section("FLASH_code")
int modem_set_radio(void *par)
{
    int res = -1;

    if (par != NULL) {


	switch (((GNS110_PARAM_STRUCT *) par)->gns110_modem_type) {

	    /* ������ ����� */
	case GNS110_MODEM_OLD:
	    res = 0;
	    break;

	    /* ����� am3 */
	case GNS110_MODEM_AM3:
	    res = am3_set_radio(par);
	    break;

	    /* ����� ������ */
	case GNS110_MODEM_BENTHOS:
	    res = amt_set_radio(par);
	    break;

	    /* ��� ������ */
	default:
	    res = 0;
	    break;
	}
    }
    return res;
}

/**
 * ������ ����� ������ am3
 */
int  modem_check_modem_time(void* par)
{
    int res = -1;
    TIME_DATE td;

    if (par != NULL) {

	switch (((GNS110_PARAM_STRUCT *) par)->gns110_modem_type) {


         /* ����� am3 */
	case GNS110_MODEM_AM3:
	  if (am3_init() < 0) {
		return -1;
	  }
	 /* ��� ����� � �������� */
         res = am3_get_curr_time(&td);

         am3_close();
    	break;


	default:
	    break;
	}

   }
    return res;
}

/** 
 * ��������� ��������� ������, � ����������� �� ����
 */
#pragma section("FLASH_code")
int modem_check_params(void *par)
{
    int res = -1;

    if (par != NULL) {

	switch (((GNS110_PARAM_STRUCT *) par)->gns110_modem_type) {

	    /* ����� am3 */
	case GNS110_MODEM_AM3:
	    res = am3_check_modem(par);
	    break;

	default:
	    break;
	}
    }
    return res;
}


/** 
 * ����������������� ������� � �����
 */
int modem_convey_buf(void *cmd, int size)
{
    int res;
    res = am3_convey_buf(cmd, size);
    return res;
}

