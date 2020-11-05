/*************************************************************************************************************
舸轮综合电子工作室(GIE工作室)        出品
Gearing Interated Electronics Studio

十管IN14废土风辉光气象站+万年历

联网校时和步进电机所用中断的相关函数，来自互联网非原创


关注贴吧ID：tnt34 获得最新工作成果
关注B站UP主：GIE工作室 获得更多视频资源

2018-7-31第一版
版权所有 禁止用于任何商业用途！
注：本程序中DS3231、EPS8266、DS18B20相关驱动文件来自互联网，本工作室版权仅限于glow_tube_display.c & glow_tube_display.h
***********************************************************************************************************/
#include <sys.h>
#include "timer.h"
#include "esp8266.h"

extern NTP NetTime;
extern void clock_irq(void);
clock localTime;
/**
  *@brief timer2 used for local time counting
  *      TIM2 is 72MHz
  */
void timer2ForClock(void)
{
  RCC->APB1ENR |= 0x00000001;
  TIM2->PSC = 7199;
  TIM2->ARR = 9999;
  TIM2->EGR  |= 0x0001;
  TIM2->DIER |= 0x0001;
  TIM2->CR1  |= 0x0001;
  NVIC->ISER[0] |= 1<<28;
  NVIC->IP[28] = 2<<4;
}


int localmonth[12]={31,28,31,30,31,30,31,31,30,31,30,31};
u8 oneHourFlag=0;
void TIM2_IRQHandler(void)
{
  if(TIM2->SR&1)
  {
    localTime.sec += 1;
    if(60 == localTime.sec)
    {
      localTime.sec = 0;
      localTime.min += 1;
      if(60 == localTime.min)
      {
        localTime.min = 0;
        localTime.hour += 1;
        if(24 == localTime.hour)
        {
          oneHourFlag = 1;
          localTime.hour = 0;
          localTime.day += 1;
          localTime.dateTemp +=1;
          localTime.date = localTime.dateTemp%7;
          if(localTime.day > NetTime.daysInMonth )
          {
            localTime.day = 1;
            localTime.month += 1;
            NetTime.daysInMonth = localmonth[localTime.month-1];
            if(localTime.month > 11)
            {
              localTime.month = 0;
              localTime.year += 1;
            }
          }
        }
      }
    }
    
  }
  
  TIM2->SR = 0;
}

void TIM3_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能
	
	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器


	TIM_Cmd(TIM3, ENABLE);  //使能TIMx					 
}
//定时器3中断服务程序
void TIM3_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
		{
		//if(Radio->Process() != RF_IDLE)
			clock_irq();
			TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx更新中断标志 
		}
}

