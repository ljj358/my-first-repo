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

#include "flashdb.h"
#include "drv_param.h"
#include "drv/drv_ain.h"

#define LUAT_LOG_TAG "env"
#include "luat_log.h"

extern char MyMcuID[30];

extern struct fdb_kvdb env_kvdb;

static int l_env_get(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);

    if (name == NULL) {
        return 0;
    }

    const char *result = fdb_kv_get(&env_kvdb, name);
    lua_pushstring(L, result);

    return 1;
}

static int l_env_del(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);

    if (name == NULL) {
        return 0;
    }

    int result = fdb_kv_del(&env_kvdb, name);
    lua_pushinteger(L, result);

    return 1;
}

static int l_env_set(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);

    if (name == NULL) {
        return 0;
    }

    const char *val = luaL_checkstring(L, 2);

    if (val == NULL) {
        return 0;
    }

    int result = fdb_kv_set(&env_kvdb, name, val);
    if (result != FDB_NO_ERR) {
        lua_pushinteger(L, 0);
    } else {
        lua_pushinteger(L, 1);
    }

    return result;
}

static void setBoolField (lua_State *L, const char *key, int value) {
  if (value < 0)  /* undefined? */
    return;  /* does not set field */
  lua_pushboolean(L, value);
  lua_setfield(L, -2, key);
}

static void setNumberField (lua_State *L, const char *key, float value) {
  lua_pushnumber(L, value);
  lua_setfield(L, -2, key);
}

static void setIntField (lua_State *L, const char *key, int value) {
  lua_pushinteger(L, value);
  lua_setfield(L, -2, key);
}

static void setStringField (lua_State *L, const char *key, const char *value) {
  lua_pushstring(L, value);
  lua_setfield(L, -2, key);
}

extern int _giModemRssi;
static int l_env_info(lua_State *L)
{
    lua_createtable(L, 0, 11);  /* 9 = number of fields */

    setStringField(L, "did", gptSysParam->acDevUid);

    setStringField(L, "hw_ver", gptSysParam->acHwVersion);
    setStringField(L, "hw_date", gptSysParam->acHwDate);
    setStringField(L, "boot_ver", gptSysParam->acBootVersion);
    setStringField(L, "fw_ver", gptSysParam->acFwVersion);

    setStringField(L, "modem_model", gptSysParam->acModemModel);
    setStringField(L, "imei", gptSysParam->acModemImei);
    setStringField(L, "iccid", gptSysParam->acSimCcid);
	setStringField(L, "mcuid", MyMcuID);

    setIntField(L, "run_time", rt_tick_get_millisecond() / 1000);
    setIntField(L, "boot_count", gptSysParam->ulBootCount);
    setNumberField(L, "board_temp", ainGetBoardTemp());
    setNumberField(L, "power_vol", ainGetPowerVoltage());

    setIntField(L, "modem_rssi", _giModemRssi);

    return 1;
}

#include "rotable2.h"
static const rotable_Reg_t reg_env[] =
{
    { "set",      ROREG_FUNC(l_env_set)},
    { "get",      ROREG_FUNC(l_env_get)},
    { "del",      ROREG_FUNC(l_env_del)},
    { "info",     ROREG_FUNC(l_env_info)},

    { NULL,         ROREG_INT(0) }
};

LUAMOD_API int luaopen_env(lua_State *L)
{
    luat_newlib2(L, reg_env);
    return 1;
}
