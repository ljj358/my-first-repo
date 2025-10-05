/*
 * Combination_lock.c
 *
 * created: 2024/1/12
 *  author:
 */
#include "lvgl_ui.h"
#include "Combination_lock.h"
static lv_obj_t *hint_label = NULL;    // ��ʾ�ı�
static lv_obj_t *pwd_text_area = NULL; // ���������

static const char *keyboard_map[] =
    {
        "1", "2", "3", "\n",
        "4", "5", "6", "\n",
        "7", "8", "9", "\n",
        "0", "X", "OK", ""};

static void my_task(lv_task_t *task)
{
    lv_label_set_text(hint_label,"""\xE8\x88\xB5"/*��*/"""\xE6\x9C\xBA"/*��*/"""\xE5\xBD\x93"/*��*/"""\xE5\x89\x8D"/*ǰ*/"""\xE8\xA7\x92"/*��*/"""\xE5\xBA\xA6"/*��*/":0" );
    PCA_MG9XX(1, 0);
}

// ȷ�ϰ������¼��ص�
static void btn_event_callback(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED)
    {
        uint16_t ID = lv_btnmatrix_get_active_btn(obj);       // ��ȡID
        const char *txt = lv_btnmatrix_get_btn_text(obj, ID); // ��ȡ�ı�
        if (ID != 10 && ID != 11 && ID < 12)
            lv_textarea_add_text(pwd_text_area, txt);
        else if (ID == 10)
            lv_textarea_set_text(pwd_text_area, "");
        else if (ID == 11)
        {
            const char *text_data = lv_textarea_get_text(pwd_text_area);
            if (strcmp(text_data, "2023") == 0)
            {
                PCA_MG9XX(1, 180);
                lv_label_set_text(hint_label, """\xE8\x88\xB5"/*��*/"""\xE6\x9C\xBA"/*��*/"""\xE5\xBD\x93"/*��*/"""\xE5\x89\x8D"/*ǰ*/"""\xE8\xA7\x92"/*��*/"""\xE5\xBA\xA6"/*��*/":90");
                lv_task_t *task = lv_task_create(my_task, 2000, LV_TASK_PRIO_MID, NULL); // 2s�����Ƕ����ó�0��
                lv_task_once(task);                                                      // ��ʱ��ִ��1�κ�ɾ��
            }
        }
    }
}

static void event_Combination_lock(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_SHORT_CLICKED)
    { /* ���󱻵�� */

        lv_obj_set_hidden(main_create, false); // ��ʾ�б�
        lv_obj_t *parent_obj = lv_obj_get_parent(obj);
        lv_obj_del_async(parent_obj);
    }
}

void Combination_lock(void)
{
    PCA_MG9XX(1, 0);
    lv_obj_t *tile1 = lv_obj_create(lv_scr_act(), NULL);
    // LV_HOR_RES ��ǰ��Ļ��ˮƽ�ֱ���
    // LV_VER_RES ��ǰ��Ļ�Ĵ�ֱ�ֱ���
    lv_obj_set_size(tile1, LV_HOR_RES, LV_VER_RES);

    lv_obj_t *label; // ���ذ�ť
    lv_obj_t *btn1 = lv_btn_create(tile1, NULL);
    lv_obj_set_event_cb(btn1, event_Combination_lock);
    lv_obj_align(btn1, tile1, LV_ALIGN_IN_TOP_LEFT, 5, 5);
    label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, "Esc");

    lv_obj_t *pwd_main_cont = lv_obj_create(tile1, NULL); // ����������������
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
    lv_obj_set_size(pwd_main_cont, 400, 480);                                  // ���ô�С800x480
    lv_obj_align(pwd_main_cont, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);              // λ��

    // ����һ����ʽ
    static lv_style_t btnmatrix_style;
    // �����ʽ�е��������Ժ����з�����ڴ档
    lv_style_reset(&btnmatrix_style);
    // ��ʼ����ʽ
    lv_style_init(&btnmatrix_style);
    lv_style_set_radius(&btnmatrix_style, LV_STATE_DEFAULT, 5);                // ������ʽ��Բ�ǻ���
    lv_style_set_border_width(&btnmatrix_style, LV_STATE_DEFAULT, 2);          // ���ñ߿���
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
    // lv_btnmatrix_set_recolor(btnm1, true);//�ı�������ɫ
    lv_btnmatrix_set_map(btnm1, keyboard_map);

    lv_obj_add_style(btnm1, LV_BTNMATRIX_PART_BG, &btnmatrix_style);   // �����������ʽ
    lv_obj_add_style(btnm1, LV_BTNMATRIX_PART_BTN, &btnmatrix_style1); // �����������ʽ
    lv_obj_set_size(btnm1, 400, 300);                                  // ���ô�С800x480
    lv_obj_align(btnm1, NULL, LV_ALIGN_IN_TOP_MID, 0, 180);            // λ��
    lv_obj_set_event_cb(btnm1, btn_event_callback);                    // ���þ���ť�ص�����

    


    pwd_text_area = lv_textarea_create(pwd_main_cont, NULL); // ���������ı���
                                                             // ����һ����ʽ
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
    lv_obj_add_style(label, LV_LABEL_PART_MAIN, &style);           // �������������
    lv_obj_add_style(label, NULL, &TXT_style);                     // �����������ʽ
    lv_label_set_text(label, """\xE8\xBE\x93"/*��*/"""\xE5\x85\xA5"/*��*/"""\xE5\xAF\x86"/*��*/"""\xE7\xA0\x81"/*��*/":");
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 10);         // ����λ��


    lv_obj_align(pwd_text_area, label, LV_ALIGN_OUT_RIGHT_TOP, 0, -10);         // ����λ��
    lv_textarea_set_text(pwd_text_area,"" );                               // �������

    hint_label = lv_label_create(pwd_main_cont, NULL);                                                   // ������ǩ
    lv_obj_set_style_local_text_color(hint_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK); // �ı����óɺ�ɫ
    lv_obj_add_style(hint_label, LV_LABEL_PART_MAIN, &style);                                            // ��������
    lv_label_set_text(hint_label, """\xE8\x88\xB5"/*��*/"""\xE6\x9C\xBA"/*��*/"""\xE5\xBD\x93"/*��*/"""\xE5\x89\x8D"/*ǰ*/"""\xE8\xA7\x92"/*��*/"""\xE5\xBA\xA6"/*��*/":0");
    lv_obj_align(hint_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 150);
}
