#include "paj7620u2_iic.h"
#include "paj7620u2.h"
#include "ls1b_gpio.h"
#include "rtthread.h"
// ����IIC��ʼ�ź�
void GS_IIC_Start()
{
	GS_SDA_OUT(); // sda�����
	GS_IIC_SDA(1);
	GS_IIC_SCL(1);
	delay_us(4);
	GS_IIC_SDA(0); // START:when CLK is high,DATA change form high to low
	delay_us(4);
	GS_IIC_SCL(0); // ǯסI2C���ߣ�׼�����ͻ��������
}

// ����IICֹͣ�ź�
void GS_IIC_Stop()
{
	GS_SDA_OUT(); // sda�����
	GS_IIC_SCL(0);
	GS_IIC_SDA(0); // STOP:when CLK is high DATA change form low to high
	delay_us(4);
	GS_IIC_SCL(1);
	GS_IIC_SDA(1); // ����I2C���߽����ź�
	delay_us(4);
}

// �ȴ�Ӧ���źŵ���
// ����ֵ��1������Ӧ��ʧ��
//         0������Ӧ��ɹ�
unsigned char GS_IIC_Wait_Ack()
{

	unsigned char ucErrTime = 0;
	GS_SDA_IN(); // SDA����Ϊ����
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
	GS_IIC_SCL(0); // ʱ�����0
	return 0;
}

// ����ACKӦ��
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

// ������ACKӦ��
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

// IIC����һ���ֽ�
// ���شӻ�����Ӧ��
// 1����Ӧ��
// 0����Ӧ��
void GS_IIC_Send_Byte(unsigned char txd)
{
	unsigned char t;
	GS_SDA_OUT();
	GS_IIC_SCL(0); // ����ʱ�ӿ�ʼ���ݴ���
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

// ��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK
unsigned char GS_IIC_Read_Byte(unsigned char ack)
{

	unsigned char i, receive = 0;

	GS_SDA_IN(); // SDA����Ϊ����
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
		GS_IIC_NAck(); // ����nACK
	else
		GS_IIC_Ack(); // ����ACK

	return receive;
}

// PAJ7620U2дһ���ֽ�����
unsigned char GS_Write_Byte(unsigned char REG_Address, unsigned char REG_data)
{
	/* �ر��ж� */
	rt_base_t level = rt_hw_interrupt_disable();
	GS_IIC_Start();
	GS_IIC_Send_Byte(PAJ7620_ID);
	if (GS_IIC_Wait_Ack())
	{
		GS_IIC_Stop(); // �ͷ�����
		/* �ָ��ж� */
		rt_hw_interrupt_enable(level);
		return 1; // ûӦ�����˳�
	}
	GS_IIC_Send_Byte(REG_Address);
	GS_IIC_Wait_Ack();
	GS_IIC_Send_Byte(REG_data);
	GS_IIC_Wait_Ack();
	GS_IIC_Stop();
	/* �ָ��ж� */
	rt_hw_interrupt_enable(level);
	return 0;
}

// PAJ7620U2��һ���ֽ�����
unsigned char GS_Read_Byte(unsigned char REG_Address)
{
	/* �ر��ж� */
	rt_base_t level = rt_hw_interrupt_disable();
	unsigned char REG_data;

	GS_IIC_Start();
	GS_IIC_Send_Byte(PAJ7620_ID); // ��д����
	if (GS_IIC_Wait_Ack())
	{
		GS_IIC_Stop(); // �ͷ�����
		/* �ָ��ж� */
		rt_hw_interrupt_enable(level);
		return 0; // ûӦ�����˳�
	}
	GS_IIC_Send_Byte(REG_Address);
	GS_IIC_Wait_Ack();
	GS_IIC_Start();
	GS_IIC_Send_Byte(PAJ7620_ID | 0x01); // ��������
	GS_IIC_Wait_Ack();
	REG_data = GS_IIC_Read_Byte(0);
	GS_IIC_Stop();
	/* �ָ��ж� */
	rt_hw_interrupt_enable(level);
	return REG_data;
}
// PAJ7620U2��n���ֽ�����
unsigned char GS_Read_nByte(unsigned char REG_Address, unsigned short len, unsigned char *buf)
{
	/* �ر��ж� */
	rt_base_t level = rt_hw_interrupt_disable();
	GS_IIC_Start();
	GS_IIC_Send_Byte(PAJ7620_ID); // ��д����
	if (GS_IIC_Wait_Ack())
	{
		GS_IIC_Stop(); // �ͷ�����
		/* �ָ��ж� */
		rt_hw_interrupt_enable(level);
		return 1; // ûӦ�����˳�
	}
	GS_IIC_Send_Byte(REG_Address);
	GS_IIC_Wait_Ack();

	GS_IIC_Start();
	GS_IIC_Send_Byte(PAJ7620_ID | 0x01); // ��������
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
	GS_IIC_Stop(); // �ͷ�����
	/* �ָ��ж� */
	rt_hw_interrupt_enable(level);
	return 0;
}
// PAJ7620����
void GS_WakeUp()
{
	/* �ر��ж� */
	rt_base_t level = rt_hw_interrupt_disable();
	GS_IIC_Start();
	GS_IIC_Send_Byte(PAJ7620_ID); // ��д����
	GS_IIC_Stop();				  // �ͷ�����
	/* �ָ��ж� */
	rt_hw_interrupt_enable(level);
}






// PAJ2670 I2C��ʼ��
void GS_i2c_init()
{
	gpio_enable(41, DIR_OUT);
	gpio_enable(40, DIR_OUT);

	GS_IIC_SDA(1);
	GS_IIC_SCL(1);
}
