#include <rtdevice.h>
#include <rtthread.h>
#include <rtdbg.h>
#include "string.h"
#include <stdio.h>
#include <ctype.h>
#include "at.h"
#include "ring_buffer.h"
#include "ns16550.h"
#include "ls1b_gpio.h"

// 定义信号量
struct rt_semaphore at_rx_sem;
static struct rt_semaphore at_resp_sem;
static struct rt_semaphore at_socket_recv_sem;

// 定义全局变量
static int at_resp_status = AT_RESP_OK;
static at_resp_t gs_resp = {0};
static ring_buffer at_socket_recv_buff = {{0}, 0, 0};
extern int at_STATUS_status;
ring_buffer uart4_buffer;

static rt_device_t usart2 = RT_NULL;

// 获取响应状态
static int at_get_resp_status(void)
{
    return at_resp_status;
}

// 设置响应状态
static void at_set_resp_status(int status)
{
    at_resp_status = status;
}

// 发送AT命令到UART
static void at_client_sendcmd(const char *cmd)
{
    ls1x_uart_write(devUART4, cmd, strlen(cmd), NULL);
    ls1x_uart_write(devUART4, "\r\n", 2, NULL);
    // UART4_Write(cmd, strlen(cmd));
    // UART4_Write("\r\n", 2);
}

int UART4_Read(char *ch)
{
    return ring_buffer_read((unsigned char *)ch, &uart4_buffer);
}

// 从UART获取一个字符
static int at_client_getchar(char *ch, unsigned int timeout)
{
    while (1)
    {
        if (UART4_Read(ch) == 0)
        {
            return 0; // 成功获取一个字符
        }
        else if (rt_sem_take(&at_rx_sem, timeout) != RT_EOK)
        {
            return -2; // 等待超时返回
        }
    }
}
static rt_sem_t uaart_sem = RT_NULL;
void uart_thread_entry(void *parameter)
{
    char buff[256];
    // 创建一个信号量，初始值为 0，类型为二值信号量
    uaart_sem = rt_sem_create("my_sem", 0, RT_IPC_FLAG_PRIO);
       while(1){
           rt_err_t result = rt_sem_take(uaart_sem, RT_WAITING_FOREVER); // 成功获取信号量，执行相应的操作
            if (result == RT_EOK)
            {
                    int i, d = ls1x_uart_read(devUART4, buff, 255, 0);
                    for (i = 0; i < d; i++)
                        {
                            ring_buffer_write(buff[i], &uart4_buffer);
                            // rt_kprintf("%x",buff[i]);
                            rt_sem_release(&at_rx_sem);
                        }
            }
       }
}
void uart4_interrupt(int vector, void *param)
{
    rt_sem_release(uaart_sem);
}

// 初始化信号量
void at_client_init(void)
{
    rt_sem_init(&at_resp_sem, "at_resp", 0, RT_IPC_FLAG_FIFO);
    rt_sem_init(&at_rx_sem, "at_rx", 0, RT_IPC_FLAG_FIFO);
    rt_sem_init(&at_socket_recv_sem, "at_sock_recv", 0, RT_IPC_FLAG_FIFO);

    ls1x_uart_init(devUART4, NULL); // 初始化串口
    ls1x_uart_open(devUART4, NULL); // 打开串口
    ls1x_disable_gpio_interrupt(58);
    ls1x_install_gpio_isr(58, INT_TRIG_EDGE_UP, uart4_interrupt, NULL); // 中断初始化
    ls1x_enable_gpio_interrupt(58);
}

// 发送数据到AT服务器
int at_client_senddata(const char *buf, unsigned int len, unsigned int timeout)
{

    ls1x_uart_write(devUART4, buf, len, NULL);
    ls1x_uart_write(devUART4, "\r\n", 2, NULL);
    if (rt_sem_take(&at_resp_sem, timeout) != RT_EOK)
    {
        return AT_RESP_TIMEOUT;
    }
    return at_get_resp_status();
}

// 发送命令并等待回应
int at_exce_cmd(const char *cmd, at_resp_t *resp, unsigned int timeout)
{
    at_client_sendcmd(cmd);
    if (rt_sem_take(&at_resp_sem, timeout) == RT_EOK)
    {
        if (resp != NULL)
        {
            memcpy(resp, &gs_resp, sizeof(at_resp_t));
        }
        return at_get_resp_status();
    }
    return AT_RESP_TIMEOUT;
}

// 等待AT客户端连接
int at_client_wait_connect(unsigned int timeout)
{
    unsigned int start_time = rt_tick_get();
    while (1)
    {
        if (rt_tick_get() - start_time > timeout)
        {
            rt_kprintf("wait AT client connect timeout(Tick %d).\r\n", timeout);
            return AT_RESP_TIMEOUT;
        }

        if (at_exce_cmd("AT", NULL, 300) == AT_RESP_OK)
            return AT_RESP_OK;
    }
}


// 从接收缓冲区获取一个字符
int at_client_recvchar(char *ch, unsigned int timeout)
{
    while (1)
    {
        if (ring_buffer_read((unsigned char *)ch, &at_socket_recv_buff) == 0)
        {
            return 0; // 成功获取一个字符
        }
        else if (rt_sem_take(&at_socket_recv_sem, timeout) != RT_EOK)
        {
            return -2; // 等待超时返回
        }
    }
}

// 处理URC发送的结果
static void urc_send_func(const char *data, unsigned int size)
{
    if (strstr(data, "SEND OK"))
    {
        at_set_resp_status(AT_RESP_OK);
    }
    else if (strstr(data, "SEND FAIL"))
    {
        at_set_resp_status(AT_RESP_ERROR);
    }
    rt_sem_release(&at_resp_sem);
}

// 处理URC接收的数据
static void urc_recv_func(const char *data, unsigned int size)
{
    int result = 0, recv_size = 0;
    unsigned int timeout;
    char ch = 0;

    sscanf(data, "+IPD,%d:", &recv_size);
    timeout = recv_size > 8 ? recv_size :10;
    //rt_kprintf("recv_size=%d\n",recv_size);
    int i;
    for (i = 0; i < recv_size; i++)
    {
        result = at_client_getchar(&ch, timeout);
        if (result)
        {
            rt_kprintf("AT client receive data fail, return status %d\r\n", result);
            return;
        }
        ring_buffer_write(ch, &at_socket_recv_buff);
        rt_sem_release(&at_socket_recv_sem);
    }
}

// 处理URC接收的数据
static void urc_CIPSTATUS_func(const char *data, unsigned int size)
{
        int  recv_size = 0;
               //int i=0;
              //for( i= 0;i<size;i++)
                //rt_kprintf("%c",data[i]);
              //rt_kprintf("\n");

        sscanf(data, "STATUS:%d", &recv_size);
        rt_kprintf("STATUS=%d\n",recv_size);
        if (recv_size==3)
            at_STATUS_status = AT_STATUS_OK;
        else
            at_STATUS_status = AT_RESP_ERROR;

}

//AT+CIPSTATUS
// URC表定义
static at_urc_t esp8266_urc_table[] = {
    {"SEND OK", "\r\n", urc_send_func},
    {"SEND FAIL", "\r\n", urc_send_func},
    {"+IPD", ":", urc_recv_func},
    {"STATUS:", "\r\n", urc_CIPSTATUS_func},
    //{"+CIPSTATUS:", "\r\n", urc_CIPSTATUS_func},
};
// 获取URC类型
static const at_urc_t *at_get_urc(const char *recv_line_buf, unsigned int recv_line_len)
{
    unsigned char prefix_len = 0, suffix_len = 0;
    int i;
    for (i = 0; i < sizeof(esp8266_urc_table) / sizeof(esp8266_urc_table[0]); i++)
    {
        prefix_len = strlen(esp8266_urc_table[i].cmd_prefix);
        suffix_len = strlen(esp8266_urc_table[i].cmd_suffix);
        if ((prefix_len ? !strncmp(recv_line_buf, esp8266_urc_table[i].cmd_prefix, prefix_len) : 1) &&
            (suffix_len ? !strncmp(recv_line_buf + recv_line_len - suffix_len, esp8266_urc_table[i].cmd_suffix, suffix_len) : 1))
        {
            return &esp8266_urc_table[i];
        }
    }
    return NULL;
}

// 读取一行数据
static int at_recv_readln(char *buff, unsigned int buff_len)
{
    char ch = 0, last_ch = 0;
    unsigned int read_len = 0;
    memset(buff, 0, buff_len);
    while (1)
    {
        at_client_getchar(&ch, RT_WAITING_FOREVER);
        if (read_len < buff_len)
        {
            buff[read_len++] = ch;
        }
        else
        {
            memset(buff, 0x00, buff_len);
            return -1; // 缓冲区溢出错误
        }
        if ((ch == '\n' && last_ch == '\r') || at_get_urc(buff, read_len))
        {
            break; // 读到一行数据或接收到URC数据
        }
        last_ch = ch;
    }
    return read_len;
}

// 解析接收的数据
void at_recv_parser(void *parameter)
{
    ring_buffer_init(&uart4_buffer); // 初始化环形缓冲区
    at_client_init();
    char recv_line_buff[128] = {0};
    int read_len = 0;
    at_resp_t tmp_resp = {{0}, 0, 0};
    const at_urc_t *urc = NULL;

    while (1)
    {
        read_len = at_recv_readln(recv_line_buff, sizeof(recv_line_buff));
        if (read_len > 0)
        {
            if ((urc = at_get_urc(recv_line_buff, read_len)) != NULL)
            {
                if (urc->func != NULL)
                {
                    urc->func(recv_line_buff, read_len);
                }
            }
            else
            {
                if (tmp_resp.buf_len < AT_MAX_RESP_LEN)
                {
                    recv_line_buff[++read_len] = '\0';
                    memcpy(tmp_resp.buf + tmp_resp.buf_len, recv_line_buff, read_len);
                    tmp_resp.buf_len += read_len;
                    tmp_resp.line_counts++;
                }
                else
                {
                    at_set_resp_status(AT_RESP_BUFF_FULL);
                }

                if (strstr(recv_line_buff, "OK"))
                {
                    memset(&gs_resp, 0, sizeof(gs_resp));
                    memcpy(&gs_resp, &tmp_resp, sizeof(gs_resp));
                    at_set_resp_status(AT_RESP_OK);
                }
                else if (strstr(recv_line_buff, "ERROR"))
                {
                    memset(&gs_resp, 0, sizeof(gs_resp));
                    memcpy(&gs_resp, &tmp_resp, sizeof(gs_resp));
                    at_set_resp_status(AT_RESP_ERROR);
                }
                memset(&tmp_resp, 0, sizeof(tmp_resp));
                rt_sem_release(&at_resp_sem);
            }
        }
    }
}
