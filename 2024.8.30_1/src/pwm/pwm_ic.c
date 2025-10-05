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
#define jd000  130 //0度对应4096的脉宽计数值
#define jd180  520 //180度对应4096的脉宽计算值

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
	ls1x_i2c_send_stop(busI2C0, pca_adrr);  // 必须要发停止信号，否则后面的读取有问题

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
    //起始信号
    ls1x_i2c_send_start(busI2C0, pca_adrr);
    //发送地址
    ls1x_i2c_send_addr(busI2C0, pca_adrr, false);
    sdata[0]=startAddress;
    //写入数据
    ls1x_i2c_write_bytes(busI2C0, sdata, 1);
    sdata[0]=buffer;
    //写入数据
    ls1x_i2c_write_bytes(busI2C0, sdata, 1);

    //停止信号
    ls1x_i2c_send_stop(busI2C0, pca_adrr);
    rt_mutex_release(I2C0_mutex);
}

void pca_setfreq(float freq)//设置PWM频率
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
/*num:舵机PWM输出引脚0~15，on:PWM上升计数值0~4096,off:PWM下降计数值0~4096
一个PWM周期分成4096份，由0开始+1计数，计到on时跳变为高电平，继续计数到off时
跳变为低电平，直到计满4096重新开始。所以当on不等于0时可作延时,当on等于0时，
off/4096的值就是PWM的占空比。*/

/*
	函数作用：初始化舵机驱动板
	参数：1.PWM频率
		  2.初始化舵机角度
*/
void PCA_MG9XX_Init(float hz,uint8_t angle)
{
	uint32_t off=0;

	pca_write(pca_mode1,0x0);
	pca_setfreq(hz);//设置PWM频率
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


