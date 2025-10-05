/*
 * task_4.c
 *
 * created: 2024/3/3
 *  author:
 */

#include "lvgl_ui.h"
#include "task_4.h"
#include "lvgl-7.0.1/lvgl.h"
#include "Quest_BH1750.h"
#include "beep.h"
#include "pwm_ic.h"
#include "led.h"
#include "Ultrasonic.h"
#include "main.h"
#include "paj7620u2.h"

//static rt_thread_t RW4_thread = NULL;
//static rt_thread_t RW4_thread_2 = NULL;
static lv_obj_t *tile1 = NULL;

static lv_obj_t *label1 = NULL;
extern char RW2_open;
static void event_task_esc(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_SHORT_CLICKED)
    { /* 对象被点击 */
        Taskshutdown = 0;
    }
}
static unsigned short data = 0;
static void RW4_thread1(void *arg)
{

    voice_rouse();
    rt_thread_delay(1000); // 等待语音模块被唤醒
    PAJ7620_Init();
    GestureData gesture;

    for (;;)
    {
        //手势识别
		gesture.detect = GS_Read_nByte(PAJ_GET_INT_FLAG1,2,&gesture.data[0]);//读取手势状态
		if(!gesture.detect)
		{
			gesture.type =(unsigned short)gesture.data[1]<<8 | gesture.data[0];
			if(gesture.type)
			{
				switch(gesture.type)
				{
					case GES_UP:             voice_broadcast(1);   lv_label_set_text(label1,"""\xE5\xBD\x93"/*当*/"""\xE5\x89\x8D"/*前*/"""\xE6\x89\x8B"/*手*/"""\xE5\x8A\xBF"/*势*/":""\xE5\x90\x91"/*向*/"""\xE4\xB8\x8A"/*上*/"");break; //向上
					case GES_DOWN:           voice_broadcast(2);   lv_label_set_text(label1,"""\xE5\xBD\x93"/*当*/"""\xE5\x89\x8D"/*前*/"""\xE6\x89\x8B"/*手*/"""\xE5\x8A\xBF"/*势*/":""\xE5\x90\x91"/*向*/"""\xE4\xB8\x8B"/*下*/"");break; //向下
					case GES_LEFT:           voice_broadcast(3);   lv_label_set_text(label1,"""\xE5\xBD\x93"/*当*/"""\xE5\x89\x8D"/*前*/"""\xE6\x89\x8B"/*手*/"""\xE5\x8A\xBF"/*势*/":""\xE5\x90\x91"/*向*/"""\xE5\xB7\xA6"/*左*/"");break; //向左
					case GES_RIGHT:          voice_broadcast(4);   lv_label_set_text(label1,"""\xE5\xBD\x93"/*当*/"""\xE5\x89\x8D"/*前*/"""\xE6\x89\x8B"/*手*/"""\xE5\x8A\xBF"/*势*/":""\xE5\x90\x91"/*向*/"""\xE5\x8F\xB3"/*右*/"");break; //向右
					case GES_FORWARD:        voice_broadcast(5);   lv_label_set_text(label1,"""\xE5\xBD\x93"/*当*/"""\xE5\x89\x8D"/*前*/"""\xE6\x89\x8B"/*手*/"""\xE5\x8A\xBF"/*势*/":""\xE5\x90\x91"/*向*/"""\xE5\x89\x8D"/*前*/"");break; //向前
					case GES_BACKWARD:       voice_broadcast(6);   lv_label_set_text(label1,"""\xE5\xBD\x93"/*当*/"""\xE5\x89\x8D"/*前*/"""\xE6\x89\x8B"/*手*/"""\xE5\x8A\xBF"/*势*/":""\xE5\x90\x91"/*向*/"""\xE5\x90\x8E"/*后*/"");break; //向后
					case GES_CLOCKWISE:      voice_broadcast(7);   lv_label_set_text(label1,"""\xE5\xBD\x93"/*当*/"""\xE5\x89\x8D"/*前*/"""\xE6\x89\x8B"/*手*/"""\xE5\x8A\xBF"/*势*/":""\xE9\xA1\xBA"/*顺*/"""\xE6\x97\xB6"/*时*/"""\xE9\x92\x88"/*针*/"");break; //顺时针
					case GES_ANTI_CLOCKWISE: voice_broadcast(8);   lv_label_set_text(label1,"""\xE5\xBD\x93"/*当*/"""\xE5\x89\x8D"/*前*/"""\xE6\x89\x8B"/*手*/"""\xE5\x8A\xBF"/*势*/":""\xE9\x80\x86"/*逆*/"""\xE6\x97\xB6"/*时*/"""\xE9\x92\x88"/*针*/"");break; //逆时针
					case GES_WAVE:           voice_broadcast(9);   lv_label_set_text(label1,"""\xE5\xBD\x93"/*当*/"""\xE5\x89\x8D"/*前*/"""\xE6\x89\x8B"/*手*/"""\xE5\x8A\xBF"/*势*/":""\xE6\x8C\xA5"/*挥*/"""\xE5\x8A\xA8"/*动*/"");break;   //挥动
				}
			}
		}
        if (Taskshutdown != 4)
        {
            RW2_open = 1;
            rt_mutex_take(ui_mutex, RT_WAITING_FOREVER);
            lv_obj_set_hidden(main_create, false); // 显示列表
            lv_obj_del_async(tile1);
            rt_mutex_release(ui_mutex);
            return ;

        }
        rt_thread_delay(130);
    }
}

void task_4(void)
{
    RW2_open= 0;
    voice_rouse();
    tile1 = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(tile1, LV_HOR_RES, 400);


    lv_obj_t *btn1 = lv_btn_create(tile1, NULL);
    lv_obj_set_event_cb(btn1, event_task_esc);
    lv_obj_align(btn1, tile1, LV_ALIGN_IN_TOP_LEFT, 5, 5);
    lv_obj_t *label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, "Esc");

    label1 = lv_label_create(tile1, NULL);
    lv_obj_set_style_local_text_font(label1, LV_CHART_PART_BG, LV_STATE_DEFAULT, &hz_simsun_3500_24);
    lv_label_set_text(label1, "");
    lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 0, -30);


    Taskshutdown = 4;
    rt_mb_send_wait(task1_thread_mailbox, RW4_thread1,3000);

}
