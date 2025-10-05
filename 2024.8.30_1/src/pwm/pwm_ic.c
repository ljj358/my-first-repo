/*
 * pwm_ic.c
 *
 * created: 2022/3/2
 *  author:
 */
#include "pwm_ic.h"
#include "ls1x_i2c_bus.h"
#include <stdint.h>
#include <stddef.h>
#include "main.h"
#include "rtthread.h"

#define pca_adrr 0x7f

#define pca_mode1 0x0
#define pca_pre 0xFE

#define LED0_ON_L 0x6
#define LED0_ON_H 0x7
#define LED0_OFF_L 0x8
#define LED0_OFF_H 0x9

#define jdMIN  115 // minimum
#define jdMAX  590 // maximum
#define jd000  130 //0�ȶ�Ӧ4096���������ֵ
#define jd180  520 //180�ȶ�Ӧ4096���������ֵ

uint8_t pca_read(uint8_t startAddress) {
   	int rt=0,rw_cnt=0;
	unsigned char addr[2];
    rt_mutex_take(I2C0_mutex, RT_WAITING_FOREVER);
	/* start transfer */
	rt = ls1x_i2c_send_start(busI2C0, pca_adrr);

	/* address device, FALSE(0) for write */
	rt = ls1x_i2c_send_addr(busI2C0, pca_adrr, false);
	addr[0]=startAddress;
	rw_cnt = ls1x_i2c_write_bytes(busI2C0, addr, 1);


	/* terminate transfer */
	ls1x_i2c_send_stop(busI2C0, pca_adrr);  // ����Ҫ��ֹͣ�źţ��������Ķ�ȡ������

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

void pca_write(uint8_t startAddress, uint8_t buffer) {
	rt_mutex_take(I2C0_mutex, RT_WAITING_FOREVER);
    //Send address to start reading from.
    unsigned char sdata[1] = {0};
    //��ʼ�ź�
    ls1x_i2c_send_start(busI2C0, pca_adrr);
    //���͵�ַ
    ls1x_i2c_send_addr(busI2C0, pca_adrr, false);
    sdata[0]=startAddress;
    //д������
    ls1x_i2c_write_bytes(busI2C0, sdata, 1);
    sdata[0]=buffer;
    //д������
    ls1x_i2c_write_bytes(busI2C0, sdata, 1);

    //ֹͣ�ź�
    ls1x_i2c_send_stop(busI2C0, pca_adrr);
    rt_mutex_release(I2C0_mutex);
}

void pca_setfreq(float freq)//����PWMƵ��
{
		uint8_t prescale,oldmode,newmode;
		double prescaleval;
		//freq *= 0.92;
		prescaleval = 25000000;
		prescaleval /= 4096;
		prescaleval /= freq;
		prescaleval -= 1;
		prescale =floor(prescaleval + 0.5f);

		oldmode = pca_read(pca_mode1);

		newmode = (oldmode&0x7F) | 0x10; // sleep

		pca_write(pca_mode1, newmode); // go to sleep

		pca_write(pca_pre, prescale); // set the prescaler

		pca_write(pca_mode1, oldmode);
		rt_thread_delay(2);

		pca_write(pca_mode1, oldmode | 0xa1);
}

void pca_setpwm(uint8_t num, uint32_t on, uint32_t off)
{
		pca_write(LED0_ON_L+4*num,on);
		pca_write(LED0_ON_H+4*num,on>>8);
		pca_write(LED0_OFF_L+4*num,off);
		pca_write(LED0_OFF_H+4*num,off>>8);
}
/*num:���PWM�������0~15��on:PWM��������ֵ0~4096,off:PWM�½�����ֵ0~4096
һ��PWM���ڷֳ�4096�ݣ���0��ʼ+1�������Ƶ�onʱ����Ϊ�ߵ�ƽ������������offʱ
����Ϊ�͵�ƽ��ֱ������4096���¿�ʼ�����Ե�on������0ʱ������ʱ,��on����0ʱ��
off/4096��ֵ����PWM��ռ�ձȡ�*/

/*
	�������ã���ʼ�����������
	������1.PWMƵ��
		  2.��ʼ������Ƕ�
*/
void PCA_MG9XX_Init(float hz,uint8_t angle)
{
	uint32_t off=0;

	pca_write(pca_mode1,0x0);
	pca_setfreq(hz);//����PWMƵ��
	off=(uint32_t)(145+angle*2.4);
	pca_setpwm(0,0,off);pca_setpwm(1,0,off);pca_setpwm(2,0,off);pca_setpwm(3,0,off);
	pca_setpwm(4,0,off);pca_setpwm(5,0,off);pca_setpwm(6,0,off);pca_setpwm(7,0,off);
	pca_setpwm(8,0,off);pca_setpwm(9,0,off);pca_setpwm(10,0,off);pca_setpwm(11,0,off);
	pca_setpwm(12,0,off);pca_setpwm(13,0,off);pca_setpwm(14,0,off);pca_setpwm(15,0,off);
	rt_thread_delay(500);
}


void PCA_MG9XX(uint8_t num,uint8_t end_angle)
{
	uint32_t off=0;

	off=(uint32_t)(100+end_angle*2.2);
	pca_setpwm(num,0,off);
}


