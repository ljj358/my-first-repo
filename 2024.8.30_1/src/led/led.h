/*
 * led.h
 *
 * created: 2022/7/6
 *  author: 
 */

#ifndef _LED_H
#define _LED_H

#define u8 unsigned char

#define LED1 49
#define LED2 48
#define LED3 30

#define ON 1
#define OFF 0

#define redRGB_ON 0x11   // 红色
#define redRGB_OFF 0X12



#define blueRGB_ON 0x21  // 蓝色
#define blueRGB_OFF 0X22

#define greenRGB_ON 0x31 // 绿色
#define greenRGB_OFF 0x32 // 绿色

#define yellowRGB 0x41    // 黄色

#define whiteRGB 0x42    // 白色

#define purpleRGB 0x43   // 紫色

#define cyanRGB 0X44     //青色

#define rgb_red 0x61
#define rgb_green 0x71
#define rgb_blue 0x81
void LED_IO_Config_Init(void);
void LEDx_Set_Status(u8 status);
void set_RGB(char led,unsigned int data);
#endif // _LED_H

