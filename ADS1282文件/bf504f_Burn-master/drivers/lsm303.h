#ifndef _LSM303_H
#define _LSM303_H

#include "globdefs.h"


/* ������ ������������� ��� ������ ������� */
typedef struct   {
        s16 x;
	s16 y;
	s16 z;
	s16 t; // + �����������
} lsm303_data;

bool lsm303_init_acc(void);
bool lsm303_init_comp(void);
bool lsm303_get_acc_data(lsm303_data*);
bool lsm303_get_comp_data(lsm303_data*);

#endif /* lsm303.h  */


