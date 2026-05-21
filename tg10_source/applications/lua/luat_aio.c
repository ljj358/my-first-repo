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

#include "drv/drv_ain.h"

#define LUAT_LOG_TAG "ain"
#include "luat_log.h"

/*
 * 读取AI通道状态
@api    aio.get_ai(chnl)
@int AI通道号, 1~4
@return int 高电平为1，低电平为0，其他值出错
@usage
dio.get_di(1)
*/
static int l_aio_get_ai(lua_State *L)
{
    int chnl = luaL_checkinteger(L, 1);

    if (!((chnl >= AIN_CHNL_AIN1) && (chnl < AIN_CHNL_MAX))) {
        lua_pushinteger(L, -1);
        return 1;
    }

    int result = ainGetVoltage(AIN_CHNL_AIN1 + (chnl - 1));
    lua_pushinteger(L, result);

    return 1;
}

#include "rotable2.h"
static const rotable_Reg_t reg_aio[] =
{
    { "get_ai",      ROREG_FUNC(l_aio_get_ai)},

    { "AIN1",        ROREG_INT(AIN_CHNL_AIN1 + 1)},
    { "AIN2",        ROREG_INT(AIN_CHNL_AIN2 + 1)},
    { "AIN3",        ROREG_INT(AIN_CHNL_AIN3 + 1)},
    { "AIN4",        ROREG_INT(AIN_CHNL_AIN4 + 1)},
    { "VIN",        ROREG_INT(AIN_CHNL_VIN + 1)},
    { "TEMP",        ROREG_INT(AIN_CHNL_TEMP + 1)},

    { NULL,         ROREG_INT(0) }
};

LUAMOD_API int luaopen_aio(lua_State *L)
{
    luat_newlib2(L, reg_aio);
    return 1;
}
