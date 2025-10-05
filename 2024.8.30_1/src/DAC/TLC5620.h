/*
 * TLC5620.h
 *
 * created: 2021/6/15
 * authour:
 */
#ifndef _TLC5620_H
#define _TLC5620_H

#define LDAC 28 // 引脚宏定义
#define LOAD 29
#define Data_in 54
#define SCLK 52

extern unsigned char tab1[200];

extern int amplitude;
extern int points;
void TLC5620_Init(); // 引脚初始化函数
void dac(unsigned short DA_DATA);
void Output_channel(unsigned char channel); // 输出通道选择
void Write_RNG(unsigned char RNG_value);    // 设置输出电压范围（详见TLC5620数据手册）
void Waveform_output(void);
void DAC_output_CH1(void);
void DAC_TIM_init(void);
void generateTriangleWave(int amplitude,int points, unsigned char *buf); // 三角波
void generateSquareWave(int amplitude,int points, unsigned char *buf, int num); // 方形波 // 锯齿波
void generateSineWave(double amplitude, int points, unsigned char *buf);     // 正弦波
#endif


