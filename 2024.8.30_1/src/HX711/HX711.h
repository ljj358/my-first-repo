#ifndef __HX711_H
#define __HX711_H
#include "ls1b_gpio.h"

#define  HX711_DOUT    	gpio_read(59)//GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1)
#define  HX711_SCK(X)  	gpio_write(58,X)//GPIO_WriteBit(GPIOA, GPIO_Pin_0, ( BitAction)X)

extern int usart_weight;
extern float GapValue;
void HX711_Init(void);
unsigned long HX711_Read(void);
void Get_Maopi(void);
long Get_Weight(void);


#endif

