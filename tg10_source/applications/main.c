 /*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-09-02     RT-Thread    first version
 */


#include <rtthread.h>
#include <rtdevice.h>
#include <dfs_fs.h>
#include <dfs_file.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include "netdb.h"
#include "sys/socket.h"
#include "drv_common.h"
#include "vconsole.h"
#include "dfs.h"
#include "stdio.h"

#include "drv_dio.h"
#include "drv_ain.h"
#include "flashdb.h"
#include "lwgps2rtt.h"
#include "drv_param.h"
#include "drivers/watchdog.h"
#include "stm32h7xx_hal.h"

#include <netdev_ipaddr.h>
#include "netdev.h"

#include "fal.h"
#include "fal_cfg.h"
#include "app_bringup_diag.h"
#include "app_runtime_debug.h"
#include "drv/drv_tm1650.h"
#include "mesh_lua.h"

#if APP_BRINGUP_ENABLE_WATCHDOG
#define WDG_ENABLE   1
#else
#define WDG_ENABLE   0
#endif

#define BEEP_PIN        GET_PIN(A, 4)
#define USB_CTRL_PIN    GET_PIN(C, 9)
#define KEY_RESET_PIN   GET_PIN(E, 10)

int rtt_luatos_init(void);
static void idle_hook(void);
static void app_stage_mark(rt_uint32_t stage);
static void app_early_stage_mark(void);

struct fdb_kvdb env_kvdb = { 0 };
volatile rt_uint32_t g_app_stage = 0;
volatile rt_uint32_t g_sd_status = 0;       /* 0: unchecked, 1: sd0 found, 2: timeout */
volatile rt_int32_t g_luatos_init_ret = 0;
volatile rt_uint32_t g_luatos_stage = 0;
volatile rt_uint32_t g_sdcard_lua_stage = 0; /* 0: not checked, 1: missing, 2: scheduled */
volatile rt_uint32_t g_mesh_dbg_stage = 0;
volatile rt_uint32_t g_mesh_dbg_arg0 = 0;
volatile rt_uint32_t g_mesh_dbg_arg1 = 0;
volatile rt_uint32_t g_mesh_dbg_cmd_count = 0;
volatile rt_uint32_t g_mesh_dbg_send_ok_count = 0;
volatile rt_uint32_t g_mesh_dbg_radio_send_count = 0;
volatile rt_uint32_t g_mesh_dbg_radio_tx_count = 0;
volatile rt_uint32_t g_mesh_dbg_tx_done_count = 0;
volatile rt_uint32_t g_mesh_dbg_rx_done_count = 0;
volatile rt_uint32_t g_mesh_dbg_radio_loop = 0;
volatile rt_uint32_t g_mesh_dbg_main_loop = 0;
volatile rt_uint32_t g_mesh_dbg_ready = 0;
volatile rt_uint32_t g_mesh_dbg_fault_magic = 0;
volatile rt_uint32_t g_mesh_dbg_fault_cfsr = 0;
volatile rt_uint32_t g_mesh_dbg_fault_hfsr = 0;
volatile rt_uint32_t g_mesh_dbg_fault_bfar = 0;
volatile rt_uint32_t g_mesh_dbg_fault_mmar = 0;
volatile rt_uint32_t g_mesh_dbg_fault_pc = 0;
volatile rt_uint32_t g_mesh_dbg_fault_lr = 0;
volatile rt_uint32_t g_mesh_dbg_fault_thread = 0;
volatile rt_uint32_t g_app_assert_magic = 0;
volatile rt_uint32_t g_app_assert_line = 0;
volatile rt_uint32_t g_app_assert_ex = 0;
volatile rt_uint32_t g_app_assert_func = 0;
volatile rt_uint32_t g_app_reset_rsr = 0;

static rt_device_t wdg_dev;         /* 看门狗设备句柄 */

DMA_HandleTypeDef hdma_usart1_rx;

#ifdef Error_Handler
#undef Error_Handler
#endif
void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}
#define Error_Handler() _Error_Handler(__FILE__, __LINE__)

static void app_stage_mark(rt_uint32_t stage)
{
    g_app_stage = stage;
}

static void app_capture_reset_reason(void)
{
    g_app_reset_rsr = RCC->RSR;
    SET_BIT(RCC->RSR, RCC_RSR_RMVF);
}

static void app_reset_reason_append(char *buf, rt_size_t size, const char *name)
{
    rt_size_t len = rt_strlen(buf);

    if (len + rt_strlen(name) + 2U < size) {
        if (len > 0U) {
            rt_snprintf(buf + len, size - len, "|");
            len = rt_strlen(buf);
        }
        rt_snprintf(buf + len, size - len, "%s", name);
    }
}

static void app_reset_reason_format(char *buf, rt_size_t size, rt_uint32_t rsr)
{
    if (buf == RT_NULL || size == 0U) {
        return;
    }

    buf[0] = '\0';

#ifdef RCC_RSR_LPWRRSTF
    if (rsr & RCC_RSR_LPWRRSTF) app_reset_reason_append(buf, size, "LPWR");
#endif
#ifdef RCC_RSR_LPWR1RSTF
    if (rsr & RCC_RSR_LPWR1RSTF) app_reset_reason_append(buf, size, "LPWR1");
#endif
#ifdef RCC_RSR_LPWR2RSTF
    if (rsr & RCC_RSR_LPWR2RSTF) app_reset_reason_append(buf, size, "LPWR2");
#endif
#ifdef RCC_RSR_WWDG1RSTF
    if (rsr & RCC_RSR_WWDG1RSTF) app_reset_reason_append(buf, size, "WWDG1");
#endif
#ifdef RCC_RSR_WWDG2RSTF
    if (rsr & RCC_RSR_WWDG2RSTF) app_reset_reason_append(buf, size, "WWDG2");
#endif
#ifdef RCC_RSR_IWDG1RSTF
    if (rsr & RCC_RSR_IWDG1RSTF) app_reset_reason_append(buf, size, "IWDG1");
#endif
#ifdef RCC_RSR_IWDG2RSTF
    if (rsr & RCC_RSR_IWDG2RSTF) app_reset_reason_append(buf, size, "IWDG2");
#endif
#ifdef RCC_RSR_SFTRSTF
    if (rsr & RCC_RSR_SFTRSTF) app_reset_reason_append(buf, size, "SFT");
#endif
#ifdef RCC_RSR_SFT1RSTF
    if (rsr & RCC_RSR_SFT1RSTF) app_reset_reason_append(buf, size, "SFT1");
#endif
#ifdef RCC_RSR_SFT2RSTF
    if (rsr & RCC_RSR_SFT2RSTF) app_reset_reason_append(buf, size, "SFT2");
#endif
#ifdef RCC_RSR_PORRSTF
    if (rsr & RCC_RSR_PORRSTF) app_reset_reason_append(buf, size, "POR");
#endif
#ifdef RCC_RSR_PINRSTF
    if (rsr & RCC_RSR_PINRSTF) app_reset_reason_append(buf, size, "PIN");
#endif
#ifdef RCC_RSR_BORRSTF
    if (rsr & RCC_RSR_BORRSTF) app_reset_reason_append(buf, size, "BOR");
#endif

    if (buf[0] == '\0') {
        rt_strncpy(buf, "none", size - 1U);
        buf[size - 1U] = '\0';
    }
}

struct app_exception_stack_frame
{
    rt_uint32_t r0;
    rt_uint32_t r1;
    rt_uint32_t r2;
    rt_uint32_t r3;
    rt_uint32_t r12;
    rt_uint32_t lr;
    rt_uint32_t pc;
    rt_uint32_t psr;
};

static rt_err_t app_exception_hook(void *context)
{
    struct app_exception_stack_frame *frame = (struct app_exception_stack_frame *)context;
    rt_thread_t self = rt_thread_self();

    g_mesh_dbg_fault_magic = 0xDEADFA17;
    g_mesh_dbg_fault_cfsr = SCB->CFSR;
    g_mesh_dbg_fault_hfsr = SCB->HFSR;
    g_mesh_dbg_fault_bfar = SCB->BFAR;
    g_mesh_dbg_fault_mmar = SCB->MMFAR;
    if (frame != RT_NULL) {
        g_mesh_dbg_fault_pc = frame->pc;
        g_mesh_dbg_fault_lr = frame->lr;
    }
    g_mesh_dbg_fault_thread = (rt_uint32_t)self;
    g_mesh_dbg_stage = 0xF001;

    return -RT_ERROR;
}

static void app_assert_hook(const char *ex, const char *func, rt_size_t line)
{
    g_app_assert_magic = 0xA55E4711;
    g_app_assert_line = (rt_uint32_t)line;
    g_app_assert_ex = (rt_uint32_t)ex;
    g_app_assert_func = (rt_uint32_t)func;
    g_mesh_dbg_stage = 0xA001;

    while (1) {
        rt_thread_mdelay(100);
    }
}

static void app_early_stage_mark(void)
{
    app_stage_mark(4);
}

static int fdb_init(void)
{
    bool not_formatable = RT_TRUE;
    bool file_mode = RT_TRUE;
    uint32_t sec_size = 512;
    uint32_t db_size = 10240;

    fdb_kvdb_control(&env_kvdb, FDB_KVDB_CTRL_SET_SEC_SIZE, &sec_size);
    fdb_kvdb_control(&env_kvdb, FDB_KVDB_CTRL_SET_FILE_MODE, &file_mode);
    fdb_kvdb_control(&env_kvdb, FDB_KVDB_CTRL_SET_MAX_SIZE, &db_size);
    fdb_kvdb_control(&env_kvdb, FDB_KVDB_CTRL_SET_NOT_FORMAT, &not_formatable);

    rt_kprintf("init env kvdb\r\n");
    if (fdb_kvdb_init(&env_kvdb, "env", "/sdcard/fdb", NULL, NULL) != FDB_NO_ERR) {
        rt_kprintf("failed to init env kvdb\r\n");
        return -1;
    }

    if (fdb_kv_get(&env_kvdb, "wt_server") == RT_NULL) {
        fdb_kv_set(&env_kvdb, "wt_server", "tg");
    }

    if (fdb_kv_get(&env_kvdb, "wt_id") == RT_NULL) {
        fdb_kv_set(&env_kvdb, "wt_id", "did");
    }

#if 0
    char *val = fdb_kv_get(&env_kvdb, "boot_count");
    int boot_count = 0;
    if (val != RT_NULL) {
        boot_count = atoi(val);
    }

    boot_count++;
    rt_kprintf("boot count:%d\r\n", boot_count);

    rt_sprintf(buf, "%d", boot_count);
    fdb_kv_set(&env_kvdb, "boot_count", buf);
#else
    paramBootCountIncrease();
#endif

    return 0;
}

int watchdogInit(rt_uint32_t timeout)
{
    int ret;
    char *device_name = "wdt";
    /* 根据设备名称查找看门狗设备，获取设备句柄 */
    wdg_dev = rt_device_find(device_name);
    if (!wdg_dev)
    {
        rt_kprintf("find %s failed!\n", device_name);
        return RT_ERROR;
    }

    /* 设置看门狗溢出时间 */
    ret = rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_SET_TIMEOUT, &timeout);
    if (ret != RT_EOK)
    {
        rt_kprintf("set %s timeout failed!\n", device_name);
        return RT_ERROR;
    }
    /* 启动看门狗 */
    ret = rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_START, RT_NULL);
    if (ret != RT_EOK)
    {
        rt_kprintf("start %s failed!\n", device_name);
        return -RT_ERROR;
    }
    /* 设置空闲线程回调函数 */
    rt_thread_idle_sethook(idle_hook);

    return ret;
}

void lua_runFile(char *path);
void lua_runString(const char *code);

#if APP_BRINGUP_MIN_MESH_MODE
static const char app_min_mesh_lua[] =
    "PROJECT=\"TB_mesh_min\"\n"
    "VERSION=\"bringup\"\n"
    "log.setLevel(log.LOG_DEBUG)\n"
    "dev_state={text_state=0,text_lightness=1,text_num=0}\n"
    "light_state={light_mode=1,light_freq=1,light_ness=1}\n"
    "dev_param={vd_uart=uart.RS232_2,timeTask_enable=false,vd_enable=false,alarm_enable=false,radar_state=false,tb_report=false,udp_server_ip=\"192.168.8.2\",udp_server_port=8899,udp_dev_port=888}\n"
    "vd_value=-1\n"
    "u={convertFloatsToInt=function(t)return t end,write_param_to_file=function()end}\n"
    "tbLog={light_log=function(src,mode,freq,lightness) log.info(\"tb_log\",src,mode,freq,lightness) end}\n"
    "function lampCtrl(mode,freq,lightness)\n"
    " mode=math.floor(tonumber(mode));freq=math.floor(tonumber(freq));lightness=math.floor(tonumber(lightness))\n"
    " light_state.light_mode=mode;light_state.light_freq=freq;light_state.light_ness=lightness\n"
    " local cmd=string.format(\"z_light_param %d %d %d\",mode,freq,lightness)\n"
    " log.info(\"udp_lamp\",cmd)\n"
    " os.execute(cmd)\n"
    " return true\n"
    "end\n"
    "print(\"minimal mesh mode: built-in mesh.lua bootstrap ready\")\n";
#endif

/*
 * 判断是否设置DID
 */
void checkDid(void)
{
    while (1) {
        rt_thread_delay(10);
        if ((rt_strlen(gptSysParam->acDevUid) != 11) ||
                (gptSysParam->acDevUid[0] != '1') ||
                (gptSysParam->acDevUid[1] != '0')) {
        } else {
            break;
        }
    }
}

void falshRomFsInit(void)
{
    struct rt_device *dev = fal_blk_device_create(OSPI_FLASH_PARTITION_NAME);

    if (dev == RT_NULL) {
        rt_kprintf("failed to create device on '%s' partition\n", OSPI_FLASH_PARTITION_NAME);
        return;
    }

    if (rt_device_find(OSPI_FLASH_PARTITION_NAME) == RT_NULL) {
        rt_kprintf("failed to find blk dev:'%s'\n", OSPI_FLASH_PARTITION_NAME);
        return;
    }

    if (dfs_mount(OSPI_FLASH_PARTITION_NAME, "/flash", "elm", 0, 0) != RT_EOK) {
        rt_kprintf("failed to mount '%s'\n", OSPI_FLASH_PARTITION_NAME);
        return;
    }

    rt_kprintf("mount '%s' OK\n", OSPI_FLASH_PARTITION_NAME);
}

int eth_ip_init(void)
{
    char *val = RT_NULL;
    struct netdev *dev;
    dev = netdev_get_by_name("W5500");

    if (dev == RT_NULL) {
        rt_kprintf("failed to get netdev W5500\n");
        return 0;
    }

    netdev_set_default(dev);
    netdev_dhcp_enabled(dev, 0);

#if APP_BRINGUP_MIN_MESH_MODE && APP_BRINGUP_ENABLE_ETH_UDP
    rt_kprintf("minimal eth udp mode: use static W5500 ip 192.168.8.245\r\n");
    netdev_set_ifconfig("W5500",
                        "192.168.8.245",
                        "192.168.8.1",
                        "255.255.255.0");
    {
        ip_addr_t addr;
        if (inet_aton("192.168.8.1", &addr)) {
            netdev_set_dns_server(dev, 0, &addr);
        }
    }
    return 0;
#endif

    if (!env_kvdb.parent.init_ok) {
        rt_kprintf("env kvdb not ready, use W5500 DHCP\r\n");
        netdev_dhcp_enabled(dev, 1);
        return 0;
    }

    val = fdb_kv_get(&env_kvdb, "eth_dhcp");
    if (val == RT_NULL) {
        fdb_kv_set(&env_kvdb, "eth_dhcp", "1");
        fdb_kv_set(&env_kvdb, "eth_ip", "192.168.8.245");
        fdb_kv_set(&env_kvdb, "eth_gw", "192.168.8.1");
        fdb_kv_set(&env_kvdb, "eth_nm", "255.255.255.0");
        fdb_kv_set(&env_kvdb, "eth_dns", "192.168.8.1");
        val = "1";
    }

    if (atoi(val) == 1) { ///< DHCP
        netdev_dhcp_enabled(dev, 1);
    } else { ///< Static
        netdev_dhcp_enabled(dev, 0);
        char apcVal[4][20];

        rt_strncpy(&apcVal[0][0], fdb_kv_get(&env_kvdb, "eth_ip"), 20);
        rt_strncpy(&apcVal[1][0], fdb_kv_get(&env_kvdb, "eth_gw"), 20);
        rt_strncpy(&apcVal[2][0], fdb_kv_get(&env_kvdb, "eth_nm"), 20);
        rt_strncpy(&apcVal[3][0], fdb_kv_get(&env_kvdb, "eth_dns"), 20);

        if ((rt_strlen(apcVal[0]) >= 8) &&
                (rt_strlen(apcVal[1]) >= 8) &&
                (rt_strlen(apcVal[2]) >= 8)) {
            netdev_set_ifconfig("W5500",
                                apcVal[0],
                    apcVal[1],
                    apcVal[2]);
        }
        if (rt_strlen(apcVal[3]) >= 8) {
            ip_addr_t addr;
            if (inet_aton(apcVal[3], &addr)) {
                netdev_set_dns_server(dev, 0, &addr);
            }
        }
    }

    return 0;
}

int wiz_init(void);
int wtIsAlive(void);
static uint32_t ulLastMainUpdateTime = 0;
static void idle_hook(void)
{
#if WDG_ENABLE
    /* 喂狗 */
    if (wdg_dev != RT_NULL) {
        rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_KEEPALIVE, RT_NULL);
    }
#endif

    uint32_t ulNow = 0;

    ulNow = rt_tick_get();
    if (ulNow > ulLastMainUpdateTime + 60 * 1000) { ///< 60s
        rt_hw_cpu_reset(); ///< 重启
    }
}

static void checkReboot(void)
{
    static int flag = 0;
    static uint32_t ulLastAliveTime = 0;
    uint32_t ulNow = 0;

    if (flag == 0) {
        if (wtIsAlive()) {
            flag = 1;
            ulLastAliveTime = rt_tick_get();
        }
    } else {
        ulNow = rt_tick_get();

        if (wtIsAlive() == 0) {
            if (ulNow > ulLastAliveTime + 10 * 60 * 1000) { ///< 60s
                rt_hw_cpu_reset(); ///< 重启
            }
        } else {
            ulLastAliveTime = ulNow;
        }
    }
}

int main(void)
{
#if !APP_BRINGUP_DIAG
#if APP_BRINGUP_RUN_SDCARD_LUA
    uint32_t ulTime = 0;
    int iFlag = 0;
#endif
#endif

    rt_thread_delay(10);

    rt_pin_write(USB_CTRL_PIN, PIN_HIGH);

    rt_kprintf("main start\r\n");
    app_stage_mark(5);

#if APP_BRINGUP_DIAG
    rt_uint32_t heartbeat = 0;
    uint8_t diag_leds_on[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t diag_leds_off[4] = {0x00, 0x00, 0x00, 0x00};

    rt_kprintf("bringup diag mode\r\n");

    while (1) {
        ulLastMainUpdateTime = rt_tick_get();
        tm1650RefreshBuf((heartbeat & 0x1U) ? diag_leds_on : diag_leds_off);
        heartbeat++;
        rt_thread_mdelay(250);
    }
#else
    easyblink(easyblinkGetLed(EB_LED_CPU), -1, 200, 400);
    easyblink(easyblinkGetLed(EB_LED_ERROR), -1, 200, 800);
    easyblink(easyblinkGetLed(EB_LED_ETHERNET), -1, 200, 1200);

    falshRomFsInit();
    app_stage_mark(6);

    ulLastMainUpdateTime = rt_tick_get();

    {
        rt_tick_t sd_wait_start = rt_tick_get();

        while (1) {
            rt_thread_delay(10);
            if (rt_device_find("sd0") != RT_NULL) {
                g_sd_status = 1;
                break;
            }

            if (rt_tick_get() - sd_wait_start > RT_TICK_PER_SECOND * 2) {
                g_sd_status = 2;
                rt_kprintf("sd0 not found, continue without sdcard\r\n");
                break;
            }
        }
    }
    app_stage_mark(7);

    ulLastMainUpdateTime = rt_tick_get();

#if !APP_SCRIPT_UPDATE_MODE && (!APP_BRINGUP_MIN_MESH_MODE || APP_BRINGUP_ENABLE_ETH_UDP)
    fdb_init();
#elif APP_BRINGUP_MIN_MESH_MODE
    rt_kprintf("minimal mesh mode: skip gps/business services\r\n");
#else
    rt_kprintf("script update mode: skip gps/fdb/business services\r\n");
#endif

#if !APP_SCRIPT_UPDATE_MODE && !APP_BRINGUP_MIN_MESH_MODE
    lwgps2rtt_init("uart1");

	rt_thread_mdelay(1000);
#elif APP_BRINGUP_MIN_MESH_MODE
    rt_kprintf("minimal mesh mode: skip gps/business lua services\r\n");
#else
    rt_kprintf("script update mode: skip gps/business lua services\r\n");
#endif

#if APP_BRINGUP_ENABLE_LUATOS
    rt_kprintf("init luatos\r\n");
    g_luatos_init_ret = rtt_luatos_init();
#else
    rt_kprintf("skip luatos during bringup\r\n");
    g_luatos_init_ret = -1;
#endif

    rt_kprintf("init over\r\n");
    app_stage_mark(8);

#if !APP_SCRIPT_UPDATE_MODE && !APP_BRINGUP_MIN_MESH_MODE
    dfs_file_unlink("/sdcard/system/firmware.bin");

    rt_memset(gptSysParam->acFwVersion, 0, sizeof(gptSysParam->acFwVersion));
    rt_sprintf(gptSysParam->acFwVersion,
               "V%d.%02d",
               APP_VERSION_CODE / 100, APP_VERSION_CODE % 100);
    paramUpdateChsm();
#endif

#if APP_BRINGUP_RUN_SDCARD_LUA
    ulTime = rt_tick_get();
#endif

#if !APP_SCRIPT_UPDATE_MODE && (!APP_BRINGUP_MIN_MESH_MODE || APP_BRINGUP_ENABLE_ETH_UDP)
    wiz_init();

    rt_thread_delay(1000);

    eth_ip_init();
#elif APP_BRINGUP_MIN_MESH_MODE
    rt_kprintf("minimal mesh mode: skip ethernet/web/ppp business network\r\n");
#else
    rt_kprintf("script update mode: shell ready, use ry to receive files\r\n");
#endif
    app_stage_mark(9);

    while (1) {

        ulLastMainUpdateTime = rt_tick_get();
        g_mesh_dbg_main_loop++;

        rt_thread_mdelay(10);

#if APP_BRINGUP_ENABLE_AUTO_REBOOT
        checkReboot();
#endif

        dioUpdateLedState();

#if APP_BRINGUP_RUN_SDCARD_LUA
        if ((iFlag == 0) && (rt_tick_get() > ulTime + 10000)) {
            iFlag = 1;

#if APP_BRINGUP_MIN_MESH_MODE
            g_sdcard_lua_stage = 2;
            lua_runString(app_min_mesh_lua);
            lua_runString((const char *)mesh_lua);
#else
            char *path = "/sdcard/script/main.lua";
            struct stat fSta;
            if (dfs_file_stat(path, &fSta) == 0) {
                g_sdcard_lua_stage = 2;
                lua_runFile("/sdcard/script/main.lua");
            } else {
                g_sdcard_lua_stage = 1;
            }
#endif
        } 
#endif
    }
#endif
}

static char *time2human(uint32_t ulMilliSec)
{
    const int BUF_SIZE = 32;
    static char acBuf[BUF_SIZE];

    uint32_t ulSec = ulMilliSec / 1000;

    if (ulSec < 60) {
        rt_snprintf(acBuf, BUF_SIZE, "%uSec", ulSec);
    } else if (ulSec < 60 * 60) {
        rt_snprintf(acBuf, BUF_SIZE, "%uMin %uSec", ulSec / 60, ulSec % 60);
    } else if (ulSec < 60 * 60 * 24) {
        rt_snprintf(acBuf, BUF_SIZE, "%uHours %uMin %uSec", ulSec / 3600, ulSec % 3600 / 60, ulSec % 60);
    } else {
        rt_snprintf(acBuf, BUF_SIZE, "%uDays %uHours %uMin %uSec",
                    ulSec / (3600 * 24), ulSec % (3600 * 24) / 3600, ulSec % 3600 / 60, ulSec % 60);
    }

    return acBuf;
}

HAL_StatusTypeDef Set_Flash_Read_Protection(uint32_t level) {
  HAL_StatusTypeDef status = HAL_OK;
  FLASH_OBProgramInitTypeDef OBInit;

  // 解锁Flash和Option Bytes
  HAL_FLASH_Unlock();
  HAL_FLASH_OB_Unlock();

  // 获取当前Option Bytes配置
  HAL_FLASHEx_OBGetConfig(&OBInit);

  // 设置读保护级别
  OBInit.RDPLevel = level;

  // 然后编程Option Bytes
  status = HAL_FLASHEx_OBProgram(&OBInit);

  if (status == HAL_OK) {
    // 启动Option Bytes编程过程
    status = HAL_FLASH_OB_Launch();
  }

  // 重新锁定Flash和Option Bytes
  HAL_FLASH_OB_Lock();
  HAL_FLASH_Lock();

  return status;
}


#ifdef FINSH_USING_MSH
#define BOOT_FLASH_SIZE (128 * 1024)
static int __recvLuaFile(uint8_t argc, char **argv)
{
    char path[96];
    char buf[256];
    char *end = RT_NULL;
    long expected = 0;
    long received = 0;
    int fd = -1;
    rt_device_t dev = RT_NULL;

    if (argc != 3) {
        rt_kprintf("usage: recv_lua <filename.lua> <size>\n");
        rt_kprintf("example: recv_lua mesh.lua 16263\n");
        return 0;
    }

    if (rt_strstr(argv[1], "/") != RT_NULL || rt_strstr(argv[1], "\\") != RT_NULL) {
        rt_kprintf("invalid filename\n");
        return 0;
    }

    expected = strtol(argv[2], &end, 10);
    if (expected <= 0 || end == argv[2]) {
        rt_kprintf("invalid size\n");
        return 0;
    }

    rt_snprintf(path, sizeof(path), "/sdcard/script/%s", argv[1]);
    fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0);
    if (fd < 0) {
        rt_kprintf("open %s failed\n", path);
        return 0;
    }

    dev = rt_console_get_device();
    if (dev == RT_NULL) {
        rt_kprintf("console device not found\n");
        close(fd);
        return 0;
    }

    rt_kprintf("ready: send %ld bytes for %s now\n", expected, path);
    while (received < expected) {
        rt_size_t want = sizeof(buf);
        rt_size_t got;

        if (expected - received < (long)want) {
            want = expected - received;
        }

        got = rt_device_read(dev, 0, buf, want);
        if (got > 0) {
            int written = write(fd, buf, got);
            if (written != (int)got) {
                rt_kprintf("write failed at %ld\n", received);
                break;
            }
            received += got;
        } else {
            rt_thread_mdelay(2);
        }
    }

    close(fd);
    rt_kprintf("recv_lua done: %ld/%ld bytes -> %s\n", received, expected, path);

    return 0;
}
MSH_CMD_EXPORT_ALIAS(__recvLuaFile, recv_lua, receive lua file by raw serial bytes);

static int __writeBootloader(uint8_t argc, char **argv)
{
    int cnt = 0;
    const struct fal_partition *part;

    const uint32_t ulSectorSize = 10 * 1024UL;
    int iSec, iRet;
    struct stat fsta;
    int fd;
    uint8_t *pucBuf = RT_NULL;
    char *path = argv[1];

    if (argc != 2) {
        rt_kprintf("invalid args\n");
        return 0;
    }

    pucBuf = rt_malloc(ulSectorSize);

    if (pucBuf == RT_NULL) {
        rt_kprintf("failed malloc buf\n");
        goto EXIT;
    }

    if (stat(path, &fsta) != 0) {
        rt_kprintf("failed to get boot file stat\n");
        goto EXIT;
    }

    rt_kprintf("boot file size:%u\n", fsta.st_size);

    if (fsta.st_size > BOOT_FLASH_SIZE) {
        rt_kprintf("boot file too big\n");
        goto EXIT;
    }

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        rt_kprintf("failed to open boot file\n");
        goto EXIT;
    }

    part = fal_partition_find("bootloader");

    if (part == RT_NULL) {
        rt_kprintf("failed to find bootloader partition\n");
        goto EXIT;
    }

    Set_Flash_Read_Protection(OB_RDP_LEVEL_0);

    while (cnt++ < 3)
    {
        if (fal_partition_erase(part, 0, BOOT_FLASH_SIZE) > 0) {
            rt_kprintf("erase ok\n");
            break;
        }
    }

    iSec = 0;
    while (iSec * ulSectorSize < fsta.st_size) {
        iRet = read(fd, pucBuf, ulSectorSize);
        if (iRet == ulSectorSize) {
            rt_kprintf("write at %u with %u bytes\n",
                       iSec * ulSectorSize,
                       iRet);
            fal_partition_write(part,
                                iSec * ulSectorSize,
                                pucBuf,
                                iRet);
        } else {
            if (iRet > 0) {
                rt_kprintf("write at %u with %u bytes\n",
                           iSec * ulSectorSize,
                           iRet);
                fal_partition_write(part,
                                    iSec * ulSectorSize,
                                    pucBuf,
                                    iRet);
                rt_kprintf("write over,%u\n", iSec * ulSectorSize + iRet);
                break;
            } else {
                rt_kprintf("read error at %u\n", iSec * ulSectorSize);
                break;
            }
        }
        iSec++;
    }

    iSec = 0;
    lseek(fd, 0, SEEK_SET);
    while (iSec * ulSectorSize < fsta.st_size) {
        iRet = read(fd, pucBuf, ulSectorSize);

        if (iRet == ulSectorSize) {
            rt_kprintf("verify at %u with %u bytes\n",
                       iSec * ulSectorSize,
                       iRet);
            if (rt_memcmp(pucBuf, (uint8_t *)(0x08000000 + iSec * ulSectorSize), ulSectorSize) != 0) {
                rt_kprintf("verify failed at 0x%08X\n", iSec * ulSectorSize);
                break;
            } else {
                rt_kprintf("verify ok at 0x%08X\n", iSec * ulSectorSize);
            }
        } else {
            if (iRet > 0) {
                rt_kprintf("verify at %u with %u bytes\n",
                           iSec * ulSectorSize,
                           iRet);
                if (rt_memcmp(pucBuf, (uint8_t *)(0x08000000 + iSec * ulSectorSize), iRet) != 0) {
                    rt_kprintf("verify failed at 0x%08X\n", iSec * ulSectorSize);
                } else {
                    rt_kprintf("verify ok at 0x%08X\n", iSec * ulSectorSize);
                }
                break;
            } else {
                rt_kprintf("read error at %u\n", iSec * ulSectorSize);
                break;
            }
        }
        iSec++;
    }

EXIT:
    if (fd >= 0) {
        close(fd);
    }

    if (pucBuf != RT_NULL) {
        rt_free(pucBuf);
    }

    Set_Flash_Read_Protection(OB_RDP_LEVEL_1);

    return 0;
}
MSH_CMD_EXPORT_ALIAS(__writeBootloader, write_boot, write boot file to flash);
#endif

char MyMcuID[30] = {0};

void my_get_mcu_id(char *str)
{
	uint32_t CpuID[3] = {0};

    CpuID[2] = HAL_GetUIDw0();
    CpuID[1] = HAL_GetUIDw1();
    CpuID[0] = HAL_GetUIDw2();
	
	rt_sprintf(str, "%X%08X%08X", CpuID[0], CpuID[1], CpuID[2]);
	
//	rt_kprintf("mcu id : %s \r\n", str);
}

static int __my_get_mcu_id(void)
{
	my_get_mcu_id(MyMcuID);
	
	return 0;
}
INIT_BOARD_EXPORT(__my_get_mcu_id);

extern int read_dev_param_to_onchip_flash(sysParamOnchip_t *data);

#ifdef FINSH_USING_MSH
extern int _giModemRssi;
static int __boardInfo(uint8_t argc, char **argv)
{
	sysParamOnchip_t data = {0};

	read_dev_param_to_onchip_flash(&data);

	rt_kprintf("on chip device id :%s \r\n", data.acDevUid);
    rt_kprintf("device id:%s\n", gptSysParam->acDevUid);
    rt_kprintf("hw version:%s\n", gptSysParam->acHwVersion);
    rt_kprintf("hw manufacture date:%s\n", gptSysParam->acHwDate);

	rt_kprintf("mcu id : %s \n", MyMcuID);
	
    rt_kprintf("RS232 log state:%d\n", gptSysParam->ucRs232_1_LogEnable);

    rt_kprintf("boot version:%s\n", gptSysParam->acBootVersion);
    rt_kprintf("fw version:%s\n", gptSysParam->acFwVersion);

    rt_kprintf("modem model:%s\n", gptSysParam->acModemModel);
    rt_kprintf("modem imei:%s\n", gptSysParam->acModemImei);
    rt_kprintf("modem rssi:%d\n", _giModemRssi);
    rt_kprintf("sim ccid:%s\n", gptSysParam->acSimCcid);

    rt_kprintf("boot count:%d\n", gptSysParam->ulBootCount);
    rt_kprintf("run time:%s\n", time2human(rt_tick_get_millisecond()));
    rt_kprintf("power in voltage:%.1fV\n", ainGetPowerVoltage());
    rt_kprintf("board temperatur:%.1f degree\n", ainGetBoardTemp());

    return 0;
}
MSH_CMD_EXPORT_ALIAS(__boardInfo, info, print basic board info);
#endif


#ifdef FINSH_USING_MSH
static int __setdhcp(uint8_t argc, char **argv)
{
    if (argc != 3) {
        rt_kprintf("example: set_dhcp W5500 1\n");
        return 0;
    }

    if (rt_strncmp(argv[1], "pp", 2) == 0) {
        rt_kprintf("unsupported 'DHCP' for netdev pp\n");
        return 0;
    }

    if (rt_strncmp(argv[1], "W5500", 5) == 0) {
        struct netdev *dev;
        dev = netdev_get_by_name("W5500");
        if (dev == RT_NULL) {
            rt_kprintf("failed to get netdev W5500\n");
            return 0;
        }

        if (atoi(argv[2]) == 0) {
            fdb_kv_set(&env_kvdb, "eth_dhcp", "0");
            netdev_dhcp_enabled(dev, 0);
        } else {
            fdb_kv_set(&env_kvdb, "eth_dhcp", "1");
            netdev_dhcp_enabled(dev, 1);
        }

        rt_kprintf("W5500 dhcp state:%d\n", dev->flags & NETDEV_FLAG_DHCP ? 1 : 0);
        return 0;
    }

    rt_kprintf("failed to get netdev %s\n", argv[1]);

    return 0;
}
MSH_CMD_EXPORT_ALIAS(__setdhcp, set_dhcp, set dhcp state of netdev);
#endif


#ifdef FINSH_USING_MSH
#ifdef PKG_LUATOS_SOC_LUAT_HEAP_SIZE
void luat_os_print_heapinfo_vm(void);
int lua_free(int argc, char **argv)
{
    luat_os_print_heapinfo_vm();
    return 0;
}
MSH_CMD_EXPORT_ALIAS(lua_free, lua_free, Show the memory usage in the lua VM.);
#endif /* RT_USING_HEAP */
#endif /* MSH_USING_BUILT_IN_COMMANDS */

static int mesh_dbg(uint8_t argc, char **argv)
{
    char reset_reason[96];

    app_reset_reason_format(reset_reason, sizeof(reset_reason), g_app_reset_rsr);
    rt_kprintf("reset rsr=0x%08X reason=%s\r\n", g_app_reset_rsr, reset_reason);
    rt_kprintf("mesh_dbg stage=0x%08X arg0=%u arg1=%u\r\n",
               g_mesh_dbg_stage, g_mesh_dbg_arg0, g_mesh_dbg_arg1);
    rt_kprintf("cmd=%u send_ok=%u rd_send=%u radio_tx=%u tx_done=%u rx_done=%u\r\n",
               g_mesh_dbg_cmd_count, g_mesh_dbg_send_ok_count,
               g_mesh_dbg_radio_send_count, g_mesh_dbg_radio_tx_count,
               g_mesh_dbg_tx_done_count, g_mesh_dbg_rx_done_count);
    rt_kprintf("main_loop=%u radio_loop=%u\r\n",
               g_mesh_dbg_main_loop, g_mesh_dbg_radio_loop);
    rt_kprintf("ready=%u\r\n", g_mesh_dbg_ready);
    rt_kprintf("fault magic=0x%08X cfsr=0x%08X hfsr=0x%08X bfar=0x%08X mmar=0x%08X pc=0x%08X lr=0x%08X thread=0x%08X\r\n",
               g_mesh_dbg_fault_magic, g_mesh_dbg_fault_cfsr,
               g_mesh_dbg_fault_hfsr, g_mesh_dbg_fault_bfar,
               g_mesh_dbg_fault_mmar, g_mesh_dbg_fault_pc,
               g_mesh_dbg_fault_lr, g_mesh_dbg_fault_thread);
    rt_kprintf("assert magic=0x%08X line=%u ex=0x%08X func=0x%08X\r\n",
               g_app_assert_magic, g_app_assert_line,
               g_app_assert_ex, g_app_assert_func);
    return 0;
}
MSH_CMD_EXPORT(mesh_dbg, show mesh runtime debug marks);

static void key_reset_thread_entry(void *parameter)
{
    rt_base_t idle_level;
    rt_tick_t pressed_start = 0;
    rt_uint8_t pressed_latched = 0;

    rt_pin_mode(KEY_RESET_PIN, PIN_MODE_INPUT);
    rt_thread_mdelay(200);
    idle_level = rt_pin_read(KEY_RESET_PIN);
    rt_kprintf("key reset: PE10 idle level=%d, hold 3s to reboot\r\n", idle_level);

    while (1) {
        rt_base_t level = rt_pin_read(KEY_RESET_PIN);
        rt_uint8_t pressed = (level != idle_level) ? 1U : 0U;

        if (pressed) {
            if (!pressed_latched) {
                pressed_latched = 1;
                pressed_start = rt_tick_get();
            } else if ((rt_tick_get() - pressed_start) >= (RT_TICK_PER_SECOND * 3)) {
                rt_kprintf("key reset: PE10 held for 3s, reboot\r\n");
                rt_thread_mdelay(50);
                rt_hw_cpu_reset();
            }
        } else {
            pressed_latched = 0;
            pressed_start = 0;
        }

        rt_thread_mdelay(20);
    }
}

static int key_reset_init(void)
{
    rt_thread_t tid = rt_thread_create("keyrst",
                                       key_reset_thread_entry,
                                       RT_NULL,
                                       1024,
                                       25,
                                       10);
    if (tid == RT_NULL) {
        rt_kprintf("key reset: create thread failed\r\n");
        return -RT_ERROR;
    }

    rt_thread_startup(tid);
    return RT_EOK;
}
INIT_APP_EXPORT(key_reset_init);

#if APP_ENABLE_UART10_LIGHT_CMD
extern int mesh_tm_z_light_param_send(int mode, int freq, int light_level);
static int uart10_cmd_split(char *line, char **argv, int max_argc);

#if APP_BRINGUP_ENABLE_ETH_UDP
static volatile rt_uint32_t g_eth_udp_rx_count = 0;
static volatile rt_int32_t g_eth_udp_last_res = 0;

static int app_json_get_int(const char *text, const char *key, int *value)
{
    char pattern[32];
    char *p;

    rt_snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    p = rt_strstr((char *)text, pattern);
    if (p == RT_NULL) {
        return -1;
    }

    p = rt_strstr(p, ":");
    if (p == RT_NULL) {
        return -1;
    }
    p++;
    while (*p == ' ' || *p == '\t' || *p == '\"') {
        p++;
    }

    *value = atoi(p);
    return 0;
}

static void eth_udp_cmd_thread_entry(void *parameter)
{
    int sock;
    struct sockaddr_in local_addr;
    struct sockaddr_in from_addr;
    socklen_t from_len;
    char buf[256];

    rt_thread_mdelay(12000);

    while (1) 
	{
        struct netdev *dev = netdev_get_by_name("W5500");

        if (dev != RT_NULL) {
            netdev_set_default(dev);
        }

        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) {
            rt_kprintf("eth_udp_cmd: socket failed, retry\r\n");
            rt_thread_mdelay(2000);
            continue;
        }

        rt_memset(&local_addr, 0, sizeof(local_addr));
        local_addr.sin_family = AF_INET;
        local_addr.sin_port = htons(888);
        local_addr.sin_addr.s_addr = inet_addr("0.0.0.0");

        if (bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) == 0) {
            break;
        }

        rt_kprintf("eth_udp_cmd: bind 888 failed, retry\r\n");
        closesocket(sock);
        rt_thread_mdelay(2000);
    }

    rt_kprintf("eth_udp_cmd: listen udp 888\r\n");

    while (1) {
        int len;
        int mode;
        int freq;
        int lightness;

        from_len = sizeof(from_addr);
        len = recvfrom(sock, buf, sizeof(buf) - 1, 0,
                       (struct sockaddr *)&from_addr, &from_len);
        if (len <= 0) {
            continue;
        }

        buf[len] = '\0';
        g_eth_udp_rx_count++;
        rt_kprintf("eth_udp_cmd recv %d bytes: %s\r\n", len, buf);

        if (rt_strncmp(buf, "z_light_param", 13) == 0) 
		{
            char *argv[4];
            int argc = uart10_cmd_split(buf, argv, 4);

            if (argc != 4) {
                rt_kprintf("eth_udp_cmd: bad z_light_param packet\r\n");
                continue;
            }

            mode = atoi(argv[1]);
            freq = atoi(argv[2]);
            lightness = atoi(argv[3]);
        } else if (rt_strstr(buf, "lightControl") == RT_NULL ||
                app_json_get_int(buf, "mode", &mode) != 0 ||
                app_json_get_int(buf, "freq", &freq) != 0 ||
                app_json_get_int(buf, "lightness", &lightness) != 0) {
            rt_kprintf("eth_udp_cmd: unsupported packet\r\n");
            continue;
        }

        g_eth_udp_last_res = mesh_tm_z_light_param_send(mode, freq, lightness);
        rt_kprintf("eth_udp_cmd: z_light_param %d %d %d res=%d\r\n",
                   mode, freq, lightness, g_eth_udp_last_res);
    }
}

static int eth_udp_cmd_init(void)
{
    rt_thread_t tid = rt_thread_create("ethudp",
                                       eth_udp_cmd_thread_entry,
                                       RT_NULL,
                                       4096,
                                       24,
                                       10);
    if (tid == RT_NULL) {
        rt_kprintf("eth_udp_cmd: create thread failed\r\n");
        return -RT_ERROR;
    }

    rt_thread_startup(tid);
    return RT_EOK;
}
INIT_APP_EXPORT(eth_udp_cmd_init);
#endif

static struct rt_semaphore g_uart10_cmd_sem;
static rt_device_t g_uart10_cmd_dev = RT_NULL;

static rt_err_t uart10_cmd_rx_ind(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(&g_uart10_cmd_sem);
    return RT_EOK;
}

static void uart10_cmd_write(const char *text)
{
    if (g_uart10_cmd_dev != RT_NULL && text != RT_NULL) 
	{
        rt_device_write(g_uart10_cmd_dev, 0, text, rt_strlen(text));
    }
}

static void uart10_cmd_printf(const char *fmt, ...)
{
    char buf[192];
    va_list args;

    va_start(args, fmt);
    rt_vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    uart10_cmd_write(buf);
}

static void uart10_cmd_eth_dbg(void)
{
    struct netdev *dev = netdev_get_by_name("W5500");
    char ip[16];
    char gw[16];
    char mask[16];

    if (dev == RT_NULL) {
        uart10_cmd_write("eth W5500 not found\r\n");
        return;
    }

    uart10_cmd_printf("eth W5500 flags=0x%04X up=%d link=%d dhcp=%d\r\n",
                      dev->flags,
                      netdev_is_up(dev),
                      netdev_is_link_up(dev),
                      netdev_is_dhcp_enabled(dev));
    inet_ntoa_r(dev->ip_addr, ip, sizeof(ip));
    inet_ntoa_r(dev->gw, gw, sizeof(gw));
    inet_ntoa_r(dev->netmask, mask, sizeof(mask));
    uart10_cmd_printf("eth ip=%s gw=%s mask=%s\r\n",
                      ip,
                      gw,
                      mask);
#if APP_BRINGUP_ENABLE_ETH_UDP
    uart10_cmd_printf("eth udp_rx=%u last_res=%d\r\n",
                      g_eth_udp_rx_count,
                      g_eth_udp_last_res);
#endif
}

static void uart10_cmd_eth_dhcp(int enable)
{
    struct netdev *dev = netdev_get_by_name("W5500");

    if (dev == RT_NULL) {
        uart10_cmd_write("eth W5500 not found\r\n");
        return;
    }

    if (env_kvdb.parent.init_ok) {
        fdb_kv_set(&env_kvdb, "eth_dhcp", enable ? "1" : "0");
    }

    netdev_dhcp_enabled(dev, enable ? RT_TRUE : RT_FALSE);
    uart10_cmd_printf("eth dhcp=%d\r\n", enable ? 1 : 0);
}

static void uart10_cmd_eth_static(const char *ip, const char *gw, const char *mask)
{
    struct netdev *dev = netdev_get_by_name("W5500");

    if (dev == RT_NULL) {
        uart10_cmd_write("eth W5500 not found\r\n");
        return;
    }

    netdev_dhcp_enabled(dev, RT_FALSE);
    netdev_set_ifconfig("W5500", (char *)ip, (char *)gw, (char *)mask);

    if (env_kvdb.parent.init_ok) {
        fdb_kv_set(&env_kvdb, "eth_dhcp", "0");
        fdb_kv_set(&env_kvdb, "eth_ip", ip);
        fdb_kv_set(&env_kvdb, "eth_gw", gw);
        fdb_kv_set(&env_kvdb, "eth_nm", mask);
        fdb_kv_set(&env_kvdb, "eth_dns", gw);
    }

    uart10_cmd_write("eth static set\r\n");
    uart10_cmd_eth_dbg();
}

static int uart10_cmd_split(char *line, char **argv, int max_argc)
{
    int argc = 0;
    char *p = line;

    while (*p != '\0' && argc < max_argc) {
        while (*p == ' ' || *p == '\t') {
            p++;
        }
        if (*p == '\0') {
            break;
        }

        argv[argc++] = p;
        while (*p != '\0' && *p != ' ' && *p != '\t') {
            p++;
        }
        if (*p != '\0') {
            *p++ = '\0';
        }
    }

    return argc;
}

static void uart10_cmd_handle_line(char *line)
{
    char *argv[8];
    int argc = uart10_cmd_split(line, argv, 8);
    int res;

    if (argc == 0) {
        return;
    }

    if (rt_strcmp(argv[0], "eth_dbg") == 0) {
        uart10_cmd_eth_dbg();
        return;
    }

    if (rt_strcmp(argv[0], "eth_dhcp") == 0) {
        if (argc != 2) {
            uart10_cmd_write("ERR usage: eth_dhcp <0|1>\r\n");
            return;
        }

        uart10_cmd_eth_dhcp(atoi(argv[1]) ? 1 : 0);
        return;
    }

    if (rt_strcmp(argv[0], "eth_static") == 0) {
        if (argc != 4) {
            uart10_cmd_write("ERR usage: eth_static <ip> <gw> <mask>\r\n");
            return;
        }

        uart10_cmd_eth_static(argv[1], argv[2], argv[3]);
        return;
    }

    if (rt_strcmp(argv[0], "z_light_param") == 0) {
        if (argc != 4) {
            uart10_cmd_write("ERR usage: z_light_param <mode> <freq> <level>\r\n");
            return;
        }

        res = mesh_tm_z_light_param_send(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
        if (res == 0) {
            uart10_cmd_write("OK z_light_param\r\n");
        } else {
            uart10_cmd_write("ERR z_light_param\r\n");
        }
        return;
    }

    uart10_cmd_write("ERR unknown command\r\n");
}


static void uart10_cmd_thread_entry(void *parameter)
{
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    rt_uint8_t rx_buf[64];
    char line[128];
    char reset_reason[96];
    rt_size_t line_len = 0;

    g_uart10_cmd_dev = rt_device_find("uart10");
    if (g_uart10_cmd_dev == RT_NULL) {
        rt_kprintf("uart10_cmd: uart10 not found\r\n");
        return;
    }

    rt_sem_init(&g_uart10_cmd_sem, "u10cmd", 0, RT_IPC_FLAG_FIFO);

    config.baud_rate = 115200;
    config.data_bits = DATA_BITS_8;
    config.stop_bits = STOP_BITS_1;
    config.parity = PARITY_NONE;
    config.bufsz = 512;
    rt_device_control(g_uart10_cmd_dev, RT_DEVICE_CTRL_CONFIG, &config);

    if (rt_device_open(g_uart10_cmd_dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX) != RT_EOK) {
        rt_kprintf("uart10_cmd: open uart10 failed\r\n");
        rt_sem_detach(&g_uart10_cmd_sem);
        g_uart10_cmd_dev = RT_NULL;
        return;
    }

    rt_device_set_rx_indicate(g_uart10_cmd_dev, uart10_cmd_rx_ind);
    app_reset_reason_format(reset_reason, sizeof(reset_reason), g_app_reset_rsr);
    uart10_cmd_write("uart10 light cmd ready\r\n");
    uart10_cmd_printf("reset rsr=0x%08X reason=%s\r\n", g_app_reset_rsr, reset_reason);
    rt_kprintf("uart10 light command port ready: z_light_param <mode> <freq> <level>\r\n");

    while (1) {
        if (rt_sem_take(&g_uart10_cmd_sem, RT_TICK_PER_SECOND) != RT_EOK) {
            continue;
        }

        while (1) {
            rt_size_t len = rt_device_read(g_uart10_cmd_dev, 0, rx_buf, sizeof(rx_buf));
            if (len == 0) {
                break;
            }

            for (rt_size_t i = 0; i < len; i++) {
                char ch = (char)rx_buf[i];
                if (ch == '\r' || ch == '\n') {
                    if (line_len > 0) {
                        line[line_len] = '\0';
                        uart10_cmd_handle_line(line);
                        line_len = 0;
                    }
                } else if (line_len < sizeof(line) - 1) {
                    line[line_len++] = ch;
                } else {
                    line_len = 0;
                    uart10_cmd_write("ERR line too long\r\n");
                }
            }
        }
    }
}

static int uart10_cmd_init(void)
{
    rt_thread_t tid = rt_thread_create("u10cmd",
                                       uart10_cmd_thread_entry,
                                       RT_NULL,
                                       2048,
                                       23,
                                       10);
    if (tid == RT_NULL) {
        rt_kprintf("uart10_cmd: create thread failed\r\n");
        return -RT_ERROR;
    }

    rt_thread_startup(tid);
    return RT_EOK;
}
INIT_APP_EXPORT(uart10_cmd_init);
#endif


#include "stm32h7xx.h"
static int vtor_config(void)
{
    /* Vector Table Relocation in Internal QSPI_FLASH */
    SCB->VTOR = ROM_START;

    return 0;
}
INIT_BOARD_EXPORT(vtor_config);


static int __mainInit(void)
{
    app_capture_reset_reason();
    app_early_stage_mark();
    rt_hw_exception_install(app_exception_hook);
    rt_assert_set_hook(app_assert_hook);

#if WDG_ENABLE
watchdogInit(15);
#endif
    rt_pin_mode(USB_CTRL_PIN, PIN_MODE_OUTPUT_OD);
    rt_pin_write(USB_CTRL_PIN, PIN_HIGH);

#if !APP_BRINGUP_DIAG
    for (int i = 0; i < PKG_EASYBLINK_MAX_LED_NUMS; ++i) {
        easyblink_init_led(i, 1);
    }

#if APP_BRINGUP_ENABLE_BEEP_RELAY
    beep_init(BEEP_PIN, 0);
#endif
#endif

    return 0;
}
INIT_ENV_EXPORT(__mainInit);


