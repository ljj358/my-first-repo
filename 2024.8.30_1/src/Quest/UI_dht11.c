/*
 * UI_tht11.c
 *
 * created: 2024/5/21
 *  author:
 */
#include "lvgl_ui.h"
#include "lvgl-7.0.1/lvgl.h"
#include "ls1b_gpio.h"
#include "dht11.h"

static rt_thread_t dht11_thread = NULL;
lv_obj_t *label1 = NULL, *label2 = NULL;

static void event_dht11_esc(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_SHORT_CLICKED)
    { /* ���󱻵�� */

        lv_obj_set_hidden(main_create, false); // ��ʾ�б�
        lv_obj_t *parent_obj = lv_obj_get_parent(obj);
        lv_obj_del_async(parent_obj);
    }
    if (dht11_thread != NULL)
    {
        if (RT_EOK == rt_thread_delete(dht11_thread))
        {
            rt_kprintf("����\"dht11_thread\"ɾ���ɹ�\r\n");
            dht11_thread = NULL;
        }
        else
            rt_kprintf("����\"dht11_thread\"ɾ��ʧ��\r\n");
    }
}

static void dth11_thread_event(void *arg)
{

    for (;;)
    {
        DHT11_REC_Data();
        lv_label_set_text_fmt(label1, ""
                                      "\xE6\xB8\xA9" /*��*/ ""
                                      "\xE5\xBA\xA6" /*��*/ ":%d",
                              rec_data[2]);
        lv_label_set_text_fmt(label2, ""
                                      "\xE6\xB9\xBF" /*ʪ*/ ""
                                      "\xE5\xBA\xA6" /*��*/ ":%d",
                              rec_data[0]);
        rt_thread_delay(1000);
    }
}

void dht11_event_cb(void)
{
    lv_obj_t *tile1 = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(tile1, LV_HOR_RES, LV_VER_RES);

    lv_obj_t *label;
    lv_obj_t *btn1 = lv_btn_create(tile1, NULL);
    lv_obj_set_event_cb(btn1, event_dht11_esc);
    lv_obj_align(btn1, tile1, LV_ALIGN_IN_TOP_LEFT, 5, 5);
    label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, "Esc");

    // ������һ����ǩ
    label1 = lv_label_create(tile1, NULL);
    lv_obj_add_style(label1, LV_LABEL_PART_MAIN, &style); // �������������
    lv_label_set_long_mode(label1, LV_LABEL_LONG_BREAK);  // ����
    lv_label_set_align(label1, LV_LABEL_ALIGN_LEFT);      // �����
    lv_label_set_text(label1, ""
                              "\xE6\xB8\xA9" /*��*/ ""
                              "\xE5\xBA\xA6" /*��*/ ":0");
    lv_obj_set_width(label1, 150);
    lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 0, -30);

    // �����ڶ�����ǩ
    label2 = lv_label_create(tile1, NULL);
    lv_obj_add_style(label2, LV_LABEL_PART_MAIN, &style); // �������������
    lv_label_set_long_mode(label2, LV_LABEL_LONG_BREAK);  // ����
    lv_label_set_align(label2, LV_LABEL_ALIGN_LEFT);      // �����
    lv_label_set_text(label2, ""
                              "\xE6\xB9\xBF" /*ʪ*/ ""
                              "\xE5\xBA\xA6" /*��*/ ":0");
    lv_obj_set_width(label2, 150);
    lv_obj_align(label2, NULL, LV_ALIGN_CENTER, 0, 30);

    if (dht11_thread == NULL)
    { // ��ֹ�ظ������߳�
        // �����߳�
        dht11_thread = rt_thread_create("dht11_thread",
                                        dth11_thread_event,
                                        NULL,     // arg
                                        1024 * 1, // statck size
                                        10,       // priority
                                        1);       // slice ticks

        if (dht11_thread == NULL)
        {
            rt_kprintf("����\"dht11_thread\"����ʧ��\r\n");
            return;
        }
        else
        {
            rt_kprintf("����\dht11_thread\"�����ɹ�\r\n");
            rt_thread_startup(dht11_thread);
        }
    }
}
