/*
 *	ads1282_user.c
 *  Created on: Mar 24, 2018
 *  Author: root
 *  2017-08-19 add ASYNC I/O ;delete
 *  2017-08-19 add the time stamp 48 bits;128 bits in total
 *  2018-03-17 add ioctl
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <getopt.h>
#include <linux/types.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/mman.h>
#include "direction.h"
#define ADS1282_MAGIC	'x'//小写字母
//初始化内存参数配置
#define IOCINIT			_IOW(ADS1282_MAGIC,0,int)
//用户态————>>>核态写参数进来
#define IOCWREG			_IOW(ADS1282_MAGIC,1,int)

#define IOC_MAXNR		2

static int ad_led_1;
#define AD_LED1_H write(ad_led_1,"1",1)//;fflush(ad_led_1)
#define AD_LED1_L write(ad_led_1,"0",1)//;fflush(ad_led_1)


static const char *device ="/dev/ads1282";
 uint8_t *AD_Data_1=NULL;
 uint8_t *AD_Data_2=NULL;
 int current_buf;
 int fd;
 time_t	 time_1,time_2;
 pthread_mutex_t time_lock = PTHREAD_MUTEX_INITIALIZER;
 pthread_mutex_t data_lock = PTHREAD_MUTEX_INITIALIZER;
 pthread_cond_t data_cond =  PTHREAD_COND_INITIALIZER;
 struct tm *ptm;
 struct timeval tv_1,tv_2;
 struct msg{
	 uint8_t		sampling_rate;
	 uint8_t 		gain;
	 unsigned int 	data;
 };

 volatile  int Samp_speed;
 volatile  int Samp_time;
 volatile  int length_set;
 int16_t 	current_direction[6]={0};//电子罗盘数据粗放数组

static void pabort(const char *s)
{
	perror(s);
	abort();
}
void *Data_Write(void *arg)
{
	int fd_out;
	int fd_close;
	char file_name[80]={0};
	char para_string[80]={0};
	struct tm *ptm;
	int inet_sock;
	struct ifreq ifr;
	printf("Write phread is running...!\n");
	ad_led_1 = open("/sys/class/leds/led_1/brightness", O_RDWR);
	inet_sock = socket(AF_INET, SOCK_DGRAM, 0);
	strcpy(ifr.ifr_name, "eth0");//get net ip
	//	strcpy(ifr.ifr_name, "wlan0");//get wifi ip
	if (ioctl(inet_sock, SIOCGIFADDR, &ifr) <  0)
		perror("ioctl error!\n");
	for(;;)
	{
		pthread_mutex_lock(&data_lock);//线程互斥锁
		pthread_cond_wait(&data_cond,&data_lock);//条件变量，无条件等待
		pthread_mutex_unlock(&data_lock);//解锁
		if(current_buf==1)
		{
			pthread_mutex_lock(&time_lock);
			ptm=localtime(&time_2);
			pthread_mutex_unlock(&time_lock);
		}
		else
		{
			pthread_mutex_lock(&time_lock);
			ptm=localtime(&time_1);
			pthread_mutex_unlock(&time_lock);
		}
		Get_Direction();
		AD_LED1_H;
		sprintf(file_name,"/var/ftp/pub/%d%02d%02d%02d%02d%02d",ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec);
		fd_out=open(file_name,O_RDWR | O_CREAT);
		if(fd_out==-1)
		{
			perror("Open fd_out file Error\n");
			exit(1);
		}
		if(current_buf==1)
		{
			pthread_mutex_lock(&time_lock);
			sprintf(para_string,"%s:%d:%d:direction %d %d %d %d %d %d:\n",inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr),length_set,Samp_speed,current_direction[0],current_direction[1],current_direction[2],current_direction[3],current_direction[4],current_direction[5]);
			pthread_mutex_unlock(&time_lock);
			write(fd_out, para_string,80);
			write(fd_out, AD_Data_2,length_set*4*3+48);
		}
		else
		{
			pthread_mutex_lock(&time_lock);
			sprintf(para_string,"%s:%d:%d:direction %d %d %d %d %d %d:\n",inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr),length_set,Samp_speed,current_direction[0],current_direction[1],current_direction[2],current_direction[3],current_direction[4],current_direction[5]);
			pthread_mutex_unlock(&time_lock);
			write(fd_out, para_string,80);
			write(fd_out, AD_Data_1,length_set*4*3+48);
		}
		fd_close=close(fd_out);
		if(fd_close==-1)
		{
			perror("Close fd_out file Error....!\r\n");
			exit(1);
		}
		printf("%s\n",file_name);
		memset(file_name,0,sizeof(file_name));
		AD_LED1_L;
	}

}
void ads1282_user_read(int signum)
{
	if(current_buf==1)
	{
		pthread_mutex_lock(&time_lock);
		time_1=time(NULL);
		gettimeofday(&tv_1,NULL);
		pthread_mutex_unlock(&time_lock);
		read(fd,AD_Data_1,length_set*4+48);
		current_buf=2;
	}
	else
	{
		pthread_mutex_lock(&time_lock);
		time_2=time(NULL);
		gettimeofday(&tv_2,NULL);
		pthread_mutex_unlock(&time_lock);
		read(fd,AD_Data_2,length_set*4+48);
		current_buf=1;
	}
	pthread_mutex_lock(&data_lock);
	pthread_cond_signal(&data_cond);//激活一个等待该条件的进程
	pthread_mutex_unlock(&data_lock);
}
int main(int argc,char *argv[])
{
	int ret=0;
	int oflags;
	char sets_string[128];
	char TempStr[5];
	int fd_sets;
	pthread_t tidp;
	struct msg my_msg;
	memset(&my_msg,0,sizeof(my_msg));
	if((fd_sets=open("/var/ftp/set/sets", O_RDONLY))==-1)
	//sets文件中存放内容为--:freq 2000:secperfile 60:gain 0x78:sampling_rate 0xDF
	{
		printf("Open Sets file Error\n");
		exit(1);
	}
	read(fd_sets, sets_string, 128);
	if(sets_string[1]=='f')
	{
		TempStr[4]='\0';
		memcpy(TempStr,&(sets_string[6]),4);
		Samp_speed=atoi(TempStr);//samp_speed=2000
	}
	if(sets_string[11]=='s')
	{
		TempStr[2]='\0';
		memcpy(TempStr,&(sets_string[22]),2);//将22起始的2个字节复制到T数组中
		Samp_time=atoi(TempStr);//采样时间/s
	}
	if(sets_string[25]=='g')
	{
		TempStr[4]='\0';
		memcpy(TempStr,&(sets_string[30]),4);
		my_msg.gain=strtol(TempStr,NULL,16);
	}
	if(sets_string[35]=='s')
	{
		TempStr[4]='\0';
		memcpy(TempStr,&(sets_string[49]),4);
		my_msg.sampling_rate=strtol(TempStr,NULL,16);
	}
	printf("gain=%x,sampling_rate=%x\r\n",my_msg.gain,my_msg.sampling_rate);
	length_set=Samp_speed * Samp_time;
	my_msg.data=length_set;
	if(close(fd_sets)==-1)
	{
		pabort("fd_sets close error....!");
		exit(1);
	}
	sprintf(sets_string,":freq %d:length_set %d:\r\n",Samp_speed,length_set);
	printf("%s\n",sets_string);

	AD_Data_1=malloc(length_set*4*3+48);//数据加时间戳的空间
	AD_Data_2=malloc(length_set*4*3+48);
	if(AD_Data_1==NULL || AD_Data_2== NULL)
	{
		perror("AD_BUF MALLOC error!\n");
		exit(1);
	}
	current_buf=1;
	Init_direction();
	fd=open(device,O_RDWR,S_IRUSR | S_IWUSR);
	if(fd<0)
		pabort("can't open device!!!!\n");
	Get_Direction();
	ret=ioctl(fd,IOCINIT,&my_msg);
	if(ret)
	{
		perror("ioctl init");
		exit(-5);
	}
	ret=ioctl(fd,IOCWREG,&my_msg);
	if(ret)
	{
		perror("ioctl write");
		exit(-5);
	}
	ret=pthread_create(&tidp,NULL,Data_Write,NULL);
	if(ret)
	{
		perror("write phread is not created...\n");
		exit(1);
	}
	sleep(1);
	signal(SIGIO,ads1282_user_read);
	fcntl(fd,F_SETOWN,getpid());
	oflags=fcntl(fd,F_GETFL);
	fcntl(fd,F_SETFL,oflags | FASYNC);
	for(;;)
	{
		usleep(1);
	}
	free(AD_Data_1);
	free(AD_Data_2);
	AD_Data_1=NULL;
	AD_Data_2=NULL;
	return ret;
}



