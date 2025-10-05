/*
 * ch455g.c
 *
 * created: 2023-05-11
 *  author:
 */

#include "ls1b.h"
#include "ls1b_gpio.h"
#include "tick.h"
#include "bsp.h"
#include "ch455g.h"
#include "ls1x_i2c_bus.h"
#include "rtthread.h"
#include "main.h"
void CH455_Write( unsigned short cmd )	//写命令
{
    unsigned char cmd_data[1]= {0},cmd1_data[1]= {0};
    unsigned char i;
    cmd_data[0] =((unsigned char)(cmd>>7)&CH455_I2C_MASK)|CH455_I2C_ADDR;
    cmd1_data[0] =(unsigned char)cmd;
    rt_mutex_take(I2C0_mutex, RT_WAITING_FOREVER);
    ls1x_i2c_send_start(busI2C0,CH455_I2C_ADDR);             //启动总线
    ls1x_i2c_send_addr(busI2C0,CH455_I2C_ADDR,0);	//发送设备地址+写信号
    ls1x_i2c_write_bytes(busI2C0,cmd_data,1);
    ls1x_i2c_send_addr(busI2C0,CH455_I2C_ADDR,0);	//发送设备地址+写信号
    ls1x_i2c_write_bytes(busI2C0,cmd1_data,1);
    ls1x_i2c_send_stop(busI2C0,CH455_I2C_ADDR);                 //结束总线
    rt_mutex_release(I2C0_mutex);
}

unsigned char CH455_Read( void )		//读取按键
{
    unsigned char keycode[1];
    unsigned short cmd_data1[1];
    unsigned char temp_data;
    unsigned char i;
    cmd_data1[0] =(unsigned char)(CH455_GET_KEY>>7)&CH455_I2C_MASK|0x01|CH455_I2C_ADDR;
    rt_mutex_take(I2C0_mutex, RT_WAITING_FOREVER);
    ls1x_i2c_send_start(busI2C0, CH455_I2C_ADDR);   //启动总线
    ls1x_i2c_send_addr(busI2C0,CH455_I2C_ADDR,0);	//发送设备地址+写信号
    ls1x_i2c_write_bytes(busI2C0,cmd_data1,1);
    ls1x_i2c_send_addr(busI2C0,CH455_I2C_ADDR,1);//发送设备地址加读信号
    ls1x_i2c_read_bytes(busI2C0,keycode,1);      //读取数据
    ls1x_i2c_send_stop(busI2C0,CH455_I2C_ADDR);               //结束总线
    rt_mutex_release(I2C0_mutex);
    temp_data = keycode[0];
    return temp_data;

}
//需要循环调用（100ms）

unsigned char Keyboard_proc(void)
{
    unsigned char keyID  = 0;
    unsigned char temp = 0;
    keyID = CH455_Read();
     if(keyID>0x40)
     {
        switch(keyID)
        {
        case 4+0x40:
            temp = 1;
            break;
        case 5+0x40:
            temp = 5;
            break;
        case 6+0x40:
            temp = 9;
            break;
        case 7+0x40:
            temp = 13;
            break;
        case 12+0x40:
            temp = 2;
            break;
        case 13+0x40:
            temp = 6;
            break;
        case 14+0x40:
            temp = 10;
            break;
        case 15+0x40:
            temp = 14;
            break;
        case 20+0x40:
            temp = 3;
            break;
        case 21+0x40:
            temp = 7;
            break;
        case 22+0x40:
            temp = 11;
            break;
        case 23+0x40:
            temp = 15;
            break;
        case 28+0x40:
            temp = 4;
            break;
        case 29+0x40:
            temp = 8;
            break;
        case 30+0x40:
            temp = 12;

            break;
        case 31+0x40:
            temp = 16;
            break;
        }
       while(CH455_Read()>0x40)rt_thread_delay(50);

     }
    return temp;
}




