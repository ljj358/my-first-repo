/*
 * steering_engine.c
 *
 * created: 2024/1/3
 *  author: 
 */

#include "steering_engine.h"
#include "lvgl_ui.h"
#include "lvgl-7.0.1/lvgl.h"
#include "pwm_ic.h"
lv_obj_t *label_steering_engine = NULL;
static void event_steering_engine_esc(lv_obj_t *obj, lv_event_t event)
{
        if (event == LV_EVENT_SHORT_CLICKED)
        { /* 对象被点击 */

                lv_obj_set_hidden(main_create, false); // 显示列表
                lv_obj_t *parent_obj = lv_obj_get_parent(obj);
                lv_obj_del_async(parent_obj);
        }
}

static void event_steering_engine(lv_obj_t *obj, lv_event_t event)
{
     if (event == LV_EVENT_VALUE_CHANGED)
        { /* 对象被点击 */
                    unsigned int steering_engine_data = 0;
                   steering_engine_data = lv_slider_get_value(obj);
                   lv_label_set_text_fmt(label_steering_engine, """\xE8\x88\xB5"/*舵*/"""\xE6\x9C\xBA"/*机*/"""\xE8\xA7\x92"/*角*/"""\xE5\xBA\xA6"/*度*/":%d",steering_engine_data );
                   PCA_MG9XX(1,steering_engine_data);
                 
        }
       
}

void Quest_steering_engine(void)
{
      

        lv_obj_t *tile1 = lv_obj_create(lv_scr_act(), NULL);
        lv_obj_set_size(tile1, LV_HOR_RES, LV_VER_RES);

        lv_obj_t *label;
        lv_obj_t *btn1 = lv_btn_create(tile1, NULL);
        lv_obj_set_event_cb(btn1, event_steering_engine_esc);
        lv_obj_align(btn1, tile1, LV_ALIGN_IN_TOP_LEFT, 5, 5);
        label = lv_label_create(btn1, NULL);
        lv_label_set_text(label, "Esc");
        
        lv_obj_t *slider_steering_engine = lv_slider_create(tile1, NULL);
        lv_obj_align(slider_steering_engine, NULL, LV_ALIGN_CENTER, 0, 0);
        lv_slider_set_range(slider_steering_engine, 0, 180); // 如果不设置，默认是(0-100)
        lv_obj_set_size(slider_steering_engine, 400, 30);
        lv_obj_set_event_cb(slider_steering_engine, event_steering_engine);
        label_steering_engine = lv_label_create(tile1, NULL);
        lv_obj_set_style_local_text_font(label_steering_engine, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, &hz_simsun_3500_24);
        lv_label_set_text(label_steering_engine, """\xE8\x88\xB5"/*舵*/"""\xE6\x9C\xBA"/*机*/"""\xE8\xA7\x92"/*角*/"""\xE5\xBA\xA6"/*度*/":0"); // 设置初始值
        lv_obj_align(label_steering_engine, NULL, LV_ALIGN_CENTER, 0, 50);
        
        
}
