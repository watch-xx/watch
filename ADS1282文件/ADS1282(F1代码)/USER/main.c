#include "stm32f10x.h"
#include "ads1282.h"
#include "usart.h"
#include "spi.h"
#include "delay.h"

 int main(void)
 {	
  u32 data;	 
	double volote;
	uart_init(115200); 
	delay_init();
	ADS1282_Init();

	delay_ms(100);
	
	
  while(1)
	{		
			
//	data=ADS1282_ReadData();         //通过寄存器读
		data=ADS1282_ReadDataContinu();  //连续读，转换完就送DOUT.
		data=data/1718;                  //uv
    printf("%d\r\n",data);	
  	 
	}
 }


//		regs0=ADS1282_ReadBytes(0x2000);
//		regs1=ADS1282_ReadBytes(0x2100);
//		regs2=ADS1282_ReadBytes(0x2200);
//		regs3=ADS1282_ReadBytes(0x2300);
//		regs4=ADS1282_ReadBytes(0x2400);
//		regs5=ADS1282_ReadBytes(0x2500);
//		regs6=ADS1282_ReadBytes(0x2600);
//		regs7=ADS1282_ReadBytes(0x2700); 
//		printf("regs0  %x\r\n",regs0);
//		printf("regs1  %x\r\n",regs1);
//		printf("regs2  %x\r\n",regs2);
//		printf("regs3  %x\r\n",regs3);
//		printf("regs4  %x\r\n",regs4);
//		printf("regs5  %x\r\n",regs5);
//		printf("regs6  %x\r\n",regs6);
//		printf("regs7  %x\r\n",regs7);
//		