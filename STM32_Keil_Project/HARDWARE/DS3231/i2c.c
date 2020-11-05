#include "i2c.h"
#include "gpio.h"
#include "delay.h"

/*I2C起始*/
void IIC_Start(void)
{
	IIC_SDA_OUT();	

	SDA_H();
	delay_us(2);	
	SCL_H();
	delay_us(2);		
	SDA_L();
	delay_us(2);	
	SCL_L();
	delay_us(2);
}
/*I2C停止*/
void IIC_Stop(void)
{
	IIC_SDA_OUT();	

	SDA_L();	
	delay_us(2);
	SCL_L();	
	delay_us(2);
	SDA_H();
	delay_us(2);
}
/*I2C发送应答*/
void IIC_Ack(u8 a)
{
	IIC_SDA_OUT();	

	if(a)	
	SDA_H();
	else	
	SDA_L();

	delay_us(2);
	SCL_H();	
	delay_us(2);
	SCL_L();
	delay_us(2);

}
/*I2C写入一个字节*/
u8 IIC_Write_Byte(u8 dat)
{
	u8 i;
	u8 iic_ack=0;	

	IIC_SDA_OUT();	

	for(i = 0;i < 8;i++)
	{
		if(dat & 0x80)	
		SDA_H();
		else	
		SDA_L();
			
		delay_us(2);
		SCL_H();
	    delay_us(2);
		SCL_L();
		dat<<=1;
	}

	SDA_H();	//释放数据线

	IIC_SDA_IN();	//设置成输入

	delay_us(2);
	SCL_H();
	delay_us(2);
	
	iic_ack |= IN_SDA();	//读入应答位
	SCL_L();
	return iic_ack;	//返回应答信号
}
/*I2C读取一个字节*/
u8 IIC_Read_Byte(void)
{
	u8 i;
	u8 x=0;

	SDA_H();	//首先置数据线为高电平

	IIC_SDA_IN();	//设置成输入

	for(i = 0;i < 8;i++)
	{
		x <<= 1;	//读入数据，高位在前

		delay_us(2);
		SCL_H();	//突变
		delay_us(2);
		
		if(IN_SDA())	x |= 0x01;	//收到高电平

		SCL_L();
		delay_us(2);
	}	//数据接收完成

	SCL_L();

	return x;	//返回读取到的数据
}








