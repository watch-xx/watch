#include "ads1282.h"


void ADS1282_Init()
{   
	 
	 GPIO_InitTypeDef GPIO_InitStructure;
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能PORTA,PORTE时钟

	 GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4;//KEY0-KEY1
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	 GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA4
	 
	 GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5|GPIO_Pin_6;//KEY0-KEY1
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //设置成推挽输出
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 	 GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA5,6
	   
   GPIO_SetBits(GPIOA,GPIO_Pin_5);						 //PB.5 输出高
   GPIO_SetBits(GPIOA,GPIO_Pin_6); 						 //PE.5 输出高 
	 SPI2_Init();
	
	 RESET=1;
	 delay_ms(1);
	 RESET=0;
	 delay_ms(1);
	 RESET=1;
	 delay_ms(1);

//写寄存器前先退出连续模式
   ADS1282_WriyeCommond(ADC_SDATAC);//退出连续读模式
	 delay_ms(1);

   //寄存器初始化，必须延时
   ADS1282_WriteBytes(0x4100,0x42);           //250采样率
	 delay_ms(1);
// ADS1282_WriteBytes(0x4200,0x0e);           //PGA=64
	 delay_ms(1);
	 
	 
//写偏移量	 
//	 ADS1282_WriteBytes(0x4500,0x00);
//	 delay_ms(1);
//	 ADS1282_WriteBytes(0x4600,0x55);
//	 delay_ms(1);
//	 ADS1282_WriteBytes(0x4700,0xfe);
	

///出事化完可进入连续模式，

	 ADS1282_WriyeCommond(ADC_RDATAC);//进入连续读模式	
	 
	///同步转换
	 SYNC=1;
	 delay_ms(1);
	 SYNC=0;
	 delay_ms(1);
	 SYNC=1;
	 delay_ms(1);
	 
}

void ADS1282_WriyeCommond(u8 data)//写命令
{
   SPI2_ReadWriteByte(data); 
}

void ADS1282_WriteBytes(u16 com,u8 data)
{
  SPI2_ReadWriteByte(com>>8);
	delay_us(10);
	SPI2_ReadWriteByte(com);
	delay_us(10);
	SPI2_ReadWriteByte(data);
	delay_us(10);
}

u8 ADS1282_ReadBytes(u16 com)
{
	u8 data;
  SPI2_ReadWriteByte(com>>8);
	delay_us(10);
	SPI2_ReadWriteByte(com);
	delay_us(10);
	data=SPI2_ReadWriteByte(0x00);
	delay_us(10);
	return data;

}


u32 ADS1282_ReadData()
{
	u32 data;
  SPI2_ReadWriteByte(0x12);
	delay_us(10);
	while(DRDY);
	data=SPI2_ReadWriteByte(0x00);
	delay_us(10);
	data<<=8;
	
	data|=SPI2_ReadWriteByte(0x00);
	delay_us(10);
	data<<=8;
	
	data|=SPI2_ReadWriteByte(0x00);
	delay_us(10);
	data<<=8;
	
	data|=SPI2_ReadWriteByte(0x00);
	delay_us(10);
  return data;
}

u32 ADS1282_ReadDataContinu()
{
	u32 data;
  while(DRDY);
	data=SPI2_ReadWriteByte(0x00);
	delay_us(10);
	data<<=8;
	
	data|=SPI2_ReadWriteByte(0x00);
	delay_us(10);
	data<<=8;
	
	data|=SPI2_ReadWriteByte(0x00);
	delay_us(10);
	data<<=8;
	
	data|=SPI2_ReadWriteByte(0x00);
	delay_us(10);
  return data;

}
