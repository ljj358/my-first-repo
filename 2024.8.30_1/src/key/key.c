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
		rt_thread_delay(20);                 /* （消抖） */
		if ( gpio_read( KEY_1 ) == 0 )  /* 表示的确被按下了 */
		{
			while ( gpio_read( KEY_1 ) == 0 )rt_thread_delay(20); /* 等待抖动完成 */
			return 1;
		}
	}
	else if ( gpio_read( KEY_2 ) == 0 )
	{
		rt_thread_delay(20);                 /* （消抖） */
		if ( gpio_read( KEY_2 ) == 0 )  /* 表示的确被按下了 */
		{
			while ( gpio_read( KEY_2 ) == 0 )rt_thread_delay(20);  /* 等待抖动完成 */
			return 2;
		}
	}
	else if ( gpio_read( KEY_3 ) ==0 )
	{
		rt_thread_delay(20);             /* （消抖） */
		if ( gpio_read( KEY_3 ) ==0 )    /* 表示的确被按下了 */
		{
			while ( gpio_read( KEY_3 ) == 0 )rt_thread_delay(20);  /* 等待抖动完成 */
			return 3;
		}
	}
	else if ( gpio_read( KEY_4 ) == 0 )
	{
		rt_thread_delay(20);                 /* （消抖） */
		if ( gpio_read( KEY_4 ) == 0 )  /* 表示的确被按下了 */
		{
			while ( gpio_read( KEY_4 ) == 0 )rt_thread_delay(20);      /* 等待抖动完成 */
			return 4;
		}
	}
	return -1;
}

/*按键IO初始化函数 */
void KEY_Init( void )
{
	/* 配置按键IO为输入模式 */
	gpio_enable( KEY_1, DIR_IN );
	gpio_enable( KEY_2, DIR_IN );
	gpio_enable( KEY_3, DIR_IN );
	gpio_enable( KEY_4, DIR_IN );
}







