#include "drv_console.h"

#include <rtthread.h>
#include <rtdevice.h>
#include "drv_common.h"
#include "vconsole.h"
#include "drv_dio.h"
#include "drv_ain.h"
#include "lwgps2rtt.h"

#include "netdb.h"
#include "netdev.h"

#include "ipc/ringbuffer.h"
#include "sys/errno.h"
#include "sys/socket.h"
#include "sys/select.h"

#include "drv/drv_param.h"
#include "luat_malloc.h"
#include "app_bringup_diag.h"

#include "flashdb.h"
#ifdef RT_USING_FINSH
#include <finsh.h>
#endif

#define WT_SVR_TG      1
#define WT_SVR_OCEIOT  2

#define WT_ID_DID      1
#define WT_ID_IMEI     2

extern char MyMcuID[30];

extern char acWebTemModemImei[20];


//static char didSetFlag = 0;
volatile static char wtIdType = WT_ID_DID;
//static char wtServer = WT_SVR_TG;

rt_device_t devVc0 = RT_NULL;
rt_device_t devVcom = RT_NULL;
rt_device_t devUart2 = RT_NULL;
rt_device_t devPppUart = RT_NULL;

static struct rt_ringbuffer *_SendBuff = RT_NULL;
static uint32_t _gulLastRecvTime = 0;
volatile static uint8_t _gucWtEnFlag = 0;
volatile static int wt_error = 0;

extern rt_device_t luat_log_uart_device;

extern struct fdb_kvdb env_kvdb;

static struct rt_semaphore rx_sem;

static rt_uint8_t *pucVcomBuf = RT_NULL;
static volatile rt_uint8_t g_ppp_msh_bridge_en = APP_BRINGUP_MIN_MESH_MODE ? 0 : 1;
volatile static int wt_sock = -1;
static uint16_t _gusBuffTime = 50;

static int _giSendThreadInitOK = 0;

uint16_t setConsoleBuffTime(uint16_t usTime)
{
    uint16_t usTmp = _gusBuffTime;
    if (usTime > 50) {
        usTime = 50;
    }

    _gusBuffTime = usTime;

    return usTmp;
}

/**
 * @brief sendHeartBeat
 * @param buf 此参数仅用于传递一个数据buf，不传实际数据
 */
void sendHeartBeat(char *buf)
{
    extern int _giModemRssi;
    static lwgps_t gps_info;
    /*
        {
            "battery_voltage": "%.1f",
            "board_temperature": "%.1f",
            "net_rssi": "%d",
            "lon": "%f",
            "lat": "%f",
            "mem_sys": "%d",
            "mem_lua": "%d",
            "run_time": "%u"
        }
     */


    lwgps2rtt_get_gps_info(&gps_info);

    rt_size_t aulSysMemBuf[3];
    rt_size_t aulLuaMemBuf[3];
    rt_memory_info(&aulSysMemBuf[0], &aulSysMemBuf[1], &aulSysMemBuf[2]);
    luat_meminfo_luavm(&aulLuaMemBuf[0], &aulLuaMemBuf[1], &aulLuaMemBuf[2]);

    rt_sprintf(buf, "#INF#{\"battery_voltage\":\"%.1f\",\"board_temperature\":\"%.1f\",\"net_rssi\":\"%d\",\"location\":{\"lon\":\"%f\",\"lat\":\"%f\"},\"mem_sys\":\"%d\",\"mem_lua\":\"%d\",\"run_time\":\"%u\"}",
               ainGetPowerVoltage(),
               ainGetBoardTemp(),
               _giModemRssi,
               gps_info.longitude,
               gps_info.latitude,
               aulSysMemBuf[1] * 100 / aulSysMemBuf[0],
               aulLuaMemBuf[1] * 100 / aulLuaMemBuf[0],
               rt_tick_get_millisecond() / 1000
               );

//    rt_kprintf("send hb:");
//    rt_kprintf(buf);
    send(wt_sock, buf, rt_strlen(buf), 0);
}

int wtIsAlive(void)
{
    return rt_tick_get_millisecond() < (_gulLastRecvTime + 2 * 60 * 1000);
}

void tcpclient(const char *url, int port)
{
    char *data_buf;
    struct hostent *host;
    int bytes_received, result;
    struct sockaddr_in server_addr;

    const int BUFSZ = 2048;
    fd_set inputs, testfds;
    struct timeval timeout;

    FD_ZERO(&inputs);

    rt_kprintf("[tem]try to connect to web treminal server %s:%d\n", url, port);

    /* 通过函数入口参数 url 获得 host 地址（如果是域名，会做域名解析） */
    host = gethostbyname(url);

    /* 分配用于存放接收数据的缓冲 */
    data_buf = rt_malloc(BUFSZ);
    if (data_buf == RT_NULL)
    {
        rt_kprintf("[tem]No memory\n");
        return;
    }

    /* 创建一个 socket，类型是 SOCKET_STREAM，TCP 类型 */
    if ((wt_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        /* 创建 socket 失败 */
        rt_kprintf("[tem]Socket error\n");

        /* 释放接收缓冲 */
        rt_free(data_buf);
        return;
    }

    /* 初始化预连接的服务端地址 */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    /* 连接到服务端 */
    if (connect(wt_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        /* 连接失败 */
        rt_kprintf("[tem]Connecting to wt server failied!\n");
        closesocket(wt_sock);

        /* 释放接收缓冲 */
        rt_free(data_buf);

        rt_thread_delay(5000);

        return;
    }

    /* 设置接收超时 */
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    if (setsockopt(wt_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        rt_kprintf("[tem]Set socket receive timeout failed!\n");
        closesocket(wt_sock);

        /* 释放接收缓冲 */
        rt_free(data_buf);

        rt_thread_delay(5000);

        return;
    }

    wt_error = 0;

    FD_SET(wt_sock, &inputs);

    rt_memset(data_buf, 0, BUFSZ);
    if (wtIdType == WT_ID_IMEI) {
		
//		if(acWebTemModemImei[0] != 0)
//		{
//			rt_sprintf(data_buf, "#2DID#%s#", acWebTemModemImei);
//		}
//		else
//		{
//			rt_sprintf(data_buf, "#2DID#%s#", acWebTemModemImei);
//		}
		
		rt_sprintf(data_buf, "#2DID#%s#", MyMcuID);
		rt_kprintf("[tem]imei : %s \r\n",data_buf);
    } else {
        rt_sprintf(data_buf, "#2DID#%s#", gptSysParam->acDevUid);
		rt_kprintf("[tem]did : %s \r\n",data_buf);
    }
    send(wt_sock, data_buf, rt_strlen(data_buf), 0);

    memset(data_buf, 0, BUFSZ);
    bytes_received = recv(wt_sock, data_buf, BUFSZ - 1, 0);
    if (bytes_received > 0) {
        rt_kprintf("[tem]recv:%s\n", data_buf);

        _gulLastRecvTime = rt_tick_get_millisecond();

        rt_thread_delay(100);

        rt_kprintf("[tem]web console loop start\n");
    } else {
        rt_kprintf("[tem]recv error:%d\n", bytes_received);
        wt_error = 1;
    }

//    sendHeartBeat(data_buf);

    while (1) 
    {
        if (wt_sock < 0) {
            break;
        }

        testfds = inputs;
        timeout.tv_sec = 0;
        timeout.tv_usec = 50000;
        result = select(FD_SETSIZE, &testfds, NULL, NULL, &timeout);

        if (result > 0) {
            if (FD_ISSET(wt_sock, &testfds)) {
                bytes_received = recv(wt_sock, data_buf, BUFSZ - 1, 0);

                if (bytes_received < 0) {
                    /* 接收失败，关闭这个连接 */
                   rt_kprintf("[tem]recv error:%d\r\n", bytes_received);
                    wt_error = 1;
                } else if (bytes_received == 0) {
                    /* 打印 recv 函数返回值为 0 的警告信息 */
                   rt_kprintf("\r\n[tem]Received warning,recv function return 0.\r\n");

                    wt_error = 1;
                } else {
                    vconsole_input(devVc0, data_buf, bytes_received);

                    _gulLastRecvTime = rt_tick_get_millisecond();
                    _gucWtEnFlag = 1;
                }
            }
        }

        if (wt_error != 0) {
            closesocket(wt_sock);
            FD_CLR(wt_sock, &inputs);
            wt_sock = -1;
			_gucWtEnFlag = 0;
			rt_kprintf("[tem]wt_error != 0 \r\n");
			
			rt_thread_mdelay(1 * 60 * 1000);
			
			break;
        }

        if ((_gucWtEnFlag == 1) && (wt_sock >= 0)) {
            if (rt_tick_get_millisecond() > _gulLastRecvTime + 1 * 60 * 1000) { ///< 超时
                _gucWtEnFlag = 0;
                rt_kprintf("\r\n[tem]received timeout, stop sending\r\n");
            }
        }

        if (wt_sock >= 0) {
            if (rt_tick_get_millisecond() > _gulLastRecvTime + 5 * 60 * 1000) { ///< 超时
                closesocket(wt_sock);
                FD_CLR(wt_sock, &inputs);
                wt_sock = -1;
                _gucWtEnFlag = 0;
                rt_kprintf("\r\n[tem]received timeout,close the socket.\r\n");
                break;
            }
        }
		
		
		rt_thread_mdelay(1);
    }

//    rt_kprintf("\nreceived error,close the socket.\r\n");

    rt_free(data_buf);
}

static void _web_terminal(void *parameter)
{
    char *pcVal;
    struct netdev *ndev;

#if 0
    while (1) {
        rt_thread_delay(100);
        ndev = netdev_default;
        if (ndev != NULL) {
            if (netdev_is_internet_up(ndev)) {
                break;
            }
        }
    }
#else
    rt_thread_delay(10000);
#endif

//    pcVal = fdb_kv_get(&env_kvdb, "wt_server");
//    rt_kprintf("[tem]wt_server:%s\r\n", pcVal);
//    if (rt_strncmp(pcVal, "oceiot", 6) == 0) {
//        wtServer = WT_SVR_OCEIOT;
//    } else {
//        wtServer = WT_SVR_TG;
//    }

//    pcVal = fdb_kv_get(&env_kvdb, "wt_id");
//    rt_kprintf("[tem]wt_id:%s\r\n", pcVal);
//    if (rt_strncmp(pcVal, "imei", 4) == 0) {
//        wtIdType = WT_ID_IMEI;
//    } else {
//        wtIdType = WT_ID_DID;
//    }
//    if (didSetFlag != 1) {
//        wtIdType = WT_ID_IMEI;
//    }

    while (1) {
//        while (1) {
//            rt_thread_delay(100);
//            ndev = netdev_default;
//            if (ndev != NULL) {
//                if (netdev_is_internet_up(ndev)) {
//                    break;
//                }
//            }
//        }
		
		if ((rt_strlen(gptSysParam->acDevUid) == 11) &&
            (rt_memcmp(gptSysParam->acDevUid, "10230100001", rt_strlen("10230100001")) != 0) &&
			(gptSysParam->acDevUid[0] == '1') &&
			(gptSysParam->acDevUid[1] == '0')) 
		{
			wtIdType = WT_ID_DID;
			rt_kprintf("[tem]choose WT_ID_DID \r\n");
        } 
		else 
		{
			wtIdType = WT_ID_IMEI;
			rt_kprintf("[tem]choose WT_ID_IMEI \r\n");
        }
		
		

		tcpclient("8.140.244.249", 18001);

//        if (wtServer == WT_SVR_TG) {
//            tcpclient("123.57.81.127", 17005);
//        } else if (wtServer == WT_SVR_OCEIOT) {
//            tcpclient("123.57.73.106", 17005);
//        }

        rt_thread_delay(1000);
    }
}

void send_to_wt(uint8_t *pucData, int iLen)
{
    int ret;

    if (_gucWtEnFlag == 0) {
        return;
    }
    if (wt_sock < 0) {
        return;
    }
    if (wt_error != 0) {
        return;
    }
    if (iLen <= 0) {
        return;
    }
    ret = send(wt_sock, pucData, iLen, 0);
    if (ret < 0)
    {
        wt_error = ret;
    }
}

/* 接收数据回调函数 */
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(&rx_sem);

    return RT_EOK;
}

#if defined(RT_USING_FINSH) && defined(PPP_CLIENT_NAME)
static int ppp_msh_bridge(int argc, char **argv)
{
    if (argc != 2)
    {
        rt_kprintf("Usage: ppp_msh_bridge <0|1>\r\n");
        rt_kprintf("  0: disable PPP uart -> msh input bridge\r\n");
        rt_kprintf("  1: enable  PPP uart -> msh input bridge\r\n");
        return -RT_ERROR;
    }

    if (argv[1][0] == '1')
    {
        g_ppp_msh_bridge_en = 1;
        rt_kprintf("ppp_msh_bridge: enabled, device=%s\r\n", PPP_CLIENT_NAME);
    }
    else
    {
        g_ppp_msh_bridge_en = 0;
        rt_kprintf("ppp_msh_bridge: disabled\r\n");
    }

    return RT_EOK;
}
MSH_CMD_EXPORT(ppp_msh_bridge, enable/disable PPP uart feed to msh);
#endif

static void sendLoop(void)
{
    static uint32_t ulLastTick = 0;

    static char aucBuf[1024];

    if (_SendBuff == RT_NULL) {
        return;
    }

    uint16_t usLen = rt_ringbuffer_data_len(_SendBuff);

    if (usLen <= 0) {
        return;
    }

    if ((usLen > 512) || (rt_tick_get() >= ulLastTick + _gusBuffTime)) {
        if (usLen > sizeof(aucBuf)) {
            usLen = sizeof(aucBuf);
        }
        rt_ringbuffer_get(_SendBuff, aucBuf, usLen);
        if (devVcom != RT_NULL) {
            rt_device_write(devVcom, 0, aucBuf, usLen);
        }

        if (devUart2 != RT_NULL) {
            if (gptSysParam->ucRs232_1_LogEnable) {
                rt_device_write(devUart2, 0, aucBuf, usLen);
            }
        }

        send_to_wt(aucBuf, usLen);

        ulLastTick = rt_tick_get();
        _giSendThreadInitOK = 1;
    }
}

/* callback function */
static rt_size_t outfunction(rt_device_t device, rt_uint8_t *buff, rt_size_t size)
{
    if (_SendBuff != RT_NULL) {
        if (_giSendThreadInitOK) {
            while (size + rt_ringbuffer_data_len(_SendBuff) >= rt_ringbuffer_get_size(_SendBuff)) {
                rt_thread_delay(20);
            }
        }
        rt_ringbuffer_put(_SendBuff, buff, size);
    }

    return size;
}

//wget http://lamp.lidatongchina.com/upload/files/firmwares/firmware_V1.46.bin /sdcard/download/1.bin

rt_device_t vConsoleInit(void)
{
    _SendBuff = rt_ringbuffer_create(2048);

    RT_ASSERT(_SendBuff != RT_NULL);
    
    param_init();

    /* create a console */
    devVc0 = vconsole_create("vc0", outfunction);
    /* switch the console and return the old device */

    vconsole_switch(devVc0);

    rt_kprintf("[tem]ucRs232_1_LogEnable:%d\r\n", gptSysParam->ucRs232_1_LogEnable);
    
    if (gptSysParam->ucRs232_1_LogEnable) {
        devUart2 = rt_device_find("uart2");
        if (devUart2 != RT_NULL) {
            if (rt_device_open(devUart2, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM)) {
                rt_kprintf("[tem]uart2 open failed!\r\n");
            } else {
                rt_kprintf("[tem]uart2 open ok!\r\n");
            }
        } else {
            rt_kprintf("[tem]uart2 not found!\r\n");
        }
    }

    return devVc0;
}

static void _thread_entry(void *parameter)
{
    int vcomFlag = 0, pppUartFlag = 0, len;
    
    while (1) {
        if (vcomFlag == 0) {
            if (rt_device_find("vcom") != RT_NULL) {
                devVcom = rt_device_find("vcom");
                rt_device_open(devVcom, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_INT_TX);
                rt_device_set_rx_indicate(devVcom, uart_input);

                vcomFlag = 1;
            }
        }

        if (!APP_BRINGUP_MIN_MESH_MODE && !APP_ENABLE_UART10_LIGHT_CMD && pppUartFlag == 0) {
#ifdef PPP_CLIENT_NAME
            devPppUart = rt_device_find(PPP_CLIENT_NAME);
            if (devPppUart != RT_NULL) {
                if (rt_device_open(devPppUart, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM | RT_DEVICE_FLAG_INT_RX) == RT_EOK) {
                    rt_device_set_rx_indicate(devPppUart, uart_input);
                    pppUartFlag = 1;
                    rt_kprintf("[tem]%s open ok for optional msh bridge\r\n", PPP_CLIENT_NAME);
                } else {
                    devPppUart = RT_NULL;
                }
            }
#endif
        }

        if (devVcom != RT_NULL) {
            if (rt_sem_take(&rx_sem, 10) == RT_EOK)
            {
                if ((len = rt_device_read(devVcom, 0, (void *)pucVcomBuf, 2048)) > 0) {
                    vconsole_input(devVc0, pucVcomBuf, len);
                }
            }
        } else {
            rt_thread_delay(10);
        }

        if ((g_ppp_msh_bridge_en != 0) && (devPppUart != RT_NULL)) {
            if ((len = rt_device_read(devPppUart, 0, (void *)pucVcomBuf, 2048)) > 0) {
                vconsole_input(devVc0, pucVcomBuf, len);
            }
        }

        sendLoop();

        /*
         * 判断是否设置DID
         */
//        if ((rt_strlen(gptSysParam->acDevUid) == 11) &&
//                (gptSysParam->acDevUid[0] == '1') &&
//                (gptSysParam->acDevUid[1] == '0')) {
//            didSetFlag = 1;
//        } else {
//            didSetFlag = 0;
//        }
		
		rt_thread_mdelay(1);
    }
}

static int __drv_console_init(void)
{
    rt_thread_t _thread;
    pucVcomBuf = (rt_uint8_t *)rt_malloc(2048);

    RT_ASSERT(pucVcomBuf != RT_NULL);

    luat_log_uart_device = devVc0;

    rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);
    

    _thread = rt_thread_create("vcnsl", _thread_entry, RT_NULL,
                                 4096 + 2048, 26, 20);
    RT_ASSERT(_thread);
    rt_thread_startup(_thread);

    if (!APP_SCRIPT_UPDATE_MODE && !APP_BRINGUP_MIN_MESH_MODE) {
        _thread = rt_thread_create("web_terminal", _web_terminal, RT_NULL,
                                     4096, 26, 20);
        RT_ASSERT(_thread);
        rt_thread_startup(_thread);
    } else if (APP_BRINGUP_MIN_MESH_MODE) {
        rt_kprintf("[tem]minimal mesh mode: skip web terminal and PPP bridge\r\n");
    }

    return 0;
}
INIT_ENV_EXPORT(__drv_console_init);



