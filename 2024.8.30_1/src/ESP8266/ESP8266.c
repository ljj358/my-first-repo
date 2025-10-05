/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-10-05     lzh28       the first version
 */
#include <rtdevice.h>
#include <rtthread.h>
#include <rtdbg.h>
#include "string.h"
#include <stdio.h>
#include <ctype.h>
#include "ESP8266.h"
#include "at.h"
#include "bkrc_voice.h"
#include "Stepper_motor.h"
#include "ls1b_gpio.h"
int at_STATUS_status = AT_RESP_OK;
rt_mq_t zibee_mqt = RT_NULL;

#define AT_SEND_CMD(cmd, resp, timeout)                \
    do                                                 \
    {                                                  \
        if (at_exce_cmd((cmd), (resp), (timeout)) < 0) \
        {                                              \
            result = -1;                               \
            goto __exit;                               \
        }                                              \
    } while (0)

int esp8266_init(void)
{
    at_resp_t resp = {0};
    gpio_enable(40, DIR_OUT); // 高压模块
    gpio_write(40, 0); // 1正常工作 0复位
    rt_thread_mdelay(100);
    gpio_write(40, 1); // 1正常工作 0复位
    rt_thread_mdelay(1000);
    int result = 0;
    unsigned int retry_num = 5;

    /* 检查是否和AT服务器（模块）建立连接 */
    if (at_client_wait_connect(3000))
    {
        return -1;
    }

    /* 设置esp8266模块基本参数 */
    while (retry_num--)
    {

        /* disable echo */
        AT_SEND_CMD("ATE0", &resp, 1000); // 关闭回显
        /* set current mode to Wi-Fi station */
        AT_SEND_CMD("AT+CWMODE=1", &resp, 1000); // Station模式（客户端）

        /* reset module */
        AT_SEND_CMD("AT+RST", &resp, 1000); // 复位
        /* reset waiting delay */
        rt_thread_mdelay(1000);

        /* disable echo */
        AT_SEND_CMD("ATE0", &resp, 1000); // 关闭回显

        AT_SEND_CMD("AT+CIPMUX=0", &resp, 1000); // 单路连接,AT+CIPSEND=Length
        rt_thread_mdelay(1000);

        
        AT_SEND_CMD("AT+CWJAP=\"ZC_LZH\",\"405405405\"", &resp, 1000); // 连接路由器AP
        rt_thread_mdelay(15000);

        AT_SEND_CMD("AT+CIPSTART=\"TCP\",\"192.168.16.254\",60000", &resp, 1000); // 连接路由器AP
        rt_thread_mdelay(1000);

        break;

    __exit:
        if (result != 0)
        {
            rt_thread_mdelay(1000);
            printf("esp8266 device initialize retry...\r\n");
        }
    }
    rt_kprintf("ESP8266 init ok\n");
    return result;
}

int esp8266_txd(const char *buf, unsigned int len)
{
    char at_cmd_buf[20] = {0};
    sprintf(at_cmd_buf, "AT+CIPSEND=%d", len);
    if (at_exce_cmd(at_cmd_buf, NULL, 2000) == AT_RESP_OK)
    {
        rt_thread_mdelay(10);
        if (at_client_senddata(buf, len, 1000) == AT_RESP_OK)
            return 0; // 成功
    }
    return 1; // 失败
}
extern char Stepper_mode;
static void WIFIRx_Check(unsigned char data);
static char wifi_data[] = {0x55, 0xBC, 0x01, 0x01, 0x00, 0x00, 0x21, 0xbb};
void esp8266_client_test(void *parameter)
{

    // Stepper_motor_Init(1500, 2048);     //频率 1.5k

    at_resp_t resp = {0};
    unsigned int start_time = rt_tick_get();
    zibee_mqt = rt_mq_create("zibee_mqt", 1, 200, RT_IPC_FLAG_FIFO);
    esp8266_init();
    int wifiinit = 0;
    while (1)
    {       
            char ch = 0;
            int i = at_client_recvchar(&ch, 10);
            if (i == 0)
            {
                WIFIRx_Check(ch);
            }

            if (rt_tick_get() - start_time > 1000)
            {
                start_time = rt_tick_get();
                at_STATUS_status = AT_RESP_ERROR;
                at_exce_cmd("AT+CIPSTATUS", NULL, 2000);
                rt_thread_mdelay(100);
                if(at_STATUS_status!=AT_STATUS_OK){
                    if(++wifiinit>=3){
                        wifiinit=0;
                        esp8266_init();
                        rt_kprintf("重新连接服务器\n");
                        rt_thread_mdelay(1000);
                    }
                }
                else
                    rt_kprintf("不需要重新连接服务器\n");
                //data[4] = Stepper_mode == 1 ? 1 : 0;
                //data[5] = Stepper_mode == 2 ? 1 : 0;
                //data[6] = (data[2] + data[3] + data[4] + data[5]) % 256;
                //esp8266_txd(data, sizeof(data));
            }
    }
}
extern char Stepper_mode;
extern char drivers_id;
static void WIFIRx_Check(unsigned char data)
{
    static unsigned char zigbeeData[8] = {0};
    int i = 0;
    if (rt_mq_send(zibee_mqt, &data, 1) != RT_EOK)
        rt_kprintf("WIFI数据写入FIFO失败");
    while (zigbeeData[0] != 0x55)
    {
        if (rt_mq_recv(zibee_mqt, &zigbeeData[0], 1, RT_WAITING_NO) != RT_EOK)
            return;
    }

    if (zibee_mqt->entry < 7)
        return;

    for (i = 1; i < 8; i++)
    {
        
        if (rt_mq_recv(zibee_mqt, &zigbeeData[i], 1, RT_WAITING_NO) != RT_EOK)
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
            if (zigbeeData[4] == 1)
            {
                voice_broadcast(1);
                Stepper_motor_open();
            }
            else if (zigbeeData[5] == 1)
            {

                Stepper_motor_reset();
            }
             else if (zigbeeData[3] == 1)
            {
                wifi_data[2] = drivers_id;
                wifi_data[4] = Stepper_mode == 1 ? 1 : 0;
                wifi_data[5] = Stepper_mode == 2 ? 1 : 0;
                wifi_data[6] = (wifi_data[2] + wifi_data[3] + wifi_data[4] + wifi_data[5]) % 256;
                esp8266_txd(wifi_data, sizeof(wifi_data));
            }
        }
    }
}
