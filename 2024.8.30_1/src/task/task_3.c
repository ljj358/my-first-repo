/*
 * task_3.c
 *
 * created: 2024/3/3
 *  author:
 */

#include "lvgl_ui.h"
#include "task_3.h"
#include "lvgl-7.0.1/lvgl.h"
#include "graphic_library.h"
#include "led.h"
#include "main.h"
#include  "DS1302.h"

static lv_obj_t *tile1 = NULL;
static void event_task_esc(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_SHORT_CLICKED)
    { /* 对象被点击 */
        Taskshutdown = 0;
    }
}

static void RW3_thread1(void)
{
    rt_thread_delay(1000);
    for (;;)
    {
        if (Taskshutdown != 3)
        {
            rt_mutex_take(ui_mutex, RT_WAITING_FOREVER);
            lv_obj_set_hidden(main_create, false); // 显示列表
            lv_obj_del_async(tile1);
            rt_mutex_release(ui_mutex);
            return ;
        }
        rt_thread_delay(1000);
    }
}


LV_FONT_DECLARE(font_32);
void task_3(void)
{
    tile1 = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(tile1, LV_HOR_RES, 400);

    lv_obj_set_style_local_bg_color(tile1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);

    lv_obj_t *btn1 = lv_btn_create(tile1, NULL);
    lv_obj_set_event_cb(btn1, event_task_esc);
    lv_obj_align(btn1, tile1, LV_ALIGN_IN_TOP_LEFT, 5, 5);
    lv_obj_t *label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, "Esc");

    Taskshutdown = 3;
    rt_mb_send_wait(task1_thread_mailbox, RW3_thread1,3000);
}

