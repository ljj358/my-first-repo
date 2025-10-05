/*
 * Ultrasonic.c
 *
 * created: 2023/2/23
 *  author:
 */
#include "rtthread.h"
#include "Ultrasonic.h"

extern void *devPWM2;
float last_distance = 0;
volatile unsigned int distance = 0;
volatile static unsigned int Timer = 0;
extern void *devPWM2;
pwm_cfg_t pwm2_cfg;
static void pwmtimer_callback(void *pwm, int *stopit) //定时器中断函数
{
    Timer++;
    //if(Timer>=42949672000)
         //Timer = 0;
}
// 超声波测距

void timer_init(void)
{
    /**
     * @brief 定时器初始化
     * @param 1 void
     * @return void
     */
    pwm2_cfg.hi_ns = 5000;            //高电平时间
    pwm2_cfg.lo_ns = 0;               //低电平时间
    pwm2_cfg.mode = PWM_CONTINUE_TIMER;     //脉冲持续模式
    pwm2_cfg.cb = pwmtimer_callback;        //定时器中断服务函数
    pwm2_cfg.isr = NULL;                    //工作在定时器模式

    ls1x_pwm_init(devPWM2, NULL);       //PWM初始化
    ls1x_pwm_open(devPWM2, (void *)&pwm2_cfg); 
    ls1x_pwm_timer_stop(devPWM2);              
}

void ECHO_irqhandler(int IRQn, void *param)
{
    /**
     * @brief 超声波接收引脚中断
     * @param 1 void
     * @return void
     */
    ls1x_pwm_timer_stop(devPWM2);
    distance = Timer * 1.7 / 1.2;

    Timer = 0;
}

void Ultrasonic_init(void)
{
    /**
     * @brief 初始化超声波
     * @param 1 void
     * @return void
     */
    gpio_enable(51, DIR_OUT);
    gpio_enable(50, DIR_IN);
    ls1x_install_gpio_isr(50, INT_TRIG_EDGE_DOWN, ECHO_irqhandler, NULL);   //下降沿触发超声波接收服务函数
    ls1x_enable_gpio_interrupt(50);    //使能中断
    gpio_write(51, 0);      //发送引脚默认为低电平
    timer_init();           //定时器初始化
}

unsigned int Ultrasonic_ranging(void)
{
    /**
     * @brief 超声波测距
     * @param 1 void
     * @return void
     */
    uint8_t i = 0;
    unsigned int j = 0;
    distance = 0;
    /* 关闭中断 */
	rt_base_t level = rt_hw_interrupt_disable();
    for (i = 0; i < 4; i++)
    {
        gpio_write(51, 1);
        delay_us(12);
        gpio_write(51, 0);
        delay_us(12);
    }
    	  /* 恢复中断 */
    rt_hw_interrupt_enable(level);
    ls1x_pwm_timer_start(devPWM2, (void *)&pwm2_cfg); // 打开定时器
    while(j<100){
        if(distance!=0)
            break;
        rt_thread_mdelay(1);
        j++;
    }
    if (distance >= 2000)
    {
        distance = 2000;
    }
    if (distance <= 0)
    {
        distance = last_distance;
    }
    last_distance = distance;
    return last_distance;
}
/* 功能：5个数据去掉最大和最小然后求中间三个数的平均值
   参数：distance需要计算的数组
*/
unsigned int average(unsigned int *distance)       //去最大与最小，3个中位数取平均值
{
	unsigned int num,j,i;
	for( i=0;i<5;i++)
	{
		for(j=0;j<5-i;j++)
		{
			if(distance[j]>distance[j+1])
			{
				num=distance[j];
				distance[j]=distance[j+1];
				distance[j+1]=num;
			}
		}
	}
	num=(distance[1]+distance[2]+distance[3])/3;
	return num;
}

/* 功能：5个数据去掉最大和最小然后求平均值
   参数：distance需要计算的数组
*/
unsigned int distance_ranging(void)
{
     unsigned char i=0;unsigned int distance[5]={0},dis = 0;
	 for(i=0;i<5;i++)
	  {
			        rt_thread_delay(200);
		            distance[i]=Ultrasonic_ranging();
	  }
	//	rt_kprintf("距离1:%dmm,2:%dmm,3:%dmm,4:%dmm,5:%dmm,6:%dmm,7:%dmm,8:%dmm,9:%dmm,10:%dmm\r\n",distance[0],distance[1]\
		,distance[2],distance[3],distance[4],distance[5],distance[6],distance[7],distance[8],distance[9]);
	 dis=average(distance);//去最大与最小，3个中位数取平均值
	 rt_kprintf("超声波距离%d\r\n",dis);
	 return dis ;
}
