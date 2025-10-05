/*
 * Combination_lock.c
 *
 * created: 2024/1/12
 *  author:
 */
#include "lvgl_ui.h"
#include "Combination_lock.h"
static lv_obj_t *hint_label = NULL;    // 提示文本
static lv_obj_t *pwd_text_area = NULL; // 密码输入框

static const char *keyboard_map[] =
    {
        "1", "2", "3", "\n",
        "4", "5", "6", "\n",
        "7", "8", "9", "\n",
        "0", "X", "OK", ""};

static void my_task(lv_task_t *task)
{
    lv_label_set_text(hint_label,"""\xE8\x88\xB5"/*舵*/"""\xE6\x9C\xBA"/*机*/"""\xE5\xBD\x93"/*当*/"""\xE5\x89\x8D"/*前*/"""\xE8\xA7\x92"/*角*/"""\xE5\xBA\xA6"/*度*/":0" );
    PCA_MG9XX(1, 0);
}

// 确认按键的事件回调
static void btn_event_callback(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED)
    {
        uint16_t ID = lv_btnmatrix_get_active_btn(obj);       // 获取ID
        const char *txt = lv_btnmatrix_get_btn_text(obj, ID); // 获取文本
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
                lv_label_set_text(hint_label, """\xE8\x88\xB5"/*舵*/"""\xE6\x9C\xBA"/*机*/"""\xE5\xBD\x93"/*当*/"""\xE5\x89\x8D"/*前*/"""\xE8\xA7\x92"/*角*/"""\xE5\xBA\xA6"/*度*/":90");
                lv_task_t *task = lv_task_create(my_task, 2000, LV_TASK_PRIO_MID, NULL); // 2s后舵机角度设置成0度
                lv_task_once(task);                                                      // 定时器执行1次后删除
            }
        }
    }
}

static void event_Combination_lock(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_SHORT_CLICKED)
    { /* 对象被点击 */

        lv_obj_set_hidden(main_create, false); // 显示列表
        lv_obj_t *parent_obj = lv_obj_get_parent(obj);
        lv_obj_del_async(parent_obj);
    }
}

void Combination_lock(void)
{
    PCA_MG9XX(1, 0);
    lv_obj_t *tile1 = lv_obj_create(lv_scr_act(), NULL);
    // LV_HOR_RES 当前屏幕的水平分辨率
    // LV_VER_RES 当前屏幕的垂直分辨率
    lv_obj_set_size(tile1, LV_HOR_RES, LV_VER_RES);

    lv_obj_t *label; // 返回按钮
    lv_obj_t *btn1 = lv_btn_create(tile1, NULL);
    lv_obj_set_event_cb(btn1, event_Combination_lock);
    lv_obj_align(btn1, tile1, LV_ALIGN_IN_TOP_LEFT, 5, 5);
    label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, "Esc");

    lv_obj_t *pwd_main_cont = lv_obj_create(tile1, NULL); // 密码输入界面的容器
                                                          // 创建一个样式
    static lv_style_t main_cont_style;
    // 清除样式中的所有属性和所有分配的内存。
    lv_style_reset(&main_cont_style);
    // 初始化样式
    lv_style_init(&main_cont_style);
    lv_style_set_radius(&main_cont_style, LV_STATE_DEFAULT, 20);               // 设置样式的圆角弧度
    lv_style_set_border_width(&main_cont_style, LV_STATE_DEFAULT, 0);          // 设置边框宽度
    lv_style_set_bg_opa(&main_cont_style, LV_STATE_DEFAULT, LV_OPA_100);       // 设置样式背景的透明度，不透明
    lv_style_set_bg_color(&main_cont_style, LV_STATE_DEFAULT, LV_COLOR_WHITE); // 白色背景
    lv_style_set_pad_inner(&main_cont_style, LV_STATE_DEFAULT, 0);             // 不填充
    lv_obj_add_style(pwd_main_cont, NULL, &main_cont_style);                   // 给对象添加样式
    lv_obj_set_size(pwd_main_cont, 400, 480);                                  // 设置大小800x480
    lv_obj_align(pwd_main_cont, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);              // 位置

    // 创建一个样式
    static lv_style_t btnmatrix_style;
    // 清除样式中的所有属性和所有分配的内存。
    lv_style_reset(&btnmatrix_style);
    // 初始化样式
    lv_style_init(&btnmatrix_style);
    lv_style_set_radius(&btnmatrix_style, LV_STATE_DEFAULT, 5);                // 设置样式的圆角弧度
    lv_style_set_border_width(&btnmatrix_style, LV_STATE_DEFAULT, 2);          // 设置边框宽度
    lv_style_set_bg_opa(&btnmatrix_style, LV_STATE_DEFAULT, LV_OPA_100);       // 设置样式背景的透明度，不透明
    lv_style_set_bg_color(&btnmatrix_style, LV_STATE_DEFAULT, LV_COLOR_WHITE); // 白色背景
    lv_style_set_pad_inner(&btnmatrix_style, LV_STATE_DEFAULT, 10);            // 设置对象内部的行间距（子对象间的垂直距离）

    // 创建一个样式
    static lv_style_t btnmatrix_style1;
    // 清除样式中的所有属性和所有分配的内存。
    lv_style_reset(&btnmatrix_style1);
    // 初始化样式
    lv_style_init(&btnmatrix_style1);
    lv_style_set_radius(&btnmatrix_style1, LV_STATE_DEFAULT, 20);                 // 设置样式的圆角弧度
    lv_style_set_border_width(&btnmatrix_style1, LV_STATE_DEFAULT, 2);            // 设置边框宽度
    lv_style_set_bg_opa(&btnmatrix_style1, LV_STATE_DEFAULT, LV_OPA_100);         // 设置样式背景的透明度，不透明
    lv_style_set_bg_color(&btnmatrix_style1, LV_STATE_DEFAULT, LV_COLOR_WHITE);   // 白色背景
    lv_style_set_text_color(&btnmatrix_style1, LV_STATE_DEFAULT, LV_COLOR_BLACK); // 设置文本颜色为黑色

    lv_obj_t *btnm1 = lv_btnmatrix_create(pwd_main_cont, NULL);
    // lv_btnmatrix_set_recolor(btnm1, true);//文本重新着色
    lv_btnmatrix_set_map(btnm1, keyboard_map);

    lv_obj_add_style(btnm1, LV_BTNMATRIX_PART_BG, &btnmatrix_style);   // 给对象添加样式
    lv_obj_add_style(btnm1, LV_BTNMATRIX_PART_BTN, &btnmatrix_style1); // 给对象添加样式
    lv_obj_set_size(btnm1, 400, 300);                                  // 设置大小800x480
    lv_obj_align(btnm1, NULL, LV_ALIGN_IN_TOP_MID, 0, 180);            // 位置
    lv_obj_set_event_cb(btnm1, btn_event_callback);                    // 设置矩阵按钮回掉函数

    


    pwd_text_area = lv_textarea_create(pwd_main_cont, NULL); // 密码输入文本框
                                                             // 创建一个样式
    static lv_style_t TXT_style;
    // 清除样式中的所有属性和所有分配的内存。
    lv_style_reset(&TXT_style);
    // 初始化样式
    lv_style_init(&TXT_style);
    lv_style_set_radius(&TXT_style, LV_STATE_DEFAULT, 0);                  // 设置样式的圆角弧度
    lv_style_set_border_width(&TXT_style, LV_STATE_DEFAULT, 0);            // 设置边框宽度
    lv_style_set_bg_opa(&TXT_style, LV_STATE_DEFAULT, LV_OPA_100);         // 设置样式背景的透明度，不透明
    lv_style_set_bg_color(&TXT_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);   // 白色背景
    lv_style_set_pad_inner(&TXT_style, LV_STATE_DEFAULT, 0);               // 填充
    lv_style_set_text_color(&TXT_style, LV_STATE_DEFAULT, LV_COLOR_BLACK); // 设置文本颜色为黑色
    lv_obj_add_style(pwd_text_area, NULL, &TXT_style);                     // 给对象添加样式
    lv_obj_add_style(pwd_text_area, LV_LABEL_PART_MAIN, &style);           // 给对象添加字体
    lv_obj_set_size(pwd_text_area, 200, 100);                              // 对象大小
    
    label = lv_label_create(pwd_main_cont, NULL);
    lv_obj_add_style(label, LV_LABEL_PART_MAIN, &style);           // 给对象添加字体
    lv_obj_add_style(label, NULL, &TXT_style);                     // 给对象添加样式
    lv_label_set_text(label, """\xE8\xBE\x93"/*输*/"""\xE5\x85\xA5"/*入*/"""\xE5\xAF\x86"/*密*/"""\xE7\xA0\x81"/*码*/":");
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 10);         // 对象位置


    lv_obj_align(pwd_text_area, label, LV_ALIGN_OUT_RIGHT_TOP, 0, -10);         // 对象位置
    lv_textarea_set_text(pwd_text_area,"" );                               // 清空内容

    hint_label = lv_label_create(pwd_main_cont, NULL);                                                   // 创建标签
    lv_obj_set_style_local_text_color(hint_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK); // 文本设置成黑色
    lv_obj_add_style(hint_label, LV_LABEL_PART_MAIN, &style);                                            // 设置字体
    lv_label_set_text(hint_label, """\xE8\x88\xB5"/*舵*/"""\xE6\x9C\xBA"/*机*/"""\xE5\xBD\x93"/*当*/"""\xE5\x89\x8D"/*前*/"""\xE8\xA7\x92"/*角*/"""\xE5\xBA\xA6"/*度*/":0");
    lv_obj_align(hint_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 150);
}
