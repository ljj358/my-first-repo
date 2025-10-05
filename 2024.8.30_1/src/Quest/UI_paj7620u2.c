/*
 * UI_paj7620u2.c
 *
 * created: 2024/5/21
 *  author: 
 */

#include "UI_paj7620u2.h"
#include "paj7620u2.h"
#include "lvgl_ui.h"
#include "lvgl-7.0.1/lvgl.h"
#include "ls1b_gpio.h"

static rt_thread_t paj7620u2_thread = NULL;
static lv_obj_t *label1 = NULL;
extern char RW2_open;
static void event_paj7620u2_esc(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_SHORT_CLICKED)
    { /* ���󱻵�� */

        lv_obj_set_hidden(main_create, false); // ��ʾ�б�
        lv_obj_t *parent_obj = lv_obj_get_parent(obj);
        lv_obj_del_async(parent_obj);
        RW2_open = 1;
    }
    if (paj7620u2_thread != NULL)
    {
        if (RT_EOK == rt_thread_delete(paj7620u2_thread))
        {
            rt_kprintf("����\"paj7620u2_thread\"ɾ���ɹ�\r\n");
            paj7620u2_thread = NULL;
        }
        else
            rt_kprintf("����\"paj7620u2_thread\"ɾ��ʧ��\r\n");
    }
}

static void paj7620u2_thread_event(void *arg)
{
    PAJ7620_Init();
    char buf[15] = {0};
    GestureData gesture;
    for (;;)
    {
       rt_thread_mdelay(130);

		//����ʶ��
		gesture.detect = GS_Read_nByte(PAJ_GET_INT_FLAG1,2,&gesture.data[0]);//��ȡ����״̬
		if(!gesture.detect)
		{
			gesture.type =(unsigned short)gesture.data[1]<<8 | gesture.data[0];
			if(gesture.type)
			{
				switch(gesture.type)
				{
					case GES_UP:               sprintf(buf,"""\xE5\x90\x91"/*��*/"""\xE4\xB8\x8A"/*��*/"\r\n");            gesture.valid=1;   break; //����
					case GES_DOWN:             sprintf(buf,"""\xE5\x90\x91"/*��*/"""\xE4\xB8\x8B"/*��*/"\r\n");            gesture.valid=1;   break; //����
					case GES_LEFT:             sprintf(buf,"""\xE5\x90\x91"/*��*/"""\xE5\xB7\xA6"/*��*/"\r\n");            gesture.valid=1;   break; //����
					case GES_RIGHT:            sprintf(buf,"""\xE5\x90\x91"/*��*/"""\xE5\x8F\xB3"/*��*/"\r\n");            gesture.valid=1;   break; //����
					case GES_FORWARD:          sprintf(buf,"""\xE5\x90\x91"/*��*/"""\xE5\x89\x8D"/*ǰ*/"\r\n");            gesture.valid=1;   break; //��ǰ
					case GES_BACKWARD:         sprintf(buf,"""\xE5\x90\x91"/*��*/"""\xE5\x90\x8E"/*��*/"\r\n");            gesture.valid=1;   break; //���
					case GES_CLOCKWISE:        sprintf(buf,"""\xE9\xA1\xBA"/*˳*/"""\xE6\x97\xB6"/*ʱ*/"""\xE9\x92\x88"/*��*/"\r\n");          gesture.valid=1;   break; //˳ʱ��
					case GES_ANTI_CLOCKWISE:   sprintf(buf,"""\xE9\x80\x86"/*��*/"""\xE6\x97\xB6"/*ʱ*/"""\xE9\x92\x88"/*��*/"\r\n");          gesture.valid=1;   break; //��ʱ��
					case GES_WAVE:             sprintf(buf,"""\xE6\x8C\xA5"/*��*/"""\xE5\x8A\xA8"/*��*/"\r\n");            gesture.valid=1;   break; //�Ӷ�
					default: gesture.valid=0;   break;
				}
				lv_label_set_text(label1,buf);
			}
		}
    }
}


   

void UI_paj7620u2_event_cb(void){
    RW2_open = 0;
    lv_obj_t *tile1 = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(tile1, LV_HOR_RES, LV_VER_RES);

    lv_obj_t *label;
    lv_obj_t *btn1 = lv_btn_create(tile1, NULL);
    lv_obj_set_event_cb(btn1, event_paj7620u2_esc);
    lv_obj_align(btn1, tile1, LV_ALIGN_IN_TOP_LEFT, 5, 5);
    label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, "Esc");

    // ������һ����ǩ
    label1 = lv_label_create(tile1, NULL);
    lv_obj_add_style(label1, LV_LABEL_PART_MAIN, &style); // �������������
    lv_label_set_long_mode(label1, LV_LABEL_LONG_BREAK);  // ����
    lv_label_set_align(label1, LV_LABEL_ALIGN_LEFT);      // �����
    lv_label_set_text(label1, """\xE6\x89\x8B"/*��*/"""\xE5\x8A\xBF"/*��*/":");
    lv_obj_set_width(label1, 150);
    lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 0, -30);

    

    if (paj7620u2_thread == NULL)
    { // ��ֹ�ظ������߳�
        // �����߳�
        paj7620u2_thread = rt_thread_create("paj7620u2_thread",
                                        paj7620u2_thread_event,
                                        NULL,     // arg
                                        1024 * 10, // statck size
                                        10,       // priority
                                        1);       // slice ticks

        if (paj7620u2_thread == NULL)
        {
            rt_kprintf("����\"paj7620u2_thread\"����ʧ��\r\n");
            return;
        }
        else
        {
            rt_kprintf("����\paj7620u2_thread\"�����ɹ�\r\n");
            rt_thread_startup(paj7620u2_thread);
        }
    }
    

}
    


