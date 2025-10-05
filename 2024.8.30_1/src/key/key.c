/*
 * test.c
 *
 * created: 2022/7/6
 *  author:
 */
#include "ls1b_gpio.h"
#include "key.h"

 char key(void)
{
	if ( gpio_read(KEY_1) == 0 )
	{
		rt_thread_delay(20);                 /* �������� */
		if ( gpio_read( KEY_1 ) == 0 )  /* ��ʾ��ȷ�������� */
		{
			while ( gpio_read( KEY_1 ) == 0 )rt_thread_delay(20); /* �ȴ�������� */
			return 1;
		}
	}
	else if ( gpio_read( KEY_2 ) == 0 )
	{
		rt_thread_delay(20);                 /* �������� */
		if ( gpio_read( KEY_2 ) == 0 )  /* ��ʾ��ȷ�������� */
		{
			while ( gpio_read( KEY_2 ) == 0 )rt_thread_delay(20);  /* �ȴ�������� */
			return 2;
		}
	}
	else if ( gpio_read( KEY_3 ) ==0 )
	{
		rt_thread_delay(20);             /* �������� */
		if ( gpio_read( KEY_3 ) ==0 )    /* ��ʾ��ȷ�������� */
		{
			while ( gpio_read( KEY_3 ) == 0 )rt_thread_delay(20);  /* �ȴ�������� */
			return 3;
		}
	}
	else if ( gpio_read( KEY_4 ) == 0 )
	{
		rt_thread_delay(20);                 /* �������� */
		if ( gpio_read( KEY_4 ) == 0 )  /* ��ʾ��ȷ�������� */
		{
			while ( gpio_read( KEY_4 ) == 0 )rt_thread_delay(20);      /* �ȴ�������� */
			return 4;
		}
	}
	return -1;
}

/*����IO��ʼ������ */
void KEY_Init( void )
{
	/* ���ð���IOΪ����ģʽ */
	gpio_enable( KEY_1, DIR_IN );
	gpio_enable( KEY_2, DIR_IN );
	gpio_enable( KEY_3, DIR_IN );
	gpio_enable( KEY_4, DIR_IN );
}







