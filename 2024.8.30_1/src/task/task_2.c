/*
 * task_2.c
 *
 * created: 2024/3/3
 *  author:
 */

#include "lvgl_ui.h"
#include "task_2.h"
#include "lvgl-7.0.1/lvgl.h"
#include "led.h"
#include "beep.h"
#include "key.h"
#include "matrix.h"
#include "ShowData.h"
#include "main.h"
//static rt_thread_t RW2_thread = NULL;
static lv_obj_t *pwd_ta = NULL;
static lv_obj_t *kb;
static lv_obj_t *tile1  = NULL;
extern rt_thread_t rt_thread_dom;
static char open_dz = 0;
static void event_task_esc(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_SHORT_CLICKED)
    {                                          /* 对象被点击 */

        Taskshutdown = 0;
    }
}

static void RW2_thread1(void)
{
    rt_thread_delay(1000); // 让界面先加载出来

    for (;;)
    {
        rt_thread_delay(100);
        if (Taskshutdown != 2)
        {
            rt_mutex_take(ui_mutex, RT_WAITING_FOREVER);
            lv_obj_set_hidden(main_create, false); // 显示列表
            lv_obj_del_async(tile1);
            rt_mutex_release(ui_mutex);
            return ;
        }
    }
}
LV_FONT_DECLARE(myFont1);
LV_FONT_DECLARE(myFont2);
void task_2(void)
{
    tile1 = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(tile1, LV_HOR_RES, LV_VER_RES);


    lv_obj_t *btn1 = lv_btn_create(tile1, NULL);
    lv_obj_set_event_cb(btn1, event_task_esc);
    lv_obj_align(btn1, tile1, LV_ALIGN_IN_TOP_LEFT, 5, 5);
    lv_obj_t *label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, "Esc");

    label = lv_label_create(tile1, NULL);
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_text_font(label, LV_CHART_PART_BG, LV_STATE_DEFAULT, &myFont1);
    lv_label_set_text(label, "37.8""\xC2\xB0"/*°*/"");

    label = lv_label_create(tile1, NULL);
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 50);
    lv_obj_set_style_local_text_font(label, LV_CHART_PART_BG, LV_STATE_DEFAULT, &myFont2);
    lv_label_set_text(label, "37.8""\xC2\xB0"/*°*/"");

    Taskshutdown = 2;
    rt_mb_send_wait(task1_thread_mailbox, RW2_thread1,3000);

}
