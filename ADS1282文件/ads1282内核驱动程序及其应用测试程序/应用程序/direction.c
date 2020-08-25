/*
 * direction.c
 *
 *  Created on: Mar 24, 2018
 *      Author: root
 */
#include <stdio.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

#include "direction.h"
//#define I2C_FILE_NAME "/dev/i2c-5"//x版
#define I2C_FILE_NAME "/dev/i2c-2"//liu版
int i2c_file;
unsigned char DIRECTION_BUF[10]; //接收数据缓存区
int16_t T_X,T_Y,T_Z,T_T;
int16_t current_direction[6];//电子罗盘数据粗放数组
int outfd_direction;

static int set_i2c_register(int file,
                            unsigned char addr,
                            unsigned char reg,
                            unsigned char value) {

    unsigned char outbuf[2];
    struct i2c_rdwr_ioctl_data packets;//声明设备初始化结构体？？？？
    struct i2c_msg messages[1];//声明消息结构体

    ioctl(file,I2C_TIMEOUT,2);//设备驱动管理函数
    ioctl(file,I2C_RETRIES,1);

    messages[0].addr  = addr;
    messages[0].flags = 0;
    messages[0].len   = sizeof(outbuf);
    messages[0].buf   = outbuf;

    /* The first byte indicates which register we'll write */
    outbuf[0] = reg;

    /*
     * The second byte indicates the value to write.  Note that for many
     * devices, we can write multiple, sequential registers at once by
     * simply making outbuf bigger.
     */
    outbuf[1] = value;

    /* Transfer the i2c packets to the kernel and verify it worked */
    packets.msgs  = messages;
    packets.nmsgs = 1;
    if(ioctl(file, I2C_RDWR, &packets) < 0) {
        perror("Unable to send data");
        return 1;
    }

    return 0;
}


static int get_i2c_register(int file,
                            unsigned char addr,
                            unsigned char reg,
                            unsigned char *val) {
    unsigned char inbuf, outbuf;
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];

    /*
     * In order to read a register, we first do a "dummy write" by writing
     * 0 bytes to the register we want to read from.  This is similar to
     * the packet in set_i2c_register, except it's 1 byte rather than 2.
     */
    ioctl(file,I2C_TIMEOUT,2);
    ioctl(file,I2C_RETRIES,1);
    outbuf = reg;
    inbuf = 0;
    messages[0].addr  = addr;
    messages[0].flags = 0;
    messages[0].len   = sizeof(outbuf);
    messages[0].buf   = &outbuf;

    /* The data will get returned in this structure */
    messages[1].addr  = addr;
    messages[1].flags = I2C_M_RD;/* | I2C_M_RD I2C_M_NOSTART*/
    messages[1].len   = sizeof(inbuf);
    messages[1].buf   = &inbuf;

    /* Send the request to the kernel and get the result back */
    packets.msgs      = messages;
    packets.nmsgs     = 2;
    if(ioctl(file, I2C_RDWR, &packets) < 0) {
        perror("Unable to send data");
        return 1;
    }
    *val = inbuf;

    return 0;
}

int Single_Write(unsigned char SlaveAddress,unsigned char REG_Address,unsigned char REG_data)
{
	return set_i2c_register(i2c_file,SlaveAddress,REG_Address,REG_data);
}
char Single_Read(unsigned char SlaveAddress,unsigned char REG_Address)
{
	unsigned char val=0;
	get_i2c_register(i2c_file,SlaveAddress,REG_Address,&val);
	return val;
}
void Init_ADXL345(void)
{
	Single_Write(ADXL345_Addr,0x31,0x0B);   //测量范围,正负16g，13位模式
	// Single_Write(ADXL345_Addr,0x2C,0x0e);   //速率设定为100hz 参考pdf13页
	Single_Write(ADXL345_Addr,0x2D,0x08);   //选择电源模式   参考pdf24页
	Single_Write(ADXL345_Addr,0x2E,0x80);   //使能 DATA_READY 中断
	Single_Write(ADXL345_Addr,0x1E,0x00);   //X 偏移量 根据测试传感器的状态写入pdf29页
	Single_Write(ADXL345_Addr,0x1F,0x00);   //Y 偏移量 根据测试传感器的状态写入pdf29页
	Single_Write(ADXL345_Addr,0x20,0x00);   //Z 偏移量 根据测试传感器的状态写入pdf29?

}
void Init_HMC5883(void)
{
	Single_Write(HMC5883L_Addr,0x00,0x14);   //
	Single_Write(HMC5883L_Addr,0x02,0x00);   //
}
void Init_direction(void)
{
	// Open a connection to the I2C userspace control file.
	if ((i2c_file = open(I2C_FILE_NAME, O_RDWR)) < 0)
	{
		perror("Unable to open i2c control file");
	    exit(1);
	}
	Init_ADXL345();
	Init_HMC5883();
	if((outfd_direction=open("/var/ftp/tmp.log/direction_out", O_WRONLY|O_CREAT))==-1)
	{
		printf("Open direction_out file Error\n");
	   	exit(1);
	}
}
void READ_HMC5883L(void)
{
    DIRECTION_BUF[1]=Single_Read(HMC5883L_Addr,0x03);//OUT_X_L_A
    DIRECTION_BUF[2]=Single_Read(HMC5883L_Addr,0x04);//OUT_X_H_A

    DIRECTION_BUF[3]=Single_Read(HMC5883L_Addr,0x07);//OUT_Y_L_A
    DIRECTION_BUF[4]=Single_Read(HMC5883L_Addr,0x08);//OUT_Y_H_A

    DIRECTION_BUF[5]=Single_Read(HMC5883L_Addr,0x05);//OUT_Z_L_A
    DIRECTION_BUF[6]=Single_Read(HMC5883L_Addr,0x06);//OUT_Z_H_A

    T_X=(DIRECTION_BUF[1] << 8) | DIRECTION_BUF[2]; //Combine MSB and LSB of X Data output register
    T_Y=(DIRECTION_BUF[3] << 8) | DIRECTION_BUF[4]; //Combine MSB and LSB of Y Data output register
    T_Z=(DIRECTION_BUF[5] << 8) | DIRECTION_BUF[6]; //Combine MSB and LSB of Y Data output register

    if(T_X>0x7fff)T_X-=0xffff;
    if(T_Y>0x7fff)T_Y-=0xffff;
    if(T_Z>0x7fff)T_Z-=0xffff;
}
void READ_ADXL345(void)
{
	DIRECTION_BUF[0]=Single_Read(ADXL345_Addr,0x32);
    DIRECTION_BUF[1]=Single_Read(ADXL345_Addr,0x33);
    T_X=	(DIRECTION_BUF[1]<<8)|DIRECTION_BUF[0];

    DIRECTION_BUF[2]=Single_Read(ADXL345_Addr,0x34);
    DIRECTION_BUF[3]=Single_Read(ADXL345_Addr,0x35);
    T_Y=	(DIRECTION_BUF[3]<<8)|DIRECTION_BUF[2];

    DIRECTION_BUF[4]=Single_Read(ADXL345_Addr,0x36);
    DIRECTION_BUF[5]=Single_Read(ADXL345_Addr,0x37);
    T_Z=	(DIRECTION_BUF[5]<<8)|DIRECTION_BUF[4];
}
void Get_Direction(void)
{
	READ_HMC5883L();
	current_direction[0]=T_X;
	current_direction[1]=T_Y;
	current_direction[2]=T_Z;
	READ_ADXL345();
    current_direction[3]=T_X;
	current_direction[4]=T_Y;
	current_direction[5]=T_Z;

	write(outfd_direction,current_direction,12);

}



