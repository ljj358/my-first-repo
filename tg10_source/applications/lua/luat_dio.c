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

#include "drv/drv_dio.h"
#include "beep.h"

#define LUAT_LOG_TAG "dio"
#include "luat_log.h"

/*
 * 读取DI通道状态
@api    dio.get_di(chnl)
@int DI通道号, 1~4
@return int 高电平为1，低电平为0，其他值出错
@usage
dio.get_di(1)
*/
static int l_dio_get_di(lua_State *L)
{
    int chnl = luaL_checkinteger(L, 1);

    if (!((chnl >= 1) && (chnl <= 4))) {
        lua_pushinteger(L, -1);
        return 1;
    }

    int result = dioChannelGet(DIO_CHNL_DI1 + (chnl - 1));
    lua_pushinteger(L, result);

    return 1;
}

/*
 * 读取DO通道状态
@api    dio.get_do(chnl)
@int DO通道号, 1~4
@return int 高电平为1，低电平为0，其他值出错
@usage
dio.get_do(1)
*/
static int l_dio_get_do(lua_State *L)
{
    int chnl = luaL_checkinteger(L, 1);

    if (!((chnl >= 1) && (chnl <= 4))) {
        lua_pushinteger(L, -1);
        return 1;
    }

    int result = dioChannelGet(DIO_CHNL_DO1 + (chnl - 1));
    lua_pushinteger(L, result);

    return 1;
}

/*
 * 设置DO通道状态
@api    dio.set_do(chnl)
@int DO通道号, 1~4
@int DO状态, 高电平为1，低电平为0，其他值出错
@return int 正常为0，其他值出错
@usage
dio.set_do(1, 0)
*/
static int l_dio_set_do(lua_State *L)
{
    int chnl = luaL_checkinteger(L, 1);

    if (!((chnl >= 1) && (chnl <= 4))) {
        lua_pushinteger(L, -1);
        return 1;
    }

    int sta = luaL_checkinteger(L, 2);

    if (!((sta == 0) || (sta == 1))) {
        lua_pushinteger(L, -1);
        return 1;
    }

    dioChannelSet(DIO_CHNL_DO1 + (chnl - 1), sta);
    lua_pushinteger(L, 0);

    return 1;
}

static uint32_t _gaulPwmPeriod[4];
static uint32_t _gaulPwmPulse[4];
/*
 * 设置DO通道状态
@api    dio.set_pwm(chnl, duty, freq)
@int DO通道号, 1~4
@int PWM占空比，0~100
@int PWM频率，100~100000，可选值
@return int 正常为0，其他值出错
@usage
dio.set_pwm(1, 0)
*/
static int l_dio_set_pwm(lua_State *L)
{
    int chnl = luaL_checkinteger(L, 1);

    if (!((chnl >= 1) && (chnl <= 4))) {
        lua_pushinteger(L, -1);
        return 1;
    }

    int duty = luaL_checkinteger(L, 2);

    if (!((duty >= 0) && (duty <= 100))) {
        lua_pushinteger(L, -1);
        return 1;
    }

    int freq = luaL_optinteger(L, 3, -1);

    if ((freq >= 100) && (freq <= 100000)) {
        _gaulPwmPeriod[chnl - 1] = freq;
    } else {
        freq = _gaulPwmPeriod[chnl - 1];
    }

    int period = 1000000 / freq;

    dioPwmSet(DIO_CHNL_DO1 + (chnl - 1), period, duty * period / 100);

    lua_pushinteger(L, 0);

    return 1;
}

/*
 * 读取继电器通道状态
@api    dio.get_relay(chnl)
@int 继电器通道号, 1~2
@return int 吸合为1，断开为0，其他值出错
@usage
dio.get_relay(1)
*/
static int l_dio_get_relay(lua_State *L)
{
    int chnl = luaL_checkinteger(L, 1);

    if (!((chnl >= 1) && (chnl <= 4))) {
        lua_pushinteger(L, -1);
        return 1;
    }

    int result = dioChannelGet(DIO_CHNL_RL1 + (chnl - 1));
    lua_pushinteger(L, result);

    return 1;
}

/*
 * 设置继电器通道状态
@api    dio.set_relay(chnl)
@int 继电器通道号, 1~2
@int 继电器状态, 吸合为1，断开为0，其他值出错
@return int 正常为0，其他值出错
@usage
dio.set_relay(1, 0)
*/
static int l_dio_set_relay(lua_State *L)
{
    int chnl = luaL_checkinteger(L, 1);

    if (!((chnl >= 1) && (chnl <= 4))) {
        lua_pushinteger(L, -1);
        return 1;
    }

    int sta = luaL_checkinteger(L, 2);

    if (!((sta == 0) || (sta == 1))) {
        lua_pushinteger(L, -1);
        return 1;
    }

    dioChannelSet(DIO_CHNL_RL1 + (chnl - 1), sta);
    lua_pushinteger(L, 0);

    return 1;
}

/*
 * 蜂鸣器发声
@api    dio.beep(period, duty, count)
@int 周期, 10~10000，单位ms
@int 占空比, 1~100，不填为100
@int 重复次数, 不填为1
@return int 正常为0，其他值出错
@usage
dio.beep(1000, 30, 3)
*/
static int l_dio_beep(lua_State *L)
{
    int period = luaL_checkinteger(L, 1);

    if (!((period >= 10) && (period <= 10000))) {
        lua_pushinteger(L, -1);
        return 1;
    }

    int duty = luaL_optinteger(L, 2, 100);

    if (!((duty >= 1) && (duty <= 100))) {
        lua_pushinteger(L, -1);
        return 1;
    }

    int count = luaL_optinteger(L, 3, 1);

    if (!((count >= 1) && (count <= 100))) {
        lua_pushinteger(L, -1);
        return 1;
    }

    beep(count, period, duty, 0);
    lua_pushinteger(L, 0);

    return 1;
}

#include "rotable2.h"
static const rotable_Reg_t reg_dio[] =
{
    { "get_di",      ROREG_FUNC(l_dio_get_di)},
    { "get_do",      ROREG_FUNC(l_dio_get_do)},
    { "set_do",      ROREG_FUNC(l_dio_set_do)},
    { "set_pwm",      ROREG_FUNC(l_dio_set_pwm)},
    { "set_relay",       ROREG_FUNC(l_dio_set_relay)},
    { "get_relay",       ROREG_FUNC(l_dio_get_relay)},
    { "beep",         ROREG_FUNC(l_dio_beep)},

    { NULL,         ROREG_INT(0) }
};

LUAMOD_API int luaopen_dio(lua_State *L)
{
    luat_newlib2(L, reg_dio);
    return 1;
}
