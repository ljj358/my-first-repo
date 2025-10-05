/*
 * Quest_BH1750.c
 *
 * created: 2024/1/7
 *  author: 
 */
#include "lvgl_ui.h"
#include "lvgl-7.0.1/lvgl.h"
#include "Quest_BH1750.h"
#include "rtthread.h"
#include "main.h"
lv_obj_t *label_x1,*label_x2,*label_x3,*label_x4,*label_x5,*label_x6,*label_x7,*label_x8,*label_y,*chart;
lv_chart_series_t* ser1;
static rt_thread_t BH1750_thread = NULL;
static void event_BH1750_esc(lv_obj_t *obj, lv_event_t event)
{
        if (event == LV_EVENT_SHORT_CLICKED)
        { /* 对象被点击 */

                lv_obj_set_hidden(main_create, false); // 显示列表
                lv_obj_t *parent_obj = lv_obj_get_parent(obj);
                lv_obj_del_async(parent_obj);
                if(RT_EOK ==  rt_thread_delete(BH1750_thread)){
                    rt_kprintf("BH1750_thread删除成功\r\n");
                }
                else rt_kprintf("BH1750_thread删除失败\r\n");
                BH1750_thread = NULL;
        }
}

static void BH1750_thread1(void *arg)
{
    char i = 0,j = 0,buf[100]={0};
    unsigned int bh1750_data[40] = {0},max = 0;
    int tim1 = 0,tim2 = 0;
    for (;;)
    {
        uint16_t lx = BH1750_Test();
        
        if(i>=40)
        {
            for(j = 0;j<39;j++)
            {
                bh1750_data[j] = bh1750_data[j+1];
            }
            bh1750_data[39] = lx;
        }
        else bh1750_data[i++] = lx ;
        max = 0;
        for(j = 0;j<40;j++)
        {
            if(bh1750_data[j]>=max) max = bh1750_data[j];
        }
        max +=(max*0.05);     //不建议怎么写
        
        
        rt_mutex_take(ui_mutex,RT_WAITING_FOREVER );
        if(tim1++ == 5)
        {
            tim1 = 0;
            tim2++ ;
            lv_label_set_text_fmt(label_x8,"%ds",tim2);
            lv_label_set_text_fmt(label_x7,"%ds",tim2-1);
            lv_label_set_text_fmt(label_x6,"%ds",tim2-2>0?tim2-2:0);
            lv_label_set_text_fmt(label_x5,"%ds",tim2-3>0?tim2-3:0);
            lv_label_set_text_fmt(label_x4,"%ds",tim2-4>0?tim2-4:0);
            lv_label_set_text_fmt(label_x3,"%ds",tim2-5>0?tim2-5:0);
            lv_label_set_text_fmt(label_x2,"%ds",tim2-6>0?tim2-6:0);
            lv_label_set_text_fmt(label_x1,"%ds",tim2-7>0?tim2-7:0);
        }
        lv_label_set_text_fmt(label_y, "%dlx\n\n\n%dlx\n\n%dlx\n\n\n%dlx\n\n%dlx\n\n\n%dlx\n\n%dlx\n\n\n%dlx\n\n%dlx\n\n\n%dlx\n\n",max,(int)(max*0.9),(int)(max*0.8),(int)(max*0.7),(int)(max*0.6),(int)(max*0.5),(int)(max*0.4),(int)(max*0.3),(int)(max*0.2),(int)(max*0.1));
        lv_chart_set_range(chart,0,max);
        lv_chart_set_next(chart, ser1, lx);
        
        rt_mutex_release(ui_mutex);
    }
}
void Quest_BH1750(void){
         // 创建线程

    BH1750_thread = rt_thread_create("BH1750",
                                     BH1750_thread1,
                                     NULL,     // arg
                                     1024 * 4, // statck size
                                     15,        // priority
                                     1);      // slice ticks

    if (BH1750_thread == NULL)
    {
        rt_kprintf("任务“BH1750_thread”创建失败\r\n");
        return ;
    }
    else
    {
        rt_kprintf("任务“BH1750_thread”创建成功\r\n");
        rt_thread_startup(BH1750_thread);
    }

        lv_obj_t *tile1 = lv_obj_create(lv_scr_act(), NULL);
        lv_obj_set_size(tile1, LV_HOR_RES, LV_VER_RES);
        // 设置背景颜色为黑色
        lv_obj_set_style_local_bg_color(tile1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
        
        lv_obj_t *label;
        lv_obj_t *btn1 = lv_btn_create(tile1, NULL);
        lv_obj_set_event_cb(btn1, event_BH1750_esc);
        lv_obj_align(btn1, tile1, LV_ALIGN_IN_TOP_LEFT, 5, 5);
        label = lv_label_create(btn1, NULL);
        lv_label_set_text(label, "Esc");
        
        
        chart = lv_chart_create(tile1,NULL);
        lv_obj_set_size(chart, 580, 430);

        lv_obj_align(chart,NULL, LV_ALIGN_CENTER, 105, -20);
        
        ser1 = lv_chart_add_series(chart, LV_COLOR_CYAN);
        
        lv_chart_set_type(chart, LV_CHART_TYPE_LINE);   /*Show lines and points too*/
        lv_chart_set_range(chart,0,200);
        lv_chart_set_point_count(chart, 40);
        lv_chart_set_div_line_count(chart, 8, 0);

        //背景为白色
        lv_obj_set_style_local_bg_color(chart,LV_CHART_PART_BG,LV_STATE_DEFAULT,LV_COLOR_WHITE);
        
        // 设置标签刻度线的颜色为黄色
        lv_obj_set_style_local_line_color(chart,LV_CHART_PART_BG ,LV_STATE_DEFAULT,LV_COLOR_YELLOW);
        
        lv_obj_set_style_local_text_font(chart, LV_CHART_PART_BG, LV_STATE_DEFAULT, &hz_simsun_3500_24);
        lv_obj_set_style_local_text_color(chart,LV_CHART_PART_BG,LV_STATE_DEFAULT,LV_COLOR_YELLOW);
        
        // 设置背景刻度线的颜色为黑色
        lv_obj_set_style_local_line_color(chart,LV_CHART_PART_SERIES_BG ,LV_STATE_DEFAULT,LV_COLOR_BLACK);
        //去掉数据点
        lv_obj_set_style_local_size(chart,LV_CHART_PART_SERIES  ,LV_STATE_DEFAULT,0);


       
        label_x1 = lv_label_create(tile1, NULL);
        lv_label_set_text(label_x1, "0s");
        lv_obj_align(label_x1,chart, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
        
        label_x2 = lv_label_create(tile1, NULL);
        lv_label_set_text(label_x2, "0s");
        lv_obj_align(label_x2,chart, LV_ALIGN_OUT_BOTTOM_LEFT, 80*1, 0);
        
        label_x3 = lv_label_create(tile1, NULL);
        lv_label_set_text(label_x3, "0s");
        lv_obj_align(label_x3,chart, LV_ALIGN_OUT_BOTTOM_LEFT, 80*2, 0);
        
        label_x4 = lv_label_create(tile1, NULL);
        lv_label_set_text(label_x4, "0s");
        lv_obj_align(label_x4,chart, LV_ALIGN_OUT_BOTTOM_LEFT, 80*3, 0);
        
        label_x5 = lv_label_create(tile1, NULL);
        lv_label_set_text(label_x5, "0s");
        lv_obj_align(label_x5,chart, LV_ALIGN_OUT_BOTTOM_LEFT, 80*4, 0);
        
        label_x6 = lv_label_create(tile1, NULL);
        lv_label_set_text(label_x6, "0s");
        lv_obj_align(label_x6,chart, LV_ALIGN_OUT_BOTTOM_LEFT, 80*5, 0);
        
        label_x7 = lv_label_create(tile1, NULL);
        lv_label_set_text(label_x7, "0s");
        lv_obj_align(label_x7,chart, LV_ALIGN_OUT_BOTTOM_LEFT, 80*6, 0);
        
        label_x8 = lv_label_create(tile1, NULL);
        lv_label_set_text(label_x8, "0s");
        lv_obj_align(label_x8,chart, LV_ALIGN_OUT_BOTTOM_LEFT, 78*7, 0);
        
        label_y = lv_label_create(tile1, NULL);
        lv_label_set_text_fmt(label_y, "%dlx\n\n\n%dlx\n\n%dlx\n\n\n%dlx\n\n%dlx\n\n\n%dlx\n\n%dlx\n\n\n%dlx\n\n%dlx\n\n\n%dlx\n\n",1,2,3,4,5,6,7,8,9,10);
        lv_obj_align(label_y,chart, LV_ALIGN_OUT_LEFT_TOP, -30, 0);


        
        
}
