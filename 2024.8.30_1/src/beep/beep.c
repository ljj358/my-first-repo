#include "beep.h"
#include "ls1b_gpio.h"
#include "ls1x_fb.h"
#include "ls1x_pwm.h"
#include "bsp.h"
#include "beep.h"

//unsigned short music[]={3816,3401,3030,2857,2551,2272,2024};
unsigned short music[] = {955, 851, 758, 715, 637, 568, 506,247,220,196,175,165,147,131,1911};
pwm_cfg_t pwm1_cfg;
extern void *devPWM1;
static void pwm1_interrupt(void *pwm, int *stopit) // 中断服务函数
{
     BEEP_flip();
}

void BEEP_TIM_init(void)
{
    /**
     * @brief 定时器初始化
     * @param 1 void
     * @return void
     */
    //pwm1_cfg.hi_ns = 2000000;              //  5ms

    //pwm1_cfg.hi_ns =   300000;              //  300Us
    //pwm1_cfg.hi_ns =   400000;              //  400Us
    pwm1_cfg.hi_ns =   800000;              //  800Us
    //pwm1_cfg.hi_ns =   700000;              //  700Us

    //pwm1_cfg.hi_ns =   52000;              // 高电平时间 52Us

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
void BEEP_Init(void)
{
    /**
     * @brief 蜂鸣器初始化
     * @param 1 void
     * @return void
     */
    gpio_enable(BEEP, DIR_OUT);
    gpio_write(BEEP, 0);
}
void BEEP_open_TIM(void){   //通过定时器产生频率
    BEEP_TIM_init();
}
void BEEP_OFF_TIM(void){
    ls1x_pwm_timer_stop(devPWM1);
    BEEP_Off();
}
void BEEP_open(int tim){    //通过延时产生频率
    if(tim<1||tim>10000) return;
    unsigned int tickcount = get_clock_ticks();  //获取cpu时钟
    while(get_clock_ticks()-tickcount<tim)
    {
        BEEP_flip();
        rt_thread_delay(1);
    }
}

void BEEP_music(int cnt,int tim){
    if(cnt<1||cnt>7) return;
    else if(tim<1||cnt>10000) return;
    unsigned int tickcount = get_clock_ticks();  //获取cpu时钟
    while(get_clock_ticks()-tickcount<tim)
    {
        BEEP_flip();
        delay_us(music[cnt-1]);
    }
    BEEP_Off();//关闭蜂鸣器
}


