#include "drv_ain.h"

#include <rtthread.h>
#include <rtdevice.h>

static const rt_uint32_t _gaulAdcChnl[] = {
    200 + 0,  ///< ADC2 channel 0
    100 + 4,  ///< ADC1 channel 4
    100 + 8,  ///< ADC1 channel 8
    100 + 9,  ///< ADC1 channel 9
    200 + 1,  ///< ADC2 channel 1
    100 + 19, ///< ADC1 channel 19
};

#define REFER_VOLTAGE       2.500f      /* 参考电压 3.3V,数据精度乘以100保留2位小数*/
#define CONVERT_BITS        (1 << 16)   /* 转换位数为16位 */


static int __userAdcInit(void)
{
    rt_err_t ret = RT_EOK;
    static rt_adc_device_t adc_dev;

    /* 查找设备 */
    adc_dev = (rt_adc_device_t)rt_device_find("adc1");
    if (adc_dev == RT_NULL)
    {
        rt_kprintf("adc sample run failed! can't find %s device!\n", "adc1");
        return RT_ERROR;
    }
    /* 使能设备 */
    ret = rt_adc_enable(adc_dev, 0);

    /* 查找设备 */
    adc_dev = (rt_adc_device_t)rt_device_find("adc2");
    if (adc_dev == RT_NULL)
    {
        rt_kprintf("adc sample run failed! can't find %s device!\n", "adc2");
        return RT_ERROR;
    }
    /* 使能设备 */
    ret = rt_adc_enable(adc_dev, 0);

    return ret;
}
INIT_ENV_EXPORT(__userAdcInit);

float ainGetVoltage(int chnnl)
{
    rt_adc_device_t adc_dev;
    char name[10];
    rt_uint32_t value;
    float vol;

    if (chnnl >= AIN_CHNL_MAX) {
        return 0;
    }

    chnnl = (int)_gaulAdcChnl[chnnl];

    rt_sprintf(name, "adc%x", chnnl / 100);

    chnnl %= 100;

    /* 查找设备 */
    adc_dev = (rt_adc_device_t)rt_device_find(name);
    if (adc_dev == RT_NULL)
    {
        //        rt_kprintf("adc sample run failed! can't find %s device!\n", ADC_DEV_NAME);
        return RT_ERROR;
    }
    /* 使能设备 */
//    ret = rt_adc_enable(adc_dev, chnnl);
    /* 读取采样值 */
    value = 0;
    for (int i = 0; i < 10; ++i) {
        value += rt_adc_read(adc_dev, chnnl);
        rt_thread_delay(2);
    }
    value /= 10;
//    rt_kprintf("the value is :%d \n", value);
    vol = value * REFER_VOLTAGE / CONVERT_BITS;
//    rt_kprintf("the voltage is :%.2f \n", vol);
    /* 关闭通道 */
//    ret = rt_adc_disable(adc_dev, chnnl);
    return vol;
}

static float getTemp(float Rt)
{
    const  float Rp = 10000.0;       ///< 10K NTC
    const  float T2 = (273.15+25.0); ///< 10K NTC
    const  float Bx = 3950.0;        ///< 10K NTC
    const  float Ka = 273.15;        ///< 10K NTC

    float Temp = 0.0f;
    Temp = Rt / Rp;
    Temp = log(Temp);
    Temp /=Bx;
    Temp += (1 / T2);
    Temp = 1 / Temp;
    Temp -= Ka;
    return Temp;
}

float ainGetPowerVoltage(void)
{
    return ainGetVoltage(AIN_CHNL_VIN) * 21;
}

float ainGetBoardTemp(void)
{
    float val;
    val = ainGetVoltage(AIN_CHNL_TEMP);
    val = (REFER_VOLTAGE - val) * 10000 / val;

    return getTemp(val);
}

#ifdef FINSH_USING_MSH
static int __ainGet(uint8_t argc, char **argv)
{
    int chnl;
    if (argc != 2) {
        rt_kprintf("example:ain_get 1\n");
        return 0;
    }

    chnl = atoi(argv[1]);

    float vol = ainGetVoltage(chnl - 1) * 6;

    rt_kprintf("voltage of channel %d:%.2fV\n", chnl, vol);

    return 0;
}
MSH_CMD_EXPORT_ALIAS(__ainGet, ain_get, get ain voltage);
#endif
