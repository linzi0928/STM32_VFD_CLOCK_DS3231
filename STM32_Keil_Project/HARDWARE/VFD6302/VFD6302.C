/*************************************************************************************************************
舸轮综合电子工作室(GIE工作室)        移植自LGL VFD Stduio
Gearing Interated Electronics Studio

8位VFD时钟驱动程序

关注B站UP主：GIE工作室 获得更多视频资源及动态

2020-10-14第一版 by Vanilla's Lab
*************************************************************************************************************/
#include "VFD6302.H"
//VFD写入
void write_6302(unsigned char w_data){
  unsigned char i;
  for (i = 0; i < 8; i++)
  {
    
    if ( (w_data & 0x01) == 0x01)
    {
      DIN = 1;
    }
    else
    {
      DIN = 0;
    }
		CLK = 0;
		delay_us(5);
    w_data >>= 1;
    CLK = 1;
		delay_us(5);
  }
}

//VFD显示
void S1201_show(void)
{
  CS = 0;//开始传输
  write_6302(0xe8);//地址寄存器起始位置
  CS = 1; //停止传输
}

//初始化VFD
void VFD_init()
{
  //设置有多少个数字字符
  CS = 0;
  write_6302(0xe0);
  delay_ms(5);
  write_6302(0x07);//8个
  CS = 1;
  delay_ms(5);

  //设置亮度
  CS = 0;
  write_6302(0xe4);
  delay_ms(5);
  write_6302(200);//0~255（16进制：0x00~0xff） 
  CS = 1;
  delay_ms(5);
}

/*（不推荐使用）
 (x,chr)
 x:显示该字符的位置0-7
 chr:该字符在datasheet里的位置?
 */
void S1201_WriteOneChar(unsigned char x, unsigned char chr)
{
  CS = 0;  //开始传输
  write_6302(0x20 + x); //地址寄存器起始位置
  write_6302(chr + 0x30);
  CS = 1; //停止传输
  S1201_show();
}

/*
 改进版
 (x,chr)
 x:显示该字符的位置0-7
 chr:该字符的ASCII
 */
void S1201_WriteOneChar1(unsigned char x, unsigned char chr)
{
  CS = 0;  //开始传输
  write_6302(0x20 + x); //地址寄存器起始位置
  write_6302(chr);
  CS = 1; //停止传输
  S1201_show();
}

/*
 在指定位置打印字符串
 (x,str)
 x:显示该字符串的位置0-7
 str:要显示的字符串
 */
void S1201_WriteStr(int x, char* str)
{
  u8 i;
	for(i = 0;i < strlen(str);i++)
	{
    S1201_WriteOneChar1(i+x,str[i]);   
  }
}

//VFD清屏
void S1201_clear()
{
  u8 i;
	for(i = 0;i<8;i++)
	{
    S1201_WriteOneChar1(i,16);
  }
}
void bri_set(u8 bri_val)
{
  CS = 0;
  write_6302(0xe4);
  delay_us(10);
  write_6302(bri_val);//0~255（16进制：0x00~0xff） 
  CS = 1;
  delay_us(10);
}
void VFD_IO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//关闭jtag，使能SWD，可以用SWD模式调试

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_6|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_8;	 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//速度50MHz
 	GPIO_Init(GPIOA, &GPIO_InitStructure);	  //初始化GPIOD3,6
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;	
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//速度50MHz
 	GPIO_Init(GPIOB, &GPIO_InitStructure);	  
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2;	
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//速度50MHz
 	GPIO_Init(GPIOA, &GPIO_InitStructure);	  
//	DIN=1;
	//CLK=1;
	RST = 0;
	EN = 1;
	delay_ms(5);
	RST = 1;
}
