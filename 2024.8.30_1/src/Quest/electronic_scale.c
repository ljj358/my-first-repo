/*
 * electronic_scale.c
 *
 * created: 2024/1/4
 *  author: 
 */

#include "lvgl_ui.h"
#include "lvgl-7.0.1/lvgl.h"
#include "electronic_scale.h"
#include "HX711.h"
#include "main.h"
lv_obj_t *labe2 = NULL;
static rt_thread_t electronic_scale_thread = NULL;
static lv_obj_t * spinbox;
static void electronic_scale_esc(lv_obj_t *obj, lv_event_t event)
{
        if (event == LV_EVENT_SHORT_CLICKED)
        { /* 对象被点击 */

                lv_obj_set_hidden(main_create, false); // 显示列表
                lv_obj_t *parent_obj = lv_obj_get_parent(obj);
                lv_obj_del_async(parent_obj);
                 if(RT_EOK ==  rt_thread_delete(electronic_scale_thread)){
                    rt_kprintf("electronic_scale_thread删除成功\r\n");
                }
                else rt_kprintf("electronic_scale_thread删除失败\r\n");
                electronic_scale_thread = NULL;
        }
}
 static void lv_spinbox_increment_event_cb(lv_obj_t * btn, lv_event_t e)
    {
            if(e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT) {
                    lv_spinbox_increment(spinbox); //+
                    //rt_kprintf("%0.1f\r\n",lv_spinbox_get_value(spinbox)/10.0);
                    GapValue = lv_spinbox_get_value(spinbox)/100.0;
            }
    }

    static void lv_spinbox_decrement_event_cb(lv_obj_t * btn, lv_event_t e)
    {
            if(e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT) {
                    lv_spinbox_decrement(spinbox); //-
                    //rt_kprintf("%0.2f\r\n",lv_spinbox_get_value(spinbox)/10.0);
                    GapValue = lv_spinbox_get_value(spinbox)/100.0;
           }
    }
static void electronic_scale_thread1(void *arg){
    Get_Maopi();
    float data = 10;
    char buf[10] = {0};
    while(1){
        data = Get_Weight()/10.0;
        rt_kprintf("data = %d\r\n",data);
        sprintf(buf,"%""\xE9\x87\x8D"/*重*/"""\xE9\x87\x8F"/*量*/":%0.1fg",data);
        
        rt_mutex_take(ui_mutex,RT_WAITING_FOREVER );
        lv_label_set_text_fmt(labe2,buf);
        rt_mutex_release(ui_mutex);
        
        rt_thread_delay(1000);
    }
    
}
void electronic_scale(void){
            // 创建线程

    electronic_scale_thread = rt_thread_create("electronic_scale",
                                     electronic_scale_thread1,
                                     NULL,     // arg
                                     1024 * 4, // statck size
                                     15,        // priority
                                     1);      // slice ticks

        if (electronic_scale_thread == NULL)
        {
            rt_kprintf("任务“electronic_scale_thread”创建失败\r\n");
            return ;
        }
        else
        {
            rt_kprintf("任务“electronic_scale_thread”创建成功\r\n");
            rt_thread_startup(electronic_scale_thread);
        }
    
        lv_obj_t *tile1 = lv_obj_create(lv_scr_act(), NULL);
        lv_obj_set_size(tile1, LV_HOR_RES, LV_VER_RES);

        lv_obj_t *label;
        lv_obj_t *btn1 = lv_btn_create(tile1, NULL);
        lv_obj_set_event_cb(btn1, electronic_scale_esc);
        lv_obj_align(btn1, tile1, LV_ALIGN_IN_TOP_LEFT, 5, 5);
        label = lv_label_create(btn1, NULL);
        lv_label_set_text(label, "Esc");

        labe2 = lv_label_create(tile1, NULL);
        lv_obj_align(labe2, tile1, LV_ALIGN_CENTER, -50, 0);
        lv_obj_set_style_local_text_font(labe2, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, &hz_simsun_3500_24);
        lv_label_set_text(labe2, """\xE9\x87\x8D"/*重*/"""\xE9\x87\x8F"/*量*/":0g");

        spinbox = lv_spinbox_create(tile1, NULL);
        lv_spinbox_set_range(spinbox, 160*100, 170*100);
        lv_spinbox_set_digit_format(spinbox, 5, 3);
        lv_spinbox_set_step(spinbox, 1);
        lv_spinbox_step_prev(spinbox);
        lv_obj_set_width(spinbox, 100);
        lv_obj_align(spinbox, labe2, LV_ALIGN_OUT_BOTTOM_MID, 0,50);
        lv_spinbox_set_value(spinbox, (int)(GapValue*100));
        
        
        
        lv_obj_t *labe3 = lv_label_create(tile1, NULL);
        lv_obj_align(labe3, spinbox, LV_ALIGN_OUT_BOTTOM_MID, -40, 20);
        lv_obj_set_style_local_text_font(labe3, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, &hz_simsun_3500_24);
        lv_label_set_text(labe3, """\xE8\xAE\xBE"/*设*/"""\xE7\xBD\xAE"/*置*/"""\xE4\xBC\xA0"/*传*/"""\xE6\x84\x9F"/*感*/"""\xE5\x99\xA8"/*器*/"""\xE6\x9B\xB2"/*曲*/"""\xE7\x8E\x87"/*率*/"");

        lv_coord_t h = lv_obj_get_height(spinbox);
        lv_obj_t * btn2 = lv_btn_create(tile1, NULL);
        lv_obj_set_size(btn2, h, h);
        labe3 = lv_label_create(btn2, NULL);
        lv_label_set_text(labe3, "+");
        lv_obj_align(btn2, spinbox, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
        lv_theme_apply(btn2, LV_THEME_SPINBOX_BTN);
        lv_obj_set_style_local_value_str(btn2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_PLUS);
        lv_obj_set_event_cb(btn2, lv_spinbox_increment_event_cb);
        
        btn2 = lv_btn_create(tile1, btn2);
        labe3 = lv_label_create(btn2, NULL);
        lv_label_set_text(labe3, "-");
        lv_obj_align(btn2, spinbox, LV_ALIGN_OUT_LEFT_MID, -5, 0);
        lv_obj_set_event_cb(btn2, lv_spinbox_decrement_event_cb);
        lv_obj_set_style_local_value_str(btn2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_MINUS);
        
       
}

