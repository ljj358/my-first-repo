#ifndef __PAJ7620U2_IIC_H
#define __PAJ7620U2_IIC_H
//////////////////////////////////////////////////////////////////////////////////
// ������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
// STM32F10x������  PAJ7620������ģ��IIC����	��ģ��I2C��������ͷ�ļ���
// STM32ѧϰ�����⣬�����qȺ����: 643807576

#include "ls1b_gpio.h"

// GPIO�������ã������� PF7��ģ���SCL�š�PF8��ģ���SDA�ţ�
#define GS_IIC_SCL(IN) gpio_write(41, IN) // SCL��SCL_OUT��
#define GS_IIC_SDA(IN) gpio_write(40, IN) // SDA_OUT�����ڷ���SDA���ݸ�IIC������ģ��
#define GS_READ_SDA gpio_read(40)         // SDA_IN�����ڶ�ȡIIC������ģ���SDA����

// I/O��������(�Ĵ�������).
#define GS_SDA_IN() gpio_enable(40, DIR_IN)
#define GS_SDA_OUT() gpio_enable(40, DIR_OUT)

// PS:  I/O���򲻻�����? :ɵ��ʽ���� ---> https://xinso.blog.csdn.net/article/details/115862486

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
