/*
 * task_5.c
 *
 * created: 2024/3/3
 *  author:
 */

#include "lvgl_ui.h"
#include "task_5.h"
#include "lvgl-7.0.1/lvgl.h"
#include "led.h"
#include "rc522.h"
#include "beep.h"
#include "infrared.h"
#include "Ultrasonic.h"
#include "main.h"
#include  "DS1302.h"

//static rt_thread_t RW5_thread = NULL, RW5_thread_2 = NULL;
static lv_obj_t *tile1 = NULL;

static lv_obj_t *scale_label = NULL;
static lv_obj_t * btn2,*btn3,*btn4;
static char RW5 = 0;
static void event_task_esc(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_SHORT_CLICKED)
    { /* 对象被点击 */

        Taskshutdown = 0;
    }
}

static void RW5_thread1(void *arg)
{

    rt_thread_delay(1000); // 让界面先加载出来
    DS1302_Init();
    for (;;)
    {
        DS1302_ReadTime();
        rt_kprintf("20%d年%d月%d日 %d时%d分%d秒 星期%d\n",DS1302_Time[0],DS1302_Time[1],DS1302_Time[2],DS1302_Time[3],DS1302_Time[4],DS1302_Time[5],DS1302_Time[6]);
        lv_label_set_text_fmt(scale_label, "20%d""\xE5\xB9\xB4"/*年*/"%d""\xE6\x9C\x88"/*月*/"%d""\xE6\x97\xA5"/*日*/" %d""\xE6\x97\xB6"/*时*/"%d""\xE5\x88\x86"/*分*/"%d""\xE7\xA7\x92"/*秒*/" ""\xE6\x98\x9F"/*星*/"""\xE6\x9C\x9F"/*期*/"%d",DS1302_Time[0],DS1302_Time[1],DS1302_Time[2],DS1302_Time[3],DS1302_Time[4],DS1302_Time[5],DS1302_Time[6]);
        if (Taskshutdown != 5)
        {
            rt_mutex_take(ui_mutex, RT_WAITING_FOREVER);
            lv_obj_set_hidden(main_create, false); // 显示列表
            lv_obj_del_async(tile1);
            rt_mutex_release(ui_mutex);
            return ;
        }
        rt_thread_mdelay(1000);
    }
}

static void event_voice_bin(lv_obj_t *obj, lv_event_t event)
{

    if (event == LV_EVENT_SHORT_CLICKED)
    { /* 对象被点击 */
        if (obj == btn2) {
            voice_broadcast(0x0a);
        } else if (obj == btn3) {
            voice_broadcast(0x0b);
        } else if (obj == btn4) {
            voice_broadcast(0x0c);
        }
    }
}

void task_5(void)
{

    voice_rouse();
    tile1 = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(tile1, LV_HOR_RES, 400);

    lv_obj_t *btn1 = lv_btn_create(tile1, NULL);
    lv_obj_set_event_cb(btn1, event_task_esc);
    lv_obj_align(btn1, tile1, LV_ALIGN_IN_TOP_LEFT, 5, 5);
    lv_obj_t *label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, "Esc");

    scale_label = lv_label_create(tile1, NULL);
    lv_obj_set_style_local_text_font(scale_label, LV_CHART_PART_BG, LV_STATE_DEFAULT, &hz_simsun_3500_24);
    lv_label_set_text(scale_label, "");
    lv_obj_align(scale_label, NULL, LV_ALIGN_CENTER, -200, 0);


    btn2 = lv_btn_create(tile1, NULL);
    lv_obj_align(btn2, scale_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    label = lv_label_create(btn2, NULL);
    lv_obj_set_style_local_text_font(label, LV_CHART_PART_BG, LV_STATE_DEFAULT, &hz_simsun_3500_24);
    lv_label_set_text(label, """\xE8\xAF\x8D"/*词*/"""\xE6\x9D\xA1"/*条*/"1");
    lv_obj_set_event_cb(btn2, event_voice_bin);

    btn3 = lv_btn_create(tile1, NULL);
    lv_obj_align(btn3, btn2, LV_ALIGN_OUT_RIGHT_TOP, 50, 0);
    label = lv_label_create(btn3, NULL);
    lv_obj_set_style_local_text_font(label, LV_CHART_PART_BG, LV_STATE_DEFAULT, &hz_simsun_3500_24);
    lv_label_set_text(label, """\xE8\xAF\x8D"/*词*/"""\xE6\x9D\xA1"/*条*/"2");
    lv_obj_set_event_cb(btn3, event_voice_bin);

    btn4 = lv_btn_create(tile1, NULL);
    lv_obj_align(btn4,btn3, LV_ALIGN_OUT_RIGHT_TOP, 50, 0);
    label = lv_label_create(btn4, NULL);
    lv_obj_set_style_local_text_font(label, LV_CHART_PART_BG, LV_STATE_DEFAULT, &hz_simsun_3500_24);
    lv_label_set_text(label, """\xE8\xAF\x8D"/*词*/"""\xE6\x9D\xA1"/*条*/"3");
    lv_obj_set_event_cb(btn4, event_voice_bin);


    Taskshutdown = 5;


    rt_mb_send_wait(task1_thread_mailbox, RW5_thread1,3000);

}
