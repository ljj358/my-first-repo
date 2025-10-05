#include "MAX30102.h"
#include "paj7620u2_iic.h"

unsigned char max30102_Bus_Write(unsigned char Register_Address, unsigned char Word_Data)
{

	/* 采用串行EEPROM随即读取指令序列，连续读取若干字节 */

	/* 第1步：发起I2C总线启动信号 */
	GS_IIC_Start();

	/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	GS_IIC_Send_Byte(max30102_WR_address | I2C_WR); /* 此处是写指令 */

	/* 第3步：发送ACK */
	if (GS_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM器件无应答 */
	}

	/* 第4步：发送字节地址 */
	GS_IIC_Send_Byte(Register_Address);
	if (GS_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM器件无应答 */
	}

	/* 第5步：开始写入数据 */
	GS_IIC_Send_Byte(Word_Data);

	/* 第6步：发送ACK */
	if (GS_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM器件无应答 */
	}

	/* 发送I2C总线停止信号 */
	GS_IIC_Stop();
	return 1; /* 执行成功 */

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
	/* 发送I2C总线停止信号 */
	GS_IIC_Stop();
	return 0;
}

unsigned char max30102_Bus_Read(unsigned char Register_Address)
{
	unsigned char data;

	/* 第1步：发起I2C总线启动信号 */
	GS_IIC_Start();

	/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	GS_IIC_Send_Byte(max30102_WR_address | I2C_WR); /* 此处是写指令 */

	/* 第3步：发送ACK */
	if (GS_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM器件无应答 */
	}

	/* 第4步：发送字节地址， */
	GS_IIC_Send_Byte((unsigned char)Register_Address);
	if (GS_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM器件无应答 */
	}

	/* 第6步：重新启动I2C总线。下面开始读取数据 */
	GS_IIC_Start();

	/* 第7步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	GS_IIC_Send_Byte(max30102_WR_address | I2C_RD); /* 此处是读指令 */

	/* 第8步：发送ACK */
	if (GS_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM器件无应答 */
	}

	/* 第9步：读取数据 */
	{
		data = GS_IIC_Read_Byte(0); /* 读1个字节 */

		GS_IIC_NAck(); /* 最后1个字节读完后，CPU产生NACK信号(驱动SDA = 1) */
	}
	/* 发送I2C总线停止信号 */
	GS_IIC_Stop();
	return data; /* 执行成功 返回data值 */

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
	/* 发送I2C总线停止信号 */
	GS_IIC_Stop();
	return 0;
}

void max30102_FIFO_ReadBytes(unsigned char Register_Address, unsigned char *Data)
{
	max30102_Bus_Read(REG_INTR_STATUS_1);
	max30102_Bus_Read(REG_INTR_STATUS_2);

	/* 第1步：发起I2C总线启动信号 */
	GS_IIC_Start();

	/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	GS_IIC_Send_Byte(max30102_WR_address | I2C_WR); /* 此处是写指令 */

	/* 第3步：发送ACK */
	if (GS_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM器件无应答 */
	}

	/* 第4步：发送字节地址， */
	GS_IIC_Send_Byte((unsigned char)Register_Address);
	if (GS_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM器件无应答 */
	}

	/* 第6步：重新启动I2C总线。下面开始读取数据 */
	GS_IIC_Start();

	/* 第7步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	GS_IIC_Send_Byte(max30102_WR_address | I2C_RD); /* 此处是读指令 */

	/* 第8步：发送ACK */
	if (GS_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM器件无应答 */
	}

	/* 第9步：读取数据 */
	Data[0] = GS_IIC_Read_Byte(1);
	Data[1] = GS_IIC_Read_Byte(1);
	Data[2] = GS_IIC_Read_Byte(1);
	Data[3] = GS_IIC_Read_Byte(1);
	Data[4] = GS_IIC_Read_Byte(1);
	Data[5] = GS_IIC_Read_Byte(0);
	/* 最后1个字节读完后，CPU产生NACK信号(驱动SDA = 1) */
	/* 发送I2C总线停止信号 */
	GS_IIC_Stop();

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
	/* 发送I2C总线停止信号 */
	GS_IIC_Stop();
}

// 定义最大亮度值
#define MAX_BRIGHTNESS 255

// 红外 LED 传感器数据缓冲区（500 个采样点）
static unsigned int aun_ir_buffer[500];

// 红外缓冲区的长度，表示目前存储的采样数据量
static signed int n_ir_buffer_length;

// 红光 LED 传感器数据缓冲区（500 个采样点）
static unsigned int aun_red_buffer[500];

// 血氧值（SPO2）
static signed int n_sp02;

// 血氧值的有效性标志，1 表示有效，0 表示无效
static signed char ch_spo2_valid;

// 心率值
static signed int n_heart_rate;

// 心率的有效性标志，1 表示有效，0 表示无效
static signed char ch_hr_valid;

// 用于计算反映心跳的板载 LED 亮度的变量
static unsigned int un_min, un_max, un_prev_data;

// 循环计数器
static int i;

// 当前 LED 亮度值
static signed int n_brightness;

// 临时浮点变量，用于计算亮度变化
static float f_temp;

// 临时数据缓冲区，用于从 MAX30102 FIFO 读取数据
static unsigned char temp[6];

// 心率显示值和血氧显示值
static unsigned char dis_hr = 0, dis_spo2 = 0;

void max30102_init(void)
{
	gpio_enable(44, DIR_IN);

	GS_i2c_init();

	max30102_reset();

	max30102_Bus_Write(REG_INTR_ENABLE_1, 0xc0); // INTR setting
	max30102_Bus_Write(REG_INTR_ENABLE_2, 0x00);
	max30102_Bus_Write(REG_FIFO_WR_PTR, 0x00); // FIFO_WR_PTR[4:0]
	max30102_Bus_Write(REG_OVF_COUNTER, 0x00); // OVF_COUNTER[4:0]
	max30102_Bus_Write(REG_FIFO_RD_PTR, 0x00); // FIFO_RD_PTR[4:0]
	max30102_Bus_Write(REG_FIFO_CONFIG, 0x0f); // sample avg = 1, fifo rollover=false, fifo almost full = 17
	max30102_Bus_Write(REG_MODE_CONFIG, 0x03); // 0x02 for Red only, 0x03 for SpO2 mode 0x07 multimode LED
	max30102_Bus_Write(REG_SPO2_CONFIG, 0x27); // SPO2_ADC range = 4096nA, SPO2 sample rate (100 Hz), LED pulseWidth (400uS)
	max30102_Bus_Write(REG_LED1_PA, 0x24);	   // Choose value for ~ 7mA for LED1
	max30102_Bus_Write(REG_LED2_PA, 0x24);	   // Choose value for ~ 7mA for LED2
	max30102_Bus_Write(REG_PILOT_PA, 0x7f);	   // Choose value for ~ 25mA for Pilot LED


	// 初始化变量，设定数据的最小值为最大可能值，最大值为 0
	un_min = 0x3FFFF;
	un_max = 0;

	// 设置红外缓冲区长度为 500，即在 100sps 下存储 5 秒的采样数据
	n_ir_buffer_length = 500;

	// 读取最初的 500 个采样点，并确定信号的最小值和最大值
	for (i = 0; i < n_ir_buffer_length; i++)
	{
		// 等待中断引脚为低电平，表示有新数据可读
		while (gpio_read(44) == 1)
			rt_thread_mdelay(1);

		// 从 MAX30102 的 FIFO 读取 6 字节数据（包含红光和红外光的采样数据）
		max30102_FIFO_ReadBytes(REG_FIFO_DATA, temp);

		// 将读取的 3 字节数据组合成红光 LED 传感器的实际值
		aun_red_buffer[i] = (long)((long)((long)temp[0] & 0x03) << 16) | (long)temp[1] << 8 | (long)temp[2];

		// 将读取的 3 字节数据组合成红外 LED 传感器的实际值
		aun_ir_buffer[i] = (long)((long)((long)temp[3] & 0x03) << 16) | (long)temp[4] << 8 | (long)temp[5];

		// // 更新红光信号的最小值
		// if (un_min > aun_red_buffer[i])
		//     un_min = aun_red_buffer[i];

		// // 更新红光信号的最大值
		// if (un_max < aun_red_buffer[i])
		//     un_max = aun_red_buffer[i];
	}

	// 保存最后一个红光采样点的值
	//  un_prev_data = aun_red_buffer[i];

	// 在采集了前 500 个样本后，计算心率和血氧值
	maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);
}

void max30102_reset(void)
{
	max30102_Bus_Write(REG_MODE_CONFIG, 0x40);
	max30102_Bus_Write(REG_MODE_CONFIG, 0x40);
}

unsigned max30102_read_data(unsigned char *HR, unsigned char *spo2)
{
	i = 0;
	// un_min = 0x3FFFF;
	// un_max = 0;

	// 将前 100 个采样点的数据丢弃，保留后 400 个数据
	for (i = 100; i < 500; i++)
	{
		// 将缓冲区的最后 400 个数据向前移动 100 个位置
		aun_red_buffer[i - 100] = aun_red_buffer[i];
		aun_ir_buffer[i - 100] = aun_ir_buffer[i];

		// // 更新信号的最小值和最大值
		// if (un_min > aun_red_buffer[i])
		//     un_min = aun_red_buffer[i];
		// if (un_max < aun_red_buffer[i])
		//     un_max = aun_red_buffer[i];
	}

	// 采集接下来的 100 个新样本点，并计算亮度
	for (i = 400; i < 500; i++)
	{
		// un_prev_data = aun_red_buffer[i - 1];

		// 等待中断引脚为低电平，表示有新数据可读
		while (gpio_read(44) == 1)
			rt_thread_mdelay(1);

		// 从 MAX30102 的 FIFO 读取新样本
		max30102_FIFO_ReadBytes(REG_FIFO_DATA, temp);

		// 将读取的 3 字节数据组合成红光 LED 传感器的实际值
		aun_red_buffer[i] = (long)((long)((long)temp[0] & 0x03) << 16) | (long)temp[1] << 8 | (long)temp[2];

		// 将读取的 3 字节数据组合成红外 LED 传感器的实际值
		aun_ir_buffer[i] = (long)((long)((long)temp[3] & 0x03) << 16) | (long)temp[4] << 8 | (long)temp[5];

		// // 根据红光信号的变化调整 LED 亮度
		// if (aun_red_buffer[i] > un_prev_data)
		// {
		//     // 如果当前采样点的值大于上一个采样点，则降低亮度
		//     f_temp = aun_red_buffer[i] - un_prev_data;
		//     f_temp /= (un_max - un_min);
		//     f_temp *= MAX_BRIGHTNESS;
		//     n_brightness -= (int)f_temp;
		//     if (n_brightness < 0)
		//         n_brightness = 0;
		// }
		// else
		// {
		//     // 如果当前采样点的值小于上一个采样点，则增加亮度
		//     f_temp = un_prev_data - aun_red_buffer[i];
		//     f_temp /= (un_max - un_min);
		//     f_temp *= MAX_BRIGHTNESS;
		//     n_brightness += (int)f_temp;
		//     if (n_brightness > MAX_BRIGHTNESS)
		//         n_brightness = MAX_BRIGHTNESS;
		// }

		// 如果心率和血氧值有效，且心率小于 120，则更新显示值
		if (ch_hr_valid == 1 && n_heart_rate < 120)
		{
			dis_hr = n_heart_rate;
			dis_spo2 = n_sp02;
		}
		else
		{
			dis_hr = 0;
			dis_spo2 = 0;
		}
	}

	// 计算心率和血氧值
	maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);
	if (dis_hr != 0 && dis_spo2 != 0)
	{
		*HR = dis_hr;
		*spo2 = dis_spo2;
		return 1;
	}
	return 0;
}

