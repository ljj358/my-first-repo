/*
 * TLC5620.h
 *
 * created: 2021/6/15
 * authour:
 */
#ifndef _TLC5620_H
#define _TLC5620_H

#define LDAC 28 // ���ź궨��
#define LOAD 29
#define Data_in 54
#define SCLK 52

extern unsigned char tab1[200];

extern int amplitude;
extern int points;
void TLC5620_Init(); // ���ų�ʼ������
void dac(unsigned short DA_DATA);
void Output_channel(unsigned char channel); // ���ͨ��ѡ��
void Write_RNG(unsigned char RNG_value);    // ���������ѹ��Χ�����TLC5620�����ֲᣩ
void Waveform_output(void);
void DAC_output_CH1(void);
void DAC_TIM_init(void);
void generateTriangleWave(int amplitude,int points, unsigned char *buf); // ���ǲ�
void generateSquareWave(int amplitude,int points, unsigned char *buf, int num); // ���β� // ��ݲ�
void generateSineWave(double amplitude, int points, unsigned char *buf);     // ���Ҳ�
#endif


