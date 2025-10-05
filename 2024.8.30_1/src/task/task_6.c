/*
 * task_6.c
 *
 * created: 2024/3/3
 *  author:
 */

#include "lvgl_ui.h"
#include "task_6.h"
#include "lvgl-7.0.1/lvgl.h"
#include "ch455g.h"
#include "pwm_ic.h"
#include "main.h"
#include "as608.h"
#include "StepperMotor.h"
static lv_obj_t *tile1 = NULL;
//static rt_thread_t RW6_thread = NULL;

static lv_obj_t *hint_label = NULL; // ��ʾ�ı�
static rt_thread_t as608_thread = NULL;
static lv_obj_t *pwd_text_area = NULL; // �����ı�
static char RW = 0;
static int num1;
static void Add_FR(void);
static void event_task_esc(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_SHORT_CLICKED)
    { /* ���󱻵�� */
        Taskshutdown = 0;
    }
}


static void RW6_thread1(void *arg)
{
    UART4_Config_Init();
    StepperMotor();          //�������
    int pageID, mathscore,i = 0;
    for (;;)
    {
        if (press_FR(&pageID, &mathscore))
        {
           lv_label_set_text_fmt(hint_label,"""\xE7\xA1\xAE"/*ȷ*/"""\xE6\x9C\x89"/*��*/"""\xE6\xAD\xA4"/*��*/"""\xE4\xBA\xBA"/*��*/" ID:%d ""\xE5\x8C\xB9"/*ƥ*/"""\xE9\x85\x8D"/*��*/"""\xE5\xBE\x97"/*��*/"""\xE5\x88\x86"/*��*/":%d", pageID, mathscore);
           rt_kprintf("ȷ�д��� ID:%d ƥ��÷�:%d", pageID, mathscore);
           step_28byj48_angles(90,0);
        }
        if (RW == 1)
        {
            lv_label_set_text(hint_label, EnsureMessage_UTF8(Del_FR(num1)));
            RW = 0;
        }
        else if (RW == 2)
        {
            lv_label_set_text(hint_label, ""
                                          "\xE8\xAF\xB7" /*��*/ ""
                                          "\xE6\x8C\x89" /*��*/ ""
                                          "\xE6\x8C\x87" /*ָ*/ ""
                                          "\xE7\xBA\xB9" /*��*/ "");
                                          Add_FR();
                                          RW = 0;
        }
        if(Taskshutdown != 6)
        {
            rt_mutex_take(ui_mutex, RT_WAITING_FOREVER);
            lv_obj_set_hidden(main_create, false); // ��ʾ�б�
            lv_obj_del_async(tile1);
            rt_mutex_release(ui_mutex);
            return ;

       }
       rt_thread_mdelay(200);
    }

}


static const char *keyboard_map[] =
    {
        LV_SYMBOL_TRASH, ":", "add", "\n",
        "1", "2", "3", "\n",
        "4", "5", "6", "\n",
        "7", "8", "9", "\n",
        "0", LV_SYMBOL_BACKSPACE, LV_SYMBOL_NEW_LINE, ""};

// ȷ�ϰ������¼��ص�
static void btn_event_callback(lv_obj_t *obj, lv_event_t event)
{
    static char enter = 0;
    if (event == LV_EVENT_VALUE_CHANGED)
    {
        uint16_t ID = lv_btnmatrix_get_active_btn(obj);       // ��ȡID
        const char *txt = lv_btnmatrix_get_btn_text(obj, ID); // ��ȡ�ı�
        char *buf[100] = {0};
        if (ID != 13 && ID != 0 && ID != 1 && ID != 2 && ID != 14 && ID <= 14)
            lv_textarea_add_text(pwd_text_area, txt);
        else if (ID == 13)
            lv_textarea_set_text(pwd_text_area, "");
        else if (ID == 0)
        {
            enter = 1;
            lv_label_set_text(hint_label, ""
                                          "\xE8\xBE\x93" /*��*/ ""
                                          "\xE5\x85\xA5" /*��*/ ""
                                          "\xE6\x8C\x87" /*ָ*/ ""
                                          "\xE7\xBA\xB9" /*��*/ "ID"
                                          "\xE5\x90\x8E" /*��*/ ""
                                          "\xE6\x8C\x89" /*��*/ ""
                                          "\xE4\xB8\x8B" /*��*/ ""
                                          "\xE5\x9B\x9E" /*��*/ ""
                                          "\xE8\xBD\xA6" /*��*/ ""
                                          "\xE9\x94\xAE" /*��*/ ""
                                          "\xE5\x88\xA0" /*ɾ*/ ""
                                          "\xE9\x99\xA4" /*��*/ ""
                                          "\xE6\x8C\x87" /*ָ*/ ""
                                          "\xE7\xBA\xB9" /*��*/ "");
        }
        else if (ID == 2)
        {
            enter = 2;
            lv_label_set_text(hint_label, ""
                                          "\xE8\xAF\xB7" /*��*/ ""
                                          "\xE7\xBB\x99" /*��*/ ""
                                          "\xE6\x8C\x87" /*ָ*/ ""
                                          "\xE7\xBA\xB9" /*��*/ ""
                                          "\xE8\xAE\xBE" /*��*/ ""
                                          "\xE7\xBD\xAE" /*��*/ ""
                                          "\xE4\xB8\x80" /*һ*/ ""
                                          "\xE4\xB8\xAA" /*��*/ "ID"
                                          "\xE7\x84\xB6" /*Ȼ*/ ""
                                          "\xE5\x90\x8E" /*��*/ ""
                                          "\xE6\x8C\x89" /*��*/ ""
                                          "\xE4\xB8\x8B" /*��*/ ""
                                          "\xE5\x9B\x9E" /*��*/ ""
                                          "\xE8\xBD\xA6" /*��*/ "");
        }
        else if (ID == 14)
        {
            if (enter == 1)
            {
                const char *text_data = lv_textarea_get_text(pwd_text_area);
                num1 = atoi(text_data);
                if (num1 > 300)
                    lv_label_set_text(hint_label, "ID>300,"
                                                  "\xE6\x8C\x87" /*ָ*/ ""
                                                  "\xE7\xBA\xB9" /*��*/ ""
                                                  "\xE5\x88\xA0" /*ɾ*/ ""
                                                  "\xE9\x99\xA4" /*��*/ ""
                                                  "\xE5\xA4\xB1" /*ʧ*/ ""
                                                  "\xE8\xB4\xA5" /*��*/ "");
                else
                {
                    enter = 0;
                    RW = 1;
                }
            }
            if (enter == 2)
            {
                const char *text_data = lv_textarea_get_text(pwd_text_area);
                num1 = atoi(text_data);
                if (num1 > 300)
                    lv_label_set_text(hint_label, "ID>300,"
                                                  "\xE6\x8C\x87" /*ָ*/ ""
                                                  "\xE7\xBA\xB9" /*��*/ ""
                                                  "\xE6\xB7\xBB" /*��*/ ""
                                                  "\xE5\x8A\xA0" /*��*/ ""
                                                  "\xE5\xA4\xB1" /*ʧ*/ ""
                                                  "\xE8\xB4\xA5" /*��*/ "");
                else
                {
                    enter = 0;
                    RW = 2;
                }
            }
        }
    }
}

void task_6(void)
{

    tile1 = lv_obj_create(lv_scr_act(), NULL);
    if(tile1==NULL){
        rt_kprintf("����6\"tile1\"����ʧ��");
        rt_thread_mdelay(100);
    }

    lv_obj_set_size(tile1, LV_HOR_RES, 400);

    lv_obj_t *btn1 = lv_btn_create(tile1, NULL);
    if(btn1==NULL){
        rt_kprintf("����6\"btn1\"����ʧ��");
        rt_thread_mdelay(100);
    }
    lv_obj_set_event_cb(btn1, event_task_esc);
    lv_obj_align(btn1, tile1, LV_ALIGN_IN_TOP_LEFT, 5, 5);
    lv_obj_t *label = lv_label_create(btn1, NULL);
    if(label==NULL){
        rt_kprintf("����6\"label\"����ʧ��");
        rt_thread_mdelay(100);
    }
    lv_label_set_text(label, "Esc");
// ����������������
    lv_obj_t *pwd_main_cont = lv_obj_create(tile1, NULL);
    if(pwd_main_cont==NULL){
        rt_kprintf("����6\"pwd_main_cont\"����ʧ��");
        rt_thread_mdelay(100);
    }
    // ����һ����ʽ
    static lv_style_t main_cont_style;
    // �����ʽ�е��������Ժ����з�����ڴ档
    lv_style_reset(&main_cont_style);
    // ��ʼ����ʽ
    lv_style_init(&main_cont_style);
    lv_style_set_radius(&main_cont_style, LV_STATE_DEFAULT, 20);               // ������ʽ��Բ�ǻ���
    lv_style_set_border_width(&main_cont_style, LV_STATE_DEFAULT, 0);          // ���ñ߿���
    lv_style_set_bg_opa(&main_cont_style, LV_STATE_DEFAULT, LV_OPA_100);       // ������ʽ������͸���ȣ���͸��
    lv_style_set_bg_color(&main_cont_style, LV_STATE_DEFAULT, LV_COLOR_WHITE); // ��ɫ����
    lv_style_set_pad_inner(&main_cont_style, LV_STATE_DEFAULT, 0);             // �����
    lv_obj_add_style(pwd_main_cont, NULL, &main_cont_style);                   // �����������ʽ
    lv_obj_set_size(pwd_main_cont, 400, 400);                                  // ���ô�С800x480
    lv_obj_align(pwd_main_cont, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);              // λ��

    // ����һ����ʽ
    static lv_style_t btnmatrix_style;
    // �����ʽ�е��������Ժ����з�����ڴ档
    lv_style_reset(&btnmatrix_style);
    // ��ʼ����ʽ
    lv_style_init(&btnmatrix_style);
    lv_style_set_radius(&btnmatrix_style, LV_STATE_DEFAULT, 20);                // ������ʽ��Բ�ǻ���
    lv_style_set_border_width(&btnmatrix_style, LV_STATE_DEFAULT, 0);          // ���ñ߿���
    lv_style_set_bg_opa(&btnmatrix_style, LV_STATE_DEFAULT, LV_OPA_100);       // ������ʽ������͸���ȣ���͸��
    lv_style_set_bg_color(&btnmatrix_style, LV_STATE_DEFAULT, LV_COLOR_WHITE); // ��ɫ����
    lv_style_set_pad_inner(&btnmatrix_style, LV_STATE_DEFAULT, 10);            // ���ö����ڲ����м�ࣨ�Ӷ����Ĵ�ֱ���룩

    // ����һ����ʽ
    static lv_style_t btnmatrix_style1;
    // �����ʽ�е��������Ժ����з�����ڴ档
    lv_style_reset(&btnmatrix_style1);
    // ��ʼ����ʽ
    lv_style_init(&btnmatrix_style1);
    lv_style_set_radius(&btnmatrix_style1, LV_STATE_DEFAULT, 20);                 // ������ʽ��Բ�ǻ���
    lv_style_set_border_width(&btnmatrix_style1, LV_STATE_DEFAULT, 2);            // ���ñ߿���
    lv_style_set_bg_opa(&btnmatrix_style1, LV_STATE_DEFAULT, LV_OPA_100);         // ������ʽ������͸���ȣ���͸��
    lv_style_set_bg_color(&btnmatrix_style1, LV_STATE_DEFAULT, LV_COLOR_WHITE);   // ��ɫ����
    lv_style_set_text_color(&btnmatrix_style1, LV_STATE_DEFAULT, LV_COLOR_BLACK); // �����ı���ɫΪ��ɫ

    lv_obj_t *btnm1 = lv_btnmatrix_create(pwd_main_cont, NULL);
    if(btnm1==NULL){
        rt_kprintf("����6\"btnm1\"����ʧ��");
        rt_thread_mdelay(100);
    }
    // lv_btnmatrix_set_recolor(btnm1, true);//�ı�������ɫ
    lv_btnmatrix_set_map(btnm1, keyboard_map);

    lv_obj_add_style(btnm1, LV_BTNMATRIX_PART_BG, &btnmatrix_style);   // �����������ʽ
    lv_obj_add_style(btnm1, LV_BTNMATRIX_PART_BTN, &btnmatrix_style1); // �����������ʽ
    lv_obj_set_size(btnm1, 400, 300);                                  // ���ô�С800x480
    lv_obj_align(btnm1, NULL, LV_ALIGN_IN_TOP_MID, 0, 100);            // λ��
    lv_obj_set_event_cb(btnm1, btn_event_callback);                    // ���þ���ť�ص�����

    pwd_text_area = lv_textarea_create(pwd_main_cont, NULL); // ���������ı���
    if(pwd_text_area==NULL){
        rt_kprintf("����6\"pwd_text_area\"����ʧ��");
        rt_thread_mdelay(100);
    }
    static lv_style_t TXT_style;
    // �����ʽ�е��������Ժ����з�����ڴ档
    lv_style_reset(&TXT_style);
    // ��ʼ����ʽ
    lv_style_init(&TXT_style);
    lv_style_set_radius(&TXT_style, LV_STATE_DEFAULT, 0);                  // ������ʽ��Բ�ǻ���
    lv_style_set_border_width(&TXT_style, LV_STATE_DEFAULT, 0);            // ���ñ߿���
    lv_style_set_bg_opa(&TXT_style, LV_STATE_DEFAULT, LV_OPA_100);         // ������ʽ������͸���ȣ���͸��
    lv_style_set_bg_color(&TXT_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);   // ��ɫ����
    lv_style_set_pad_inner(&TXT_style, LV_STATE_DEFAULT, 0);               // ���
    lv_style_set_text_color(&TXT_style, LV_STATE_DEFAULT, LV_COLOR_BLACK); // �����ı���ɫΪ��ɫ
    lv_obj_add_style(pwd_text_area, NULL, &TXT_style);                     // �����������ʽ
    lv_obj_add_style(pwd_text_area, LV_LABEL_PART_MAIN, &style);           // �������������
    lv_obj_set_size(pwd_text_area, 200, 100);                              // �����С

    label = lv_label_create(pwd_main_cont, NULL);
    if(label==NULL){
        rt_kprintf("����6\"label\"����ʧ��");
        rt_thread_mdelay(100);
    }
    lv_obj_add_style(label, LV_LABEL_PART_MAIN, &style); // �������������
    lv_obj_add_style(label, NULL, &TXT_style);           // �����������ʽ
    lv_label_set_text(label, "ID:");
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 10); // ����λ��

    lv_obj_align(pwd_text_area, label, LV_ALIGN_OUT_RIGHT_TOP, 0, -10); // ����λ��
    lv_textarea_set_text(pwd_text_area, "");                            // �������

    hint_label = lv_label_create(pwd_main_cont, NULL);                                                   // ������ǩ
    if(hint_label==NULL){
        rt_kprintf("����6\"hint_label\"����ʧ��");
        rt_thread_mdelay(100);
    }
    lv_obj_set_style_local_text_color(hint_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK); // �ı����óɺ�ɫ
    lv_obj_add_style(hint_label, LV_LABEL_PART_MAIN, &style);                                            // ��������
    lv_label_set_text(hint_label, "");
    lv_obj_align(hint_label, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 70);

    Taskshutdown = 6;

    rt_mb_send_wait(task1_thread_mailbox, RW6_thread1,3000);

}


// ¼ָ��
static void Add_FR(void)
{

        unsigned char i, ensure, processnum = 0;
        unsigned short ID;
        while (1)
        {
            switch (processnum)
            {
            case 0:
                i++;
                ensure = PS_GetImage();
                if (ensure == 0x00)
                {
                    ensure = PS_GenChar(CharBuffer1); // ��������
                    if (ensure == 0x00)
                    {

                        lv_label_set_text(hint_label, ""
                                                      "\xE6\x8C\x87" /*ָ*/ ""
                                                      "\xE7\xBA\xB9" /*��*/ ""
                                                      "\xE6\xAD\xA3" /*��*/ ""
                                                      "\xE5\xB8\xB8" /*��*/ "");
                        i = 0;
                        processnum = 1; // �����ڶ���
                    }
                    else
                        lv_label_set_text(hint_label, EnsureMessage_UTF8(ensure));
                }
                else
                    lv_label_set_text(hint_label, EnsureMessage_UTF8(ensure));
                break;

            case 1:
                i++;
                lv_label_set_text(hint_label, ""
                                              "\xE8\xAF\xB7" /*��*/ ""
                                              "\xE6\x8C\x89" /*��*/ ""
                                              "\xE5\x86\x8D" /*��*/ ""
                                              "\xE6\x8C\x89" /*��*/ ""
                                              "\xE4\xB8\x80" /*һ*/ ""
                                              "\xE6\xAC\xA1" /*��*/ ""
                                              "\xE6\x8C\x87" /*ָ*/ ""
                                              "\xE7\xBA\xB9" /*��*/ "");
                ensure = PS_GetImage();
                if (ensure == 0x00)
                {

                    ensure = PS_GenChar(CharBuffer2); // ��������

                    if (ensure == 0x00)
                    {
                        lv_label_set_text(hint_label, ""
                                                      "\xE6\x8C\x87" /*ָ*/ ""
                                                      "\xE7\xBA\xB9" /*��*/ ""
                                                      "\xE6\xAD\xA3" /*��*/ ""
                                                      "\xE5\xB8\xB8" /*��*/ "");
                        i = 0;
                        processnum = 2; // ����������
                    }
                    else
                        lv_label_set_text(hint_label, EnsureMessage_UTF8(ensure));
                }
                else
                    lv_label_set_text(hint_label, EnsureMessage_UTF8(ensure));
                break;

            case 2:
                lv_label_set_text(hint_label, ""
                                              "\xE5\xAF\xB9" /*��*/ ""
                                              "\xE6\xAF\x94" /*��*/ ""
                                              "\xE4\xB8\xA4" /*��*/ ""
                                              "\xE6\xAC\xA1" /*��*/ ""
                                              "\xE6\x8C\x87" /*ָ*/ ""
                                              "\xE7\xBA\xB9" /*��*/ "");
                ensure = PS_Match();
                if (ensure == 0x00)
                {
                    lv_label_set_text(hint_label, ""
                                                  "\xE5\xAF\xB9" /*��*/ ""
                                                  "\xE6\xAF\x94" /*��*/ ""
                                                  "\xE6\x88\x90" /*��*/ ""
                                                  "\xE5\x8A\x9F" /*��*/ ","
                                                  "\xE4\xB8\xA4" /*��*/ ""
                                                  "\xE6\xAC\xA1" /*��*/ ""
                                                  "\xE6\x8C\x87" /*ָ*/ ""
                                                  "\xE7\xBA\xB9" /*��*/ ""
                                                  "\xE4\xB8\x80" /*һ*/ ""
                                                  "\xE6\xA0\xB7" /*��*/ "");

                    processnum = 3; // �������Ĳ�
                }
                else
                {
                    lv_label_set_text(hint_label, ""
                                                  "\xE5\xAF\xB9" /*��*/ ""
                                                  "\xE6\xAF\x94" /*��*/ ""
                                                  "\xE5\xA4\xB1" /*ʧ*/ ""
                                                  "\xE8\xB4\xA5" /*��*/ ""
                                                  "\xEF\xBC\x8C" /*��*/ ""
                                                  "\xE8\xAF\xB7" /*��*/ ""
                                                  "\xE9\x87\x8D" /*��*/ ""
                                                  "\xE6\x96\xB0" /*��*/ ""
                                                  "\xE5\xBD\x95" /*¼*/ ""
                                                  "\xE5\x85\xA5" /*��*/ ""
                                                  "\xE6\x8C\x87" /*ָ*/ ""
                                                  "\xE7\xBA\xB9" /*��*/ "");

                    lv_label_set_text(hint_label, EnsureMessage_UTF8(ensure));
                    i = 0;
                    processnum = 0; // ���ص�һ��
                }
                rt_thread_mdelay(1200);
                break;

            case 3:
                lv_label_set_text(hint_label, ""
                                              "\xE7\x94\x9F" /*��*/ ""
                                              "\xE6\x88\x90" /*��*/ ""
                                              "\xE6\x8C\x87" /*ָ*/ ""
                                              "\xE7\xBA\xB9" /*��*/ ""
                                              "\xE6\xA8\xA1" /*ģ*/ ""
                                              "\xE6\x9D\xBF" /*��*/ "");
                ensure = PS_RegModel();
                if (ensure == 0x00)
                {

                    lv_label_set_text(hint_label, ""
                                                  "\xE7\x94\x9F" /*��*/ ""
                                                  "\xE6\x88\x90" /*��*/ ""
                                                  "\xE6\x8C\x87" /*ָ*/ ""
                                                  "\xE7\xBA\xB9" /*��*/ ""
                                                  "\xE6\xA8\xA1" /*ģ*/ ""
                                                  "\xE6\x9D\xBF" /*��*/ ""
                                                  "\xE6\x88\x90" /*��*/ ""
                                                  "\xE5\x8A\x9F" /*��*/ "");
                    processnum = 4; // �������岽
                }
                else
                {
                    processnum = 0;
                    lv_label_set_text(hint_label, EnsureMessage_UTF8(ensure));
                }
                rt_thread_mdelay(1200);
                break;

            case 4:

                ensure = PS_StoreChar(CharBuffer2, num1); // ����ģ��
                if (ensure == 0x00)
                {
                    lv_label_set_text(hint_label, ""
                                                  "\xE5\xBD\x95" /*¼*/ ""
                                                  "\xE5\x85\xA5" /*��*/ ""
                                                  "\xE6\x8C\x87" /*ָ*/ ""
                                                  "\xE7\xBA\xB9" /*��*/ ""
                                                  "\xE6\x88\x90" /*��*/ ""
                                                  "\xE5\x8A\x9F" /*��*/ "");

                    PS_ValidTempleteNum(&ValidN); // ����ָ�Ƹ���

                    return;
                }
                else
                {
                    processnum = 0;
                    lv_label_set_text(hint_label, EnsureMessage_UTF8(ensure));
                }
                break;
            }
            rt_thread_mdelay(400);
            if (i == 5) // ����5��û�а���ָ���˳�
            {

                break;
            }
        }


}
