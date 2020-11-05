#ifndef _i2c_h
#define _i2c_h

#include "stm32f10x.h"

#define SCL_H() GPIO_SetBits(GPIOB,GPIO_Pin_10)
#define SCL_L() GPIO_ResetBits(GPIOB,GPIO_Pin_10)

#define SDA_H() GPIO_SetBits(GPIOB,GPIO_Pin_11)
#define SDA_L() GPIO_ResetBits(GPIOB,GPIO_Pin_11)

#define IN_SDA() GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11)

#define ds_a = 0 IN_SDA

void IIC_Start(void);
void IIC_Stop(void);
void IIC_Ack(u8 a);
u8 IIC_Write_Byte(u8 dat);
u8 IIC_Read_Byte(void);

#endif
