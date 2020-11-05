/*************************************************************************************************************
舸轮综合电子工作室(GIE工作室)        出品
Gearing Interated Electronics Studio

十管IN14废土风辉光气象站+万年历

本文件是Wifi联网自动校时与处理的相关函数，来自互联网非原创


关注贴吧ID：tnt34 获得最新工作成果
关注B站UP主：GIE工作室 获得更多视频资源

2018-7-31第一版
版权所有 禁止用于任何商业用途！
注：本程序中DS3231、EPS8266、DS18B20相关驱动文件来自互联网，本工作室版权仅限于glow_tube_display.c & glow_tube_display.h
***********************************************************************************************************/

#include "esp8266.h"
#include <sys.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <usart.h>
#include <timer.h>

NTP NetTime;
extern clock localTime;


void Usart1forEsp8266_Init(unsigned char Fck,unsigned int Bot)
{

	RCC->APB2ENR |= 1<<14;
  GPIOA->CRH   &= 0xFFFFF00F;
  GPIOA->CRH   |= 0x000008b0;
  GPIOA->LCKR  |= 0x0600;
  
	USART1->BRR = Fck*1000000/Bot;
  USART1->CR1 = 8;
	USART1->CR1 = 0x212c;
//  USART1->CR1&=~0x20;  // Clear Receive Interruption
  USART1->CR1 |= 1<<13;  // Enable USART1  
  USART1->CR1 |= 1<<4;   // Enable IDEL
	NVIC->ISER[1] |=1<<5;
  NVIC->IP[37]   = 0x00;
}


void U1Putchar(char data)
{
while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
					USART_SendData(USART1,data);
}

void U1Putstr(char *sp)
{
	while(*sp) U1Putchar(*sp++);
}

void U1_Printf(const char *restric, ...)
{
  char Dis_Buf[128];
  va_list ap;
  va_start(ap,restric);
  vsprintf((char*)Dis_Buf,restric,ap);
  va_end(ap);
  U1Putstr(Dis_Buf);
}




u8 USART1_RX_BUF[128];
u8 USART1_RX_STA;

void USART1_IRQHandler(void)
{
  
  if(USART1->SR&0x20)
  {
     USART1_RX_BUF[USART1_RX_STA++]=USART1->DR;
  }
  if(USART1->SR&0x10)
  {
    USART1->DR;
    USART1_RX_BUF[USART1_RX_STA]='\0';
    USART1_RX_STA|=1<<7;
  }
//  if(USART1_RX_STA>127)
//    USART1_RX_STA|=1<<8;
}


u8* ESP8266_CheckCmd(char* str)// Chack string
{
  char *strx=0;
  if(USART1_RX_STA&0x80)
  {
    USART1_RX_BUF[USART1_RX_STA&0x7f]=0;
    strx=strstr((const char*)USART1_RX_BUF,(const char*)str);
    
  }
  return (u8*)strx;
}

u8 ESP8266_SendCmd(char *cmd,char *ack,u16 waittime)// cmd =1 
{
  u8 res=0;
  U1_Printf("%s\r\n",cmd);
  if(ack&&waittime)
  {
    while(--waittime)
    {
      ysm(10);
      if(USART1_RX_STA&0X80)
      {
        if(NULL != ESP8266_CheckCmd(ack))
        {
          res=0;
//          OLED_Printf("%s....%s\n",cmd,ack);
          break;
        }
        else res=1;
        USART1_RX_STA=0;
      }
    }
    if(waittime==0) res=1;
  }
  return res;
}



void ESP8266_Init(void)
{
  u8 state=0;
 // OLED_xyset(0,2);
  int j;
  USART1_RX_STA = 0;
  memset(USART1_RX_BUF,0,sizeof(USART1_RX_BUF));
  state = ESP8266_SendCmd("AT+CWMODE=3","OK",20);   //Set WiFi mode 3 SoftAP+Station mode
//  if(!state) OLED_Printf("ModeSet...OK\n"); else OLED_Printf("ModeSet...Error\n");
  state = ESP8266_SendCmd("AT+RST","OK",20);
 // if(!state) OLED_Printf("Reset...OK\n"); else OLED_Printf("Reset...Error\n");
	for(j=0;j<10;j++)
	{
	S1201_WriteStr(0,"NTP_CALC");
	ysm(190);
	}
	for(j=9;j>=0;j--)
	{
	S1201_WriteStr(0,"WIFI_CON");
	ysm(190);
	}
//  state = ESP8266_SendCmd("AT+CWJAP=\"ChinaNet-xDVy\",\"yszekdsq\"","OK",10000);  // Connect router
 state = ESP8266_SendCmd("AT+CWJAP=\"nova 5 pro\",\"7104021730114\"","OK",1000);  // Connect router
  if(!state) S1201_WriteStr(0,"WIFI_ERR"); else S1201_WriteStr(0,"WIFI_OK ");
  state = ESP8266_SendCmd("AT+CIPMUX=0","OK",300);
//  if(!state) OLED_Printf("ConnSet...OK\n"); else OLED_Printf("ConnSet..Error\n");
//  state = ESP8266_SendCmd("AT+CIPSTART=\"UDP\",\"1.cn.pool.ntp.org\",123","OK",300); //make connection with NTP
//  if(!state) OLED_Printf("ConnNtp...OK\n"); else OLED_Printf("ConnNtp..Error\n");
}


int month[12]={31,28,31,30,31,30,31,31,30,31,30,31};
u32 datetemp;

u8 getTimeFromNTPServer(void)
{
  u8 packetBuffer[48];
  u32 timeOut=0xffffff;
  u8 i;
  u16 year=1900;
  u32 yearSec;
  
//  ESP8266_SendCmd("AT+CIPSTART=\"UDP\",\"1.cn.pool.ntp.org\",123","OK",100); //make connection with NTP
  U1_Printf("AT+CIPSTART=\"UDP\",\"1.cn.pool.ntp.org\",123\r\n");
  USART1_RX_STA = 0;
  memset(USART1_RX_BUF,0,sizeof(USART1_RX_BUF));
  ysm(100);
  memset(packetBuffer,0,sizeof(packetBuffer));
  ESP8266_SendCmd("AT+CIPSEND=48","OK",100);
  packetBuffer[0] = 0xe3;  // LI, Version, Mode
  packetBuffer[1] = 0;            // Stratum, or type of clock
  packetBuffer[2] = 6;            // Polling Interval
  packetBuffer[3] = 0xEC;         // Peer Clock Precision
  packetBuffer[12] = 49; 
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  USART1_RX_STA = 0;
  memset(USART1_RX_BUF,0,sizeof(USART1_RX_BUF));
  
  for(i=0;i<48;i++)
  {
    U1Putchar(packetBuffer[i]);
  }
  
  while(timeOut--)
  {
    if(USART1_RX_STA&0x80)
    {
      if((USART1_RX_STA-0x80)>=60) 
      {
        USART1_RX_STA = 0;
        break;
      }
      else
      {
        USART1_RX_STA = 0;
        memset(USART1_RX_BUF,0,sizeof(USART1_RX_BUF));
      }
      
    }
  }
  if(0 == timeOut) return 1;
  
//  for(i=10;i<128;i++)
//  UPutchar(USART1_RX_BUF[i]);
//  USART1_RX_STA = 0;
  
  if(0x24 == USART1_RX_BUF[38])
  {
    NetTime.li = ((u8)USART1_RX_BUF[11] & 0xc0)>>6;
    NetTime.secTemp = (u8)USART1_RX_BUF[70];
    NetTime.secTemp <<= 8;
    NetTime.secTemp |= (u8)USART1_RX_BUF[71];
    NetTime.secTemp <<= 8;
    NetTime.secTemp |= (u8)USART1_RX_BUF[72];
    NetTime.secTemp <<= 8;
    NetTime.secTemp |= (u8)USART1_RX_BUF[73];
    USART1_RX_STA = 0;
  }
  else
  {
    USART1_RX_STA = 0;
    return 1;
  }   
  if(3 == NetTime.li) return 2;
  NetTime.secTemp += 28800;  //UTC/GMT+08:00 8h==2800sec
  datetemp = NetTime.secTemp;
  datetemp = datetemp/86400;
  datetemp += 1;
  NetTime.date = datetemp%7;
  
  do
  {  
    if(((0 == year%4) && (0 != year%100)) || 0==year%400)
    {
      yearSec = 31622400;
    }
    else 
			yearSec = 31536000;
    if(NetTime.secTemp < yearSec) break;
    else 
    {
      NetTime.secTemp -= yearSec;
      year++;
    }      
  }while(1); // while(1)
  NetTime.year = year;
  
  if(((0 == year%4) && (0 != year%100)) || 0==year%400)
  {
    month[1] = 29;
  }
  
  for(i=0;i<12;i++)
  {
    if(NetTime.secTemp < month[i]*86400)  //There are 86400sec in 1 day;
      break;
    else
      NetTime.secTemp -= month[i]*86400;
  }
  NetTime.daysInMonth = month[i];
  NetTime.month = i;
  
  NetTime.day = NetTime.secTemp/86400 + 1;
  NetTime.secTemp = NetTime.secTemp % 86400;
  NetTime.hour = NetTime.secTemp/3600;
  NetTime.secTemp = NetTime.secTemp%3600;
  NetTime.min = NetTime.secTemp/60;
  NetTime.sec = NetTime.secTemp%60;
//  U2_Printf("%d %d %d %d:%d:%d\r\n",NetTime.year,NetTime.month,NetTime.day,NetTime.hour,\
//  NetTime.min,NetTime.sec);
  
  return 0;

}


u8 getTime(void)
{
  u8 temp=1;
  u8 timeOut=100;
  while(temp&&timeOut--)
  {
    temp = getTimeFromNTPServer();
  }
  if(0 == timeOut)
  return 1;
  
  localTime.year = NetTime.year;
  localTime.month = NetTime.month;
  localTime.day = NetTime.day;
  localTime.hour = NetTime.hour;
  localTime.min = NetTime.min;
  localTime.sec = NetTime.sec;
  localTime.date = NetTime.date;
  localTime.dateTemp = (u8)datetemp;
  return 0;
  
}

void ysm(unsigned int ms)
{
	u32 ss ;
	ss=ms*9000;
	while(ss)
	{
		if(ss>16000000)
		{
			SysTick->LOAD=16000000;
			ss-=16000000;
		}
		else
		{
			SysTick->LOAD=ss;
			ss=0;
		}
		SysTick->VAL=0;
		SysTick->CTRL=1;
		while(!(0x10000&SysTick->CTRL));
		SysTick->CTRL=0;
	}
}











