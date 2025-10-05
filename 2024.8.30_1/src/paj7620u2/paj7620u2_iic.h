#ifndef __PAJ7620U2_IIC_H
#define __PAJ7620U2_IIC_H
//////////////////////////////////////////////////////////////////////////////////
// 本程序只供学习使用，未经作者许可，不得用于其它任何用途
// STM32F10x开发板  PAJ7620传感器模块IIC驱动	（模块I2C引脚配置头文件）
// STM32学习有问题，请加入q群交流: 643807576

#include "ls1b_gpio.h"

// GPIO引脚配置（本例程 PF7接模块的SCL脚、PF8接模块的SDA脚）
#define GS_IIC_SCL(IN) gpio_write(41, IN) // SCL（SCL_OUT）
#define GS_IIC_SDA(IN) gpio_write(40, IN) // SDA_OUT，用于发送SDA数据给IIC传感器模块
#define GS_READ_SDA gpio_read(40)         // SDA_IN，用于读取IIC传感器模块的SDA数据

// I/O方向配置(寄存器操作).
#define GS_SDA_IN() gpio_enable(40, DIR_IN)
#define GS_SDA_OUT() gpio_enable(40, DIR_OUT)

// PS:  I/O方向不会配置? :傻瓜式操作 ---> https://xinso.blog.csdn.net/article/details/115862486

unsigned char GS_Write_Byte(unsigned char REG_Address, unsigned char REG_data);
unsigned char GS_Read_Byte(unsigned char REG_Address);
unsigned char GS_Read_nByte(unsigned char REG_Address, unsigned short len, unsigned char *buf);
void GS_WakeUp(void);
void GS_i2c_init();

void GS_IIC_Start();
void GS_IIC_Stop();
unsigned char GS_IIC_Wait_Ack();
void GS_IIC_Ack();
void GS_IIC_NAck();
void GS_IIC_Send_Byte(unsigned char txd);
unsigned char GS_IIC_Read_Byte(unsigned char ack);



#endif
