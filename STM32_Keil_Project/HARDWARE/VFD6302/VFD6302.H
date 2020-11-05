#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "string.h"
#define DIN	PAout(7)
#define CLK	PAout(6)
#define CS	PAout(5)
#define RST	PAout(4)
#define EN	PAout(3)
#define LED_BLUE PAout(8)
void write_6302(unsigned char w_data);
void S1201_show(void);
void VFD_init(void);
void bri_set(u8 bri_val);
void S1201_WriteOneChar(unsigned char x, unsigned char chr);
void S1201_WriteOneChar1(unsigned char x, unsigned char chr);
void S1201_WriteStr(int x, char* str);
void S1201_clear(void);
void VFD_IO_Init(void);
