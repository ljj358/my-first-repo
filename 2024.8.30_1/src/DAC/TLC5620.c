#include "TLC5620.h"
#include "ls1b_gpio.h"
#include <math.h>

#include "ls1x_fb.h"
#include "ls1x_pwm.h"
#include "bsp.h"


#define TLC5620_LDAC(val) gpio_write(28, val)    
#define TLC5620_LOAD(val) gpio_write(29, val)    
#define TLC5620_Data_in(val) gpio_write(54, val) 
#define TLC5620_SCLK(val) gpio_write(52, val)    

int amplitude = 127; // 设置波形最大振幅（波峰）
int points = 1830;     // 设置波形的点数（时间）

unsigned char tab1[200];
/*
  生成一个正弦波信号，并将其存储在提供的缓冲区中
  参数:
    amplitude: 正弦波的振幅

    points:    正弦波的点数
    buf: 用于存储生成的正弦波信号的缓冲区
*/
void generateSineWave(double amplitude, int points, unsigned char *buf) {
    if (amplitude < 0 || amplitude > 127) {
        // 振幅必须在 0 和 127 之间
        return;
    }

    if (buf == NULL) {
        // 缓冲区不能为空
        return;
    }

    int i;
    for (i = 0; i < points; i++) {
        // 生成正弦波值
        double value = amplitude * sin(2 * M_PI * i / (double) points);

        // 将值映射到 0-255 范围内，并存储到缓冲区中
        int mapped_value = (int) ((value + amplitude) * (255.0 / (2 * amplitude)));
        if (mapped_value < 0) mapped_value = 0;
        if (mapped_value > 255) mapped_value = 255;

        buf[i] = (unsigned char) mapped_value;
    }
}

/*
  生成一个三角波信号，并将其存储在提供的缓冲区中
  参数:
    amplitude: 三角波的振幅
    points:    三角波的点数
    buf: 用于存储生成的三角波信号的缓冲区
*/
void generateTriangleWave(int amplitude, int points, unsigned char *buf)
{
    int i;
    for (i = 0; i < points; i++)
    {
        // 计算三角波的值
        int value = (int)(amplitude * (1.0 - fabs(((2.0 * i) / points) - 1)));
        buf[i] = value;
    }
}

/*
  生成一个方波信号，并将其存储在提供的缓冲区中
  参数:
    amplitude: 方波的振幅
    points:    方波的点数
    num:       选择波形
    1:         锯齿波
    2:         方波
    buf: 用于存储生成的方波信号的缓冲区
*/
void generateSquareWave(int amplitude, int points, unsigned char *buf, int num)
{
    int halfPoints = points / 2;
    int i;
    int value = 0;
    for (i = 0; i < points; i++)
    {
        if (i < halfPoints)
        {
            if (num == 1)
            {
                // 输出高电平

                value += 4;
                buf[i] = value;
            }
            else if (num == 2)
            {
                // 输出高电平
                buf[i] = amplitude;
            }
        }
        else
        {
            // 输出低电平
            buf[i] = 0;
        }
    }
}

//**********************************************************
// 引脚初始化函数
void TLC5620_Init(void)
{
    gpio_enable(LDAC, DIR_OUT);
    gpio_enable(LOAD, DIR_OUT);
    gpio_enable(Data_in, DIR_OUT);
    gpio_enable(SCLK, DIR_OUT);

    gpio_write(LDAC, 0);
    gpio_write(LOAD, 1);
    gpio_write(Data_in, 0);
    gpio_write(SCLK, 0);
}

// DA转换
void dac(unsigned short DA_DATA)
{
    unsigned char i;

    for (i = 0; i < 8; i++)
    {
        TLC5620_SCLK(1);
        TLC5620_Data_in(DA_DATA & 0x80);
        TLC5620_SCLK(0);
        DA_DATA = DA_DATA << 1; // 数据向左移
    }
    TLC5620_LOAD(0);
    TLC5620_LOAD(1);

}
//**********************************************************
// 输出通道选择
void Output_channel(unsigned char channel)
{
    switch (channel)
    {
    case 0:
        TLC5620_SCLK(1);
        TLC5620_Data_in(0);
        TLC5620_SCLK(0);
        TLC5620_SCLK(1);
        TLC5620_Data_in(0);
        TLC5620_SCLK(0);
        break;
    case 1:
        TLC5620_SCLK(1);
        TLC5620_Data_in(0);
        TLC5620_SCLK(0);
        TLC5620_SCLK(1);
        TLC5620_Data_in(1);
        TLC5620_SCLK(0);
        break;
    case 2:
        TLC5620_SCLK(1);
        TLC5620_Data_in(1);
        TLC5620_SCLK(0);
        TLC5620_SCLK(1);
        TLC5620_Data_in(0);
        TLC5620_SCLK(0);
        break;
    case 3:
        TLC5620_SCLK(1);
        TLC5620_Data_in(1);
        TLC5620_SCLK(0);
        TLC5620_SCLK(1);
        TLC5620_Data_in(1);
        TLC5620_SCLK(0);
        break;
    default:
        break;
    }
}

//**********************************************************
// 设置输出电压范围（详见TLC5620数据手册）
void Write_RNG(unsigned char RNG_value) //1放大两倍，0不放大
{
    TLC5620_SCLK(1);
    TLC5620_Data_in(RNG_value);
    TLC5620_SCLK(0);
}

void Waveform_output(void)
{
    static unsigned short j = 0;
    gpio_write(LDAC, 0);
    gpio_write(LOAD, 1);
    Output_channel(1); // 选择通道A
    Write_RNG(1);
    if (j == points)
        j = 0;
    if (points == 128)
        dac(tab1[j++]);
    else if (points == 64)
        dac(tab1[j++]);
    else if (points == 32)
        dac(tab1[j++]);
}



void DAC_output_CH1(void)
{
    static unsigned short I = 0;
    
        TLC5620_SCLK(1);   //通道1
        TLC5620_Data_in(0);
        TLC5620_SCLK(0);
        TLC5620_SCLK(1);
        TLC5620_Data_in(0);
        TLC5620_SCLK(0);
        
        TLC5620_SCLK(1);
        TLC5620_Data_in(0);  //不放大
        TLC5620_SCLK(0);
        
        if (I == points) I = 0;
        dac(tab1[I++]);

}
void DAC_output_CH2(void)
{
    static unsigned short I = 0;

        TLC5620_SCLK(1);  //通道2
        TLC5620_Data_in(0);
        TLC5620_SCLK(0);
        TLC5620_SCLK(1);
        TLC5620_Data_in(1);
        TLC5620_SCLK(0);

        TLC5620_SCLK(1);
        TLC5620_Data_in(0);  //不放大
        TLC5620_SCLK(0);

        if (I == points) I = 0;
        dac(tab1[I++]);

}
void DAC_output_CH3(void)
{
    static unsigned short I = 0;

        TLC5620_SCLK(1);    //通道3
        TLC5620_Data_in(1);
        TLC5620_SCLK(0);
        TLC5620_SCLK(1);
        TLC5620_Data_in(0);
        TLC5620_SCLK(0);

        TLC5620_SCLK(1);
        TLC5620_Data_in(0);  //不放大
        TLC5620_SCLK(0);

        if (I == points) I = 0;
        dac(tab1[I++]);

}
void DAC_output_CH4(void)
{
    static unsigned short I = 0;

        TLC5620_SCLK(1);    //通道3
        TLC5620_Data_in(1);
        TLC5620_SCLK(0);
        TLC5620_SCLK(1);
        TLC5620_Data_in(1);
        TLC5620_SCLK(0);

        TLC5620_SCLK(1);
        TLC5620_Data_in(0);  //不放大
        TLC5620_SCLK(0);

        if (I == points) I = 0;
        dac(tab1[I++]);

}




pwm_cfg_t pwm1_cfg;
extern void *devPWM1;
static void pwm1_interrupt(void *pwm, int *stopit) // 中断服务函数
{
     static data = 0;
     //gpio_write(58, !gpio_read(58));
     DAC_output_CH1();
        
}

void DAC_TIM_init(void)
{
    //generateSquareWave(128,192,tab6,2);
    generateSquareWave(amplitude,points,tab1,2);
    //gpio_enable(58, DIR_OUT);
    //gpio_write(58, 0);
    /**
     * @brief 定时器初始化
     * @param 1 void
     * @return void
     */
    //pwm1_cfg.hi_ns = 2000000;              //  2ms
    //pwm1_cfg.hi_ns = 5000000;              //  5ms
    //pwm1_cfg.hi_ns =   300000;              //  300Us
    //pwm1_cfg.hi_ns =   400000;              //  400Us
    //pwm1_cfg.hi_ns =   800000;              //  800Us
    //pwm1_cfg.hi_ns =   700000;              //  700Us

    pwm1_cfg.hi_ns =   52000;              // 高电平时间 52Us

    //pwm1_cfg.hi_ns =   31510;              // 高电平时间 33Us



    pwm1_cfg.lo_ns = 0;              // 低电平时间
    pwm1_cfg.mode = PWM_CONTINUE_TIMER; // 脉冲持续模式
    pwm1_cfg.cb = pwm1_interrupt;            // 定时器中断服务函数
    pwm1_cfg.isr = NULL;                // 工作在定时器模式

    ls1x_pwm_init(devPWM1, NULL); // PWM初始化
    ls1x_pwm_open(devPWM1, (void *)&pwm1_cfg);
    ls1x_pwm_timer_stop(devPWM1);
    ls1x_pwm_timer_start(devPWM1, (void *)&pwm1_cfg);

}

