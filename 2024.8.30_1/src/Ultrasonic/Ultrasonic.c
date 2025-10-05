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
static void pwmtimer_callback(void *pwm, int *stopit) //��ʱ���жϺ���
{
    Timer++;
    //if(Timer>=42949672000)
         //Timer = 0;
}
// ���������

void timer_init(void)
{
    /**
     * @brief ��ʱ����ʼ��
     * @param 1 void
     * @return void
     */
    pwm2_cfg.hi_ns = 5000;            //�ߵ�ƽʱ��
    pwm2_cfg.lo_ns = 0;               //�͵�ƽʱ��
    pwm2_cfg.mode = PWM_CONTINUE_TIMER;     //�������ģʽ
    pwm2_cfg.cb = pwmtimer_callback;        //��ʱ���жϷ�����
    pwm2_cfg.isr = NULL;                    //�����ڶ�ʱ��ģʽ

    ls1x_pwm_init(devPWM2, NULL);       //PWM��ʼ��
    ls1x_pwm_open(devPWM2, (void *)&pwm2_cfg); 
    ls1x_pwm_timer_stop(devPWM2);              
}

void ECHO_irqhandler(int IRQn, void *param)
{
    /**
     * @brief ���������������ж�
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
     * @brief ��ʼ��������
     * @param 1 void
     * @return void
     */
    gpio_enable(51, DIR_OUT);
    gpio_enable(50, DIR_IN);
    ls1x_install_gpio_isr(50, INT_TRIG_EDGE_DOWN, ECHO_irqhandler, NULL);   //�½��ش������������շ�����
    ls1x_enable_gpio_interrupt(50);    //ʹ���ж�
    gpio_write(51, 0);      //��������Ĭ��Ϊ�͵�ƽ
    timer_init();           //��ʱ����ʼ��
}

unsigned int Ultrasonic_ranging(void)
{
    /**
     * @brief ���������
     * @param 1 void
     * @return void
     */
    uint8_t i = 0;
    unsigned int j = 0;
    distance = 0;
    /* �ر��ж� */
	rt_base_t level = rt_hw_interrupt_disable();
    for (i = 0; i < 4; i++)
    {
        gpio_write(51, 1);
        delay_us(12);
        gpio_write(51, 0);
        delay_us(12);
    }
    	  /* �ָ��ж� */
    rt_hw_interrupt_enable(level);
    ls1x_pwm_timer_start(devPWM2, (void *)&pwm2_cfg); // �򿪶�ʱ��
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
/* ���ܣ�5������ȥ��������СȻ�����м���������ƽ��ֵ
   ������distance��Ҫ���������
*/
unsigned int average(unsigned int *distance)       //ȥ�������С��3����λ��ȡƽ��ֵ
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

/* ���ܣ�5������ȥ��������СȻ����ƽ��ֵ
   ������distance��Ҫ���������
*/
unsigned int distance_ranging(void)
{
     unsigned char i=0;unsigned int distance[5]={0},dis = 0;
	 for(i=0;i<5;i++)
	  {
			        rt_thread_delay(200);
		            distance[i]=Ultrasonic_ranging();
	  }
	//	rt_kprintf("����1:%dmm,2:%dmm,3:%dmm,4:%dmm,5:%dmm,6:%dmm,7:%dmm,8:%dmm,9:%dmm,10:%dmm\r\n",distance[0],distance[1]\
		,distance[2],distance[3],distance[4],distance[5],distance[6],distance[7],distance[8],distance[9]);
	 dis=average(distance);//ȥ�������С��3����λ��ȡƽ��ֵ
	 rt_kprintf("����������%d\r\n",dis);
	 return dis ;
}
