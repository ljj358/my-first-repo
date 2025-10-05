/*
 * task_7.c
 *
 * created: 2024/3/3
 *  author:
 */

#include "lvgl_ui.h"
#include "task_7.h"
#include "lvgl-7.0.1/lvgl.h"
#include "TLC5620.h"
#include "ls1x_i2c_bus.h"
#include "i2c/ads1015.h"
#include "main.h"
#include "led.h"
#include "rc522.h"
extern char RW2_open;
static lv_obj_t *tile1 = NULL;
//static rt_thread_t RW7_thread = NULL;
extern void *devPWM1;
static void event_task_esc(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_SHORT_CLICKED)
    { /* 对象被点击 */
        Taskshutdown = 0;

    }
}
static void RW7_thread1(void *arg)
{
    rt_thread_mdelay(1000);
    BEEP_music(1,500);
    BEEP_music(2,500);
    BEEP_music(3,500);
    BEEP_music(1,500);

    BEEP_music(2,500);
    BEEP_music(5,500);
    BEEP_music(5,500);
    rt_thread_delay(500);

    BEEP_music(3,500);
    BEEP_music(6,250);
    BEEP_music(6,250);
    BEEP_music(6,500);
    BEEP_music(6,500);

    BEEP_music(5,500);
    BEEP_music(5,500);
    BEEP_music(3,500);
    rt_thread_delay(500);

    BEEP_music(1,750);
    BEEP_music(1,250);
    BEEP_music(1,500);
    BEEP_music(6,250);
    BEEP_music(6,250);

    BEEP_music(5,500);
    BEEP_music(3,500);
    BEEP_music(5,500);
    rt_thread_delay(500);

    BEEP_music(2,250);
    BEEP_music(2,500);
    BEEP_music(3,250);
    BEEP_music(3,250);
    BEEP_music(2,250);
    BEEP_music(1,250);
    BEEP_music(3,250);

    BEEP_music(2,1000);
    BEEP_music(12,250);
    BEEP_music(13,250);
    BEEP_music(14,250);
    BEEP_music(2,250);

    BEEP_music(1,500);
    BEEP_music(2,500);
    BEEP_music(3,500);
    BEEP_music(1,500);

    BEEP_music(2,500);
    BEEP_music(5,250);
    BEEP_music(5,250);
    BEEP_music(5,500);
    rt_thread_delay(500);

    BEEP_music(3,500);
    BEEP_music(6,500);
    BEEP_music(6,500);
    BEEP_music(15,500);

    BEEP_music(7,500);
    BEEP_music(6,500);
    BEEP_music(5,500);
    rt_thread_delay(500);

    BEEP_music(1,750);
    BEEP_music(1,250);
    BEEP_music(1,500);
    BEEP_music(6,500);

    BEEP_music(5,500);
    BEEP_music(5,500);
    BEEP_music(3,500);
    rt_thread_delay(500);

    BEEP_music(2,250);
    BEEP_music(2,500);
    BEEP_music(3,250);
    BEEP_music(3,250);
    BEEP_music(2,250);
    BEEP_music(1,250);
    BEEP_music(2,250);
    BEEP_music(1,1000);
    for (;;)
    {
        rt_thread_mdelay(100);
        if(Taskshutdown != 7)
        {
            rt_mutex_take(ui_mutex, RT_WAITING_FOREVER);
            lv_obj_set_hidden(main_create, false); // 显示列表
            lv_obj_del_async(tile1);
            rt_mutex_release(ui_mutex);
            return ;
        }

    }
}

void task_7(void)
{
    RW2_open = 0;
    tile1 = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(tile1, LV_HOR_RES, 400);

    lv_obj_t *btn1 = lv_btn_create(tile1, NULL);
    lv_obj_set_event_cb(btn1, event_task_esc);
    lv_obj_align(btn1, tile1, LV_ALIGN_IN_TOP_LEFT, 5, 5);
    lv_obj_t *label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, "Esc");

    Taskshutdown = 7;

    rt_mb_send_wait(task1_thread_mailbox, RW7_thread1,3000);

   // DAC_TIM_init();
}
