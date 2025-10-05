/*
 * matrix_keyboard.c
 *
 * created: 2024/1/5
 *  author: 
 */


#include "matrix_keyboard.h"
#include "lvgl_ui.h"
#include <stdlib.h>
#include <stdio.h>
#include "lvgl-7.0.1/lvgl.h"
#include "ch455g.h"
#include "main.h"
lv_obj_t *matrix_keyboard_label = NULL;
static rt_thread_t matrix_keyboard_thread = NULL;
static void event_matrix_keyboard_ESC(lv_obj_t *obj, lv_event_t event)
{
        if (event == LV_EVENT_SHORT_CLICKED)
        { /* 对象被点击 */

                lv_obj_set_hidden(main_create, false); // 显示列表
                lv_obj_t *parent_obj = lv_obj_get_parent(obj);
                lv_obj_del_async(parent_obj);
                if(RT_EOK ==  rt_thread_delete(matrix_keyboard_thread)){
                    rt_kprintf("matrix_keyboard_thread删除成功\r\n");
                }
                else rt_kprintf("matrix_keyboard_thread删除失败\r\n");
                matrix_keyboard_label = NULL;
        }
}


static void matrix_keyboard_thread1(void *arg)
{
  char key_data = 0;
  char data[10] ;
  CH455_Write( CH455_8SEG_ON );   // 矩阵键盘
    for (;;)
   {
        key_data = Keyboard_proc();
        if(key_data!=0) {
           // rt_kprintf("KEY = %d\r\n",key_data);
            sprintf(data,"%d ",key_data);
            rt_mutex_take(ui_mutex,RT_WAITING_FOREVER );
            lv_label_ins_text(matrix_keyboard_label,LV_LABEL_POS_LAST,data);
            rt_mutex_release(ui_mutex);
        }
        rt_thread_delay(100);
    }
}
void matrix_keyboard(void){

        lv_obj_t *tile1 = lv_obj_create(lv_scr_act(), NULL);
        lv_obj_set_size(tile1, LV_HOR_RES, LV_VER_RES);

        lv_obj_t *btn1 = lv_btn_create(tile1, NULL);
        lv_obj_set_event_cb(btn1, event_matrix_keyboard_ESC);
        lv_obj_align(btn1, tile1, LV_ALIGN_IN_TOP_LEFT, 5, 5);
        matrix_keyboard_label = lv_label_create(btn1, NULL);
        lv_label_set_text(matrix_keyboard_label, "Esc");
        
        matrix_keyboard_label = lv_label_create(tile1, NULL);
        lv_obj_align(matrix_keyboard_label, NULL, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(matrix_keyboard_label, "KEY = ");
        
        matrix_keyboard_thread = rt_thread_create("matrix_keyboard_thread",
                                     matrix_keyboard_thread1,
                                     NULL,     // arg
                                     1024 * 4, // statck size
                                     15,        // priority
                                     1);      // slice ticks

    if (matrix_keyboard_thread == NULL)
    {
        rt_kprintf("任务“matrix_keyboard_thread”创建失败\r\n");
        return -1;
    }
    else
    {
        rt_kprintf("任务“matrix_keyboard_thread”创建成功\r\n");
        rt_thread_startup(matrix_keyboard_thread);
    }

}
