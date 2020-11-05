#include "ds3231.h"
 /******************************************************************************
* @ File name --> ds3231.c
* @ Author    --> By@ Sam Chan
* @ Version   --> V1.0
* @ Date      --> 02 - 01 - 2014
* @ Brief     --> 高精度始终芯片DS3231驱动函数
*
* @ Copyright (C) 20**
* @ All rights reserved
*******************************************************************************
*
*                                  File Update
* @ Version   --> V1.
* @ Author    -->
* @ Date      -->
* @ Revise    -->
*
******************************************************************************/

/******************************************************************************
                               定义显示时间格式
                         要改变显示的格式请修改此数组
******************************************************************************/

u8 Display_Time[8] = {0x30,0x30,0x3a,0x30,0x30,0x3a,0x30,0x30};	
					//时间显示缓存   格式  00:00:00

u8 Display_Date[13] = {0x32,0x30,0x31,0x33,0x2f,0x31,0x30,0x2f,0x32,0x30,0x20,0x37,0x57};
					//日期显示缓存   格式  2013/10/20 7W

/******************************************************************************
                               定义相关的变量函数
******************************************************************************/

Time_Typedef TimeValue;	//定义时间数据指针


u8 Time_Buffer[7];	//时间日历数据缓存


/******************************************************************************
* Function Name --> DS3231某寄存器写入一个字节数据
* Description   --> none
* Input         --> REG_ADD：要操作寄存器地址
*                   dat：要写入的数据
* Output        --> none
* Reaturn       --> none 
******************************************************************************/
void DS3231_Write_Byte(u8 REG_ADD,u8 dat)
{
	IIC_Start();
	if(!(IIC_Write_Byte(DS3231_Write_ADD)))	//发送写命令并检查应答位
	{
		IIC_Write_Byte(REG_ADD);
		IIC_Write_Byte(dat);	//发送数据
	}
	IIC_Stop();
}
/******************************************************************************
* Function Name --> DS3231某寄存器读取一个字节数据
* Description   --> none
* Input         --> REG_ADD：要操作寄存器地址
* Output        --> none
* Reaturn       --> 读取到的寄存器的数值 
******************************************************************************/
u8 DS3231_Read_Byte(u8 REG_ADD)
{
	u8 ReData;
	IIC_Start();
	if(!(IIC_Write_Byte(DS3231_Write_ADD)))	//发送写命令并检查应答位
	{
		IIC_Write_Byte(REG_ADD);	//确定要操作的寄存器
		IIC_Start();	//重启总线
		IIC_Write_Byte(DS3231_Read_ADD);	//发送读取命令
		ReData = IIC_Read_Byte();	//读取数据
		IIC_Ack(1);	//发送非应答信号结束数据传送
	}
	IIC_Stop();
	return ReData;
}
/******************************************************************************
* Function Name --> DS3231对时间日历寄存器操作，写入数据或者读取数据
* Description   --> 连续写入n字节或者连续读取n字节数据
* Input         --> REG_ADD：要操作寄存器起始地址
*                   *WBuff：写入数据缓存
*                   num：写入数据数量
*                   mode：操作模式。0：写入数据操作。1：读取数据操作
* Output        --> none
* Reaturn       --> none
******************************************************************************/
void DS3231_Operate_Register(u8 REG_ADD,u8 *pBuff,u8 num,u8 mode)
{
	u8 i;
	if(mode)	//读取数据
	{
		IIC_Start();
		if(!(IIC_Write_Byte(DS3231_Write_ADD)))	//发送写命令并检查应答位
		{
			IIC_Write_Byte(REG_ADD);	//定位起始寄存器地址
			IIC_Start();	//重启总线
			IIC_Write_Byte(DS3231_Read_ADD);	//发送读取命令
			for(i = 0;i < num;i++)
			{
				*pBuff = IIC_Read_Byte();	//读取数据
				if(i == (num - 1))	IIC_Ack(1);	//发送非应答信号
				else IIC_Ack(0);	//发送应答信号
				pBuff++;
			}
		}
		IIC_Stop();	
	}
	else	//写入数据
	{		 	
		IIC_Start();
		if(!(IIC_Write_Byte(DS3231_Write_ADD)))	//发送写命令并检查应答位
		{
			IIC_Write_Byte(REG_ADD);	//定位起始寄存器地址
			for(i = 0;i < num;i++)
			{
				IIC_Write_Byte(*pBuff);	//写入数据
				pBuff++;
			}
		}
		IIC_Stop();
	}
}
/******************************************************************************
* Function Name --> DS3231读取或者写入时间信息
* Description   --> 连续写入n字节或者连续读取n字节数据
* Input         --> *pBuff：写入数据缓存
*                   mode：操作模式。0：写入数据操作。1：读取数据操作
* Output        --> none
* Reaturn       --> none
******************************************************************************/
void DS3231_ReadWrite_Time(u8 mode)
{
	u8 Time_Register[8];	//定义时间缓存
	
	if(mode)	//读取时间信息
	{
		DS3231_Operate_Register(Address_second,Time_Register,7,1);	//从秒地址（0x00）开始读取时间日历数据
		
		/******将数据复制到时间结构体中，方便后面程序调用******/
		TimeValue.second = Time_Register[0] & Shield_secondBit;	//秒数据
		if(TimeValue.second!=second_last)
		{
			sec_flag=1;
			second_last=TimeValue.second;
		}
		else
			sec_flag=0;
		TimeValue.minute = Time_Register[1] & Shield_minuteBit;	//分钟数据
		TimeValue.hour   = Time_Register[2] & Shield_hourBit;	//小时数据
		TimeValue.week   = Time_Register[3] & Shield_weekBit;	//星期数据
		TimeValue.date   = Time_Register[4] & Shield_dateBit;	//日数据
		TimeValue.month  = Time_Register[5] & Shield_monthBit;	//月数据
		TimeValue.year   = Time_Register[6] | DS3231_YEARDATA;	//年数据
	}
	else
	{
		/******从时间结构体中复制数据进来******/
		Time_Register[0] = TimeValue.second;	//秒
		Time_Register[1] = TimeValue.minute;	//分钟
		Time_Register[2] = TimeValue.hour | Hour_Mode24;	//小时
		Time_Register[3] = TimeValue.week;	//星期
		Time_Register[4] = TimeValue.date;	//日
		Time_Register[5] = TimeValue.month;	//月
		Time_Register[6] = (u8)TimeValue.year;	//年
		
		DS3231_Operate_Register(Address_second,Time_Register,7,0);	//从秒地址（0x00）开始写入时间日历数据
	}
}
/******************************************************************************
* Function Name --> 时间日历初始化
* Description   --> none
* Input         --> *TimeVAL：RTC芯片寄存器值指针
* Output        --> none
* Reaturn       --> none
******************************************************************************/
void DS3231_Time_Init(Time_Typedef *TimeVAL)
{	
	//时间日历数据
	Time_Buffer[0] = TimeVAL->second;
	Time_Buffer[1] = TimeVAL->minute;
	Time_Buffer[2] = TimeVAL->hour;
	Time_Buffer[3] = TimeVAL->week;
	Time_Buffer[4] = TimeVAL->date;
	Time_Buffer[5] = TimeVAL->month;
	Time_Buffer[6] = (u8)TimeVAL->year;
	
	DS3231_Operate_Register(Address_second,Time_Buffer,7,0);	//从秒（0x00）开始写入7组数据
	DS3231_Write_Byte(Address_control, OSC_Enable);
	DS3231_Write_Byte(Address_control_status, Clear_OSF_Flag);
}
/******************************************************************************
* Function Name --> DS3231检测函数
* Description   --> 将读取到的时间日期信息转换成ASCII后保存到时间格式数组中
* Input         --> none
* Output        --> none
* Reaturn       --> 0: 正常
*                   1: 不正常或者需要初始化时间信息
******************************************************************************/
u8 DS3231_Check(void)
{
	if(DS3231_Read_Byte(Address_control_status) & 0x80)  //晶振停止工作了
	{
		return 1;  //异常
	}
	else if(DS3231_Read_Byte(Address_control) & 0x80)  //或者 EOSC被禁止了
	{
		return 1;  //异常
	}
	else	return 0;  //正常
}
/******************************************************************************
* Function Name --> 时间日历数据处理函数
* Description   --> 将读取到的时间日期信息转换成ASCII后保存到时间格式数组中
* Input         --> none
* Output        --> none
* Reaturn       --> none
******************************************************************************/
void Time_Handle(void)
{
	/******************************************************
	                   读取时间日期信息
	******************************************************/
	
	DS3231_ReadWrite_Time(1);	//获取时间日历数据
	
	/******************************************************
	            时间信息转换为ASCII码可视字符
	******************************************************/
	
	Display_Time[6] = (TimeValue.second >> 4) + 0x30;
	Display_Time[7] = (TimeValue.second & 0x0f) + 0x30;	//Second

	Display_Time[3] = (TimeValue.minute >> 4) + 0x30;
	Display_Time[4] = (TimeValue.minute & 0x0f) + 0x30;	//Minute

	Display_Time[0] = (TimeValue.hour >> 4) + 0x30;
	Display_Time[1] = (TimeValue.hour & 0x0f) + 0x30;	//Hour 

	Display_Date[8] = (TimeValue.date >> 4) + 0x30;
	Display_Date[9] = (TimeValue.date & 0x0f) + 0x30;	//Date

	Display_Date[5] = (TimeValue.month >> 4) + 0x30;
	Display_Date[6] = (TimeValue.month & 0x0f) + 0x30;	//Month

	Display_Date[0] = (u8)(TimeValue.year >> 12) + 0x30;
	Display_Date[1] = (u8)((TimeValue.year & 0x0f00) >> 8) + 0x30;
	Display_Date[2] = (TimeValue.year >> 4) + 0x30;
	Display_Date[3] = (TimeValue.year & 0x0f) + 0x30;	//Year

	Display_Date[11] = (TimeValue.week & 0x0f) + 0x30;	//week

}
/******************************************************************************
* Function Name --> 读取芯片温度寄存器
* Description   --> 温度寄存器地址为0x11和0x12，这两寄存器为只读
* Input         --> none
* Output        --> *Temp：最终温度显示字符缓存
* Reaturn       --> none
******************************************************************************/
void DS3231_Read_Temp(u8 *Temp)
{
	u8 temph,templ;
	float temp_dec;

	temph = DS3231_Read_Byte(Address_temp_MSB);	//读取温度高8bits
	templ = DS3231_Read_Byte(Address_temp_LSB) >> 6;	//读取温度低2bits

	//温度值转换
	if(temph & 0x80)	//判断温度值的正负
	{	//负温度值
		temph = ~temph;	//高位取反
		templ = ~templ + 0x01;	//低位取反加1
		Temp[0] = 0x2d;	//显示“-”
	}
	else	Temp[0] = 0x20;	//正温度不显示符号，显示正号填0x2b

	//小数部分计算处理
	temp_dec = (float)templ * (float)0.25;	//0.25℃分辨率

	//整数部分计算处理
	temph = temph & 0x70;	//去掉符号位
	Temp[1] = temph % 1000 / 100 + 0x30;	//百位
	Temp[2] = temph % 100 / 10 + 0x30;	//十位
	Temp[3] = temph % 10 + 0x30;	//个位
	Temp[4] = 0x2e;	//.

	//小数部分处理
	Temp[5] = (u8)(temp_dec * 10) + 0x30;	//小数点后一位
	Temp[6] = (u8)(temp_dec * 100) % 10 + 0x30;	//小数点后二位

	if(Temp[1] == 0x30)	Temp[1] = 0x20;	//百位为0时不显示
	if(Temp[2] == 0x30)	Temp[2] = 0x20;	//十位为0时不显示
}

