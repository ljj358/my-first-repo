/*
 * UI_StepperMotor.c
 *
 * created: 2024/5/21
 *  author:
 */

#include "UI_StepperMotor.h"
static rt_thread_t StepperMotor_thread = NULL;

static void event_StepperMotor_esc(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_SHORT_CLICKED)
    { /* 对象被点击 */

        lv_obj_set_hidden(main_create, false); // 显示列表
        lv_obj_t *parent_obj = lv_obj_get_parent(obj);
        lv_obj_del_async(parent_obj);
    }
    if (StepperMotor_thread != NULL)
    {
        if (RT_EOK == rt_thread_delete(StepperMotor_thread))
        {
            rt_kprintf("任务\"SM\"删除成功\r\n");
            StepperMotor_thread = NULL;
        }
        else
            rt_kprintf("任务\"SM\"删除失败\r\n");
    }
}
char StepperMotor_open = 0;
static void StepperMotor_thread_event(void *arg)
{

    for (;;)
    {
        switch (StepperMotor_open)
        {
        case 1:
            step_28byj48_angles(45, 0);
            StepperMotor_open = 0;
            break;
        case 2:
            step_28byj48_angles(90, 0);
            StepperMotor_open = 0;
            break;
        case 3:
            step_28byj48_angles(45, 1);
            StepperMotor_open = 0;
            break;
        case 4:
            step_28byj48_angles(90, 1);
            StepperMotor_open = 0;
            break;
        }
        rt_thread_delay(1000);
    }
}
static void event_handler(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        const char *btn_text = lv_list_get_btn_text(obj);
        if (strcmp(btn_text, "+45") == 0)
        StepperMotor_open = 1;
        else if (strcmp(btn_text, "+90") == 0)
        StepperMotor_open = 2;
        else if (strcmp(btn_text, "-45") == 0)
        StepperMotor_open = 3;
        else if (strcmp(btn_text, "-90") == 0)
        StepperMotor_open = 4;
    }
}
void UI_StepperMotor_event_cb(void)
{
    lv_obj_t *tile1 = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(tile1, LV_HOR_RES, LV_VER_RES);

    lv_obj_t *label;
    lv_obj_t *btn1 = lv_btn_create(tile1, NULL);
    lv_obj_set_event_cb(btn1, event_StepperMotor_esc);
    lv_obj_align(btn1, tile1, LV_ALIGN_IN_TOP_LEFT, 5, 5);
    label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, "Esc");

    // 创建第一个按钮
    lv_obj_t *btn_add45 = lv_btn_create(tile1, NULL);
    lv_obj_set_event_cb(btn_add45, event_handler);           // 绑定事件处理函数
    lv_obj_align(btn_add45, NULL, LV_ALIGN_CENTER, 0, -100); // 设置按钮位置

    label = lv_label_create(btn_add45, NULL); // 创建并附加标签到按钮
    lv_label_set_text(label, "+45");          // 设置标签文本

    // 创建第二个按钮
    lv_obj_t *btn_add90 = lv_btn_create(tile1, NULL);
    lv_obj_set_event_cb(btn_add90, event_handler);          // 绑定事件处理函数
    lv_obj_align(btn_add90, NULL, LV_ALIGN_CENTER, 0, -50); // 设置按钮位置

    label = lv_label_create(btn_add90, NULL); // 创建并附加标签到按钮
    lv_label_set_text(label, "+90");          // 设置标签文本

    // 创建第三个按钮
    lv_obj_t *btn_45 = lv_btn_create(tile1, NULL);
    lv_obj_set_event_cb(btn_45, event_handler);        // 绑定事件处理函数
    lv_obj_align(btn_45, NULL, LV_ALIGN_CENTER, 0, 0); // 设置按钮位置

    label = lv_label_create(btn_45, NULL); // 创建并附加标签到按钮
    lv_label_set_text(label, "-45");       // 设置标签文本

    // 创建第四个按钮
    lv_obj_t *btn_90 = lv_btn_create(tile1, NULL);
    lv_obj_set_event_cb(btn_90, event_handler);         // 绑定事件处理函数
    lv_obj_align(btn_90, NULL, LV_ALIGN_CENTER, 0, 50); // 设置按钮位置

    label = lv_label_create(btn_90, NULL); // 创建并附加标签到按钮
    lv_label_set_text(label, "-90");       // 设置标签文本

    if (StepperMotor_thread == NULL)
    { // 防止重复创建线程
        // 创建线程
        StepperMotor_thread = rt_thread_create("SM",
                                               StepperMotor_thread_event,
                                               NULL,     // arg
                                               1024 * 1, // statck size
                                               10,       // priority
                                               1);       // slice ticks

        if (StepperMotor_thread == NULL)
        {
            rt_kprintf("任务\"SM\"创建失败\r\n");
            return;
        }
        else
        {
            rt_kprintf("任务\SM\"创建成功\r\n");
            rt_thread_startup(StepperMotor_thread);
        }
    }
}
