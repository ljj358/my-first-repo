#include "Stepper_motor.h"

#include "ls1x_i2c_bus.h"
#include <stdint.h>
#include <stddef.h>
#include "main.h"
#include "rtthread.h"
#include "ls1b_gpio.h"

#define pca_adrr 0x7f

#define pca_mode1 0x0
#define pca_pre 0xFE

#define LED0_ON_L 0x6
#define LED0_ON_H 0x7
#define LED0_OFF_L 0x8
#define LED0_OFF_H 0x9

#define jdMIN 115 // minimum
#define jdMAX 590 // maximum
#define jd000 130 // 0�ȶ�Ӧ4096���������ֵ
#define jd180 520 // 180�ȶ�Ӧ4096���������ֵ

char Stepper_mode = 0; // 1 = ���ģʽ ��2 ��̨��λģʽ
static uint8_t pca_read(uint8_t startAddress)
{
	int rt = 0, rw_cnt = 0;
	unsigned char addr[2];
	rt_mutex_take(I2C0_mutex, RT_WAITING_FOREVER);
	/* start transfer */
	rt = ls1x_i2c_send_start(busI2C0, pca_adrr);

	/* address device, FALSE(0) for write */
	rt = ls1x_i2c_send_addr(busI2C0, pca_adrr, false);
	addr[0] = startAddress;
	rw_cnt = ls1x_i2c_write_bytes(busI2C0, addr, 1);

	/* terminate transfer */
	ls1x_i2c_send_stop(busI2C0, pca_adrr); // ����Ҫ��ֹͣ�źţ��������Ķ�ȡ������

	/* restart: start transfer */
	ls1x_i2c_send_start(busI2C0, pca_adrr);

	/* address device, TRUE(1) for READ */
	ls1x_i2c_send_addr(busI2C0, pca_adrr, true);

	//-----------------------------------------------------
	// read out data
	//-----------------------------------------------------
	ls1x_i2c_read_bytes(busI2C0, addr, 1);

	/* terminate transfer */
	ls1x_i2c_send_stop(busI2C0, pca_adrr);
	rt_mutex_release(I2C0_mutex);
	return addr[0];
}

static void pca_write(uint8_t startAddress, uint8_t buffer)
{
	rt_mutex_take(I2C0_mutex, RT_WAITING_FOREVER);
	// Send address to start reading from.
	unsigned char sdata[1] = {0};
	// ��ʼ�ź�
	ls1x_i2c_send_start(busI2C0, pca_adrr);
	// ���͵�ַ
	ls1x_i2c_send_addr(busI2C0, pca_adrr, false);
	sdata[0] = startAddress;
	// д������
	ls1x_i2c_write_bytes(busI2C0, sdata, 1);
	sdata[0] = buffer;
	// д������
	ls1x_i2c_write_bytes(busI2C0, sdata, 1);

	// ֹͣ�ź�
	ls1x_i2c_send_stop(busI2C0, pca_adrr);
	rt_mutex_release(I2C0_mutex);
}

void set_frequency(float freq) // ����PWMƵ��
{
	uint8_t prescale, oldmode, newmode;
	double prescaleval;
	// freq *= 0.92;
	prescaleval = 25000000;
	prescaleval /= 4096;
	prescaleval /= freq;
	prescaleval -= 1;
	prescale = floor(prescaleval + 0.5f);

	oldmode = pca_read(pca_mode1);

	newmode = (oldmode & 0x7F) | 0x10; // sleep

	pca_write(pca_mode1, newmode); // go to sleep

	pca_write(pca_pre, prescale); // set the prescaler

	pca_write(pca_mode1, oldmode);
	rt_thread_delay(2);

	pca_write(pca_mode1, oldmode | 0xa1);
}
/*num:���PWM�������0~15��on:PWM��������ֵ0~4096,off:PWM�½�����ֵ0~4096
һ��PWM���ڷֳ�4096�ݣ���0��ʼ+1�������Ƶ�onʱ����Ϊ�ߵ�ƽ������������offʱ
����Ϊ�͵�ƽ��ֱ������4096���¿�ʼ�����Ե�on������0ʱ������ʱ,��on����0ʱ��
off/4096��ֵ����PWM��ռ�ձȡ�*/
void set_Duty_cycle(uint8_t num, uint32_t on, uint32_t off)
{
	pca_write(LED0_ON_L + 4 * num, on);
	pca_write(LED0_ON_H + 4 * num, on >> 8);
	pca_write(LED0_OFF_L + 4 * num, off);
	pca_write(LED0_OFF_H + 4 * num, off >> 8);
}

static void gpio_interrput_isr(int vector, void *param)
{

	if (gpio_read(39) == 1&&gpio_read(51)==0)
	{
		gpio_write(51, 1); // dir  0���� ��1����
	}
	else if (gpio_read(38) == 1&&gpio_read(51)==1)
	{
		
		gpio_write(51, 0); // dir  0���� ��1����
		gpio_write(50, 1); // EN   1���ܣ�0ʹ��
		gpio_write(45, 1); // 1���� 0ʹ��
		if (Stepper_mode == 1)
		{
			voice_broadcast(2);
			
		}
		Stepper_mode = 0;
	}
}
/*
	�������ã���ʼ��PWMоƬ
	������1.PWMƵ��
		  2.��ʼ������Ƕ�
*/
void Stepper_motor_Init(float hz, int angle)
{
	gpio_enable(50, DIR_OUT); // EN
	gpio_enable(51, DIR_OUT); // DIR
	gpio_enable(38, DIR_IN);  //  ��� SDA
	gpio_enable(39, DIR_IN);  //  �ұ� SCL
	gpio_enable(45, DIR_OUT); // ��ѹģ��

	ls1x_disable_gpio_interrupt(38);
	ls1x_install_gpio_isr(38, INT_TRIG_EDGE_UP, gpio_interrput_isr, NULL);
	ls1x_enable_gpio_interrupt(38);

	ls1x_disable_gpio_interrupt(39);
	ls1x_install_gpio_isr(39, INT_TRIG_EDGE_UP, gpio_interrput_isr, NULL);
	ls1x_enable_gpio_interrupt(39);

	gpio_write(45, 1); // 1���� 0ʹ��
	gpio_write(51, 0); // dir  0���� ��1����
	gpio_write(50, 1); // EN   1���ܣ�0ʹ��

	pca_write(pca_mode1, 0x0);
	set_frequency(hz); // ����PWMƵ��
	set_Duty_cycle(0, 0, angle);
	set_Duty_cycle(1, 0, angle);
	set_Duty_cycle(2, 0, angle);
	set_Duty_cycle(3, 0, angle);
	set_Duty_cycle(4, 0, angle);
	set_Duty_cycle(5, 0, angle);
	set_Duty_cycle(6, 0, angle);
	set_Duty_cycle(7, 0, angle);
	set_Duty_cycle(8, 0, angle);
	set_Duty_cycle(9, 0, angle);
	set_Duty_cycle(10, 0, angle);
	set_Duty_cycle(11, 0, angle);
	set_Duty_cycle(12, 0, angle);
	set_Duty_cycle(13, 0, angle);
	set_Duty_cycle(14, 0, angle);
	set_Duty_cycle(15, 0, angle);
	set_Duty_cycle(4, 0, angle);
}

void Stepper_motor_open(void)
{
	if (Stepper_mode == 0)
	{
		Stepper_mode = 1;
		if (gpio_read(39) != 1)
			gpio_write(51, 0); // dir  0���� ��1����
		else
			gpio_write(51, 1); // dir  0���� ��1����
		gpio_write(45, 0);	   // 1���� 0ʹ��
		gpio_write(50, 0);	   // EN   1���ܣ�0ʹ��
	}
}

void Stepper_motor_reset(void)
{
	if (Stepper_mode == 0)
	{
		if (gpio_read(38) != 1)
		{
			gpio_write(51, 1); // dir  0���� ��1����
			gpio_write(50, 0); // EN   1���ܣ�0ʹ��
			Stepper_mode = 2;
		}
			
	}
}
