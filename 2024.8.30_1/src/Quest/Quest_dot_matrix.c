/*
 * Quest_dot_matrix.c
 *
 * created: 2024/1/4
 *  author: 
 */
#include "lvgl_ui.h"
#include "lvgl-7.0.1/lvgl.h"
#include "matrix.h"
#include "ShowData.h"
static rt_thread_t dot_matrix_thread = NULL;
char direction = 1;
static void event_dot_matrix_esc(lv_obj_t *obj, lv_event_t event)
{
        if (event == LV_EVENT_SHORT_CLICKED)
        { /* 对象被点击 */

                lv_obj_set_hidden(main_create, false); // 显示列表
                lv_obj_t *parent_obj = lv_obj_get_parent(obj);
                lv_obj_del_async(parent_obj);
                LED_ALL_OFF();
                if(RT_EOK ==  rt_thread_delete(dot_matrix_thread)){
                    rt_kprintf("dot_matrix_thread删除成功\r\n");
                }
                else rt_kprintf("dot_matrix_thread删除失败\r\n");
                dot_matrix_thread = NULL;
        }
}
static void event_left_to_right(lv_obj_t *obj, lv_event_t event)
{
        if (event == LV_EVENT_SHORT_CLICKED)
        { /* 对象被点击 */

               direction = 1;

        }
}

static void event_top_to_bottom(lv_obj_t *obj, lv_event_t event)
{
        if (event == LV_EVENT_SHORT_CLICKED)
        { /* 对象被点击 */

               direction = 2;
        }
}
static rt_tick_t tim_static = 0;
static void event_figure(lv_obj_t *obj, lv_event_t event)
{
        if (event == LV_EVENT_SHORT_CLICKED)
        { /* 对象被点击 */

               direction = 3;
               tim_static = rt_tick_get()/1000;
        }
}

static void dot_matrix_thread1(void *arg)
{
   rt_tick_t tim = 0;
  

    for (;;)
    {
        switch (direction){
            case 1: Dis_1_Roll(Roll_buff,32*2);break;
            case 2: Dis_Roll(dz_date,32*2);break;
            case 3: tim = ((int)(rt_tick_get()/1000)- tim_static)%100;
                    Dis_Number (&dz_number[tim/10][0],&dz_number[tim%10][0]);
                    break;
        }
        rt_thread_delay(1);
    }
}
void Quest_dot_matrix(void){
        // 创建线程

    dot_matrix_thread = rt_thread_create("dot_matrix",
                                     dot_matrix_thread1,
                                     NULL,     // arg
                                     1024 * 4, // statck size
                                     15,        // priority
                                     1);      // slice ticks

    if (dot_matrix_thread == NULL)
    {
        rt_kprintf("任务“dot_matrix_thread”创建失败\r\n");
        return ;
    }
    else
    {
        rt_kprintf("任务“dot_matrix_thread”创建成功\r\n");
        rt_thread_startup(dot_matrix_thread);
    }
    
        lv_obj_t *tile1 = lv_obj_create(lv_scr_act(), NULL);
        lv_obj_set_size(tile1, LV_HOR_RES, LV_VER_RES);

        lv_obj_t *label;
        lv_obj_t *btn1 = lv_btn_create(tile1, NULL);
        lv_obj_set_event_cb(btn1, event_dot_matrix_esc);
        lv_obj_align(btn1, tile1, LV_ALIGN_IN_TOP_LEFT, 5, 5);
        label = lv_label_create(btn1, NULL);
        lv_label_set_text(label, "Esc");
        
         lv_obj_t *labe2;
        lv_obj_t *btn2 = lv_btn_create(tile1, NULL);
        lv_obj_set_event_cb(btn2, event_left_to_right);
        lv_obj_set_size(btn2, 200, 100);
        lv_obj_align(btn2, tile1, LV_ALIGN_CENTER, 0, -150);
        labe2 = lv_label_create(btn2, NULL);
        lv_label_set_text(labe2, "left -> right");
        
         lv_obj_t *labe3;
        lv_obj_t *btn3 = lv_btn_create(tile1, btn2);
        lv_obj_set_event_cb(btn3, event_top_to_bottom);
        lv_obj_align(btn3, tile1, LV_ALIGN_CENTER, 0, 0);
        labe3 = lv_label_create(btn3, NULL);
        lv_label_set_text(labe3, "top -> bottom");
        
        lv_obj_t *labe4;
        lv_obj_t *btn4 = lv_btn_create(tile1, btn2);
        lv_obj_set_event_cb(btn4, event_figure);
        lv_obj_align(btn4, tile1, LV_ALIGN_CENTER, 0,150);
        labe4 = lv_label_create(btn4, NULL);
        lv_label_set_text(labe4, "figure");
        
}
