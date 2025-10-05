#ifndef _BEEP_H
#define _BEEP_H
#include "ls1b_gpio.h"
#include "ls1x_fb.h"
#include "ls1x_pwm.h"
#include "bsp.h"

#define BEEP 42



#define BEEP_On()  gpio_write(BEEP, 1);    //¿ªÆô·äÃùÆ÷
#define BEEP_Off() gpio_write(BEEP, 0);    //¹Ø±Õ·äÃùÆ÷
#define BEEP_flip() gpio_write(BEEP, !gpio_read(BEEP));    //¿ªÆô·äÃùÆ÷
void BEEP_Init(void);
void BEEP_open(int tim);
void BEEP_music(int cnt,int tim);
void BEEP_OFF_TIM(void);
void BEEP_open_TIM(void);
#endif // _BEEP_H

