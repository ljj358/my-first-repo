/*
 * lvgl_ui.c
 *
 * created: 2021/10/30
 *  author:
 */
#include "lvgl-7.0.1/lvgl.h"
lv_style_t style;
#include "Quest_LED.h"
#include "steering_engine.h"
#include "electronic_scale.h"
#include "matrix_keyboard.h"
#include "Quest_RFID.h"
#include "Quest_BH1750.h"
#include "Combination_lock.h"
#include "computer.h"
#include "task_1.h"
#include "task_2.h"
#include "task_3.h"
#include "task_4.h"
#include "task_5.h"
#include "task_6.h"
#include "task_7.h"
#include "task_8.h"
#include "UI_dht11.h"
#include "UI_StepperMotor.h"
#include "UI_paj7620u2.h"
#include "UI_as608.h"
#include "Quest_dot_matrix.h"
#include "Quest_RFID.h"
#include "main.h"
#include "UI_MAX30102.h"
// 全局键盘对象指针
static lv_obj_t *keyboard = NULL;
lv_obj_t *main_create = NULL;
static void textarea_event_handler(lv_obj_t *ta, lv_event_t event);
static lv_obj_t *list_btn_rw1 = NULL, *list_btn_rw2 = NULL, *list_btn_rw3 = NULL, *list_btn_rw4 = NULL, *list_btn_rw5 = NULL, *list_btn_rw6 = NULL, *list_btn_rw7 = NULL, *list_btn_rw8 = NULL;

//static void my_event_cb(lv_obj_t *btn, lv_event_t event)
//{
    //if (event == LV_EVENT_CLICKED)
    //{
        //const char *btn_text = lv_list_get_btn_text(btn);
        //lv_obj_set_hidden(main_create, true); // 隐藏列表
        //if (strcmp(btn_text, "dht11") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, dht11_event_cb, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE8\xAE\xA1" /*计*/ ""
                                  //"\xE7\xAE\x97" /*算*/ ""
                                  //"\xE6\x9C\xBA" /*机*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, computer, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE4\xBB\xBB" /*任*/ ""
                                  //"\xE5\x8A\xA1" /*务*/ ""
                                  //"\xE4\xB8\x80" /*一*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, task_1, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE4\xBB\xBB" /*任*/ ""
                                  //"\xE5\x8A\xA1" /*务*/ ""
                                  //"\xE4\xBA\x8C" /*二*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, task_2, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE4\xBB\xBB" /*任*/ ""
                                  //"\xE5\x8A\xA1" /*务*/ ""
                                  //"\xE4\xB8\x89" /*三*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, task_3, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE4\xBB\xBB" /*任*/ ""
                                  //"\xE5\x8A\xA1" /*务*/ ""
                                  //"\xE5\x9B\x9B" /*四*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, task_4, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE4\xBB\xBB" /*任*/ ""
                                  //"\xE5\x8A\xA1" /*务*/ ""
                                  //"\xE4\xBA\x94" /*五*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, task_5, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE4\xBB\xBB" /*任*/ ""
                                  //"\xE5\x8A\xA1" /*务*/ ""
                                  //"\xE5\x85\xAD" /*六*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, task_6, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE4\xBB\xBB" /*任*/ ""
                                  //"\xE5\x8A\xA1" /*务*/ ""
                                  //"\xE4\xB8\x83" /*七*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, task_7, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE4\xBB\xBB" /*任*/ ""
                                  //"\xE5\x8A\xA1" /*务*/ ""
                                  //"\xE5\x85\xAB" /*八*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, task_8, 3000);
        //}
        //else if (strcmp(btn_text, "LED") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, Quest_LED, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE8\x88\xB5" /*舵*/ ""
                                  //"\xE6\x9C\xBA" /*机*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, Quest_steering_engine, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE7\x82\xB9" /*点*/ ""
                                  //"\xE9\x98\xB5" /*阵*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, Quest_dot_matrix, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE7\x94\xB5" /*电*/ ""
                                  //"\xE5\xAD\x90" /*子*/ ""
                                  //"\xE7\xA7\xA4" /*秤*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, electronic_scale, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE7\x9F\xA9" /*矩*/ ""
                                  //"\xE9\x98\xB5" /*阵*/ ""
                                  //"\xE9\x94\xAE" /*键*/ ""
                                  //"\xE7\x9B\x98" /*盘*/ "") == 0)
        //{
            //matrix_keyboard();
        //}
        //else if (strcmp(btn_text, "RFID") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, Quest_RFID, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE5\x85\x89" /*光*/ ""
                                  //"\xE7\x85\xA7" /*照*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, Quest_BH1750, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE5\xAF\x86" /*密*/ ""
                                  //"\xE7\xA0\x81" /*码*/ ""
                                  //"\xE9\x94\x81" /*锁*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, Combination_lock, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE6\xAD\xA5" /*步*/ ""
                                  //"\xE8\xBF\x9B" /*进*/ ""
                                  //"\xE7\x94\xB5" /*电*/ ""
                                  //"\xE6\x9C\xBA" /*机*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, UI_StepperMotor_event_cb, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE6\x89\x8B" /*手*/ ""
                                  //"\xE5\x8A\xBF" /*势*/ ""
                                  //"\xE6\xA3\x80" /*检*/ ""
                                  //"\xE6\xB5\x8B" /*测*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, UI_paj7620u2_event_cb, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE6\x8C\x87" /*指*/ ""
                                  //"\xE7\xBA\xB9" /*纹*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, UI_as608_event_cb, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE5\xBF\x83" /*心*/ ""
                                  //"\xE7\x8E\x87" /*率*/ "\\"
                                  //"\xE8\xA1\x80" /*血*/ ""
                                  //"\xE6\xB0\xA7" /*氧*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, task_MAX30102, 3000);
        //}
    //}
//}
extern rt_thread_t dom_thread;
static void event_open_esc(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_SHORT_CLICKED)
    { /* 对象被点击 */
        const char *btn_text = lv_list_get_btn_text(obj);
        if (strcmp(btn_text, ""
                             "\xE5\xBC\x80" /*开*/ ""
                             "\xE5\xA7\x8B" /*始*/ ""
                             "\xE9\x99\xA4" /*除*/ ""
                             "\xE5\xB0\x98" /*尘*/ "") == 0)
        {
            voice_broadcast(1);
            Stepper_motor_open();
        }
        else if (strcmp(btn_text, ""
                                  "\xE6\xBB\x91" /*滑*/ ""
                                  "\xE5\x8F\xB0" /*台*/ ""
                                  "\xE5\xA4\x8D" /*复*/ ""
                                  "\xE4\xBD\x8D" /*位*/ "") == 0)
        {
            Stepper_motor_reset();
        }
    }
}
extern char drivers_id;
extern lv_font_t myFont2;
void lv_ex_list_1(void)
{
    main_create = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(main_create, LV_HOR_RES, LV_VER_RES);
    // 设置边框不透明度为0
    lv_obj_set_style_local_border_opa(main_create, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    // 设置边框宽度为0
    lv_obj_set_style_local_border_width(main_create, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

    lv_obj_set_style_local_bg_color(main_create, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_bg_opa(main_create, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);


    //LV_IMG_DECLARE(img_1);
    //lv_obj_t *icon = lv_img_create(main_create, NULL);
    //lv_img_set_src(icon, &img_1);
    //lv_obj_set_style_local_bg_opa(icon, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100); // 设置颜色覆盖度100%，数值越低，颜色越透。

    lv_obj_t *btn1 = lv_btn_create(main_create, NULL);
    lv_obj_set_event_cb(btn1, event_open_esc);
    lv_obj_align(btn1, main_create, LV_ALIGN_CENTER, -150, -200);
    lv_obj_set_size(btn1, 400, 132);
    lv_obj_set_style_local_bg_color(btn1,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_RED);
    lv_obj_t *label = lv_label_create(btn1, NULL);
    lv_obj_set_style_local_text_font(label, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, &myFont2); // 给对象添加字体

    //lv_obj_set_style_local_text_color(label,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_RED);
    
    
    lv_label_set_text(label, ""
                             "\xE5\xBC\x80" /*开*/ ""
                             "\xE5\xA7\x8B" /*始*/ ""
                             "\xE9\x99\xA4" /*除*/ ""
                             "\xE5\xB0\x98" /*尘*/ "");



    btn1 = lv_btn_create(main_create, NULL);
    lv_obj_set_event_cb(btn1, event_open_esc);
    lv_obj_align(btn1, main_create, LV_ALIGN_CENTER, -150, 100);
    lv_obj_set_size(btn1, 400, 132);
    lv_obj_set_style_local_bg_color(btn1,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_RED);
    label = lv_label_create(btn1, NULL);
    lv_obj_set_style_local_text_font(label, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, &myFont2); // 给对象添加字体
    lv_label_set_text(label, ""
                             "\xE6\xBB\x91" /*滑*/ ""
                             "\xE5\x8F\xB0" /*台*/ ""
                             "\xE5\xA4\x8D" /*复*/ ""
                             "\xE4\xBD\x8D" /*位*/ "");

    label = lv_label_create(main_create, NULL);
    lv_obj_align(label, main_create, LV_ALIGN_IN_LEFT_MID, 80, -30);
    lv_obj_set_style_local_text_font(label, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, &myFont2); // 给对象添加字体
    lv_obj_set_style_local_text_color(label,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_BLACK);
    lv_label_set_text_fmt(label, ""
                                 "\xE8\xAE\xBE" /*设*/ ""
                                 "\xE5\xA4\x87" /*备*/ ""
                                 "\xE7\xBC\x96" /*编*/ ""
                                 "\xE5\x8F\xB7" /*号*/ ":%d "
                                 "\xE5\x8F\xB7" /*号*/ "",
                          drivers_id);

    // lv_obj_t *list1 = lv_list_create(main_create, NULL);
    // lv_obj_set_size(list1, 200, 480);

    // // 初始化样式
    // lv_style_init(&style);
    // // 设置文本字体
    // lv_style_set_text_font(&style, LV_STATE_DEFAULT, &hz_simsun_3500_24);
    // // 添加样式到列表
    // lv_obj_add_style(list1, LV_LABEL_PART_MAIN, &style);

    // lv_obj_align(list1, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 0);
    // // 初始化列表项样式
    // static lv_style_t style_btn;
    // lv_style_init(&style_btn);

    // // 设置背景透明
    // lv_style_set_bg_opa(&style_btn, LV_STATE_DEFAULT, LV_OPA_TRANSP);

    // // 设置边框不透明度为0
    // lv_style_set_border_opa(&style_btn, LV_STATE_DEFAULT, LV_OPA_TRANSP);

    // // 设置边框宽度为0
    // lv_style_set_border_width(&style_btn, LV_STATE_DEFAULT, 0);

    // // 将样式应用于列表项
    // lv_obj_add_style(list1, LV_LIST_PART_BG, &style_btn);
    // lv_obj_add_style(list1, LV_LIST_PART_EDGE_FLASH, &style_btn);
    // lv_obj_add_style(list1, LV_LIST_PART_SCROLLABLE, &style_btn);
    // // 将样式应用于列表的滚动条
    // lv_obj_add_style(list1, LV_LIST_PART_SCROLLBAR, &style_btn);

    // /*Add buttons to the list*/
    // lv_obj_t *list_btn;

    // list_btn_rw1 = lv_list_add_btn(list1, NULL, ""
    //                                             "\xE4\xBB\xBB" /*任*/ ""
    //                                             "\xE5\x8A\xA1" /*务*/ ""
    //                                             "\xE4\xB8\x80" /*一*/ "");
    // lv_obj_set_event_cb(list_btn_rw1, my_event_cb);
    // lv_obj_add_style(list_btn_rw1, LV_BTN_PART_MAIN, &style_btn);

    // list_btn_rw2 = lv_list_add_btn(list1, NULL, ""
    //                                             "\xE4\xBB\xBB" /*任*/ ""
    //                                             "\xE5\x8A\xA1" /*务*/ ""
    //                                             "\xE4\xBA\x8C" /*二*/ "");
    // lv_obj_set_event_cb(list_btn_rw2, my_event_cb);
    // lv_obj_add_style(list_btn_rw2, LV_BTN_PART_MAIN, &style_btn);

    // list_btn_rw3 = lv_list_add_btn(list1, NULL, ""
    //                                             "\xE4\xBB\xBB" /*任*/ ""
    //                                             "\xE5\x8A\xA1" /*务*/ ""
    //                                             "\xE4\xB8\x89" /*三*/ "");
    // lv_obj_set_event_cb(list_btn_rw3, my_event_cb);
    // lv_obj_add_style(list_btn_rw3, LV_BTN_PART_MAIN, &style_btn);

    // list_btn_rw4 = lv_list_add_btn(list1, NULL, ""
    //                                             "\xE4\xBB\xBB" /*任*/ ""
    //                                             "\xE5\x8A\xA1" /*务*/ ""
    //                                             "\xE5\x9B\x9B" /*四*/ "");
    // lv_obj_set_event_cb(list_btn_rw4, my_event_cb);
    // lv_obj_add_style(list_btn_rw4, LV_BTN_PART_MAIN, &style_btn);

    // list_btn_rw5 = lv_list_add_btn(list1, NULL, ""
    //                                             "\xE4\xBB\xBB" /*任*/ ""
    //                                             "\xE5\x8A\xA1" /*务*/ ""
    //                                             "\xE4\xBA\x94" /*五*/ "");
    // lv_obj_set_event_cb(list_btn_rw5, my_event_cb);
    // lv_obj_add_style(list_btn_rw5, LV_BTN_PART_MAIN, &style_btn);

    // list_btn_rw6 = lv_list_add_btn(list1, NULL, ""
    //                                             "\xE4\xBB\xBB" /*任*/ ""
    //                                             "\xE5\x8A\xA1" /*务*/ ""
    //                                             "\xE5\x85\xAD" /*六*/ "");
    // lv_obj_set_event_cb(list_btn_rw6, my_event_cb);
    // lv_obj_add_style(list_btn_rw6, LV_BTN_PART_MAIN, &style_btn);

    // list_btn_rw7 = lv_list_add_btn(list1, NULL, ""
    //                                             "\xE4\xBB\xBB" /*任*/ ""
    //                                             "\xE5\x8A\xA1" /*务*/ ""
    //                                             "\xE4\xB8\x83" /*七*/ "");
    // lv_obj_set_event_cb(list_btn_rw7, my_event_cb);
    // lv_obj_add_style(list_btn_rw7, LV_BTN_PART_MAIN, &style_btn);

    // list_btn_rw8 = lv_list_add_btn(list1, NULL, ""
    //                                             "\xE4\xBB\xBB" /*任*/ ""
    //                                             "\xE5\x8A\xA1" /*务*/ ""
    //                                             "\xE5\x85\xAB" /*八*/ "");
    // lv_obj_set_event_cb(list_btn_rw8, my_event_cb);
    // lv_obj_add_style(list_btn_rw8, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, "LED"); // RGB
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, ""
    //                                         "\xE8\x88\xB5" /*舵*/ ""
    //                                         "\xE6\x9C\xBA" /*机*/ ""); // 舵机
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, ""
    //                                         "\xE7\x82\xB9" /*点*/ ""
    //                                         "\xE9\x98\xB5" /*阵*/ ""); // 点阵
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, ""
    //                                         "\xE7\x94\xB5" /*电*/ ""
    //                                         "\xE5\xAD\x90" /*子*/ ""
    //                                         "\xE7\xA7\xA4" /*秤*/ ""); // 电子秤
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, ""
    //                                         "\xE7\x9F\xA9" /*矩*/ ""
    //                                         "\xE9\x98\xB5" /*阵*/ ""
    //                                         "\xE9\x94\xAE" /*键*/ ""
    //                                         "\xE7\x9B\x98" /*盘*/ ""); // 矩阵键盘
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, "RFID"); // RFID
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, ""
    //                                         "\xE5\x85\x89" /*光*/ ""
    //                                         "\xE7\x85\xA7" /*照*/ ""); // RFID
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, ""
    //                                         "\xE5\xAF\x86" /*密*/ ""
    //                                         "\xE7\xA0\x81" /*码*/ ""
    //                                         "\xE9\x94\x81" /*锁*/ ""); // RFID
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, ""
    //                                         "\xE8\xAE\xA1" /*计*/ ""
    //                                         "\xE7\xAE\x97" /*算*/ ""
    //                                         "\xE6\x9C\xBA" /*机*/ ""); // 计算机
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, "dht11");
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, ""
    //                                         "\xE6\xAD\xA5" /*步*/ ""
    //                                         "\xE8\xBF\x9B" /*进*/ ""
    //                                         "\xE7\x94\xB5" /*电*/ ""
    //                                         "\xE6\x9C\xBA" /*机*/ "");
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, ""
    //                                         "\xE6\x89\x8B" /*手*/ ""
    //                                         "\xE5\x8A\xBF" /*势*/ ""
    //                                         "\xE6\xA3\x80" /*检*/ ""
    //                                         "\xE6\xB5\x8B" /*测*/ "");
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, ""
    //                                         "\xE6\x8C\x87" /*指*/ ""
    //                                         "\xE7\xBA\xB9" /*纹*/ "");
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, """\xE5\xBF\x83"/*心*/"""\xE7\x8E\x87"/*率*/"\\""\xE8\xA1\x80"/*血*/"""\xE6\xB0\xA7"/*氧*/"");
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // // 在tile1_RFID上创建一个文本区域对象
    // lv_obj_t *ta1 = lv_textarea_create(main_create, NULL);

    // // 设置文本区域的背景颜色为透明
    // lv_obj_set_style_local_bg_opa(ta1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

    // // 设置文本区域的文本颜色为黑色
    // lv_obj_set_style_local_text_color(ta1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);

    // // 设置边框不透明度为0
    // lv_obj_set_style_local_border_opa(ta1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

    // // 设置边框宽度为0
    // lv_obj_set_style_local_border_width(ta1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

    // // 为文本区域添加一个样式
    // lv_obj_add_style(ta1, LV_LABEL_PART_MAIN, &style);

    // // 设置文本区域的大小为600x50
    // lv_obj_set_size(ta1, 600, 50);

    // // 将文本区域对齐到父类的左上角，并设置偏移量（200，0）
    // lv_obj_align(ta1, NULL, LV_ALIGN_IN_TOP_LEFT, 200, 0);

    // // 设置文本区域的初始文本为空
    // lv_textarea_set_text(ta1, "");
    // lv_obj_set_event_cb(ta1, textarea_event_handler);
}

// 键盘事件处理程序
//static void keyboard_event_handler(lv_obj_t *keyboard_obj, lv_event_t event)
//{
    //if (event == LV_EVENT_APPLY)
    //{
        //lv_obj_t *textarea = lv_keyboard_get_textarea(keyboard_obj);
        //const char *text = lv_textarea_get_text(textarea);
        //if (strcmp(text, "task01") == 0)
        //{
          //   手动触发第一个列表成员的点击事件
            //lv_event_send(list_btn_rw7, LV_EVENT_CLICKED, NULL);
        //}
        //else if (strcmp(text, "task02") == 0)
        //{
      //       手动触发第一个列表成员的点击事件
            //lv_event_send(list_btn_rw8, LV_EVENT_CLICKED, NULL);
        //}
    //}
    //else if (event == LV_EVENT_CANCEL)
    //{
        //lv_obj_t *textarea = lv_keyboard_get_textarea(keyboard_obj);
        //if (textarea)
        //{
            //lv_keyboard_set_textarea(keyboard_obj, NULL); // 解除键盘与当前文本区域的绑定
            //lv_obj_del(keyboard_obj);                     // 删除键盘对象
            //keyboard = NULL;                              // 重置键盘对象指针

            //return;
        //}
    //}
  //   继续处理其他键盘事件
    //lv_keyboard_def_event_cb(keyboard, event);
//}

 //文本区域事件处理程序
//static void textarea_event_handler(lv_obj_t *textarea, lv_event_t event)
//{
    //if (event == LV_EVENT_CLICKED)
    //{
        //if (keyboard == NULL)
        //{
             //创建键盘并设置属性
          //  keyboard = lv_keyboard_create(main_create, NULL);
             //创建一个新的键盘对象，并将其父对象设置为 main_screen。
             //这样键盘将显示在 main_screen 上，保持界面层次结构的一致性。

          //  lv_keyboard_set_cursor_manage(keyboard, true);
             //启用键盘光标管理。
             //当输入框聚焦时，光标将自动跟踪文本输入的位置。

          //  lv_keyboard_set_textarea(keyboard, textarea);
             //将键盘绑定到指定的文本区域对象（textarea）。
             //这样用户在键盘上输入时，输入内容将显示在该文本区域中。

          //  lv_obj_set_event_cb(keyboard, keyboard_event_handler);
             //为键盘对象设置一个事件处理程序，名为 keyboard_event_handler。
             //当键盘上发生特定事件（如按下“应用”或“取消”按钮）时，将调用该处理程序执行相应操作。

          //  lv_obj_align(keyboard, textarea, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
             //将键盘对象对齐到文本区域对象（textarea）的左下方。
             //这样键盘将出现在文本区域的正下方，使用户界面更直观和易于操作。

         //   lv_obj_set_size(keyboard, 600, 240);
             //设置键盘对象的大小为 600 像素宽和 240 像素高。
             //这确保键盘有足够的空间显示所有按键，并且适合用户界面的布局。
        //}
    //}
//}
