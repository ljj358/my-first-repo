/*
 * task_1.c
 *
 * created: 2024/3/3
 *  author:
 */

#include "lvgl_ui.h"
#include "task_1.h"
#include "lvgl-7.0.1/lvgl.h"
#include "matrix.h"
#include "ShowData.h"
#include "beep.h"
#include "led.h"
#include "main.h"


static lv_obj_t *tile1  = NULL;

static lv_obj_t *pwd_ta = NULL;
rt_tick_t TIM_data = 0;
static void event_task_esc(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_SHORT_CLICKED)
    { /* 对象被点击 */

    }
    Taskshutdown = 0;
}
extern char RW2_open;
static void RW1_thread1(void)
{
    LEDx_Set_Status(greenRGB_ON);
    rt_thread_delay(1000);
    LEDx_Set_Status(OFF);
    LEDx_Set_Status(purpleRGB);
    rt_thread_delay(1000);
    LEDx_Set_Status(OFF);
    LEDx_Set_Status(cyanRGB);
    rt_thread_delay(1000);
    LEDx_Set_Status(OFF);
    LEDx_Set_Status(whiteRGB);
    RW2_open = 0;
    rt_thread_delay(1000);
    LEDx_Set_Status(OFF);
    LEDx_Set_Status(yellowRGB);
    rt_thread_delay(1000);
    LEDx_Set_Status(OFF);

    BEEP_music(5, 1000);
    BEEP_music(5, 1000);
    BEEP_music(3, 1000);
    BEEP_music(2, 1000);
    BEEP_music(1, 1000);
    BEEP_music(1, 1000);
    BEEP_music(2, 1000);
    BEEP_music(3, 1000);
    BEEP_music(5, 1000);
    BEEP_music(5, 1000);
    RW2_open = 1;
    for (;;)
    {
        rt_thread_delay(2);
        if (Taskshutdown != 1)
        {
            rt_mutex_take(ui_mutex, RT_WAITING_FOREVER);
            lv_obj_set_hidden(main_create, false); // 显示列表
            lv_obj_del_async(tile1);
            rt_mutex_release(ui_mutex);
            return ;
        }
    }
}



void task_1(void)
{
    LEDx_Set_Status(OFF);
    tile1 = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(tile1, LV_HOR_RES, 410);

    lv_obj_t *btn1 = lv_btn_create(tile1, NULL);
    lv_obj_set_event_cb(btn1, event_task_esc);
    lv_obj_align(btn1, tile1, LV_ALIGN_IN_TOP_LEFT, 5, 5);
    lv_obj_t *label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, "Esc");                                                 
    Taskshutdown = 1;
    rt_mb_send_wait(task1_thread_mailbox, RW1_thread1,3000);
}
