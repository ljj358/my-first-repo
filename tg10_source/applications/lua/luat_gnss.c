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
#include "app_bringup_diag.h"

#define LUAT_LOG_TAG "gnss"
#include "luat_log.h"

#include "stdio.h"
#include "stdlib.h"

#ifdef RT_USING_ULOG

    #define LOG_TAG              "lua_gps"
    #include <ulog.h>

#else

    #define LOG_I printf
    #define LOG_D printf
    #define LOG_W printf
    #define LOG_E printf

#endif

volatile uint16_t gusTimeOffset = 5000;
#define TIM_PERIOD_VALUE  50000U
#define TIM_OFFSET        gusTimeOffset

#define TIMESTAMP_2023_1_1_0_0_0	1672502400

volatile uint32_t	virtual_timestamp = TIMESTAMP_2023_1_1_0_0_0 + 10000;



#define PPS_PIN  GET_PIN(A, 10)

#define TEST_PIN GET_PIN(A, 6)

static lwgps_t gps_info;
static int pps_callback = 0;
static int sync_callback = 0;

extern volatile rt_uint32_t gulUtcStamp;
static uint32_t gucUnsyncCnt = 0;
static uint8_t gucTimerCorrectedFlag = 0;
static uint32_t _gulLastStampIncreaseTime = 0;

static TIM_HandleTypeDef htim2;

static volatile rt_uint8_t pps_flag = 0;

static int msg_handler(lua_State *L, void* ptr);

//#define LED0_PIN        GET_PIN(D,  1)

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{
  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */
	
//	rt_pin_mode(LED0_PIN,PIN_MODE_OUTPUT);
//	rt_pin_write(LED0_PIN,PIN_LOW);


  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 280000000 / TIM_PERIOD_VALUE - 1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = TIM_PERIOD_VALUE;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = TIM_OFFSET - 1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_1);
  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */

  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */

  /* USER CODE END TIM2_IRQn 1 */
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    uint32_t ulVal = TIM2->CNT;
    if (htim->Instance != TIM2) {
        return;
    }
	
	virtual_timestamp++;
	
    ++gucUnsyncCnt;
	
    pps_flag = 0;
//    printf("\r\nsync callback %d,%d\r\n", ulVal, rt_tick_get());

    if (gucUnsyncCnt > 3) {
        gucUnsyncCnt = 0;
        gucTimerCorrectedFlag = 0;
    }

    if (sync_callback != 0) {
        rtos_msg_t msg;
        msg.handler = msg_handler;
        msg.ptr = &sync_callback;
        msg.arg1 = gucTimerCorrectedFlag;
        msg.arg2 = 0;
        luat_msgbus_put(&msg, 1);
        // printf("luat callback %d", rt_tick_get());
    }
    if (!APP_SCRIPT_UPDATE_MODE && ulVal != 0) {
        rt_kprintf("\r\n-------------------------------[1]%d-------------------------------\r\n", ulVal);
    }
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    uint32_t ulVal = TIM2->CNT;
    if(pps_flag == 0)
    {
        pps_flag = 1;
        ++gulUtcStamp;
//        rt_kprintf("[3333333]timestamp = %ld \r\n",rt_tick_get());
    }
//    rt_kprintf("[55555]%d,timestamp = %ld \r\n", ulVal, rt_tick_get());
	
//	rt_kprintf("[cccc]timestamp = %ld \r\n",gulUtcStamp);

    if (!APP_SCRIPT_UPDATE_MODE && ulVal != 4999) {
        rt_kprintf("\r\n-------------------------------[2]%d-------------------------------\r\n", ulVal);
    }
}

void luat_gnss_pulse_callback(void)
{
    uint16_t usPeriod;
    uint16_t usCnt = TIM2->CNT;

    if (pps_flag == 0) {
        pps_flag = 1;
        ++gulUtcStamp;
    }

    TIM2->CNT = TIM_OFFSET;

    usPeriod = TIM2->ARR;

    if (usCnt < TIM_OFFSET - 500) {

    } else if (usCnt < TIM_OFFSET) {
        usPeriod = usPeriod - (((TIM_OFFSET - usCnt)) >> 1);
        TIM2->ARR = usPeriod;
    } else if (usCnt == TIM_OFFSET) {

    } else if (usCnt < TIM_OFFSET + 500) {
        usPeriod += ((usCnt - TIM_OFFSET) >> 1);
        TIM2->ARR = usPeriod;
    } /*else if (usCnt < (TIM1_PERIOD_VALUE - (500 - TIM_OFFSET))) {

    } else if (usCnt < (TIM1_PERIOD_VALUE)) {
        usPeriod -= (((TIM1_PERIOD_VALUE + TIM_OFFSET) - usCnt) >> 1);
        TIM1->ARRH = (usPeriod >> 8);
        TIM1->ARRL = (usPeriod & 0x00FF);
    }*/ else {

    }

    usPeriod = TIM2->ARR;

    if ((usCnt >= TIM_OFFSET - 2) && (usCnt <= TIM_OFFSET + 2)) {
        if (gucTimerCorrectedFlag < 10) {
            gucTimerCorrectedFlag++;
        }
    } else {
        gucTimerCorrectedFlag = 0;
    }

    gucUnsyncCnt = 0;

    if (!APP_SCRIPT_UPDATE_MODE && ((usCnt > 5001) || (usCnt < 4998))) {
        rt_kprintf("\r\n-------------------------------[3]%d-------------------------------\r\n", usCnt);
    }

    //    rt_kprintf("TIMER period:%u\r\n", usPeriod);
//    rt_kprintf("[444444]timestamp = %ld \r\n",rt_tick_get());
}

static int msg_handler(lua_State *L, void* ptr) {
    //LLOGD("l_uart_handler");
    rtos_msg_t* msg = (rtos_msg_t*)lua_topointer(L, -1);
    lua_pop(L, 1);

    int cb = *(int *)ptr;

    lua_geti(L, LUA_REGISTRYINDEX, cb);
    if (lua_isfunction(L, -1)) {
        if (cb == sync_callback) {
            lua_pushinteger(L, msg->arg1);
            lua_call(L, 1, 0);
        } else {
            lua_call(L, 0, 0);
        }
    }

    lua_pushinteger(L, 0);
    return 1;
}

extern void gnss_pulse_callback(void *arg);

static void pps_cb(void *arg)
{
    rtos_msg_t msg;

    gnss_pulse_callback(RT_NULL);

    if (pps_callback != 0) {
        msg.handler = msg_handler;
        msg.ptr = &pps_callback;
        msg.arg1 = 0;
        msg.arg2 = 0;
        luat_msgbus_put(&msg, 1);
    }
}

static void setboolfield (lua_State *L, const char *key, int value) {
  if (value < 0)  /* undefined? */
    return;  /* does not set field */
  lua_pushboolean(L, value);
  lua_setfield(L, -2, key);
}

static void setnumberfield (lua_State *L, const char *key, float value) {
  if (value < 0)  /* undefined? */
    return;  /* does not set field */
  lua_pushnumber(L, value);
  lua_setfield(L, -2, key);
}

static void setfield (lua_State *L, const char *key, int value) {
  lua_pushinteger(L, value);
  lua_setfield(L, -2, key);
}

/*
 * 读取GNSS位置信息
@api    gnss.get_location()
@return int     定位状态
        number  经度
        number  纬度
        number  高度
@usage
sta,lat,lon,alt = gnss.get_location()
*/
static int l_gnss_get_location(lua_State *L)
{
    lwgps2rtt_get_gps_info(&gps_info);

//    lua_pushinteger(L, gps_info.fix);
//    lua_pushnumber(L, gps_info.latitude);
//    lua_pushnumber(L, gps_info.longitude);
//    lua_pushnumber(L, gps_info.altitude);

    if (gps_info.fix != 1) {
        gps_info.latitude = -1;
        gps_info.longitude = -1;
        gps_info.altitude = -1;
    }


    lua_createtable(L, 0, 4);

    setfield(L, "sta", gps_info.fix);
    setnumberfield(L, "lat", gps_info.latitude);
    setnumberfield(L, "lon", gps_info.longitude);
    setnumberfield(L, "alt", gps_info.altitude);

    rt_kprintf("lon:%f,lat:%f\r\n",
               gps_info.longitude,
               gps_info.latitude);

    return 1;
}

/*
 * 读取GNSS日期时间信息
@api    gnss.get_datetime()
@return 元表
@usage
sta,lat,lon,alt = gnss.get_datetime()
*/
static int l_gnss_get_datetime(lua_State *L)
{
    lwgps2rtt_get_gps_info(&gps_info);

    lua_createtable(L, 0, 9);  /* 9 = number of fields */

    setfield(L, "sec", gps_info.seconds);
    setfield(L, "min", gps_info.minutes);
    setfield(L, "hour", gps_info.hours);
    setfield(L, "day", gps_info.date);
    setfield(L, "month", gps_info.month);
    setfield(L, "year", gps_info.year);
    setfield(L, "wday", 0);
    setfield(L, "yday", 0);
    setboolfield(L, "isdst", 0);

    return 1;
}

static int l_set_pps_cb(lua_State *L)
{
    if (pps_callback != 0) {
        luaL_unref(L, LUA_REGISTRYINDEX, pps_callback);
        pps_callback = 0;
    }

    if (lua_isfunction(L, 1)) {
        lua_pushvalue(L, 1);
        pps_callback = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    return 0;
}

static int l_unset_pps_cb(lua_State *L)
{
    if (pps_callback != 0) {
        luaL_unref(L, LUA_REGISTRYINDEX, pps_callback);
        pps_callback = 0;
    }

    return 0;
}

static int l_set_sync_offset(lua_State *L)
{
	TIM_OC_InitTypeDef sConfigOC = {0};
    int offset = luaL_checkinteger(L, 1);

    if (!((offset >= 0) && (offset <= 999))) {
        lua_pushboolean(L, 0);
        return 1;
    }

    // TIM_PERIOD_VALUE - (10 * TIM_PERIOD_VALUE / 1000);
    gusTimeOffset = (1000 - offset) * TIM_PERIOD_VALUE / 1000;
	
	sConfigOC.OCMode = TIM_OCMODE_TIMING;
	sConfigOC.Pulse = gusTimeOffset - 1;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
	{
		Error_Handler();
		LOG_E("[ERR]TIM2 OC SET ERROR\r\n");
	}
	/* USER CODE BEGIN TIM2_Init 2 */
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_1);
	
	
    

    lua_pushboolean(L, 1);

    return 1;
}


static int l_set_sync_offset1(uint8_t argc, char **argv)
{
	TIM_OC_InitTypeDef sConfigOC = {0};
    int offset = atoi(argv[1]);

    if (!((offset >= 0) && (offset <= 999))) {
		LOG_E("[ERR]TIM2 OC SET ERROR 1111111\r\n");
        return 1;
    }

    // TIM_PERIOD_VALUE - (10 * TIM_PERIOD_VALUE / 1000);
    gusTimeOffset = (1000 - offset) * TIM_PERIOD_VALUE / 1000;
	
	sConfigOC.OCMode = TIM_OCMODE_TIMING;
	sConfigOC.Pulse = gusTimeOffset - 1;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
	{
		LOG_E("[ERR]TIM2 OC SET ERROR 2222222\r\n");
		Error_Handler();
	}
	/* USER CODE BEGIN TIM2_Init 2 */
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_1);
	
	LOG_D("[OK]TIM2 OC SET OK\r\n");

    return 1;
}
MSH_CMD_EXPORT(l_set_sync_offset1, l_set_sync_offset1);

static int l_set_sync_cb(lua_State *L)
{
    if (sync_callback != 0) {
        luaL_unref(L, LUA_REGISTRYINDEX, sync_callback);
        sync_callback = 0;
    }

    if (lua_isfunction(L, 1)) {
        lua_pushvalue(L, 1);
        sync_callback = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    return 0;
}

static int l_unset_sync_cb(lua_State *L)
{
    if (sync_callback != 0) {
        luaL_unref(L, LUA_REGISTRYINDEX, sync_callback);
        sync_callback = 0;
    }

    return 0;
}

static int __gnssInit(void)
{
#if APP_BRINGUP_MIN_MESH_MODE
    rt_kprintf("minimal mesh mode: skip gnss init\r\n");
    return 0;
#else
    MX_TIM2_Init();

    rt_pin_mode(PPS_PIN, PIN_MODE_INPUT);

    rt_pin_attach_irq(PPS_PIN, PIN_IRQ_MODE_RISING, pps_cb, NULL);

    rt_pin_irq_enable(PPS_PIN, PIN_IRQ_ENABLE);
	
	return 0;
#endif
}
INIT_ENV_EXPORT(__gnssInit);

#include "rotable2.h"
static const rotable_Reg_t reg_gnss[] =
{
    { "get_location",      ROREG_FUNC(l_gnss_get_location)},
    { "get_datetime",      ROREG_FUNC(l_gnss_get_datetime)},

    { "set_pps_cb",        ROREG_FUNC(l_set_pps_cb)},
    { "unset_pps_cb",      ROREG_FUNC(l_unset_pps_cb)},

    { "set_sync_offset",        ROREG_FUNC(l_set_sync_offset)},
    { "set_sync_cb",        ROREG_FUNC(l_set_sync_cb)},
    { "unset_sync_cb",      ROREG_FUNC(l_unset_sync_cb)},

    { NULL,         ROREG_INT(0) }
};

LUAMOD_API int luaopen_gnss(lua_State *L)
{
    luat_newlib2(L, reg_gnss);
    return 1;
}
