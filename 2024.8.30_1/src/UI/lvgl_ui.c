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
// ȫ�ּ��̶���ָ��
static lv_obj_t *keyboard = NULL;
lv_obj_t *main_create = NULL;
static void textarea_event_handler(lv_obj_t *ta, lv_event_t event);
static lv_obj_t *list_btn_rw1 = NULL, *list_btn_rw2 = NULL, *list_btn_rw3 = NULL, *list_btn_rw4 = NULL, *list_btn_rw5 = NULL, *list_btn_rw6 = NULL, *list_btn_rw7 = NULL, *list_btn_rw8 = NULL;

//static void my_event_cb(lv_obj_t *btn, lv_event_t event)
//{
    //if (event == LV_EVENT_CLICKED)
    //{
        //const char *btn_text = lv_list_get_btn_text(btn);
        //lv_obj_set_hidden(main_create, true); // �����б�
        //if (strcmp(btn_text, "dht11") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, dht11_event_cb, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE8\xAE\xA1" /*��*/ ""
                                  //"\xE7\xAE\x97" /*��*/ ""
                                  //"\xE6\x9C\xBA" /*��*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, computer, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE4\xBB\xBB" /*��*/ ""
                                  //"\xE5\x8A\xA1" /*��*/ ""
                                  //"\xE4\xB8\x80" /*һ*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, task_1, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE4\xBB\xBB" /*��*/ ""
                                  //"\xE5\x8A\xA1" /*��*/ ""
                                  //"\xE4\xBA\x8C" /*��*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, task_2, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE4\xBB\xBB" /*��*/ ""
                                  //"\xE5\x8A\xA1" /*��*/ ""
                                  //"\xE4\xB8\x89" /*��*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, task_3, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE4\xBB\xBB" /*��*/ ""
                                  //"\xE5\x8A\xA1" /*��*/ ""
                                  //"\xE5\x9B\x9B" /*��*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, task_4, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE4\xBB\xBB" /*��*/ ""
                                  //"\xE5\x8A\xA1" /*��*/ ""
                                  //"\xE4\xBA\x94" /*��*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, task_5, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE4\xBB\xBB" /*��*/ ""
                                  //"\xE5\x8A\xA1" /*��*/ ""
                                  //"\xE5\x85\xAD" /*��*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, task_6, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE4\xBB\xBB" /*��*/ ""
                                  //"\xE5\x8A\xA1" /*��*/ ""
                                  //"\xE4\xB8\x83" /*��*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, task_7, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE4\xBB\xBB" /*��*/ ""
                                  //"\xE5\x8A\xA1" /*��*/ ""
                                  //"\xE5\x85\xAB" /*��*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, task_8, 3000);
        //}
        //else if (strcmp(btn_text, "LED") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, Quest_LED, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE8\x88\xB5" /*��*/ ""
                                  //"\xE6\x9C\xBA" /*��*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, Quest_steering_engine, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE7\x82\xB9" /*��*/ ""
                                  //"\xE9\x98\xB5" /*��*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, Quest_dot_matrix, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE7\x94\xB5" /*��*/ ""
                                  //"\xE5\xAD\x90" /*��*/ ""
                                  //"\xE7\xA7\xA4" /*��*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, electronic_scale, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE7\x9F\xA9" /*��*/ ""
                                  //"\xE9\x98\xB5" /*��*/ ""
                                  //"\xE9\x94\xAE" /*��*/ ""
                                  //"\xE7\x9B\x98" /*��*/ "") == 0)
        //{
            //matrix_keyboard();
        //}
        //else if (strcmp(btn_text, "RFID") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, Quest_RFID, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE5\x85\x89" /*��*/ ""
                                  //"\xE7\x85\xA7" /*��*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, Quest_BH1750, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE5\xAF\x86" /*��*/ ""
                                  //"\xE7\xA0\x81" /*��*/ ""
                                  //"\xE9\x94\x81" /*��*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, Combination_lock, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE6\xAD\xA5" /*��*/ ""
                                  //"\xE8\xBF\x9B" /*��*/ ""
                                  //"\xE7\x94\xB5" /*��*/ ""
                                  //"\xE6\x9C\xBA" /*��*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, UI_StepperMotor_event_cb, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE6\x89\x8B" /*��*/ ""
                                  //"\xE5\x8A\xBF" /*��*/ ""
                                  //"\xE6\xA3\x80" /*��*/ ""
                                  //"\xE6\xB5\x8B" /*��*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, UI_paj7620u2_event_cb, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE6\x8C\x87" /*ָ*/ ""
                                  //"\xE7\xBA\xB9" /*��*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, UI_as608_event_cb, 3000);
        //}
        //else if (strcmp(btn_text, ""
                                  //"\xE5\xBF\x83" /*��*/ ""
                                  //"\xE7\x8E\x87" /*��*/ "\\"
                                  //"\xE8\xA1\x80" /*Ѫ*/ ""
                                  //"\xE6\xB0\xA7" /*��*/ "") == 0)
        //{
            //rt_mb_send_wait(UI_thread_mailbox, task_MAX30102, 3000);
        //}
    //}
//}
extern rt_thread_t dom_thread;
static void event_open_esc(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_SHORT_CLICKED)
    { /* ���󱻵�� */
        const char *btn_text = lv_list_get_btn_text(obj);
        if (strcmp(btn_text, ""
                             "\xE5\xBC\x80" /*��*/ ""
                             "\xE5\xA7\x8B" /*ʼ*/ ""
                             "\xE9\x99\xA4" /*��*/ ""
                             "\xE5\xB0\x98" /*��*/ "") == 0)
        {
            voice_broadcast(1);
            Stepper_motor_open();
        }
        else if (strcmp(btn_text, ""
                                  "\xE6\xBB\x91" /*��*/ ""
                                  "\xE5\x8F\xB0" /*̨*/ ""
                                  "\xE5\xA4\x8D" /*��*/ ""
                                  "\xE4\xBD\x8D" /*λ*/ "") == 0)
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
    // ���ñ߿�͸����Ϊ0
    lv_obj_set_style_local_border_opa(main_create, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    // ���ñ߿���Ϊ0
    lv_obj_set_style_local_border_width(main_create, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

    lv_obj_set_style_local_bg_color(main_create, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_bg_opa(main_create, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);


    //LV_IMG_DECLARE(img_1);
    //lv_obj_t *icon = lv_img_create(main_create, NULL);
    //lv_img_set_src(icon, &img_1);
    //lv_obj_set_style_local_bg_opa(icon, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100); // ������ɫ���Ƕ�100%����ֵԽ�ͣ���ɫԽ͸��

    lv_obj_t *btn1 = lv_btn_create(main_create, NULL);
    lv_obj_set_event_cb(btn1, event_open_esc);
    lv_obj_align(btn1, main_create, LV_ALIGN_CENTER, -150, -200);
    lv_obj_set_size(btn1, 400, 132);
    lv_obj_set_style_local_bg_color(btn1,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_RED);
    lv_obj_t *label = lv_label_create(btn1, NULL);
    lv_obj_set_style_local_text_font(label, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, &myFont2); // �������������

    //lv_obj_set_style_local_text_color(label,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_RED);
    
    
    lv_label_set_text(label, ""
                             "\xE5\xBC\x80" /*��*/ ""
                             "\xE5\xA7\x8B" /*ʼ*/ ""
                             "\xE9\x99\xA4" /*��*/ ""
                             "\xE5\xB0\x98" /*��*/ "");



    btn1 = lv_btn_create(main_create, NULL);
    lv_obj_set_event_cb(btn1, event_open_esc);
    lv_obj_align(btn1, main_create, LV_ALIGN_CENTER, -150, 100);
    lv_obj_set_size(btn1, 400, 132);
    lv_obj_set_style_local_bg_color(btn1,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_RED);
    label = lv_label_create(btn1, NULL);
    lv_obj_set_style_local_text_font(label, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, &myFont2); // �������������
    lv_label_set_text(label, ""
                             "\xE6\xBB\x91" /*��*/ ""
                             "\xE5\x8F\xB0" /*̨*/ ""
                             "\xE5\xA4\x8D" /*��*/ ""
                             "\xE4\xBD\x8D" /*λ*/ "");

    label = lv_label_create(main_create, NULL);
    lv_obj_align(label, main_create, LV_ALIGN_IN_LEFT_MID, 80, -30);
    lv_obj_set_style_local_text_font(label, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, &myFont2); // �������������
    lv_obj_set_style_local_text_color(label,LV_LABEL_PART_MAIN,LV_STATE_DEFAULT,LV_COLOR_BLACK);
    lv_label_set_text_fmt(label, ""
                                 "\xE8\xAE\xBE" /*��*/ ""
                                 "\xE5\xA4\x87" /*��*/ ""
                                 "\xE7\xBC\x96" /*��*/ ""
                                 "\xE5\x8F\xB7" /*��*/ ":%d "
                                 "\xE5\x8F\xB7" /*��*/ "",
                          drivers_id);

    // lv_obj_t *list1 = lv_list_create(main_create, NULL);
    // lv_obj_set_size(list1, 200, 480);

    // // ��ʼ����ʽ
    // lv_style_init(&style);
    // // �����ı�����
    // lv_style_set_text_font(&style, LV_STATE_DEFAULT, &hz_simsun_3500_24);
    // // �����ʽ���б�
    // lv_obj_add_style(list1, LV_LABEL_PART_MAIN, &style);

    // lv_obj_align(list1, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 0);
    // // ��ʼ���б�����ʽ
    // static lv_style_t style_btn;
    // lv_style_init(&style_btn);

    // // ���ñ���͸��
    // lv_style_set_bg_opa(&style_btn, LV_STATE_DEFAULT, LV_OPA_TRANSP);

    // // ���ñ߿�͸����Ϊ0
    // lv_style_set_border_opa(&style_btn, LV_STATE_DEFAULT, LV_OPA_TRANSP);

    // // ���ñ߿���Ϊ0
    // lv_style_set_border_width(&style_btn, LV_STATE_DEFAULT, 0);

    // // ����ʽӦ�����б���
    // lv_obj_add_style(list1, LV_LIST_PART_BG, &style_btn);
    // lv_obj_add_style(list1, LV_LIST_PART_EDGE_FLASH, &style_btn);
    // lv_obj_add_style(list1, LV_LIST_PART_SCROLLABLE, &style_btn);
    // // ����ʽӦ�����б�Ĺ�����
    // lv_obj_add_style(list1, LV_LIST_PART_SCROLLBAR, &style_btn);

    // /*Add buttons to the list*/
    // lv_obj_t *list_btn;

    // list_btn_rw1 = lv_list_add_btn(list1, NULL, ""
    //                                             "\xE4\xBB\xBB" /*��*/ ""
    //                                             "\xE5\x8A\xA1" /*��*/ ""
    //                                             "\xE4\xB8\x80" /*һ*/ "");
    // lv_obj_set_event_cb(list_btn_rw1, my_event_cb);
    // lv_obj_add_style(list_btn_rw1, LV_BTN_PART_MAIN, &style_btn);

    // list_btn_rw2 = lv_list_add_btn(list1, NULL, ""
    //                                             "\xE4\xBB\xBB" /*��*/ ""
    //                                             "\xE5\x8A\xA1" /*��*/ ""
    //                                             "\xE4\xBA\x8C" /*��*/ "");
    // lv_obj_set_event_cb(list_btn_rw2, my_event_cb);
    // lv_obj_add_style(list_btn_rw2, LV_BTN_PART_MAIN, &style_btn);

    // list_btn_rw3 = lv_list_add_btn(list1, NULL, ""
    //                                             "\xE4\xBB\xBB" /*��*/ ""
    //                                             "\xE5\x8A\xA1" /*��*/ ""
    //                                             "\xE4\xB8\x89" /*��*/ "");
    // lv_obj_set_event_cb(list_btn_rw3, my_event_cb);
    // lv_obj_add_style(list_btn_rw3, LV_BTN_PART_MAIN, &style_btn);

    // list_btn_rw4 = lv_list_add_btn(list1, NULL, ""
    //                                             "\xE4\xBB\xBB" /*��*/ ""
    //                                             "\xE5\x8A\xA1" /*��*/ ""
    //                                             "\xE5\x9B\x9B" /*��*/ "");
    // lv_obj_set_event_cb(list_btn_rw4, my_event_cb);
    // lv_obj_add_style(list_btn_rw4, LV_BTN_PART_MAIN, &style_btn);

    // list_btn_rw5 = lv_list_add_btn(list1, NULL, ""
    //                                             "\xE4\xBB\xBB" /*��*/ ""
    //                                             "\xE5\x8A\xA1" /*��*/ ""
    //                                             "\xE4\xBA\x94" /*��*/ "");
    // lv_obj_set_event_cb(list_btn_rw5, my_event_cb);
    // lv_obj_add_style(list_btn_rw5, LV_BTN_PART_MAIN, &style_btn);

    // list_btn_rw6 = lv_list_add_btn(list1, NULL, ""
    //                                             "\xE4\xBB\xBB" /*��*/ ""
    //                                             "\xE5\x8A\xA1" /*��*/ ""
    //                                             "\xE5\x85\xAD" /*��*/ "");
    // lv_obj_set_event_cb(list_btn_rw6, my_event_cb);
    // lv_obj_add_style(list_btn_rw6, LV_BTN_PART_MAIN, &style_btn);

    // list_btn_rw7 = lv_list_add_btn(list1, NULL, ""
    //                                             "\xE4\xBB\xBB" /*��*/ ""
    //                                             "\xE5\x8A\xA1" /*��*/ ""
    //                                             "\xE4\xB8\x83" /*��*/ "");
    // lv_obj_set_event_cb(list_btn_rw7, my_event_cb);
    // lv_obj_add_style(list_btn_rw7, LV_BTN_PART_MAIN, &style_btn);

    // list_btn_rw8 = lv_list_add_btn(list1, NULL, ""
    //                                             "\xE4\xBB\xBB" /*��*/ ""
    //                                             "\xE5\x8A\xA1" /*��*/ ""
    //                                             "\xE5\x85\xAB" /*��*/ "");
    // lv_obj_set_event_cb(list_btn_rw8, my_event_cb);
    // lv_obj_add_style(list_btn_rw8, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, "LED"); // RGB
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, ""
    //                                         "\xE8\x88\xB5" /*��*/ ""
    //                                         "\xE6\x9C\xBA" /*��*/ ""); // ���
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, ""
    //                                         "\xE7\x82\xB9" /*��*/ ""
    //                                         "\xE9\x98\xB5" /*��*/ ""); // ����
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, ""
    //                                         "\xE7\x94\xB5" /*��*/ ""
    //                                         "\xE5\xAD\x90" /*��*/ ""
    //                                         "\xE7\xA7\xA4" /*��*/ ""); // ���ӳ�
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, ""
    //                                         "\xE7\x9F\xA9" /*��*/ ""
    //                                         "\xE9\x98\xB5" /*��*/ ""
    //                                         "\xE9\x94\xAE" /*��*/ ""
    //                                         "\xE7\x9B\x98" /*��*/ ""); // �������
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, "RFID"); // RFID
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, ""
    //                                         "\xE5\x85\x89" /*��*/ ""
    //                                         "\xE7\x85\xA7" /*��*/ ""); // RFID
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, ""
    //                                         "\xE5\xAF\x86" /*��*/ ""
    //                                         "\xE7\xA0\x81" /*��*/ ""
    //                                         "\xE9\x94\x81" /*��*/ ""); // RFID
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, ""
    //                                         "\xE8\xAE\xA1" /*��*/ ""
    //                                         "\xE7\xAE\x97" /*��*/ ""
    //                                         "\xE6\x9C\xBA" /*��*/ ""); // �����
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, "dht11");
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, ""
    //                                         "\xE6\xAD\xA5" /*��*/ ""
    //                                         "\xE8\xBF\x9B" /*��*/ ""
    //                                         "\xE7\x94\xB5" /*��*/ ""
    //                                         "\xE6\x9C\xBA" /*��*/ "");
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, ""
    //                                         "\xE6\x89\x8B" /*��*/ ""
    //                                         "\xE5\x8A\xBF" /*��*/ ""
    //                                         "\xE6\xA3\x80" /*��*/ ""
    //                                         "\xE6\xB5\x8B" /*��*/ "");
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, ""
    //                                         "\xE6\x8C\x87" /*ָ*/ ""
    //                                         "\xE7\xBA\xB9" /*��*/ "");
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // list_btn = lv_list_add_btn(list1, NULL, """\xE5\xBF\x83"/*��*/"""\xE7\x8E\x87"/*��*/"\\""\xE8\xA1\x80"/*Ѫ*/"""\xE6\xB0\xA7"/*��*/"");
    // lv_obj_set_event_cb(list_btn, my_event_cb);
    // lv_obj_add_style(list_btn, LV_BTN_PART_MAIN, &style_btn);

    // // ��tile1_RFID�ϴ���һ���ı��������
    // lv_obj_t *ta1 = lv_textarea_create(main_create, NULL);

    // // �����ı�����ı�����ɫΪ͸��
    // lv_obj_set_style_local_bg_opa(ta1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

    // // �����ı�������ı���ɫΪ��ɫ
    // lv_obj_set_style_local_text_color(ta1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);

    // // ���ñ߿�͸����Ϊ0
    // lv_obj_set_style_local_border_opa(ta1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

    // // ���ñ߿���Ϊ0
    // lv_obj_set_style_local_border_width(ta1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

    // // Ϊ�ı��������һ����ʽ
    // lv_obj_add_style(ta1, LV_LABEL_PART_MAIN, &style);

    // // �����ı�����Ĵ�СΪ600x50
    // lv_obj_set_size(ta1, 600, 50);

    // // ���ı�������뵽��������Ͻǣ�������ƫ������200��0��
    // lv_obj_align(ta1, NULL, LV_ALIGN_IN_TOP_LEFT, 200, 0);

    // // �����ı�����ĳ�ʼ�ı�Ϊ��
    // lv_textarea_set_text(ta1, "");
    // lv_obj_set_event_cb(ta1, textarea_event_handler);
}

// �����¼��������
//static void keyboard_event_handler(lv_obj_t *keyboard_obj, lv_event_t event)
//{
    //if (event == LV_EVENT_APPLY)
    //{
        //lv_obj_t *textarea = lv_keyboard_get_textarea(keyboard_obj);
        //const char *text = lv_textarea_get_text(textarea);
        //if (strcmp(text, "task01") == 0)
        //{
          //   �ֶ�������һ���б��Ա�ĵ���¼�
            //lv_event_send(list_btn_rw7, LV_EVENT_CLICKED, NULL);
        //}
        //else if (strcmp(text, "task02") == 0)
        //{
      //       �ֶ�������һ���б��Ա�ĵ���¼�
            //lv_event_send(list_btn_rw8, LV_EVENT_CLICKED, NULL);
        //}
    //}
    //else if (event == LV_EVENT_CANCEL)
    //{
        //lv_obj_t *textarea = lv_keyboard_get_textarea(keyboard_obj);
        //if (textarea)
        //{
            //lv_keyboard_set_textarea(keyboard_obj, NULL); // ��������뵱ǰ�ı�����İ�
            //lv_obj_del(keyboard_obj);                     // ɾ�����̶���
            //keyboard = NULL;                              // ���ü��̶���ָ��

            //return;
        //}
    //}
  //   �����������������¼�
    //lv_keyboard_def_event_cb(keyboard, event);
//}

 //�ı������¼��������
//static void textarea_event_handler(lv_obj_t *textarea, lv_event_t event)
//{
    //if (event == LV_EVENT_CLICKED)
    //{
        //if (keyboard == NULL)
        //{
             //�������̲���������
          //  keyboard = lv_keyboard_create(main_create, NULL);
             //����һ���µļ��̶��󣬲����丸��������Ϊ main_screen��
             //�������̽���ʾ�� main_screen �ϣ����ֽ����νṹ��һ���ԡ�

          //  lv_keyboard_set_cursor_manage(keyboard, true);
             //���ü��̹�����
             //�������۽�ʱ����꽫�Զ������ı������λ�á�

          //  lv_keyboard_set_textarea(keyboard, textarea);
             //�����̰󶨵�ָ�����ı��������textarea����
             //�����û��ڼ���������ʱ���������ݽ���ʾ�ڸ��ı������С�

          //  lv_obj_set_event_cb(keyboard, keyboard_event_handler);
             //Ϊ���̶�������һ���¼����������Ϊ keyboard_event_handler��
             //�������Ϸ����ض��¼����簴�¡�Ӧ�á���ȡ������ť��ʱ�������øô������ִ����Ӧ������

          //  lv_obj_align(keyboard, textarea, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
             //�����̶�����뵽�ı��������textarea�������·���
             //�������̽��������ı���������·���ʹ�û������ֱ�ۺ����ڲ�����

         //   lv_obj_set_size(keyboard, 600, 240);
             //���ü��̶���Ĵ�СΪ 600 ���ؿ�� 240 ���ظߡ�
             //��ȷ���������㹻�Ŀռ���ʾ���а����������ʺ��û�����Ĳ��֡�
        //}
    //}
//}
