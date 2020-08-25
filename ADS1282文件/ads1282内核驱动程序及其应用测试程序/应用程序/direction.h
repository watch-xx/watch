/*
 * direction.h
 *
 *  Created on: Mar 24, 2018
 *      Author: root
 */

#ifndef DIRECTION_H_
#define DIRECTION_H_

#define	HMC5883L_Addr   0x1E	//电子罗盘I2C总线从地址
#define	ADXL345_Addr    0x53	//加速度传感器器件地址
void Init_direction(void);
void Get_Direction(void);
extern int16_t current_direction[6];

#endif /* DIRECTION_H_ */
