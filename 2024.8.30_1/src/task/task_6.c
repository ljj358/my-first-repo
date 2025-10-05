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

static lv_obj_t *hint_label = NULL; // 提示文本
static rt_thread_t as608_thread = NULL;
static lv_obj_t *pwd_text_area = NULL; // 输入文本
static char RW = 0;
static int num1;
static void Add_FR(void);
static void event_task_esc(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_SHORT_CLICKED)
    { /* 对象被点击 */
        Taskshutdown = 0;
    }
}


static void RW6_thread1(void *arg)
{
    UART4_Config_Init();
    StepperMotor();          //步进电机
    int pageID, mathscore,i = 0;
    for (;;)
    {
        if (press_FR(&pageID, &mathscore))
        {
           lv_label_set_text_fmt(hint_label,"""\xE7\xA1\xAE"/*确*/"""\xE6\x9C\x89"/*有*/"""\xE6\xAD\xA4"/*此*/"""\xE4\xBA\xBA"/*人*/" ID:%d ""\xE5\x8C\xB9"/*匹*/"""\xE9\x85\x8D"/*配*/"""\xE5\xBE\x97"/*得*/"""\xE5\x88\x86"/*分*/":%d", pageID, mathscore);
           rt_kprintf("确有此人 ID:%d 匹配得分:%d", pageID, mathscore);
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
                                          "\xE8\xAF\xB7" /*请*/ ""
                                          "\xE6\x8C\x89" /*按*/ ""
                                          "\xE6\x8C\x87" /*指*/ ""
                                          "\xE7\xBA\xB9" /*纹*/ "");
                                          Add_FR();
                                          RW = 0;
        }
        if(Taskshutdown != 6)
        {
            rt_mutex_take(ui_mutex, RT_WAITING_FOREVER);
            lv_obj_set_hidden(main_create, false); // 显示列表
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

// 确认按键的事件回调
static void btn_event_callback(lv_obj_t *obj, lv_event_t event)
{
    static char enter = 0;
    if (event == LV_EVENT_VALUE_CHANGED)
    {
        uint16_t ID = lv_btnmatrix_get_active_btn(obj);       // 获取ID
        const char *txt = lv_btnmatrix_get_btn_text(obj, ID); // 获取文本
        char *buf[100] = {0};
        if (ID != 13 && ID != 0 && ID != 1 && ID != 2 && ID != 14 && ID <= 14)
            lv_textarea_add_text(pwd_text_area, txt);
        else if (ID == 13)
            lv_textarea_set_text(pwd_text_area, "");
        else if (ID == 0)
        {
            enter = 1;
            lv_label_set_text(hint_label, ""
                                          "\xE8\xBE\x93" /*输*/ ""
                                          "\xE5\x85\xA5" /*入*/ ""
                                          "\xE6\x8C\x87" /*指*/ ""
                                          "\xE7\xBA\xB9" /*纹*/ "ID"
                                          "\xE5\x90\x8E" /*后*/ ""
                                          "\xE6\x8C\x89" /*按*/ ""
                                          "\xE4\xB8\x8B" /*下*/ ""
                                          "\xE5\x9B\x9E" /*回*/ ""
                                          "\xE8\xBD\xA6" /*车*/ ""
                                          "\xE9\x94\xAE" /*键*/ ""
                                          "\xE5\x88\xA0" /*删*/ ""
                                          "\xE9\x99\xA4" /*除*/ ""
                                          "\xE6\x8C\x87" /*指*/ ""
                                          "\xE7\xBA\xB9" /*纹*/ "");
        }
        else if (ID == 2)
        {
            enter = 2;
            lv_label_set_text(hint_label, ""
                                          "\xE8\xAF\xB7" /*请*/ ""
                                          "\xE7\xBB\x99" /*给*/ ""
                                          "\xE6\x8C\x87" /*指*/ ""
                                          "\xE7\xBA\xB9" /*纹*/ ""
                                          "\xE8\xAE\xBE" /*设*/ ""
                                          "\xE7\xBD\xAE" /*置*/ ""
                                          "\xE4\xB8\x80" /*一*/ ""
                                          "\xE4\xB8\xAA" /*个*/ "ID"
                                          "\xE7\x84\xB6" /*然*/ ""
                                          "\xE5\x90\x8E" /*后*/ ""
                                          "\xE6\x8C\x89" /*按*/ ""
                                          "\xE4\xB8\x8B" /*下*/ ""
                                          "\xE5\x9B\x9E" /*回*/ ""
                                          "\xE8\xBD\xA6" /*车*/ "");
        }
        else if (ID == 14)
        {
            if (enter == 1)
            {
                const char *text_data = lv_textarea_get_text(pwd_text_area);
                num1 = atoi(text_data);
                if (num1 > 300)
                    lv_label_set_text(hint_label, "ID>300,"
                                                  "\xE6\x8C\x87" /*指*/ ""
                                                  "\xE7\xBA\xB9" /*纹*/ ""
                                                  "\xE5\x88\xA0" /*删*/ ""
                                                  "\xE9\x99\xA4" /*除*/ ""
                                                  "\xE5\xA4\xB1" /*失*/ ""
                                                  "\xE8\xB4\xA5" /*败*/ "");
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
                                                  "\xE6\x8C\x87" /*指*/ ""
                                                  "\xE7\xBA\xB9" /*纹*/ ""
                                                  "\xE6\xB7\xBB" /*添*/ ""
                                                  "\xE5\x8A\xA0" /*加*/ ""
                                                  "\xE5\xA4\xB1" /*失*/ ""
                                                  "\xE8\xB4\xA5" /*败*/ "");
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
        rt_kprintf("任务6\"tile1\"创建失败");
        rt_thread_mdelay(100);
    }

    lv_obj_set_size(tile1, LV_HOR_RES, 400);

    lv_obj_t *btn1 = lv_btn_create(tile1, NULL);
    if(btn1==NULL){
        rt_kprintf("任务6\"btn1\"创建失败");
        rt_thread_mdelay(100);
    }
    lv_obj_set_event_cb(btn1, event_task_esc);
    lv_obj_align(btn1, tile1, LV_ALIGN_IN_TOP_LEFT, 5, 5);
    lv_obj_t *label = lv_label_create(btn1, NULL);
    if(label==NULL){
        rt_kprintf("任务6\"label\"创建失败");
        rt_thread_mdelay(100);
    }
    lv_label_set_text(label, "Esc");
// 密码输入界面的容器
    lv_obj_t *pwd_main_cont = lv_obj_create(tile1, NULL);
    if(pwd_main_cont==NULL){
        rt_kprintf("任务6\"pwd_main_cont\"创建失败");
        rt_thread_mdelay(100);
    }
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
    lv_obj_set_size(pwd_main_cont, 400, 400);                                  // 设置大小800x480
    lv_obj_align(pwd_main_cont, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);              // 位置

    // 创建一个样式
    static lv_style_t btnmatrix_style;
    // 清除样式中的所有属性和所有分配的内存。
    lv_style_reset(&btnmatrix_style);
    // 初始化样式
    lv_style_init(&btnmatrix_style);
    lv_style_set_radius(&btnmatrix_style, LV_STATE_DEFAULT, 20);                // 设置样式的圆角弧度
    lv_style_set_border_width(&btnmatrix_style, LV_STATE_DEFAULT, 0);          // 设置边框宽度
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
    if(btnm1==NULL){
        rt_kprintf("任务6\"btnm1\"创建失败");
        rt_thread_mdelay(100);
    }
    // lv_btnmatrix_set_recolor(btnm1, true);//文本重新着色
    lv_btnmatrix_set_map(btnm1, keyboard_map);

    lv_obj_add_style(btnm1, LV_BTNMATRIX_PART_BG, &btnmatrix_style);   // 给对象添加样式
    lv_obj_add_style(btnm1, LV_BTNMATRIX_PART_BTN, &btnmatrix_style1); // 给对象添加样式
    lv_obj_set_size(btnm1, 400, 300);                                  // 设置大小800x480
    lv_obj_align(btnm1, NULL, LV_ALIGN_IN_TOP_MID, 0, 100);            // 位置
    lv_obj_set_event_cb(btnm1, btn_event_callback);                    // 设置矩阵按钮回掉函数

    pwd_text_area = lv_textarea_create(pwd_main_cont, NULL); // 密码输入文本框
    if(pwd_text_area==NULL){
        rt_kprintf("任务6\"pwd_text_area\"创建失败");
        rt_thread_mdelay(100);
    }
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
    if(label==NULL){
        rt_kprintf("任务6\"label\"创建失败");
        rt_thread_mdelay(100);
    }
    lv_obj_add_style(label, LV_LABEL_PART_MAIN, &style); // 给对象添加字体
    lv_obj_add_style(label, NULL, &TXT_style);           // 给对象添加样式
    lv_label_set_text(label, "ID:");
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 10); // 对象位置

    lv_obj_align(pwd_text_area, label, LV_ALIGN_OUT_RIGHT_TOP, 0, -10); // 对象位置
    lv_textarea_set_text(pwd_text_area, "");                            // 清空内容

    hint_label = lv_label_create(pwd_main_cont, NULL);                                                   // 创建标签
    if(hint_label==NULL){
        rt_kprintf("任务6\"hint_label\"创建失败");
        rt_thread_mdelay(100);
    }
    lv_obj_set_style_local_text_color(hint_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK); // 文本设置成黑色
    lv_obj_add_style(hint_label, LV_LABEL_PART_MAIN, &style);                                            // 设置字体
    lv_label_set_text(hint_label, "");
    lv_obj_align(hint_label, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 70);

    Taskshutdown = 6;

    rt_mb_send_wait(task1_thread_mailbox, RW6_thread1,3000);

}


// 录指纹
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
                    ensure = PS_GenChar(CharBuffer1); // 生成特征
                    if (ensure == 0x00)
                    {

                        lv_label_set_text(hint_label, ""
                                                      "\xE6\x8C\x87" /*指*/ ""
                                                      "\xE7\xBA\xB9" /*纹*/ ""
                                                      "\xE6\xAD\xA3" /*正*/ ""
                                                      "\xE5\xB8\xB8" /*常*/ "");
                        i = 0;
                        processnum = 1; // 跳到第二步
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
                                              "\xE8\xAF\xB7" /*请*/ ""
                                              "\xE6\x8C\x89" /*按*/ ""
                                              "\xE5\x86\x8D" /*再*/ ""
                                              "\xE6\x8C\x89" /*按*/ ""
                                              "\xE4\xB8\x80" /*一*/ ""
                                              "\xE6\xAC\xA1" /*次*/ ""
                                              "\xE6\x8C\x87" /*指*/ ""
                                              "\xE7\xBA\xB9" /*纹*/ "");
                ensure = PS_GetImage();
                if (ensure == 0x00)
                {

                    ensure = PS_GenChar(CharBuffer2); // 生成特征

                    if (ensure == 0x00)
                    {
                        lv_label_set_text(hint_label, ""
                                                      "\xE6\x8C\x87" /*指*/ ""
                                                      "\xE7\xBA\xB9" /*纹*/ ""
                                                      "\xE6\xAD\xA3" /*正*/ ""
                                                      "\xE5\xB8\xB8" /*常*/ "");
                        i = 0;
                        processnum = 2; // 跳到第三步
                    }
                    else
                        lv_label_set_text(hint_label, EnsureMessage_UTF8(ensure));
                }
                else
                    lv_label_set_text(hint_label, EnsureMessage_UTF8(ensure));
                break;

            case 2:
                lv_label_set_text(hint_label, ""
                                              "\xE5\xAF\xB9" /*对*/ ""
                                              "\xE6\xAF\x94" /*比*/ ""
                                              "\xE4\xB8\xA4" /*两*/ ""
                                              "\xE6\xAC\xA1" /*次*/ ""
                                              "\xE6\x8C\x87" /*指*/ ""
                                              "\xE7\xBA\xB9" /*纹*/ "");
                ensure = PS_Match();
                if (ensure == 0x00)
                {
                    lv_label_set_text(hint_label, ""
                                                  "\xE5\xAF\xB9" /*对*/ ""
                                                  "\xE6\xAF\x94" /*比*/ ""
                                                  "\xE6\x88\x90" /*成*/ ""
                                                  "\xE5\x8A\x9F" /*功*/ ","
                                                  "\xE4\xB8\xA4" /*两*/ ""
                                                  "\xE6\xAC\xA1" /*次*/ ""
                                                  "\xE6\x8C\x87" /*指*/ ""
                                                  "\xE7\xBA\xB9" /*纹*/ ""
                                                  "\xE4\xB8\x80" /*一*/ ""
                                                  "\xE6\xA0\xB7" /*样*/ "");

                    processnum = 3; // 跳到第四步
                }
                else
                {
                    lv_label_set_text(hint_label, ""
                                                  "\xE5\xAF\xB9" /*对*/ ""
                                                  "\xE6\xAF\x94" /*比*/ ""
                                                  "\xE5\xA4\xB1" /*失*/ ""
                                                  "\xE8\xB4\xA5" /*败*/ ""
                                                  "\xEF\xBC\x8C" /*，*/ ""
                                                  "\xE8\xAF\xB7" /*请*/ ""
                                                  "\xE9\x87\x8D" /*重*/ ""
                                                  "\xE6\x96\xB0" /*新*/ ""
                                                  "\xE5\xBD\x95" /*录*/ ""
                                                  "\xE5\x85\xA5" /*入*/ ""
                                                  "\xE6\x8C\x87" /*指*/ ""
                                                  "\xE7\xBA\xB9" /*纹*/ "");

                    lv_label_set_text(hint_label, EnsureMessage_UTF8(ensure));
                    i = 0;
                    processnum = 0; // 跳回第一步
                }
                rt_thread_mdelay(1200);
                break;

            case 3:
                lv_label_set_text(hint_label, ""
                                              "\xE7\x94\x9F" /*生*/ ""
                                              "\xE6\x88\x90" /*成*/ ""
                                              "\xE6\x8C\x87" /*指*/ ""
                                              "\xE7\xBA\xB9" /*纹*/ ""
                                              "\xE6\xA8\xA1" /*模*/ ""
                                              "\xE6\x9D\xBF" /*板*/ "");
                ensure = PS_RegModel();
                if (ensure == 0x00)
                {

                    lv_label_set_text(hint_label, ""
                                                  "\xE7\x94\x9F" /*生*/ ""
                                                  "\xE6\x88\x90" /*成*/ ""
                                                  "\xE6\x8C\x87" /*指*/ ""
                                                  "\xE7\xBA\xB9" /*纹*/ ""
                                                  "\xE6\xA8\xA1" /*模*/ ""
                                                  "\xE6\x9D\xBF" /*板*/ ""
                                                  "\xE6\x88\x90" /*成*/ ""
                                                  "\xE5\x8A\x9F" /*功*/ "");
                    processnum = 4; // 跳到第五步
                }
                else
                {
                    processnum = 0;
                    lv_label_set_text(hint_label, EnsureMessage_UTF8(ensure));
                }
                rt_thread_mdelay(1200);
                break;

            case 4:

                ensure = PS_StoreChar(CharBuffer2, num1); // 储存模板
                if (ensure == 0x00)
                {
                    lv_label_set_text(hint_label, ""
                                                  "\xE5\xBD\x95" /*录*/ ""
                                                  "\xE5\x85\xA5" /*入*/ ""
                                                  "\xE6\x8C\x87" /*指*/ ""
                                                  "\xE7\xBA\xB9" /*纹*/ ""
                                                  "\xE6\x88\x90" /*成*/ ""
                                                  "\xE5\x8A\x9F" /*功*/ "");

                    PS_ValidTempleteNum(&ValidN); // 读库指纹个数

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
            if (i == 5) // 超过5次没有按手指则退出
            {

                break;
            }
        }


}
