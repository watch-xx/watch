#include "ads1282.h"


void ADS1282_Init()
{   
	 
	 GPIO_InitTypeDef GPIO_InitStructure;
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//ʹ��PORTA,PORTEʱ��

	 GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4;//KEY0-KEY1
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	 GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA4
	 
	 GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5|GPIO_Pin_6;//KEY0-KEY1
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //���ó��������
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
 	 GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA5,6
	   
   GPIO_SetBits(GPIOA,GPIO_Pin_5);						 //PB.5 �����
   GPIO_SetBits(GPIOA,GPIO_Pin_6); 						 //PE.5 ����� 
	 SPI2_Init();
	
	 RESET=1;
	 delay_ms(1);
	 RESET=0;
	 delay_ms(1);
	 RESET=1;
	 delay_ms(1);

//д�Ĵ���ǰ���˳�����ģʽ
   ADS1282_WriyeCommond(ADC_SDATAC);//�˳�������ģʽ
	 delay_ms(1);

   //�Ĵ�����ʼ����������ʱ
   ADS1282_WriteBytes(0x4100,0x42);           //250������
	 delay_ms(1);
// ADS1282_WriteBytes(0x4200,0x0e);           //PGA=64
	 delay_ms(1);
	 
	 
//дƫ����	 
//	 ADS1282_WriteBytes(0x4500,0x00);
//	 delay_ms(1);
//	 ADS1282_WriteBytes(0x4600,0x55);
//	 delay_ms(1);
//	 ADS1282_WriteBytes(0x4700,0xfe);
	

///���»���ɽ�������ģʽ��

	 ADS1282_WriyeCommond(ADC_RDATAC);//����������ģʽ	
	 
	///ͬ��ת��
	 SYNC=1;
	 delay_ms(1);
	 SYNC=0;
	 delay_ms(1);
	 SYNC=1;
	 delay_ms(1);
	 
}

void ADS1282_WriyeCommond(u8 data)//д����
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
