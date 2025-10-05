/*
 * infrared.c
 *
 * created: 2023/11/29
 *  author: 
 */
#include "ls1b_gpio.h"
/*******************************************************************
 **infrared_Config_Init
 **函数功能：初始化红外对管的IO口
 **形参：无
 **返回值：无
 **说明：
 *******************************************************************/
 void infrared_Config_Init(void)
 {
    //库开发
    /* 配置按键IO为输入模式 */
	gpio_enable(43, DIR_IN );
 }
/*******************************************************************
 **Acquisition_infrared_state
 **函数功能：获取红外对管的状态
 **形参：无
 **返回值：1遮挡，0无遮挡
 **说明：
 *******************************************************************/
unsigned char  Acquisition_infrared_state(void){
    if(gpio_read(43) == 0)return 1;
    else return 0;
}
