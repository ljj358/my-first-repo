/*
 * led.c
 *
 * created: 2022/7/6
 *  author: 
 */
 #include "led.h"
 #include "ls1b_gpio.h"
 #include "ls1b.h"
 #include "pwm_ic.h"
 /*******************************************************************
 **函数名：LED_IO_Config
 **函数功能：初始化LED的IO口
 **形参：无
 **返回值：无
 **说明：
 *******************************************************************/
 void LED_IO_Config_Init(void)
 {
    //库开发
    gpio_enable(LED1,DIR_OUT);
    gpio_enable(LED2,DIR_OUT);
    gpio_enable(LED3,DIR_OUT);

    gpio_write(LED3, OFF);
    gpio_write(LED2, OFF);
    gpio_write(LED1, OFF);
    
    set_RGB(rgb_blue,0);
    set_RGB(rgb_green,0);
    set_RGB(rgb_red,0);
 }

/*******************************************************************
 **函数名：LEDx_Status
 **函数功能：设置单个LED灯的状态
 **形参：
    status  :选择灯和状态
 **返回值：无
 **说明：
 *******************************************************************/
 void LEDx_Set_Status(u8 status)
 {
    /**
     * @brief RGB颜色配置
     * @param 1 颜色
     * @return void
     */
    if (status != OFF)
    {
        if (status == redRGB_ON)
            gpio_write(LED1, 1);
        else if (status == redRGB_OFF)
            gpio_write(LED1, 0);
        else if (status == greenRGB_ON)
            gpio_write(LED2, 1);
        else if (status == greenRGB_OFF)
            gpio_write(LED2, 0);
        else if (status == blueRGB_ON)
            gpio_write(LED3, 1);
        else if (status == blueRGB_OFF)
            gpio_write(LED3, 0);
        else if (status == yellowRGB)
        {
            gpio_write(LED1, 1);
            gpio_write(LED2, 1);
            gpio_write(LED3, 0);
        }
        else if (status == whiteRGB)
        {
            gpio_write(LED1, 1);
            gpio_write(LED2, 1);
            gpio_write(LED3, 1);
        }
        else if (status == purpleRGB)
        {
            gpio_write(LED1, 1);
            gpio_write(LED2, 0);
            gpio_write(LED3, 1);
        }
        else if (status == cyanRGB)
        {
            gpio_write(LED1, 0);
            gpio_write(LED2, 1);
            gpio_write(LED3, 1);
        }
    }

    else
    {
        gpio_write(LED3, 0);
        gpio_write(LED2, 0);
        gpio_write(LED1, 0);
    }
 }
 
void set_RGB(char led,unsigned int data){
    if(data<0||data>256)return;
    switch(led)
    {
        case rgb_red:   pca_setpwm(6,0,(data*16)-1);break;
        case rgb_green: pca_setpwm(5,0,(data*16)-1);break;
        case rgb_blue:  pca_setpwm(7,0,(data*16)-1);break;
        default : return;
    }
    
    return;
}
 



