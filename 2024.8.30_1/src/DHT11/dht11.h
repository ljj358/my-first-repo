/*
 * dht11.h
 *
 * created: 2024/5/20
 *  author: 
 */

#ifndef _DHT11_H
#define _DHT11_H

#ifdef __cplusplus
extern "C" {
#endif
#include "ls1b_gpio.h"
#include "rtthread.h"
#define dht11_data_io 38

#define DH11_GPIO_Init_OUT() gpio_enable( dht11_data_io, DIR_OUT )
#define DH11_GPIO_Init_IN() gpio_enable( dht11_data_io, DIR_IN )
#define Read_Data gpio_read(dht11_data_io)
#define dht11_high gpio_write(dht11_data_io,1)
#define dht11_low  gpio_write(dht11_data_io,0)
void DHT11_REC_Data(void);
extern unsigned int rec_data[4];
#ifdef __cplusplus
}
#endif

#endif // _DHT11_H

