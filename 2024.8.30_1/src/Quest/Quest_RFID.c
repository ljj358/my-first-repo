/*
 * Quest_RFID.c
 *
 * created: 2024/1/6
 *  author: 
 */
#include "lvgl_ui.h"
#include "lvgl-7.0.1/lvgl.h"
#include "rc522.h"
#include "rtthread.h"
#include "main.h"
#include "Quest_RFID.h"

static lv_obj_t * ta1, * ta2,*roller1,*roller2, * ta3,*kb;
int char_to_hex(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else {
        // 处理非法字符
        return -1;
    }
}
void string_to_hex_array( char *input, unsigned char *output) {
    char len = strlen(input);
    char i ;
    for ( i = 0; i < len; i++) {
        output[i] = char_to_hex(input[i]);
    }

}
static void event_RFID_esc(lv_obj_t *obj, lv_event_t event)
{
        if (event == LV_EVENT_SHORT_CLICKED)
        { /* 对象被点击 */

                lv_obj_set_hidden(main_create, false); // 显示列表
                if(kb != NULL) 
                {
                    lv_keyboard_set_textarea(kb, NULL);
                    lv_obj_del(kb);
                    kb = NULL;
                }
                lv_obj_t *parent_obj = lv_obj_get_parent(obj);
                lv_obj_del_async(parent_obj);
        }
}
 lv_obj_t * tile1_RFID = NULL;
 static void kb_event_cb(lv_obj_t * keyboard, lv_event_t e)
     {
            lv_keyboard_def_event_cb(kb, e);
            if(e == LV_EVENT_CANCEL) {
                    lv_keyboard_set_textarea(kb, NULL);
                    lv_obj_del(kb);
                    kb = NULL;
            }
    }
static void kb_create(lv_obj_t *obj)
    {
        kb = lv_keyboard_create(tile1_RFID, NULL);
        lv_obj_set_style_local_text_font(kb, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, &hz_simsun_3500_24);
        lv_keyboard_set_cursor_manage(kb, true);
        lv_obj_set_event_cb(kb, kb_event_cb);
        lv_keyboard_set_textarea(kb, obj);
        lv_obj_align(kb, NULL, LV_ALIGN_IN_LEFT_MID, 0,100);
        lv_obj_set_size(kb, 800, 240);

    }
    static void event_DATA(lv_obj_t *obj, lv_event_t event){
        if(event == LV_EVENT_CLICKED ) {
                if(kb == NULL) kb_create(obj);
                else
                {
                    lv_keyboard_set_textarea(kb, NULL);
                    lv_obj_del(kb);
                    kb = NULL;
                    kb_create(obj);
                }
                    
            }

    }
    static void event_key(lv_obj_t *obj, lv_event_t event){
         if(event == LV_EVENT_CLICKED ) {
                if(kb == NULL) kb_create(obj);
                else
                {
                    lv_keyboard_set_textarea(kb, NULL);
                    lv_obj_del(kb);
                    kb = NULL;
                    kb_create(obj);
                }

            }
    }
    
     void eroor_msdbox(char * data ){
        static const char * btns[] ={"""\xE5\x8F\x96"/*取*/"""\xE6\xB6\x88"/*消*/, ""};
            lv_obj_t * mbox1 = lv_msgbox_create(tile1_RFID, NULL);
            lv_obj_set_style_local_text_font(mbox1, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, &hz_simsun_3500_24);
            lv_msgbox_set_text(mbox1,data);
            lv_msgbox_add_btns(mbox1, btns);
            lv_obj_set_width(mbox1, 200);
            lv_obj_align(mbox1, NULL, LV_ALIGN_CENTER, 0, 0); /*Align to the corner*/
    }
    
    static void event_bint_Read(lv_obj_t *obj, lv_event_t event){
         if (event == LV_EVENT_SHORT_CLICKED) /* 对象被点击 */
        { 
            char sector = lv_roller_get_selected(roller1);
            char block = lv_roller_get_selected(roller2);
            char *text_key = lv_textarea_get_text(ta3);
            char *text_Write = lv_textarea_get_text(ta2);
            if(sector<0||sector>15)
            {
                eroor_msdbox("""\xE6\x89\x87"/*扇*/"""\xE5\x8C\xBA"/*区*/"""\xE5\x9C\xB0"/*地*/"""\xE5\x9D\x80"/*址*/"""\xE9\x94\x99"/*错*/"""\xE8\xAF\xAF"/*误*/"");
                return ;
            }
            if(block<0||block>2)
            {
                eroor_msdbox("""\xE5\x9D\x97"/*块*/"""\xE5\x9C\xB0"/*地*/"""\xE5\x9D\x80"/*址*/"""\xE9\x94\x99"/*错*/"""\xE8\xAF\xAF"/*误*/"");
                return ;
            }
            if(strlen(text_key)!=12)
            {
                eroor_msdbox("""\xE5\xAF\x86"/*密*/"""\xE9\x92\xA5"/*钥*/"""\xE9\x95\xBF"/*长*/"""\xE5\xBA\xA6"/*度*/"""\xE9\x94\x99"/*错*/"""\xE8\xAF\xAF"/*误*/"");
                return ;
            }
            unsigned char key_data[12];
            string_to_hex_array(text_key,key_data);
            char i = 0;
            for(i = 0;i<6;i++){
                key_data[i] = key_data[i*2]<<4| key_data[i*2+1];
            }
            if(Read_RFID(RFID,sector,block,key_data,1)==MI_OK)
            {
                lv_textarea_set_text(ta1, RFID);  // 设置文本框的初始文本
                rt_kprintf("RFID_DATA =%s\r\n",RFID);
            }

            rt_kprintf("密钥:%s\n扇区:%d\n块:%d\n写数据:%s\r\n",text_key,sector,block,text_Write);
        }
    }
   
     static void event_bint_Write(lv_obj_t *obj, lv_event_t event){
         if (event == LV_EVENT_SHORT_CLICKED) /* 对象被点击 */
        {
            if (event == LV_EVENT_SHORT_CLICKED) /* 对象被点击 */
        {
            char sector = lv_roller_get_selected(roller1);
            char block = lv_roller_get_selected(roller2);
            char *text_key = lv_textarea_get_text(ta3);
            char *text_Write = lv_textarea_get_text(ta2);
            if(sector<0||sector>15)
            {
                eroor_msdbox("""\xE6\x89\x87"/*扇*/"""\xE5\x8C\xBA"/*区*/"""\xE5\x9C\xB0"/*地*/"""\xE5\x9D\x80"/*址*/"""\xE9\x94\x99"/*错*/"""\xE8\xAF\xAF"/*误*/"");
                return ;
            }
            if(block<0||block>2)
            {
                eroor_msdbox("""\xE5\x9D\x97"/*块*/"""\xE5\x9C\xB0"/*地*/"""\xE5\x9D\x80"/*址*/"""\xE9\x94\x99"/*错*/"""\xE8\xAF\xAF"/*误*/"");
                return ;
            }
            if(strlen(text_key)!=12)
            {
                eroor_msdbox("""\xE5\xAF\x86"/*密*/"""\xE9\x92\xA5"/*钥*/"""\xE9\x95\xBF"/*长*/"""\xE5\xBA\xA6"/*度*/"""\xE9\x94\x99"/*错*/"""\xE8\xAF\xAF"/*误*/"");
                return ;
            }
            if(strlen(text_Write)!=16)
            {
                eroor_msdbox("""\xE6\x95\xB0"/*数*/"""\xE6\x8D\xAE"/*据*/"""\xE9\x95\xBF"/*长*/"""\xE5\xBA\xA6"/*度*/"""\xE9\x94\x99"/*错*/"""\xE8\xAF\xAF"/*误*/"");
                return ;
            }
            unsigned char key_data[12];
            string_to_hex_array(text_key,key_data);
            char i = 0;
            for(i = 0;i<6;i++){
                key_data[i] = key_data[i*2]<<4| key_data[i*2+1];
            }
            if(Write_RFID(text_Write,sector,block,key_data,1)==MI_OK)
                 rt_kprintf("RFID_DATA =%s\r\n",RFID);
            rt_kprintf("密钥:%s\n扇区:%d\n块:%d\n写数据:%s\r\n",text_key,sector,block,text_Write);
        }

        }
    }
void Quest_RFID(void){
        
        tile1_RFID = lv_obj_create(lv_scr_act(), NULL);
        lv_obj_set_size(tile1_RFID, LV_HOR_RES, LV_VER_RES);

        lv_obj_t *label;
        lv_obj_t *btn1 = lv_btn_create(tile1_RFID, NULL);
        lv_obj_set_event_cb(btn1, event_RFID_esc);
        lv_obj_align(btn1, tile1_RFID, LV_ALIGN_IN_TOP_LEFT, 5, 5);
        label = lv_label_create(btn1, NULL);
        lv_label_set_text(label, "Esc");
        
        label = lv_label_create(tile1_RFID, NULL);
        lv_obj_add_style(label, LV_LABEL_PART_MAIN, &style);
        lv_label_set_text(label, """\xE8\xAF\xBB"/*读*/"""\xE6\x95\xB0"/*数*/"""\xE6\x8D\xAE"/*据*/"");
        lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_LEFT, 350, 5);
        
        ta1 = lv_textarea_create(tile1_RFID, NULL);
        lv_textarea_set_max_length(ta1, 16);
        lv_obj_set_style_local_bg_color(ta1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_obj_set_style_local_text_color(ta1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
        lv_obj_add_style(ta1, LV_LABEL_PART_MAIN, &style);
        lv_textarea_set_cursor_hidden(ta1, true);
        lv_obj_set_size(ta1, 230, 50);
        lv_textarea_set_one_line(ta1, true);
        lv_obj_align(ta1, label, LV_ALIGN_OUT_BOTTOM_MID, -7, 5);
        lv_textarea_set_text(ta1, "null");    /*Set an initial text*/
        
        
        label = lv_label_create(tile1_RFID, NULL);
        lv_obj_add_style(label, LV_LABEL_PART_MAIN, &style);
        lv_label_set_text(label, """\xE5\x86\x99"/*写*/"""\xE6\x95\xB0"/*数*/"""\xE6\x8D\xAE"/*据*/"");
        lv_obj_align(label, ta1, LV_ALIGN_OUT_BOTTOM_MID, 7, 18);
        
        ta2 = lv_textarea_create(tile1_RFID, ta1);
        lv_textarea_set_max_length(ta2, 16);
        lv_obj_add_style(ta2, LV_LABEL_PART_MAIN, &style);
        lv_textarea_set_cursor_hidden(ta2, false);
        lv_obj_align(ta2, label, LV_ALIGN_OUT_BOTTOM_MID, -7, 5);
        lv_textarea_set_text(ta2, "0123456789123456");    /*Set an initial text*/
        lv_obj_set_event_cb(ta2, event_DATA);
        

        ta3 = lv_textarea_create(tile1_RFID, ta1);
        lv_textarea_set_max_length(ta3, 12);
        lv_obj_add_style(ta3, LV_LABEL_PART_MAIN, &style);
        lv_textarea_set_cursor_hidden(ta3, false);
        lv_obj_align(ta3, ta2, LV_ALIGN_OUT_LEFT_MID, 0, 0);
        lv_textarea_set_text(ta3, "FFFFFFFFFFFF");    /*Set an initial text*/
        lv_obj_set_event_cb(ta3, event_key);
        lv_textarea_set_accepted_chars(ta3, "0123456789ABCDEFabcdef");
        
        
        label = lv_label_create(tile1_RFID, NULL);
        lv_obj_add_style(label, LV_LABEL_PART_MAIN, &style);
        lv_label_set_text(label, """\xE5\xAF\x86"/*密*/"""\xE9\x92\xA5"/*钥*/"(HEX)");
        lv_obj_align(label, ta3, LV_ALIGN_OUT_TOP_LEFT, 0, 0);
        
        roller1 = lv_roller_create(tile1_RFID, NULL);
        lv_obj_add_style(roller1, LV_LABEL_PART_MAIN, &style);
        lv_obj_set_style_local_bg_color(roller1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_obj_set_style_local_text_color(roller1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
        lv_roller_set_options(roller1,
                                                    "0\n"
                                                    "1\n"
                                                    "2\n"
                                                    "3\n"
                                                    "4\n"
                                                    "5\n"
                                                    "6\n"
                                                    "7\n"
                                                    "8\n"
                                                    "9\n"
                                                    "10\n"
                                                    "11\n"
                                                    "12\n"
                                                    "13\n"
                                                    "14\n"
                                                    "15\n",
                                                   LV_ROLLER_MODE_INIFINITE);

            lv_roller_set_visible_row_count(roller1, 3);
            lv_obj_align(roller1, ta1, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
            lv_roller_set_selected(roller1, 1, LV_ANIM_ON);
        
        roller2 = lv_roller_create(tile1_RFID, NULL);
        lv_obj_add_style(roller2, LV_LABEL_PART_MAIN, &style);
        lv_obj_set_style_local_bg_color(roller2, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_obj_set_style_local_text_color(roller2, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);

        lv_roller_set_options(roller2,
                                                    "0\n"
                                                    "1\n"
                                                    "2\n",
                                                   LV_ROLLER_MODE_INIFINITE);

        lv_roller_set_visible_row_count(roller2, 3);
        lv_obj_align(roller2, roller1, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
        lv_roller_set_anim_time(roller2, 1000);
        lv_roller_set_selected(roller2, 1, LV_ANIM_ON);
        
        label = lv_label_create(tile1_RFID, NULL);
        lv_obj_add_style(label, LV_LABEL_PART_MAIN, &style);
        lv_label_set_text(label, """\xE5\x9D\x97"/*块*/"""\xE5\x9C\xB0"/*地*/"""\xE5\x9D\x80"/*址*/"");
        lv_obj_align(label, roller2, LV_ALIGN_OUT_TOP_MID, 0, -5);
        
        label = lv_label_create(tile1_RFID, NULL);
        lv_obj_add_style(label, LV_LABEL_PART_MAIN, &style);
        lv_label_set_text(label, """\xE6\x89\x87"/*扇*/"""\xE5\x8C\xBA"/*区*/"""\xE5\x9C\xB0"/*地*/"""\xE5\x9D\x80"/*址*/"");
        lv_obj_align(label, roller1, LV_ALIGN_OUT_TOP_MID, 0, -5);
        
        btn1 = lv_btn_create(tile1_RFID, NULL);
        lv_obj_set_event_cb(btn1, event_bint_Read);
        lv_obj_align(btn1, roller2, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
        lv_obj_set_size(btn1, 100, 60);
        lv_obj_set_style_local_radius(btn1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);  // 设置按钮的圆角半径为0，使其呈方形形状
        label = lv_label_create(btn1, NULL);
        lv_obj_set_style_local_bg_color(btn1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_CYAN);
        lv_obj_set_style_local_text_color(label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
        lv_obj_add_style(label, LV_LABEL_PART_MAIN, &style);
        lv_label_set_text(label, """\xE8\xAF\xBB"/*读*/"");
        
        btn1 = lv_btn_create(tile1_RFID, NULL);
        lv_obj_set_event_cb(btn1, event_bint_Write);
        lv_obj_set_size(btn1, 100, 60);
        lv_obj_align(btn1, roller2, LV_ALIGN_OUT_RIGHT_MID, 0, 35);
        lv_obj_set_style_local_radius(btn1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);  // 设置按钮的圆角半径为0，使其呈方形形状
        label = lv_label_create(btn1, NULL);
        lv_obj_set_style_local_bg_color(btn1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_CYAN);
        lv_obj_set_style_local_text_color(label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
        lv_obj_add_style(label, LV_LABEL_PART_MAIN, &style);
        lv_label_set_text(label, """\xE5\x86\x99"/*写*/"");
        

        
        
}

