#include "UI_MAX30102.h"
#include "lvgl_ui.h"
#include "lvgl-7.0.1/lvgl.h"
#include "main.h"
#include "algorithm.h"
#include "ls1b_gpio.h"
#include "MAX30102.h"
static lv_obj_t *tile1 = NULL;
static lv_obj_t *MAX30102_label;
static void event_task_esc(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_SHORT_CLICKED)
    { /* 对象被点击 */
        Taskshutdown = 0;
    }
}

static void MAX30102_thread1(void)
{
    rt_thread_delay(1000);
    max30102_init();
    unsigned char HR, SPO2;
    for (;;)
    {
        if (max30102_read_data(&HR, &SPO2) == 1)
        {

            rt_kprintf("HR:%3d SpO2:%3d\n ", HR, SPO2);
            rt_mutex_take(ui_mutex, RT_WAITING_FOREVER);
            lv_label_set_text_fmt(MAX30102_label, "MAX30102_HR:%d MAX30102_SpO2:%d", HR, SPO2);
            rt_mutex_release(ui_mutex);
        }
        else
            rt_kprintf("HR:0 SpO2:0\n ", HR, SPO2);

        if (Taskshutdown != 9)
        {
            rt_mutex_take(ui_mutex, RT_WAITING_FOREVER);
            lv_obj_set_hidden(main_create, false); // 显示列表
            lv_obj_del_async(tile1);
            rt_mutex_release(ui_mutex);
            return;
        }
    }
}


void task_MAX30102(void)
{
    tile1 = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(tile1, LV_HOR_RES, LV_VER_RES);

    lv_obj_set_style_local_bg_color(tile1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);

    lv_obj_t *btn1 = lv_btn_create(tile1, NULL);
    lv_obj_set_event_cb(btn1, event_task_esc);
    lv_obj_align(btn1, tile1, LV_ALIGN_IN_TOP_LEFT, 5, 5);
    lv_obj_t *label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, "Esc");

    MAX30102_label = lv_label_create(tile1, NULL);
    lv_obj_align(MAX30102_label, tile1, LV_ALIGN_CENTER, 5, 5);
    lv_label_set_text(MAX30102_label, "MAX30102_HR:0 MAX30102_SpO2:0");

    Taskshutdown = 9;
    rt_mb_send_wait(task1_thread_mailbox, MAX30102_thread1, 3000);
}
