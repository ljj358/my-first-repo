/*
 * LED.c
 *
 * created: 2024/1/2
 *  author:
 */

#include "Quest_LED.h"
#include "lvgl_ui.h"
#include "lvgl-7.0.1/lvgl.h"
#include "led.h"
static lv_obj_t *label_rgb_red = NULL;
static lv_obj_t *label_rgb_green = NULL;
static lv_obj_t *label_rgb_blue = NULL;
static void event_LED_esc(lv_obj_t *obj, lv_event_t event)
{
        if (event == LV_EVENT_SHORT_CLICKED)
        { /* 对象被点击 */

                lv_obj_set_hidden(main_create, false); // 显示列表
                LEDx_Set_Status(OFF);
                lv_obj_t *parent_obj = lv_obj_get_parent(obj);
                lv_obj_del_async(parent_obj);
        }
}
static void event_red_led(lv_obj_t *obj, lv_event_t event)
{
        if (event == LV_EVENT_VALUE_CHANGED)
        {

                if (lv_switch_get_state(obj))
                        LEDx_Set_Status(redRGB_ON);
                else
                        LEDx_Set_Status(redRGB_OFF);
        }
}
static void event_green_led(lv_obj_t *obj, lv_event_t event)
{
        if (event == LV_EVENT_VALUE_CHANGED)
        {
                if (lv_switch_get_state(obj))
                        LEDx_Set_Status(greenRGB_ON);
                else
                        LEDx_Set_Status(greenRGB_OFF);
        }
}
static void event_blue_led(lv_obj_t *obj, lv_event_t event)
{
        if (event == LV_EVENT_VALUE_CHANGED)
        {
                if (lv_switch_get_state(obj))
                        LEDx_Set_Status(blueRGB_ON);
                else
                        LEDx_Set_Status(blueRGB_OFF);
        }
}
static void event_rgb_red(lv_obj_t *obj, lv_event_t event)
{
        if (event == LV_EVENT_VALUE_CHANGED)
        {
                unsigned int rgb_data = lv_slider_get_value(obj);
                lv_label_set_text_fmt(label_rgb_red, """\xE7\xBA\xA2"/*红*/"""\xE8\x89\xB2"/*色*/"LED""\xE4\xBA\xAE"/*亮*/"""\xE5\xBA\xA6"/*度*/":%d",rgb_data);
                set_RGB(rgb_red, rgb_data);
        }
}
static void event_rgb_green(lv_obj_t *obj, lv_event_t event)
{
        if (event == LV_EVENT_VALUE_CHANGED)
        {
                unsigned int rgb_data = lv_slider_get_value(obj);
                lv_label_set_text_fmt(label_rgb_green, """\xE7\xBB\xBF"/*绿*/"""\xE8\x89\xB2"/*色*/"LED""\xE4\xBA\xAE"/*亮*/"""\xE5\xBA\xA6"/*度*/":%d",rgb_data);
                set_RGB(rgb_green, rgb_data);
        }
}
static void event_rgb_blue(lv_obj_t *obj, lv_event_t event)
{
        if (event == LV_EVENT_VALUE_CHANGED)
        {
                unsigned int rgb_data = lv_slider_get_value(obj);
                lv_label_set_text_fmt(label_rgb_blue, """\xE8\x93\x9D"/*蓝*/"""\xE8\x89\xB2"/*色*/"LED""\xE4\xBA\xAE"/*亮*/"""\xE5\xBA\xA6"/*度*/":%d",rgb_data);
                set_RGB(rgb_blue, rgb_data);
        }
}
void Quest_LED(void)
{
        LEDx_Set_Status(OFF);

        lv_obj_t *tile1 = lv_obj_create(lv_scr_act(), NULL);
        lv_obj_set_size(tile1, LV_HOR_RES, LV_VER_RES);

        lv_obj_t *label;
        lv_obj_t *btn1 = lv_btn_create(tile1, NULL);
        lv_obj_set_event_cb(btn1, event_LED_esc);
        lv_obj_align(btn1, tile1, LV_ALIGN_IN_TOP_LEFT, 5, 5);
        label = lv_label_create(btn1, NULL);
        lv_label_set_text(label, "Esc");

        /*Create a switch and apply the styles*/
        lv_obj_t *sw1 = lv_switch_create(tile1, NULL);
        lv_obj_set_size(sw1, 100, 50);
        lv_switch_off(sw1, LV_ANIM_OFF);
        lv_obj_align(sw1, tile1, LV_ALIGN_IN_LEFT_MID, 650, -150);
        lv_obj_set_event_cb(sw1, event_red_led);

        lv_obj_t *red_label = lv_label_create(tile1, NULL);
        lv_obj_add_style(red_label, LV_LABEL_PART_MAIN, &style);
        lv_label_set_text(red_label, "red");
        lv_obj_align(red_label, tile1, LV_ALIGN_IN_LEFT_MID, 690, -110);

        /*Copy the first switch and turn it ON*/
        lv_obj_t *sw2 = lv_switch_create(tile1, sw1);
        lv_obj_align(sw2, NULL, LV_ALIGN_IN_LEFT_MID, 650, 0);

        lv_obj_t *green_label = lv_label_create(tile1, NULL);
        lv_obj_add_style(green_label, LV_LABEL_PART_MAIN, &style);
        lv_label_set_text(green_label, "green");
        lv_obj_align(green_label, tile1, LV_ALIGN_IN_LEFT_MID, 690, 40);
        lv_obj_set_event_cb(sw2, event_green_led);

        /*Copy the first switch and turn it ON*/
        lv_obj_t *sw3 = lv_switch_create(tile1, sw1);
        lv_obj_align(sw3, NULL, LV_ALIGN_IN_LEFT_MID, 650, 150);
        lv_obj_t *blue_label = lv_label_create(tile1, NULL);
        lv_obj_add_style(blue_label, LV_LABEL_PART_MAIN, &style);
        lv_label_set_text(blue_label, "blue");
        lv_obj_align(blue_label, tile1, LV_ALIGN_IN_LEFT_MID, 690, 190);

        lv_obj_set_event_cb(sw3, event_blue_led);


        set_RGB(rgb_blue, 0);
        set_RGB(rgb_green, 0);
        set_RGB(rgb_red, 0);

        lv_obj_t *slider_red = lv_slider_create(tile1, NULL);
        lv_obj_align(slider_red, NULL, LV_ALIGN_IN_TOP_MID, -80, 50);
        lv_slider_set_range(slider_red, 0, 256); // 如果不设置，默认是(0-100)
        lv_obj_set_size(slider_red, 400, 30);
        lv_obj_set_event_cb(slider_red, event_rgb_red);
        label_rgb_red = lv_label_create(tile1, NULL);
        lv_label_set_text(label_rgb_red, """\xE7\xBA\xA2"/*红*/"""\xE8\x89\xB2"/*色*/"LED""\xE4\xBA\xAE"/*亮*/"""\xE5\xBA\xA6"/*度*/":0"); // 设置初始值
        lv_obj_align(label_rgb_red, NULL, LV_ALIGN_IN_TOP_MID, 0, 100);

        lv_obj_t *slider_green = lv_slider_create(tile1, slider_red);
        lv_obj_align(slider_green, NULL, LV_ALIGN_IN_TOP_MID, -0, 200);
        label_rgb_green = lv_label_create(tile1, NULL);
        lv_label_set_text(label_rgb_green, """\xE7\xBB\xBF"/*绿*/"""\xE8\x89\xB2"/*色*/"LED""\xE4\xBA\xAE"/*亮*/"""\xE5\xBA\xA6"/*度*/":0"); // 设置初始值
        lv_obj_align(label_rgb_green, NULL, LV_ALIGN_IN_TOP_MID, 0, 250);
        lv_obj_set_event_cb(slider_green, event_rgb_green);

        lv_obj_t *slider_blue = lv_slider_create(tile1, slider_red);
        lv_obj_align(slider_blue, NULL, LV_ALIGN_IN_TOP_MID, -0, 350);
        label_rgb_blue = lv_label_create(tile1, NULL);
        lv_label_set_text(label_rgb_blue, """\xE8\x93\x9D"/*蓝*/"""\xE8\x89\xB2"/*色*/"LED""\xE4\xBA\xAE"/*亮*/"""\xE5\xBA\xA6"/*度*/":0"); // 设置初始值
        lv_obj_align(label_rgb_blue, NULL, LV_ALIGN_IN_TOP_MID, 0, 400);
        lv_obj_set_event_cb(slider_blue, event_rgb_blue);
}
