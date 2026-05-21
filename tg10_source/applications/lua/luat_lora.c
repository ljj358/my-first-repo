/*
@module  uart
@summary 串口操作库
@version 1.0
@date    2020.03.30
@demo uart
@video https://www.bilibili.com/video/BV1er4y1p75y
*/
#include "luat_base.h"
#include "luat_uart.h"
#include "luat_malloc.h"
#include "luat_msgbus.h"
#include "luat_fs.h"
#include "string.h"
#include "luat_zbuff.h"

#include "lwgps2rtt.h"
#include "drv_common.h"
#include "drivers/pin.h"

#include "lora-radio-rtos-config.h"
#include "lora-Radio.h"
#include "lora-radio-timer.h"
#include "app_runtime_debug.h"

#define LUAT_LOG_TAG "lora"
#include "luat_log.h"

#define LORA_DEV_1  LORA_RADIO_0
#define LORA_DEV_2  LORA_RADIO_1

#define LORA_BW_125K  0
#define LORA_BW_250K  1
#define LORA_BW_500K  2

#define LORA_SF_6     6
#define LORA_SF_7     7
#define LORA_SF_8     8
#define LORA_SF_9     9
#define LORA_SF_10    10
#define LORA_SF_11    11
#define LORA_SF_12    12

static struct {
    uint32_t freq; ///< 频率
    int sf; ///< 扩频因子
    int bw; ///< 带宽
    int prem_len; ///< 前导码长度
	  int lowDatarateOptimize; ///< 低速率优化
} cb_lora_config[2];

static RadioEvents_t RadioEvents[2];
static int rx_callback[2] = {0};

static void dump(uint8_t *pucData, uint16_t usLen)
{
    int i;
    for (i = 0; i < usLen; ++i) {
        if ((i > 0) && (i % 8 == 0)) {
            rt_kprintf("\r\n");
        }
        rt_kprintf("%02X ", pucData[i]);
    }

    rt_kprintf("\r\n");
}

static int msg_handler(lua_State *L, void* ptr) {
    rtos_msg_t* msg = (rtos_msg_t*)lua_topointer(L, -1);

    int ret = 0;
    if ((msg->arg1 != LORA_DEV_1) && (msg->arg1 != LORA_DEV_2)) {
        goto RET;
    }
	
	ret = 1;

    if (rx_callback[msg->arg1] != 0) {
        lua_geti(L, LUA_REGISTRYINDEX, rx_callback[msg->arg1]);
        if (lua_isfunction(L, -1) && (msg->ptr != NULL) && (msg->arg2 > 0)) {
            lua_pushinteger(L, msg->arg1);
            lua_pushlstring(L, msg->ptr, msg->arg2);
            lua_pushinteger(L, msg->arg2);
            lua_call(L, 3, 0);
        }
    }

    // 给rtos.recv方法返回个空数据
    lua_pushinteger(L, 0);

RET:
    if (msg->ptr != RT_NULL) {
        rt_free(msg->ptr);
    }
    return ret;
}

static void event_cb(LORA_RADIO_DEF dev, uint8_t *pucData, uint16_t usLen)
{
    rtos_msg_t msg;

    if ((usLen <= 0) || (pucData == RT_NULL)) {
        return;
    }

    uint8_t *pucTmp = rt_malloc(usLen);
    if (pucTmp == RT_NULL) {
        return;
    }

    rt_memcpy(pucTmp, pucData, usLen);

    msg.handler = msg_handler;
    msg.ptr = pucTmp;
    msg.arg1 = dev;
    msg.arg2 = usLen;
    luat_msgbus_put(&msg, 1);
}

static void OnTxDone1( void )
{
    Radio[LORA_DEV_1].Rx(LORA_DEV_1, 0); ///<启动接收
//    LLOGI("lora1 tx done");
}

static void OnRxDone1( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    Radio[LORA_DEV_1].Rx(LORA_DEV_1, 0); ///<启动接收
//    BufferSize = size;
//    rt_memcpy( Buffer, payload, BufferSize );
//    LLOGI("lora1 recv %d bytes at rssi %d", size, rssi);
//    dump(payload, size);

    event_cb(LORA_DEV_1, payload, size);
}

static void OnTxTimeout1( void )
{
    Radio[LORA_DEV_1].Rx(LORA_DEV_1, 0); ///<启动接收
}

static void OnRxTimeout1( void )
{
    Radio[LORA_DEV_1].Rx(LORA_DEV_1, 0); ///<启动接收
}

static void OnRxError1( void )
{
    Radio[LORA_DEV_1].Rx(LORA_DEV_1, 0); ///<启动接收
}


static void OnTxDone2( void )
{
    Radio[LORA_DEV_2].Rx(LORA_DEV_2, 0); ///<启动接收
//    LLOGI("lora2 tx done");
}

static void OnRxDone2( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    Radio[LORA_DEV_2].Rx(LORA_DEV_2, 0); ///<启动接收
//    BufferSize = size;
//    rt_memcpy( Buffer, payload, BufferSize );

    LLOGI("lora2 recv %d bytes at rssi %d", size, rssi);
//    dump(payload, size);
    event_cb(LORA_DEV_2, payload, size);
}

static void OnTxTimeout2( void )
{
    Radio[LORA_DEV_2].Rx(LORA_DEV_2, 0); ///<启动接收
}

static void OnRxTimeout2( void )
{
    Radio[LORA_DEV_2].Rx(LORA_DEV_2, 0); ///<启动接收
}

static void OnRxError2( void )
{
    Radio[LORA_DEV_2].Rx(LORA_DEV_2, 0); ///<启动接收
}

static int l_lora_send(lua_State *L)
{
    int freeFlag = 0;
    char* send_buff = NULL;
    size_t send_length = 0;
    LORA_RADIO_DEF dev = (LORA_RADIO_DEF)luaL_checkinteger(L, 1);

    if (!((dev == LORA_DEV_1) || (dev == LORA_DEV_2))) {
        lua_pushinteger(L, 0);
        return 1;
    }

    if (lua_istable(L, 2)) {
        send_length = luaL_len(L, 2);
        send_buff = rt_malloc(send_length);
        freeFlag = 1;
        for (int i = 1; i <= send_length; ++i) {
            lua_rawgeti(L, 2, i);
            send_buff[i - 1] = (uint8_t)lua_tointeger(L, -1);
            lua_pop(L, 1);
        }
    } else {
        send_buff = (char *)lua_tolstring(L, 2, &send_length);
    }

//    LLOGI("lora send %d bytes", send_length);
//    dump(send_buff, send_length);



//    Radio[dev].Standby(dev);

//    Radio[dev].SetChannel( dev, lora_config[dev].freq );

//    /* default private syncword for p2p test */
//    Radio[dev].SetPublicNetwork( dev, false);

//    Radio[dev].SetTxConfig( dev, MODEM_LORA, 20, 0,
//                                   lora_config[dev].bw, lora_config[dev].sf, 2,
//                                   lora_config[dev].prem_len,
//                                   false, ///< fixLen
//                                   true, ///< crcOn
//                                   0, ///< freqHopOn
//                                   0, ///< hopPeriod
//                                   0, ///< iqInverted
//                                   3000////< timeout
//                            );


    Radio[dev].Send(dev, (uint8_t *)send_buff, (uint8_t)send_length);

    lua_pushinteger(L, send_length);

    if (freeFlag) {
        rt_free(send_buff);
    }

    return 1;
}

static int l_lora_get_config(lua_State *L)
{
    LORA_RADIO_DEF dev = (LORA_RADIO_DEF)luaL_checkinteger(L, 1);

    if (!((dev == LORA_DEV_1) || (dev == LORA_DEV_2))) {
        lua_pushinteger(L, 0);
        return 1;
    }

    lua_pushinteger(L, cb_lora_config[dev].freq);
    lua_pushinteger(L, cb_lora_config[dev].sf);
    lua_pushinteger(L, cb_lora_config[dev].bw);
    lua_pushinteger(L, cb_lora_config[dev].prem_len);
    lua_pushboolean(L, cb_lora_config[dev].lowDatarateOptimize);

    return 4;
}

static int l_lora_setup(lua_State *L)
{
    LORA_RADIO_DEF dev = (LORA_RADIO_DEF)luaL_checkinteger(L, 1);

    if (!((dev == LORA_DEV_1) || (dev == LORA_DEV_2))) {
        lua_pushinteger(L, 0);
        return 1;
    }

    cb_lora_config[dev].freq = luaL_optinteger(L, 2, 440000000); ///< 频率
    cb_lora_config[dev].sf = luaL_optinteger(L, 3, LORA_SF_9); ///< 扩频因子
    cb_lora_config[dev].bw = luaL_optinteger(L, 4, LORA_BW_125K); ///< 带宽
    cb_lora_config[dev].prem_len = luaL_optinteger(L, 5, 0x15); ///< 前导码长度
    cb_lora_config[dev].lowDatarateOptimize = luaL_optinteger(L, 6, 1); ///< 低速率优化

    if (dev == LORA_DEV_1) {
        RadioEvents[dev].TxDone = OnTxDone1;
        RadioEvents[dev].RxDone = OnRxDone1;
        RadioEvents[dev].TxTimeout = OnTxTimeout1;
        RadioEvents[dev].RxTimeout = OnRxTimeout1;
        RadioEvents[dev].RxError = OnRxError1;
    } else if (dev == LORA_DEV_2) {
        RadioEvents[dev].TxDone = OnTxDone2;
        RadioEvents[dev].RxDone = OnRxDone2;
        RadioEvents[dev].TxTimeout = OnTxTimeout2;
        RadioEvents[dev].RxTimeout = OnRxTimeout2;
        RadioEvents[dev].RxError = OnRxError2;
    }

    if(Radio[dev].Init(dev, &RadioEvents[dev]))
    {
        LLOGI("lora radio init OK!\n");
    }
    else
    {
        LLOGI("lora radio init failed!\n");

        return false;
    }

    Radio[dev].Standby(dev);

    Radio[dev].SetChannel( dev, cb_lora_config[dev].freq );

    /* default private syncword for p2p test */
    Radio[dev].SetPublicNetwork( dev, false);

    Radio[dev].SetTxConfig( dev, MODEM_LORA, 22, 0,
                                   cb_lora_config[dev].bw, cb_lora_config[dev].sf, 2,
                                   cb_lora_config[dev].prem_len,
                                   false, ///< fixLen
                                   true, ///< crcOn
                                   0, ///< freqHopOn
                                   0, ///< hopPeriod
                                   0, ///< iqInverted
                                   3000,////< timeout
																	 cb_lora_config[dev].lowDatarateOptimize ? true : false);

    Radio[dev].SetRxConfig( dev, MODEM_LORA,
                                   cb_lora_config[dev].bw, cb_lora_config[dev].sf, 2,
                                   0, cb_lora_config[dev].prem_len,
                                   0, ///< symbTimeout
                                   false, ///< fixLen
                                   0, ///< payloadLen
                                   true, ///< crcOn
                                   0, ///< freqHopOn
                                   0, ///< hopPeriod
                                   0, ///< iqInverted
                                   true, ///< rxContinuous
																	 cb_lora_config[dev].lowDatarateOptimize ? true : false);

    Radio[dev].Rx(dev, 0); ///<启动接收
    Radio[dev].RxBoosted(dev, 0);

    lua_pushinteger(L, 1);
    return 1;
}

static int l_set_rx_cb(lua_State *L)
{
    LORA_RADIO_DEF dev = (LORA_RADIO_DEF)luaL_checkinteger(L, 1);

    if (!((dev == LORA_DEV_1) || (dev == LORA_DEV_2))) {
        lua_pushinteger(L, 0);
        return 1;
    }

    if (rx_callback[dev] != 0) {
        luaL_unref(L, LUA_REGISTRYINDEX, rx_callback[dev]);
        rx_callback[dev] = 0;
    }

    if (lua_isfunction(L, 2)) {
        lua_pushvalue(L, 2);
        rx_callback[dev] = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    return 0;
}

static int l_unset_rx_cb(lua_State *L)
{
    LORA_RADIO_DEF dev = (LORA_RADIO_DEF)luaL_checkinteger(L, 1);

    if (!((dev == LORA_DEV_1) || (dev == LORA_DEV_2))) {
        lua_pushinteger(L, 0);
        return 1;
    }

    if (rx_callback[dev] != 0) {
        luaL_unref(L, LUA_REGISTRYINDEX, rx_callback[dev]);
        rx_callback[dev] = 0;
    }

    return 0;
}
#include "mesh_port.h"
#include "mesh.h"

extern int mesh_init_test(void);
extern int l_mesh_send(lua_State *L);
extern int l_mesh_set_rx_cb(lua_State *L);
extern int radio_thread_init(void);
extern int user_thread_init(void);
extern lora_config_radio_t lora_config[2];
extern lm_mesh_flash_param_t	mesh_flash_param[LM_USING_RADIO_NUM];
extern volatile rt_uint8_t lora_radio_aes_flag;

//mesh lua fun
static int l_mesh_init_test(lua_State *L)
{
	LORA_RADIO_DEF dev = (LORA_RADIO_DEF)luaL_checkinteger(L, 1);
    int ret_mesh;
    int ret_radio;
    int ret_user;

    g_mesh_dbg_ready = 0;
    app_dbg_mark2(0x250, (rt_uint32_t)dev, 0);

    if (!((dev == LORA_DEV_1))) 
	{
        lua_pushinteger(L, 0);
        return 0;
    }
	
	lora_config[dev].freq = luaL_optinteger(L, 2, 433000000); ///< 频率
	mesh_flash_param[dev].factory = luaL_optinteger(L, 3, 0); ///< 厂商号
    mesh_flash_param[dev].network = luaL_optinteger(L, 4, 0); ///< 网络号
	mesh_flash_param[dev].node_id = luaL_optinteger(L, 5, 0); ///< mesh ID号
	
	lora_radio_aes_flag = luaL_optinteger(L, 6, 0); ///< mesh ID号
	
	ret_mesh = mesh_init_test();
    app_dbg_mark2(0x251, (rt_uint32_t)ret_mesh, 0);
	
	ret_radio = radio_thread_init();
    app_dbg_mark2(0x252, (rt_uint32_t)ret_radio, 0);
	
	ret_user = user_thread_init();
    app_dbg_mark2(0x253, (rt_uint32_t)ret_user, 0);

    if ((ret_mesh == RT_EOK) && (ret_radio == RT_EOK) && (ret_user == RT_EOK)) {
        g_mesh_dbg_ready = 1;
        app_dbg_mark2(0x260, (rt_uint32_t)dev, mesh_flash_param[dev].node_id);
    }
	
	lua_pushinteger(L, 1);
	
	
    return 1;
}





#include "rotable2.h"
static const rotable_Reg_t reg_lora[] =
{
    { "setup",      		ROREG_FUNC(l_lora_setup)},
    { "send",      		ROREG_FUNC(l_lora_send)},
    { "get_config",      ROREG_FUNC(l_lora_get_config)},
    { "set_rx_cb",       ROREG_FUNC(l_set_rx_cb)},
    { "unset_rx_cb",     ROREG_FUNC(l_unset_rx_cb)},
	{ "mesh_init",			ROREG_FUNC(l_mesh_init_test)},
	{ "mesh_send",			ROREG_FUNC(l_mesh_send)},
	{ "mesh_set_rx_cb",		ROREG_FUNC(l_mesh_set_rx_cb)},

    { "DEV1",        		ROREG_INT(LORA_DEV_1)},
    { "DEV2",        		ROREG_INT(LORA_DEV_2)},

    { "BW_125K",        	ROREG_INT(LORA_BW_125K)},
    { "BW_250K",        	ROREG_INT(LORA_BW_250K)},
    { "BW_500K",        	ROREG_INT(LORA_BW_500K)},

    { "SF6",        		ROREG_INT(LORA_SF_6)},
    { "SF7",        		ROREG_INT(LORA_SF_7)},
    { "SF8",        		ROREG_INT(LORA_SF_8)},
    { "SF9",        		ROREG_INT(LORA_SF_9)},
    { "SF10",        		ROREG_INT(LORA_SF_10)},
    { "SF11",        		ROREG_INT(LORA_SF_11)},
    { "SF12",        		ROREG_INT(LORA_SF_12)},

    { NULL,         		ROREG_INT(0) }
};

LUAMOD_API int luaopen_lora(lua_State *L)
{
    luat_newlib2(L, reg_lora);
    return 1;
}
