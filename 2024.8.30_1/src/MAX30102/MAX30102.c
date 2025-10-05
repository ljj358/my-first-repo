#include "MAX30102.h"
#include "paj7620u2_iic.h"

unsigned char max30102_Bus_Write(unsigned char Register_Address, unsigned char Word_Data)
{

	/* ���ô���EEPROM�漴��ȡָ�����У�������ȡ�����ֽ� */

	/* ��1��������I2C���������ź� */
	GS_IIC_Start();

	/* ��2������������ֽڣ���7bit�ǵ�ַ��bit0�Ƕ�д����λ��0��ʾд��1��ʾ�� */
	GS_IIC_Send_Byte(max30102_WR_address | I2C_WR); /* �˴���дָ�� */

	/* ��3��������ACK */
	if (GS_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM������Ӧ�� */
	}

	/* ��4���������ֽڵ�ַ */
	GS_IIC_Send_Byte(Register_Address);
	if (GS_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM������Ӧ�� */
	}

	/* ��5������ʼд������ */
	GS_IIC_Send_Byte(Word_Data);

	/* ��6��������ACK */
	if (GS_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM������Ӧ�� */
	}

	/* ����I2C����ֹͣ�ź� */
	GS_IIC_Stop();
	return 1; /* ִ�гɹ� */

cmd_fail: /* ����ִ��ʧ�ܺ��мǷ���ֹͣ�źţ�����Ӱ��I2C�����������豸 */
	/* ����I2C����ֹͣ�ź� */
	GS_IIC_Stop();
	return 0;
}

unsigned char max30102_Bus_Read(unsigned char Register_Address)
{
	unsigned char data;

	/* ��1��������I2C���������ź� */
	GS_IIC_Start();

	/* ��2������������ֽڣ���7bit�ǵ�ַ��bit0�Ƕ�д����λ��0��ʾд��1��ʾ�� */
	GS_IIC_Send_Byte(max30102_WR_address | I2C_WR); /* �˴���дָ�� */

	/* ��3��������ACK */
	if (GS_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM������Ӧ�� */
	}

	/* ��4���������ֽڵ�ַ�� */
	GS_IIC_Send_Byte((unsigned char)Register_Address);
	if (GS_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM������Ӧ�� */
	}

	/* ��6������������I2C���ߡ����濪ʼ��ȡ���� */
	GS_IIC_Start();

	/* ��7������������ֽڣ���7bit�ǵ�ַ��bit0�Ƕ�д����λ��0��ʾд��1��ʾ�� */
	GS_IIC_Send_Byte(max30102_WR_address | I2C_RD); /* �˴��Ƕ�ָ�� */

	/* ��8��������ACK */
	if (GS_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM������Ӧ�� */
	}

	/* ��9������ȡ���� */
	{
		data = GS_IIC_Read_Byte(0); /* ��1���ֽ� */

		GS_IIC_NAck(); /* ���1���ֽڶ����CPU����NACK�ź�(����SDA = 1) */
	}
	/* ����I2C����ֹͣ�ź� */
	GS_IIC_Stop();
	return data; /* ִ�гɹ� ����dataֵ */

cmd_fail: /* ����ִ��ʧ�ܺ��мǷ���ֹͣ�źţ�����Ӱ��I2C�����������豸 */
	/* ����I2C����ֹͣ�ź� */
	GS_IIC_Stop();
	return 0;
}

void max30102_FIFO_ReadBytes(unsigned char Register_Address, unsigned char *Data)
{
	max30102_Bus_Read(REG_INTR_STATUS_1);
	max30102_Bus_Read(REG_INTR_STATUS_2);

	/* ��1��������I2C���������ź� */
	GS_IIC_Start();

	/* ��2������������ֽڣ���7bit�ǵ�ַ��bit0�Ƕ�д����λ��0��ʾд��1��ʾ�� */
	GS_IIC_Send_Byte(max30102_WR_address | I2C_WR); /* �˴���дָ�� */

	/* ��3��������ACK */
	if (GS_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM������Ӧ�� */
	}

	/* ��4���������ֽڵ�ַ�� */
	GS_IIC_Send_Byte((unsigned char)Register_Address);
	if (GS_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM������Ӧ�� */
	}

	/* ��6������������I2C���ߡ����濪ʼ��ȡ���� */
	GS_IIC_Start();

	/* ��7������������ֽڣ���7bit�ǵ�ַ��bit0�Ƕ�д����λ��0��ʾд��1��ʾ�� */
	GS_IIC_Send_Byte(max30102_WR_address | I2C_RD); /* �˴��Ƕ�ָ�� */

	/* ��8��������ACK */
	if (GS_IIC_Wait_Ack() != 0)
	{
		goto cmd_fail; /* EEPROM������Ӧ�� */
	}

	/* ��9������ȡ���� */
	Data[0] = GS_IIC_Read_Byte(1);
	Data[1] = GS_IIC_Read_Byte(1);
	Data[2] = GS_IIC_Read_Byte(1);
	Data[3] = GS_IIC_Read_Byte(1);
	Data[4] = GS_IIC_Read_Byte(1);
	Data[5] = GS_IIC_Read_Byte(0);
	/* ���1���ֽڶ����CPU����NACK�ź�(����SDA = 1) */
	/* ����I2C����ֹͣ�ź� */
	GS_IIC_Stop();

cmd_fail: /* ����ִ��ʧ�ܺ��мǷ���ֹͣ�źţ�����Ӱ��I2C�����������豸 */
	/* ����I2C����ֹͣ�ź� */
	GS_IIC_Stop();
}

// �����������ֵ
#define MAX_BRIGHTNESS 255

// ���� LED ���������ݻ�������500 �������㣩
static unsigned int aun_ir_buffer[500];

// ���⻺�����ĳ��ȣ���ʾĿǰ�洢�Ĳ���������
static signed int n_ir_buffer_length;

// ��� LED ���������ݻ�������500 �������㣩
static unsigned int aun_red_buffer[500];

// Ѫ��ֵ��SPO2��
static signed int n_sp02;

// Ѫ��ֵ����Ч�Ա�־��1 ��ʾ��Ч��0 ��ʾ��Ч
static signed char ch_spo2_valid;

// ����ֵ
static signed int n_heart_rate;

// ���ʵ���Ч�Ա�־��1 ��ʾ��Ч��0 ��ʾ��Ч
static signed char ch_hr_valid;

// ���ڼ��㷴ӳ�����İ��� LED ���ȵı���
static unsigned int un_min, un_max, un_prev_data;

// ѭ��������
static int i;

// ��ǰ LED ����ֵ
static signed int n_brightness;

// ��ʱ������������ڼ������ȱ仯
static float f_temp;

// ��ʱ���ݻ����������ڴ� MAX30102 FIFO ��ȡ����
static unsigned char temp[6];

// ������ʾֵ��Ѫ����ʾֵ
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


	// ��ʼ���������趨���ݵ���СֵΪ������ֵ�����ֵΪ 0
	un_min = 0x3FFFF;
	un_max = 0;

	// ���ú��⻺��������Ϊ 500������ 100sps �´洢 5 ��Ĳ�������
	n_ir_buffer_length = 500;

	// ��ȡ����� 500 �������㣬��ȷ���źŵ���Сֵ�����ֵ
	for (i = 0; i < n_ir_buffer_length; i++)
	{
		// �ȴ��ж�����Ϊ�͵�ƽ����ʾ�������ݿɶ�
		while (gpio_read(44) == 1)
			rt_thread_mdelay(1);

		// �� MAX30102 �� FIFO ��ȡ 6 �ֽ����ݣ��������ͺ����Ĳ������ݣ�
		max30102_FIFO_ReadBytes(REG_FIFO_DATA, temp);

		// ����ȡ�� 3 �ֽ�������ϳɺ�� LED ��������ʵ��ֵ
		aun_red_buffer[i] = (long)((long)((long)temp[0] & 0x03) << 16) | (long)temp[1] << 8 | (long)temp[2];

		// ����ȡ�� 3 �ֽ�������ϳɺ��� LED ��������ʵ��ֵ
		aun_ir_buffer[i] = (long)((long)((long)temp[3] & 0x03) << 16) | (long)temp[4] << 8 | (long)temp[5];

		// // ���º���źŵ���Сֵ
		// if (un_min > aun_red_buffer[i])
		//     un_min = aun_red_buffer[i];

		// // ���º���źŵ����ֵ
		// if (un_max < aun_red_buffer[i])
		//     un_max = aun_red_buffer[i];
	}

	// �������һ�����������ֵ
	//  un_prev_data = aun_red_buffer[i];

	// �ڲɼ���ǰ 500 �������󣬼������ʺ�Ѫ��ֵ
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

	// ��ǰ 100 ������������ݶ����������� 400 ������
	for (i = 100; i < 500; i++)
	{
		// ������������� 400 ��������ǰ�ƶ� 100 ��λ��
		aun_red_buffer[i - 100] = aun_red_buffer[i];
		aun_ir_buffer[i - 100] = aun_ir_buffer[i];

		// // �����źŵ���Сֵ�����ֵ
		// if (un_min > aun_red_buffer[i])
		//     un_min = aun_red_buffer[i];
		// if (un_max < aun_red_buffer[i])
		//     un_max = aun_red_buffer[i];
	}

	// �ɼ��������� 100 ���������㣬����������
	for (i = 400; i < 500; i++)
	{
		// un_prev_data = aun_red_buffer[i - 1];

		// �ȴ��ж�����Ϊ�͵�ƽ����ʾ�������ݿɶ�
		while (gpio_read(44) == 1)
			rt_thread_mdelay(1);

		// �� MAX30102 �� FIFO ��ȡ������
		max30102_FIFO_ReadBytes(REG_FIFO_DATA, temp);

		// ����ȡ�� 3 �ֽ�������ϳɺ�� LED ��������ʵ��ֵ
		aun_red_buffer[i] = (long)((long)((long)temp[0] & 0x03) << 16) | (long)temp[1] << 8 | (long)temp[2];

		// ����ȡ�� 3 �ֽ�������ϳɺ��� LED ��������ʵ��ֵ
		aun_ir_buffer[i] = (long)((long)((long)temp[3] & 0x03) << 16) | (long)temp[4] << 8 | (long)temp[5];

		// // ���ݺ���źŵı仯���� LED ����
		// if (aun_red_buffer[i] > un_prev_data)
		// {
		//     // �����ǰ�������ֵ������һ�������㣬�򽵵�����
		//     f_temp = aun_red_buffer[i] - un_prev_data;
		//     f_temp /= (un_max - un_min);
		//     f_temp *= MAX_BRIGHTNESS;
		//     n_brightness -= (int)f_temp;
		//     if (n_brightness < 0)
		//         n_brightness = 0;
		// }
		// else
		// {
		//     // �����ǰ�������ֵС����һ�������㣬����������
		//     f_temp = un_prev_data - aun_red_buffer[i];
		//     f_temp /= (un_max - un_min);
		//     f_temp *= MAX_BRIGHTNESS;
		//     n_brightness += (int)f_temp;
		//     if (n_brightness > MAX_BRIGHTNESS)
		//         n_brightness = MAX_BRIGHTNESS;
		// }

		// ������ʺ�Ѫ��ֵ��Ч��������С�� 120���������ʾֵ
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

	// �������ʺ�Ѫ��ֵ
	maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);
	if (dis_hr != 0 && dis_spo2 != 0)
	{
		*HR = dis_hr;
		*spo2 = dis_spo2;
		return 1;
	}
	return 0;
}

