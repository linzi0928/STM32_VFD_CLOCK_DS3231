/*************************************************************************************************************
舸轮综合电子工作室(GIE工作室)        出品
Gearing Interated Electronics Studio

8位VFD时钟驱动程序

关注B站UP主：GIE工作室 获得更多视频资源及动态

2020-10-14第一版 by Vanilla's Lab
2020-10-22第二版 by GIE Studio
*************************************************************************************************************/
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "string.h"
#include "VFD6302.H"
#include "gpio.h"
#include "i2c.h"
#include "ds3231.h"
#include "timer.h"
#define READ_PUSH PAin(0)
#define READ_CCW PAin(1)
#define READ_CW PAin(2)
int second_last;
int sec_flag,wifi_flag=0;
char dateD[]="10-21 W3";
char timeD[]="00:00:00";
int tyear,tdate,tmon,tweek,tmin,tsec,thour,modeD=0;
extern u8 getTime(void);
extern void ESP8266_Init(void);
extern void One_Step(int dir);
extern clock localTime;
extern clock NetTime;
u8 i=0,ix=0;
void time_refresh(void);
void wifi_calc(void);
int main(void)
{		
	//delay_init();	    	 //延时函数初始化	  
	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	
	VFD_IO_Init();
	VFD_init();
	uart_init(115200);
	TIM3_Int_Init(625,31);//时间显示刷新中断
	TimeValue.year=0x2020;//如果下面那行不打开，这些初始时间均无效	
	TimeValue.month=0x10;
	TimeValue.week=0x05;
	TimeValue.date=0x25;
	TimeValue.hour=0x23;
	TimeValue.minute=0x59;
	TimeValue.second=0x55;
	//DS3231_Time_Init(&TimeValue);//调试用，程序强制写入时间值
	LED_BLUE=0;
	while(1)
	{	
		for(ix=80;ix>0;ix--)
		{
		time_refresh();
		S1201_WriteStr(0,timeD);
		wifi_calc();			
		delay_ms(50);		
		}
	if(modeD==0)
	{	
			for(ix=200;ix>0;ix--)
			{
			bri_set(ix);
			delay_ms(1);
			if((ix%7)==0)
				{
					time_refresh();
					wifi_calc();
					S1201_WriteStr(0,timeD);
				}
			}
			S1201_WriteStr(0,dateD);
			for(ix=0;ix<200;ix++)
			{
			bri_set(ix);
			delay_ms(1);
			}
			for(ix=95;ix>0;ix--)
			{
			wifi_calc();
			delay_ms(10);
			}
			for(ix=200;ix>0;ix--)
			{
			wifi_calc();
			bri_set(ix);
			delay_ms(1);
			}
			for(ix=0;ix<200;ix++)
			{
			bri_set(ix);
			delay_ms(1);
			if((ix%7)==0)
				{
					time_refresh();
					wifi_calc();
					S1201_WriteStr(0,timeD);
				}
			}
		}
	}
 }
void time_refresh(void)
{
		Time_Handle();//获取时间
 		tmon=10*(TimeValue.month/16)+(TimeValue.month%16);
		tdate=10*(TimeValue.date/16)+(TimeValue.date%16);
		tweek=TimeValue.week;
		if(tweek==0)
			tweek=7;
		tmin=10*(TimeValue.minute/16)+(TimeValue.minute%16);
		tsec=10*(TimeValue.second/16)+(TimeValue.second%16);
		thour=10*(TimeValue.hour/16)+(TimeValue.hour%16);
		
		dateD[0]=tmon/10+48;dateD[1]=tmon%10+48;
		dateD[3]=tdate/10+48;dateD[4]=tdate%10+48;
		dateD[7]=tweek+48;
		timeD[0]=thour/10+48;timeD[1]=thour%10+48;
		timeD[3]=tmin/10+48;timeD[4]=tmin%10+48;
		timeD[6]=tsec/10+48;timeD[7]=tsec%10+48;
}
void wifi_calc(void)
{
	if(wifi_flag==1)
		{
			S1201_WriteStr(0,"NTP_CALC");
			ESP8266_Init();//初始化8266 wifi模块，连接到路由器
			S1201_WriteStr(0,"NTP_CONN");
			if(getTime())S1201_WriteStr(0,"NTP_ERR");//获取时间
			else 
			{
				S1201_WriteStr(0,"NET_GET");//获取时间成功后对本地时钟进行校准
				TimeValue.month=(((localTime.month+1)/10)<<4)+(((localTime.month+1)%10)&0x0F);//注意NTP的月份是0-11所以需要+1，然后通过移位转换为BCD码才能写入3231
				TimeValue.date=((localTime.day/10)<<4)+((localTime.day%10)&0x0F);
				TimeValue.hour=((localTime.hour/10)<<4)+((localTime.hour%10)&0x0F);
				TimeValue.minute=((localTime.min/10)<<4)+((localTime.min%10)&0x0F);
				TimeValue.second=((localTime.sec/10)<<4)+((localTime.sec%10)&0x0F);
				TimeValue.week=localTime.date;
				DS3231_Time_Init(&TimeValue);//向3231校准本地时间
				S1201_WriteStr(0,"TIME_GET");
				delay_ms(1000);
				wifi_flag=0;
			}
		}
		if(sec_flag==1)
		{
			sec_flag=0;
			LED_BLUE=~LED_BLUE;
		}
}
void clock_irq(void)
{
	int menu_pos=0,hourk,mink,datek,monk,weekk;
	char timeDk[2]="  ";
	if(READ_PUSH==0)
	{
		delay_ms(10);
		if(READ_PUSH==0)
		{
			bri_set(255);
			if(wifi_flag==1)
				{
					S1201_WriteStr(0,"CANCELED");
					delay_ms(1000);
					__set_FAULTMASK(1);
					NVIC_SystemReset(); 
				}
			S1201_WriteStr(0,">MENU   ");
			delay_ms(1000);
			while(READ_PUSH==0);
			while(1)
				{
					if(READ_CW==0)
					{
						menu_pos++;
					while(READ_CW==0);
					if (menu_pos>2)
						menu_pos=0;
					}
					if(READ_CCW==0)
					{
						menu_pos--;
					while(READ_CCW==0);
					if (menu_pos<0)
						menu_pos=2;
					}
					if(menu_pos==0)
					{
					S1201_WriteStr(0,">BY WIFI ");
					if(READ_PUSH==0)
						{
							delay_ms(10);
								if(READ_PUSH==0)
								{
									while(1)
									{
									S1201_WriteStr(0,"WIFIINIT");
									delay_ms(1000);
									while(READ_PUSH==0);
									wifi_flag=1;
									return;
									}
								}
						}
					}
					if(menu_pos==1)
					{
					S1201_WriteStr(0,">BY HAND ");
							if(READ_PUSH==0)
								{
									delay_ms(10);
									if(READ_PUSH==0)
									{
										while(READ_PUSH==0);
										hourk=thour;
										mink=tmin;
										monk=tmon;
										datek=tdate;
										weekk=tweek;
										S1201_WriteStr(0,timeD);
										while(1)
										{
											ix=10;
											while(ix--)
											{
												delay_ms(10);
												if(READ_CW==0)
												{
													while(READ_CW==0);
													hourk++;
													if(hourk>23)
														hourk=0;
												}
												if(READ_CCW==0)
												{
													while(READ_CCW==0);
													hourk--;
													if(hourk<0)
														hourk=23;
												}
												timeDk[0]=hourk/10+48;
												timeDk[1]=hourk%10+48;
												S1201_WriteStr(0,timeDk);
											}
											ix=10;
											while(ix--)
											{
												delay_ms(10);
												if(READ_CW==0)
												{
													while(READ_CW==0);
													hourk++;
													if(hourk>23)
														hourk=0;
												}
												if(READ_CCW==0)
												{
													while(READ_CCW==0);
													hourk--;
													if(hourk<0)
														hourk=23;
												}
												timeDk[0]=hourk/10+48;
												timeDk[1]=hourk%10+48;
												S1201_WriteStr(0,"  ");
											}
											if(READ_PUSH==0)
											{
												while(READ_PUSH==0);
												S1201_WriteStr(0,timeDk);
												while(1)
												{
																ix=10;
																while(ix--)
																{
																	delay_ms(10);
																	if(READ_CW==0)
																	{
																		while(READ_CW==0);
																		mink++;
																		if(mink>59)
																			mink=0;
																	}
																	if(READ_CCW==0)
																	{
																		while(READ_CCW==0);
																		mink--;
																		if(mink<0)
																			mink=59;
																	}
																	timeDk[0]=mink/10+48;
																	timeDk[1]=mink%10+48;
																	S1201_WriteStr(3,timeDk);
																}
																ix=10;
																while(ix--)
																{
																	delay_ms(10);
																	if(READ_CW==0)
																	{
																		while(READ_CW==0);
																		mink++;
																		if(mink>59)
																			mink=0;
																	}
																	if(READ_CCW==0)
																	{
																		while(READ_CCW==0);
																		mink--;
																		if(mink<0)
																			mink=59;
																	}
																	timeDk[0]=mink/10+48;
																	timeDk[1]=mink%10+48;
																	S1201_WriteStr(3,"  ");
																}
																if(READ_PUSH==0)
																{
																	delay_ms(10);
																	if(READ_PUSH==0)
																	{
																		S1201_WriteStr(0,dateD);
																		while(1)
																		{
																					ix=10;
																					while(ix--)
																					{
																						delay_ms(10);
																						if(READ_CW==0)
																						{
																							while(READ_CW==0);
																							monk++;
																							if(monk>12)
																								monk=1;
																						}
																						if(READ_CCW==0)
																						{
																							while(READ_CCW==0);
																							monk--;
																							if(monk<1)
																								monk=12;
																						}
																						timeDk[0]=monk/10+48;
																						timeDk[1]=monk%10+48;
																						S1201_WriteStr(0,timeDk);
																					}
																					ix=10;
																					while(ix--)
																					{
																						delay_ms(10);
																						if(READ_CW==0)
																						{
																							while(READ_CW==0);
																							monk++;
																							if(monk>12)
																								monk=1;
																						}
																						if(READ_CCW==0)
																						{
																							while(READ_CCW==0);
																							monk--;
																							if(monk<1)
																								monk=12;
																						}
																						timeDk[0]=monk/10+48;
																						timeDk[1]=monk%10+48;
																						S1201_WriteStr(0,"  ");
																					}
																					if(READ_PUSH==0)
																								{
																									delay_ms(10);
																									if(READ_PUSH==0)
																									{
																										S1201_WriteStr(0,timeDk);
																										while(1)
																										{
																													ix=10;
																													while(ix--)
																													{
																														delay_ms(10);
																														if(READ_CW==0)
																														{
																															while(READ_CW==0);
																															datek++;
																															if(datek>31)
																																datek=1;
																														}
																														if(READ_CCW==0)
																														{
																															while(READ_CCW==0);
																															datek--;
																															if(datek<1)
																																datek=31;
																														}
																														timeDk[0]=datek/10+48;
																														timeDk[1]=datek%10+48;
																														S1201_WriteStr(3,timeDk);
																													}
																													ix=10;
																													while(ix--)
																													{
																														delay_ms(10);
																														if(READ_CW==0)
																														{
																															while(READ_CW==0);
																															datek++;
																															if(datek>31)
																																datek=1;
																														}
																														if(READ_CCW==0)
																														{
																															while(READ_CCW==0);
																															datek--;
																															if(datek<1)
																																datek=31;
																														}
																														timeDk[0]=datek/10+48;
																														timeDk[1]=datek%10+48;
																														S1201_WriteStr(3,"  ");
																													}
																																			if(READ_PUSH==0)
																																			{
																																				delay_ms(10);
																																				if(READ_PUSH==0)
																																				{
																																					S1201_WriteStr(3,timeDk);
																																					while(1)
																																					{
																																								ix=10;
																																								while(ix--)
																																								{
																																									delay_ms(10);
																																									if(READ_CW==0)
																																									{
																																										while(READ_CW==0);
																																										weekk++;
																																										if(weekk>7)
																																											weekk=1;
																																									}
																																									if(READ_CCW==0)
																																									{
																																										while(READ_CCW==0);
																																										weekk--;
																																										if(weekk<1)
																																											weekk=7;
																																									}
																																									timeDk[0]=weekk+48;
																																									S1201_WriteStr(7,timeDk);
																																								}
																																								ix=10;
																																								while(ix--)
																																								{
																																									delay_ms(10);
																																									if(READ_CW==0)
																																									{
																																										while(READ_CW==0);
																																										weekk++;
																																										if(weekk>7)
																																											weekk=1;
																																									}
																																									if(READ_CCW==0)
																																									{
																																										while(READ_CCW==0);
																																										weekk--;
																																										if(weekk<1)
																																											weekk=7;
																																									}
																																									timeDk[0]=weekk+48;
																																									S1201_WriteStr(7,"  ");
																																								}
																																								if(READ_PUSH==0)
																																								{
																																									delay_ms(10);
																																									if(READ_PUSH==0)
																																									{
																																										S1201_WriteStr(0,"CALCOVER");
																																										TimeValue.month=(monk/10)*16+monk%10;
																																										if(weekk>6)
																																											weekk=0;
																																										TimeValue.week=(weekk/10)*16+weekk%10;
																																										TimeValue.date=(datek/10)*16+datek%10;
																																										TimeValue.hour=(hourk/10)*16+hourk%10;
																																										TimeValue.minute=(mink/10)*16+mink%10;
																																										DS3231_Time_Init(&TimeValue);
																																										delay_ms(700);
																																										return;
																																									}
																																								}
																																					}
																																				}
																																			}
																										}
																									}
																								}
																		}
																	}
																}
												}
											}
										}
									}
								}
					}
					if(menu_pos==2)
					{
					S1201_WriteStr(0,">RETURN  ");
							if(READ_PUSH==0)
							{
								delay_ms(10);
									if(READ_PUSH==0)
									{
										while(1)
										{
											delay_ms(10);
											while(READ_PUSH==0);
											return;
										}
									}
							}
					}
			}
		}
	}
	if(READ_CCW==0)
	{
		delay_ms(10);
		if(READ_CCW==0)
		{
			while(READ_CCW==0);
			bri_set(255);
			if(modeD==1)
			{
				modeD=0;
				S1201_WriteStr(0,"MODE  01");
				delay_ms(1000);
			}
			else
			{
				modeD=1;
				S1201_WriteStr(0,"MODE  02");
				delay_ms(1000);
			}
		}
	}
}
