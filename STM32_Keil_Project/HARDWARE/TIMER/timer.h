#ifndef _timer_h_
#define _timer_h_
#include "stm32f10x_tim.h"
void TIM3_Int_Init(u16 arr,u16 psc);
void timer2ForClock(void);
// extern void glow_tube_drv(unsigned char val595[]);
// extern int flag_1414;
// extern unsigned char display_cahce[];
extern int CT;
extern int CTR;
extern int Speed;
extern int TIMC_Flag;
extern int sflag;
typedef struct
{
  unsigned char sec;
  unsigned char min;
  unsigned char hour;
  unsigned char day;
  unsigned char date;
  unsigned char month;
  unsigned char year;
  unsigned char dateTemp;
}clock;

#endif
