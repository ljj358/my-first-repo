/*
 * task_8.c
 *
 * created: 2024/3/3
 *  author:
 */

#include "lvgl_ui.h"
#include "task_8.h"
#include "lvgl-7.0.1/lvgl.h"
#include "main.h"
//static rt_thread_t RW8_thread = NULL;
static lv_obj_t *tile1 = NULL;
static void event_task_esc(lv_obj_t *obj, lv_event_t event)
{

        if (event == LV_EVENT_SHORT_CLICKED)
        { /* 对象被点击 */

            Taskshutdown = 0;
        }
}

static void RW8_thread1(void *arg)
{
    rt_thread_delay(1000);
    int i = 1;
    for(;i<8;i++)
     BEEP_music(i,1000);
    for (;;)
    {

        rt_thread_delay(999999);
    }
}


void task_8(void){
        tile1 = lv_obj_create(lv_scr_act(), NULL);
        lv_obj_set_size(tile1, LV_HOR_RES, LV_VER_RES);

        lv_obj_t *label;
        lv_obj_t *btn1 = lv_btn_create(tile1, NULL);
        lv_obj_set_event_cb(btn1, event_task_esc);
        lv_obj_align(btn1, tile1, LV_ALIGN_IN_TOP_LEFT, 5, 5);
        label = lv_label_create(btn1, NULL);
        lv_label_set_text(label, "Esc");

        Taskshutdown = 7;

        rt_mb_send_wait(task1_thread_mailbox, RW8_thread1,3000);
}

