/*
 * display.h
 *
 * created: 2021/5/21
 *  author:
 */

#ifndef _DISPLAY_H
#define _DISPLAY_H


//#define DIO  39
//#define RCLK 38
//#define SCLK 55


#define DIO  47
#define RCLK 53
#define SCLK 55

#define High 1
#define Low  0
//DATA线输出高低电平
#define DATA_H gpio_write(DIO,High)
#define DATA_L gpio_write(DIO,Low)
//SCLK线输出高低电平
#define SCLK_H gpio_write(SCLK,High)
#define SCLK_L gpio_write(SCLK,Low)
//RCLK线输出高低电平
#define RCLK_H gpio_write(RCLK,High)
#define RCLK_L gpio_write(RCLK,Low)

void HC595_init(void);
void HC595_Send_Data(unsigned char dat);
void HC595_Out(void);


#endif // _DISPLAY_H



