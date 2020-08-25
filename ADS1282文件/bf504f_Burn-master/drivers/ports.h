#ifndef _PORTS_H
#define _PORTS_H

#include "globdefs.h"
#include "rele.h"
#include "log.h"

void init_bf_ports(void);
void init_atmega_ports(void);

void wusb_off(void);
bool wusb_on(void);
void select_debug_module(void);
void select_modem_module(void);
void select_gps_module(void);
void unselect_debug_uart(void);
void unselect_gps_module(void);
void select_sdcard_to_bf(void);
void select_sdcard_to_cr(void);
void select_analog_power(void);
void unselect_analog_power(void);
void burn_wire_on(void);
void burn_wire_off(void);
void modem_on(void);
void modem_off(void);
void old_modem_reset(void);
bool check_sd_card(void);

/* �������� */
#pragma always_inline
inline void port_wakeup(void)
{
   *pPORTHIO_SET =  PH2;		/* ������ � 1 */	
    ssync();	
}

/* ������� */
#pragma always_inline
inline void port_sleepon(void)
{
  *pPORTHIO_CLEAR = PH2;		/* ������ � ���� */	
   ssync();	
}



#endif /* portc.h  */

