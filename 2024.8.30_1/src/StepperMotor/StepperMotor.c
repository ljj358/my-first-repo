/*
 * StepperMotor.c
 *
 * created: 2024/5/21
 *  author: 
 */
#include "StepperMotor.h"
void StepperMotor(void){
	gpio_enable( 40, DIR_OUT );
	gpio_enable( 41, DIR_OUT );
	gpio_enable( 44, DIR_OUT );
	gpio_enable( 39, DIR_OUT );
	SM_IN1(0);
	SM_IN2(0);
	SM_IN3(0);
	SM_IN4(0);
}

/******
step��Ϊ���� 0-7��Ӧ1-8��
dir������ 0Ϊ������ 1Ϊ������
*******/
void step_28byj48_control(unsigned char  step,unsigned char  dir)//���������������
{
	unsigned char  temp=step;
	
	if(dir==0) temp=7-step;
	switch(temp)  
	{
		case 0:	SM_IN1(0);SM_IN2(0);SM_IN3(0);SM_IN4(1);break;
		case 1:	SM_IN1(0);SM_IN2(0);SM_IN3(1);SM_IN4(1);break;
		case 2:	SM_IN1(0);SM_IN2(0);SM_IN3(1);SM_IN4(0);break;
		case 3:	SM_IN1(0);SM_IN2(1);SM_IN3(1);SM_IN4(0);break;
		case 4:	SM_IN1(0);SM_IN2(1);SM_IN3(0);SM_IN4(0);break;
		case 5:	SM_IN1(1);SM_IN2(1);SM_IN3(0);SM_IN4(0);break;
		case 6:	SM_IN1(1);SM_IN2(0);SM_IN3(0);SM_IN4(0);break;
		case 7:	SM_IN1(1);SM_IN2(0);SM_IN3(0);SM_IN4(1);break;
	}
}
/******
angles :�Ƕ�
dir������ 0Ϊ������ 1Ϊ������
*******/
void step_28byj48_angles(unsigned int  angles,unsigned char  dir)
{
 //   int meter = (int)((float)angles*11.37f);
  int meter = angles*10;
    unsigned int i=0,j=0;
    for(;i<meter;i++){
       step_28byj48_control(j,dir);
       j++;
       if(j>=7)j=0;
       rt_thread_delay(2);
    }
       SM_IN1(0);
       SM_IN2(0);
       SM_IN3(0);
       SM_IN4(0);
       
}

