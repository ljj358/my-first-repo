/*
 * bkrc_voice.c
 *
 * created: 2022/2/28
 *  author:
 */
#include "bkrc_voice.h"
#include "ls1b.h"
#include "ls1b_gpio.h"
#include "ns16550.h"
#include "stdio.h"
#include "string.h"

char buff_uart3[256];
//0-9,万，千，百，十
static short  stringdata[]={0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0XF2CD,0XA7C7,0XD9B0,0XAECA};
static char rouse[] = {0xFA,0XFA,0XFA,0XFA,0XA1};
void UART3_Config_Init(void)
{
    /**
     * @brief 串口初始化(次要)
     * @param 1 void
     * @return void
     */
    unsigned int BaudRate = 115200;
    ls1x_uart_init(devUART3,(void *)BaudRate); //初始化串口
    ls1x_uart_open(devUART3,NULL); //打开串口
}


void UART3_Test(void)
{
     ls1x_uart_read(devUART3,buff_uart3,256,NULL);
     delay_ms(10);
}

void voice_rouse(void){    //唤醒语音模块
     ls1x_uart_write(devUART3,&rouse,5,NULL);
}

void voice_broadcast(unsigned char instruct){
     ls1x_uart_write(devUART3,&instruct,1,NULL);
}

void voice_figure( int instruct){
    char data[5] = {0},i=0,j = 0;
    while (instruct!=0){
        data[i]=instruct%10;
        instruct/=10;
        i++;
    }
    for(j = i ;j>0;j--){
        if (data[j-1] == 0 && j == 1) break;
        
        ls1x_uart_write(devUART3,&stringdata[data[j-1]],1,NULL);
        
        rt_thread_delay(700);
        
        if(data[j-1] == 0) continue;
        switch(j){
            case 5: ls1x_uart_write(devUART3,&stringdata[10],2,NULL); break;  //万
            case 4: ls1x_uart_write(devUART3,&stringdata[11],2,NULL); break;  //千
            case 3: ls1x_uart_write(devUART3,&stringdata[12],2,NULL); break;  //百
            case 2: ls1x_uart_write(devUART3,&stringdata[13],2,NULL); break;  //十
        }
        
        rt_thread_delay(700);
    }
}

/**************************************************
Voice_Drive
函数说明：执行对应词条的任务（语音识别）
输入参数：	无
返 回 值：	语音词条ID    词条内容
***************************************************/

unsigned char Voice_Drive(void)
{
    char ID_flag = 0;
    UART3_Test();
    if (buff_uart3[0] == 0x55)			// 自定义数据帧接收完毕
    {

        if (buff_uart3[1] == 0x02)
        {
            printf("%d\n",buff_uart3[2]);
            switch (buff_uart3[2])
            {
            case 0x01:

                ID_flag = 1;
                break;

            case 0x02:
                ID_flag = 2;
                break;

            case 0x03:
                ID_flag = 3 ;
                break;

            case 0x04:
                ID_flag = 4;
                break;

            case 0x05:
                ID_flag = 5;
                break;

            case 0x06:
                ID_flag = 6;
                break;
            case 0x07:
                ID_flag = 7;
                break;

            default  :
                ID_flag = 0;
                break;
            }
        }

    }
    return ID_flag;
}

