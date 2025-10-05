#include "paj7620u2_iic.h"
#include "paj7620u2.h"
#include "ls1b_gpio.h"
#include "rtthread.h"
// 产生IIC起始信号
void GS_IIC_Start()
{
	GS_SDA_OUT(); // sda线输出
	GS_IIC_SDA(1);
	GS_IIC_SCL(1);
	delay_us(4);
	GS_IIC_SDA(0); // START:when CLK is high,DATA change form high to low
	delay_us(4);
	GS_IIC_SCL(0); // 钳住I2C总线，准备发送或接收数据
}

// 产生IIC停止信号
void GS_IIC_Stop()
{
	GS_SDA_OUT(); // sda线输出
	GS_IIC_SCL(0);
	GS_IIC_SDA(0); // STOP:when CLK is high DATA change form low to high
	delay_us(4);
	GS_IIC_SCL(1);
	GS_IIC_SDA(1); // 发送I2C总线结束信号
	delay_us(4);
}

// 等待应答信号到来
// 返回值：1，接收应答失败
//         0，接收应答成功
unsigned char GS_IIC_Wait_Ack()
{

	unsigned char ucErrTime = 0;
	GS_SDA_IN(); // SDA设置为输入
	GS_IIC_SDA(1);
	delay_us(1);
	GS_IIC_SCL(1);
	delay_us(1);
	while (GS_READ_SDA)
	{
		ucErrTime++;
		if (ucErrTime > 250)
		{
			GS_IIC_Stop();
			return 1;
		}
	}
	GS_IIC_SCL(0); // 时钟输出0
	return 0;
}

// 产生ACK应答
void GS_IIC_Ack()
{
	GS_IIC_SCL(0);
	GS_SDA_OUT();
	GS_IIC_SDA(0);
	delay_us(2);
	GS_IIC_SCL(1);
	delay_us(2);
	GS_IIC_SCL(0);
}

// 不产生ACK应答
void GS_IIC_NAck()
{
	GS_IIC_SCL(0);
	GS_SDA_OUT();
	GS_IIC_SDA(1);
	delay_us(2);
	GS_IIC_SCL(1);
	delay_us(2);
	GS_IIC_SCL(0);
}

// IIC发送一个字节
// 返回从机有无应答
// 1，有应答
// 0，无应答
void GS_IIC_Send_Byte(unsigned char txd)
{
	unsigned char t;
	GS_SDA_OUT();
	GS_IIC_SCL(0); // 拉低时钟开始数据传输
	for (t = 0; t < 8; t++)
	{
		if ((txd & 0x80) >> 7)
			GS_IIC_SDA(1);
		else
			GS_IIC_SDA(0);
		txd <<= 1;
		delay_us(2);
		GS_IIC_SCL(1);
		delay_us(2);
		GS_IIC_SCL(0);
		delay_us(2);
	}
}

// 读1个字节，ack=1时，发送ACK，ack=0，发送nACK
unsigned char GS_IIC_Read_Byte(unsigned char ack)
{

	unsigned char i, receive = 0;

	GS_SDA_IN(); // SDA设置为输入
	for (i = 0; i < 8; i++)
	{
		GS_IIC_SCL(0);
		delay_us(2);
		receive <<= 1;
		GS_IIC_SCL(1);

		if (GS_READ_SDA)
			receive++;
		//GS_IIC_SCL(1);
		delay_us(1);
	}
	if (!ack)
		GS_IIC_NAck(); // 发送nACK
	else
		GS_IIC_Ack(); // 发送ACK

	return receive;
}

// PAJ7620U2写一个字节数据
unsigned char GS_Write_Byte(unsigned char REG_Address, unsigned char REG_data)
{
	/* 关闭中断 */
	rt_base_t level = rt_hw_interrupt_disable();
	GS_IIC_Start();
	GS_IIC_Send_Byte(PAJ7620_ID);
	if (GS_IIC_Wait_Ack())
	{
		GS_IIC_Stop(); // 释放总线
		/* 恢复中断 */
		rt_hw_interrupt_enable(level);
		return 1; // 没应答则退出
	}
	GS_IIC_Send_Byte(REG_Address);
	GS_IIC_Wait_Ack();
	GS_IIC_Send_Byte(REG_data);
	GS_IIC_Wait_Ack();
	GS_IIC_Stop();
	/* 恢复中断 */
	rt_hw_interrupt_enable(level);
	return 0;
}

// PAJ7620U2读一个字节数据
unsigned char GS_Read_Byte(unsigned char REG_Address)
{
	/* 关闭中断 */
	rt_base_t level = rt_hw_interrupt_disable();
	unsigned char REG_data;

	GS_IIC_Start();
	GS_IIC_Send_Byte(PAJ7620_ID); // 发写命令
	if (GS_IIC_Wait_Ack())
	{
		GS_IIC_Stop(); // 释放总线
		/* 恢复中断 */
		rt_hw_interrupt_enable(level);
		return 0; // 没应答则退出
	}
	GS_IIC_Send_Byte(REG_Address);
	GS_IIC_Wait_Ack();
	GS_IIC_Start();
	GS_IIC_Send_Byte(PAJ7620_ID | 0x01); // 发读命令
	GS_IIC_Wait_Ack();
	REG_data = GS_IIC_Read_Byte(0);
	GS_IIC_Stop();
	/* 恢复中断 */
	rt_hw_interrupt_enable(level);
	return REG_data;
}
// PAJ7620U2读n个字节数据
unsigned char GS_Read_nByte(unsigned char REG_Address, unsigned short len, unsigned char *buf)
{
	/* 关闭中断 */
	rt_base_t level = rt_hw_interrupt_disable();
	GS_IIC_Start();
	GS_IIC_Send_Byte(PAJ7620_ID); // 发写命令
	if (GS_IIC_Wait_Ack())
	{
		GS_IIC_Stop(); // 释放总线
		/* 恢复中断 */
		rt_hw_interrupt_enable(level);
		return 1; // 没应答则退出
	}
	GS_IIC_Send_Byte(REG_Address);
	GS_IIC_Wait_Ack();

	GS_IIC_Start();
	GS_IIC_Send_Byte(PAJ7620_ID | 0x01); // 发读命令
	GS_IIC_Wait_Ack();
	while (len)
	{
		if (len == 1)
		{
			*buf = GS_IIC_Read_Byte(0);
		}
		else
		{
			*buf = GS_IIC_Read_Byte(1);
		}
		buf++;
		len--;
	}
	GS_IIC_Stop(); // 释放总线
	/* 恢复中断 */
	rt_hw_interrupt_enable(level);
	return 0;
}
// PAJ7620唤醒
void GS_WakeUp()
{
	/* 关闭中断 */
	rt_base_t level = rt_hw_interrupt_disable();
	GS_IIC_Start();
	GS_IIC_Send_Byte(PAJ7620_ID); // 发写命令
	GS_IIC_Stop();				  // 释放总线
	/* 恢复中断 */
	rt_hw_interrupt_enable(level);
}






// PAJ2670 I2C初始化
void GS_i2c_init()
{
	gpio_enable(41, DIR_OUT);
	gpio_enable(40, DIR_OUT);

	GS_IIC_SDA(1);
	GS_IIC_SCL(1);
}
