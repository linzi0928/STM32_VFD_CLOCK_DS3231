/*************************************************************************************************************
�����ۺϵ��ӹ�����(GIE������)        ��ֲ��LGL VFD Stduio
Gearing Interated Electronics Studio

8λVFDʱ����������

��עBվUP����GIE������ ��ø�����Ƶ��Դ����̬

2020-10-14��һ�� by Vanilla's Lab
*************************************************************************************************************/
#include "VFD6302.H"
//VFDд��
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

//VFD��ʾ
void S1201_show(void)
{
  CS = 0;//��ʼ����
  write_6302(0xe8);//��ַ�Ĵ�����ʼλ��
  CS = 1; //ֹͣ����
}

//��ʼ��VFD
void VFD_init()
{
  //�����ж��ٸ������ַ�
  CS = 0;
  write_6302(0xe0);
  delay_ms(5);
  write_6302(0x07);//8��
  CS = 1;
  delay_ms(5);

  //��������
  CS = 0;
  write_6302(0xe4);
  delay_ms(5);
  write_6302(200);//0~255��16���ƣ�0x00~0xff�� 
  CS = 1;
  delay_ms(5);
}

/*�����Ƽ�ʹ�ã�
 (x,chr)
 x:��ʾ���ַ���λ��0-7
 chr:���ַ���datasheet���λ��?
 */
void S1201_WriteOneChar(unsigned char x, unsigned char chr)
{
  CS = 0;  //��ʼ����
  write_6302(0x20 + x); //��ַ�Ĵ�����ʼλ��
  write_6302(chr + 0x30);
  CS = 1; //ֹͣ����
  S1201_show();
}

/*
 �Ľ���
 (x,chr)
 x:��ʾ���ַ���λ��0-7
 chr:���ַ���ASCII
 */
void S1201_WriteOneChar1(unsigned char x, unsigned char chr)
{
  CS = 0;  //��ʼ����
  write_6302(0x20 + x); //��ַ�Ĵ�����ʼλ��
  write_6302(chr);
  CS = 1; //ֹͣ����
  S1201_show();
}

/*
 ��ָ��λ�ô�ӡ�ַ���
 (x,str)
 x:��ʾ���ַ�����λ��0-7
 str:Ҫ��ʾ���ַ���
 */
void S1201_WriteStr(int x, char* str)
{
  u8 i;
	for(i = 0;i < strlen(str);i++)
	{
    S1201_WriteOneChar1(i+x,str[i]);   
  }
}

//VFD����
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
  write_6302(bri_val);//0~255��16���ƣ�0x00~0xff�� 
  CS = 1;
  delay_us(10);
}
void VFD_IO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//�ر�jtag��ʹ��SWD��������SWDģʽ����

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_6|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_8;	 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//�ٶ�50MHz
 	GPIO_Init(GPIOA, &GPIO_InitStructure);	  //��ʼ��GPIOD3,6
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;	
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//�ٶ�50MHz
 	GPIO_Init(GPIOB, &GPIO_InitStructure);	  
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2;	
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//�ٶ�50MHz
 	GPIO_Init(GPIOA, &GPIO_InitStructure);	  
//	DIN=1;
	//CLK=1;
	RST = 0;
	EN = 1;
	delay_ms(5);
	RST = 1;
}