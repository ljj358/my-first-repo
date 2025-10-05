/*
 * ATK_MV1268D.c
 *
 * created: 2024/10/16
 *  author: 
 */
#include <rtdevice.h>
#include <rtthread.h>
#include <rtdbg.h>
#include "string.h"
#include <stdio.h>
#include <ctype.h>
#include "bkrc_voice.h"
#include "Stepper_motor.h"
#include "ls1b_gpio.h"
#include "ls1b.h"
#include "ns16550.h"
rt_mq_t ATK_MV1268D_mqt = RT_NULL;
static rt_sem_t uaart_sem = RT_NULL;
void uart4_interrupt(int vector, void *param);
void uart_thread_entry(void *parameter);
static void ATK_MV1268D_Rx_Check(unsigned char data);
static void ATK_MV1268D_Tx_Check(unsigned char ATK_MV1268D_address,unsigned charFeature);
static char ATK_MV1268D_tx_data[11] = {0X00,0X00,0X05,0x55,0xBC,0x00,0x01,0x00,0x00,0x00,0xbb};
extern char Stepper_mode;
extern char drivers_id;
void ATK_MV1268D_init(void){
    ls1x_uart_init(devUART4, NULL); // 初始化串口
    ls1x_uart_open(devUART4, NULL); // 打开串口
    ls1x_disable_gpio_interrupt(58);
    ls1x_install_gpio_isr(58, INT_TRIG_EDGE_UP, uart4_interrupt, NULL); // 中断初始化
    ls1x_enable_gpio_interrupt(58);
}

void uart4_interrupt(int vector, void *param)
{
    rt_sem_release(uaart_sem);
}


void uart_thread_entry(void *parameter)
{
    unsigned int start_time = rt_tick_get();
    ATK_MV1268D_init();
    char buff[256];
    // 创建一个信号量，初始值为 0，类型为二值信号量
    uaart_sem = rt_sem_create("my_sem", 0, RT_IPC_FLAG_PRIO);
    ATK_MV1268D_mqt = rt_mq_create("ATK_MV1268D", 1, 200, RT_IPC_FLAG_FIFO);
       while(1){
            rt_err_t result = rt_sem_take(uaart_sem, 1000); // 成功获取信号量，执行相应的操作
            if (result == RT_EOK)
            {
                int i, d = ls1x_uart_read(devUART4, buff, 255, 0);

                for (i = 0; i < d; i++)
                    {
                        ATK_MV1268D_Rx_Check(buff[i]);

                       // rt_kprintf("%x\n",buff[i]);
                    }
            }
            if (rt_tick_get() - start_time > 500)
            {
                start_time = rt_tick_get();
                ATK_MV1268D_Tx_Check(drivers_id,Stepper_mode);
            }
       }
}



static void ATK_MV1268D_Rx_Check(unsigned char data)
{
    static unsigned char zigbeeData[8] = {0};
    int i = 0;
    if (rt_mq_send(ATK_MV1268D_mqt, &data, 1) != RT_EOK)
        rt_kprintf("WIFI数据写入FIFO失败");
    while (zigbeeData[0] != 0x55)
    {
        if (rt_mq_recv(ATK_MV1268D_mqt, &zigbeeData[0], 1, RT_WAITING_NO) != RT_EOK)
            return;
    }

    if (ATK_MV1268D_mqt->entry < 9)
        return;

    for (i = 1; i < 8; i++)
    {

        if (rt_mq_recv(ATK_MV1268D_mqt, &zigbeeData[i], 1, RT_WAITING_NO) != RT_EOK)
            return;
            rt_kprintf("%x",zigbeeData[i]);
    }
    rt_kprintf("\n");
    zigbeeData[0] = 0x00;
    if (zigbeeData[7] != 0xBB) // 验证数据格式是否合法
        return;

    if (((zigbeeData[2] + zigbeeData[3] + zigbeeData[4] + zigbeeData[5]) % 256) != zigbeeData[6]) // 校验
        return;

    if (zigbeeData[1] == 0xBC) // 语音播报返回
    {
        if (zigbeeData[2] == drivers_id)
        {
            if (zigbeeData[4] == 1&&Stepper_mode==0)
            {
                voice_broadcast(1);
                Stepper_motor_open();
                rt_kprintf("执行除尘\n");
            }
            else if (zigbeeData[5] == 1&&Stepper_mode==0)
            {
                Stepper_motor_reset();
                rt_kprintf("执行复位\n");
            }
        }
    }
}

static void ATK_MV1268D_Tx_Check(unsigned char ATK_MV1268D_address,unsigned charFeature){
            ATK_MV1268D_tx_data[5] = ATK_MV1268D_address;
            ATK_MV1268D_tx_data[7] = charFeature==1?1:0;
            ATK_MV1268D_tx_data[8] = charFeature==2?1:0;
            ATK_MV1268D_tx_data[9] = (ATK_MV1268D_tx_data[5]+ATK_MV1268D_tx_data[6]+ATK_MV1268D_tx_data[7]+ATK_MV1268D_tx_data[8])%256;
            ls1x_uart_write(devUART4, ATK_MV1268D_tx_data, sizeof(ATK_MV1268D_tx_data), 0);
}
