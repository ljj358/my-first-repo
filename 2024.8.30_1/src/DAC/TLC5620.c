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

int amplitude = 127; // ���ò��������������壩
int points = 1830;     // ���ò��εĵ�����ʱ�䣩

unsigned char tab1[200];
/*
  ����һ�����Ҳ��źţ�������洢���ṩ�Ļ�������
  ����:
    amplitude: ���Ҳ������

    points:    ���Ҳ��ĵ���
    buf: ���ڴ洢���ɵ����Ҳ��źŵĻ�����
*/
void generateSineWave(double amplitude, int points, unsigned char *buf) {
    if (amplitude < 0 || amplitude > 127) {
        // ��������� 0 �� 127 ֮��
        return;
    }

    if (buf == NULL) {
        // ����������Ϊ��
        return;
    }

    int i;
    for (i = 0; i < points; i++) {
        // �������Ҳ�ֵ
        double value = amplitude * sin(2 * M_PI * i / (double) points);

        // ��ֵӳ�䵽 0-255 ��Χ�ڣ����洢����������
        int mapped_value = (int) ((value + amplitude) * (255.0 / (2 * amplitude)));
        if (mapped_value < 0) mapped_value = 0;
        if (mapped_value > 255) mapped_value = 255;

        buf[i] = (unsigned char) mapped_value;
    }
}

/*
  ����һ�����ǲ��źţ�������洢���ṩ�Ļ�������
  ����:
    amplitude: ���ǲ������
    points:    ���ǲ��ĵ���
    buf: ���ڴ洢���ɵ����ǲ��źŵĻ�����
*/
void generateTriangleWave(int amplitude, int points, unsigned char *buf)
{
    int i;
    for (i = 0; i < points; i++)
    {
        // �������ǲ���ֵ
        int value = (int)(amplitude * (1.0 - fabs(((2.0 * i) / points) - 1)));
        buf[i] = value;
    }
}

/*
  ����һ�������źţ�������洢���ṩ�Ļ�������
  ����:
    amplitude: ���������
    points:    �����ĵ���
    num:       ѡ����
    1:         ��ݲ�
    2:         ����
    buf: ���ڴ洢���ɵķ����źŵĻ�����
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
                // ����ߵ�ƽ

                value += 4;
                buf[i] = value;
            }
            else if (num == 2)
            {
                // ����ߵ�ƽ
                buf[i] = amplitude;
            }
        }
        else
        {
            // ����͵�ƽ
            buf[i] = 0;
        }
    }
}

//**********************************************************
// ���ų�ʼ������
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

// DAת��
void dac(unsigned short DA_DATA)
{
    unsigned char i;

    for (i = 0; i < 8; i++)
    {
        TLC5620_SCLK(1);
        TLC5620_Data_in(DA_DATA & 0x80);
        TLC5620_SCLK(0);
        DA_DATA = DA_DATA << 1; // ����������
    }
    TLC5620_LOAD(0);
    TLC5620_LOAD(1);

}
//**********************************************************
// ���ͨ��ѡ��
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
// ���������ѹ��Χ�����TLC5620�����ֲᣩ
void Write_RNG(unsigned char RNG_value) //1�Ŵ�������0���Ŵ�
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
    Output_channel(1); // ѡ��ͨ��A
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
    
        TLC5620_SCLK(1);   //ͨ��1
        TLC5620_Data_in(0);
        TLC5620_SCLK(0);
        TLC5620_SCLK(1);
        TLC5620_Data_in(0);
        TLC5620_SCLK(0);
        
        TLC5620_SCLK(1);
        TLC5620_Data_in(0);  //���Ŵ�
        TLC5620_SCLK(0);
        
        if (I == points) I = 0;
        dac(tab1[I++]);

}
void DAC_output_CH2(void)
{
    static unsigned short I = 0;

        TLC5620_SCLK(1);  //ͨ��2
        TLC5620_Data_in(0);
        TLC5620_SCLK(0);
        TLC5620_SCLK(1);
        TLC5620_Data_in(1);
        TLC5620_SCLK(0);

        TLC5620_SCLK(1);
        TLC5620_Data_in(0);  //���Ŵ�
        TLC5620_SCLK(0);

        if (I == points) I = 0;
        dac(tab1[I++]);

}
void DAC_output_CH3(void)
{
    static unsigned short I = 0;

        TLC5620_SCLK(1);    //ͨ��3
        TLC5620_Data_in(1);
        TLC5620_SCLK(0);
        TLC5620_SCLK(1);
        TLC5620_Data_in(0);
        TLC5620_SCLK(0);

        TLC5620_SCLK(1);
        TLC5620_Data_in(0);  //���Ŵ�
        TLC5620_SCLK(0);

        if (I == points) I = 0;
        dac(tab1[I++]);

}
void DAC_output_CH4(void)
{
    static unsigned short I = 0;

        TLC5620_SCLK(1);    //ͨ��3
        TLC5620_Data_in(1);
        TLC5620_SCLK(0);
        TLC5620_SCLK(1);
        TLC5620_Data_in(1);
        TLC5620_SCLK(0);

        TLC5620_SCLK(1);
        TLC5620_Data_in(0);  //���Ŵ�
        TLC5620_SCLK(0);

        if (I == points) I = 0;
        dac(tab1[I++]);

}




pwm_cfg_t pwm1_cfg;
extern void *devPWM1;
static void pwm1_interrupt(void *pwm, int *stopit) // �жϷ�����
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
     * @brief ��ʱ����ʼ��
     * @param 1 void
     * @return void
     */
    //pwm1_cfg.hi_ns = 2000000;              //  2ms
    //pwm1_cfg.hi_ns = 5000000;              //  5ms
    //pwm1_cfg.hi_ns =   300000;              //  300Us
    //pwm1_cfg.hi_ns =   400000;              //  400Us
    //pwm1_cfg.hi_ns =   800000;              //  800Us
    //pwm1_cfg.hi_ns =   700000;              //  700Us

    pwm1_cfg.hi_ns =   52000;              // �ߵ�ƽʱ�� 52Us

    //pwm1_cfg.hi_ns =   31510;              // �ߵ�ƽʱ�� 33Us



    pwm1_cfg.lo_ns = 0;              // �͵�ƽʱ��
    pwm1_cfg.mode = PWM_CONTINUE_TIMER; // �������ģʽ
    pwm1_cfg.cb = pwm1_interrupt;            // ��ʱ���жϷ�����
    pwm1_cfg.isr = NULL;                // �����ڶ�ʱ��ģʽ

    ls1x_pwm_init(devPWM1, NULL); // PWM��ʼ��
    ls1x_pwm_open(devPWM1, (void *)&pwm1_cfg);
    ls1x_pwm_timer_stop(devPWM1);
    ls1x_pwm_timer_start(devPWM1, (void *)&pwm1_cfg);

}

