# LED闪烁例程

## 简介

本例程主要功能是让板载的 RGB-LED 中的蓝色 LED 不间断闪烁。
这个例程也可以做为您的创作的基础工程。

## 硬件说明

| 名称     | 引脚     | 定时器 | 通道    |说明    |
| ------- | ------- |------- |------- |------- |
|DI1      | PA9     | TIM1   | CH2    | ENC_A  |
|DI2      | PE9     | TIM1   | CH1    | ENC_B  | 
|DI3      | PB6     | TIM4   | CH1    | ENC_A  |
|DI4      | PB7     | TIM4   | CH2    | ENC_B  |
|DO1      | PE5     | TIM15  | CH1    |        |
|DO2      | PB9     | TIM17  | CH1    |        | 
|DO3      | PB1     | TIM3   | CH4    |        |
|DO4      | PA5     | TIM2   | CH1    |        |


## 软件说明

闪灯的源代码位于 `/projects/art_pi_blink_led/applications/main.c` 中。首先定义了一个宏 `LED_PIN` ，代表闪灯的 LED 引脚编号，然后与 `GPIO_LED_B`（**PI8**）对应：

```
#define LED_PIN GET_PIN(I, 8)
```

在 main 函数中，将该引脚配置为输出模式，并在下面的 while 循环中，周期性（500毫秒）开关 LED。

```
int main(void)
{
    rt_uint32_t count = 1;

    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);

    while(count++)
    {
        rt_thread_mdelay(500);
        rt_pin_write(LED_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED_PIN, PIN_LOW);
    }
    return RT_EOK;
}
```



## 运行
### 编译&下载

编译完成后，将开发板的 ST-Link USB 口与 PC 机连接，然后将固件下载至开发板。

### 运行效果

正常运行后，蓝色 LED 会周期性闪烁。

## 注意事项

如果想要修改`LED_PIN` 宏定义，可以通过 GET_PIN 来修改。

