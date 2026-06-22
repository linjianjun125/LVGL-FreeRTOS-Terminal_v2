#include "novel_select_screen.h"

#if USE_UI_NOVEL_SELECT_SCREEN
#include <stdlib.h>
#include <stdio.h>


/* ----------------------------------------------------------------------------
 * 全局与静态对象
 * ---------------------------------------------------------------------------- */
lv_obj_t * novel_select_screen_obj = NULL;
static lv_obj_t * novel_header_obj = NULL;
static lv_obj_t * novel_list_area = NULL;

ui_screen_t novel_select_screen = {
    .init = novel_select_screen_init,
    .deinit = novel_select_screen_deinit,
    .obj = &novel_select_screen_obj,
};

static void book_click_event_cb(lv_event_t * e);
static void back_btn_event_cb(lv_event_t * e);

static const uint32_t shelf_colors[] = {0xACAFBA, 0x97A7B3, 0xB7B1A5, 0xA5B9B7, 0xD4C5BC, 0xA9B09C};

/* ----------------------------------------------------------------------------
 * 界面初始化
 * ---------------------------------------------------------------------------- */
void novel_select_screen_init(lv_obj_t * parent)
{
    uint16_t screen_w    = 480;
    uint16_t book_w      = 120;
    uint16_t cover_h     = 150; // 书封固定高度
    uint8_t  cols        = 3;
    uint16_t gap_x       = 30;
    uint16_t gap_y       = 25;  // 增加纵向间距，给换行留空间
    uint16_t start_y     = 20;

    uint16_t start_x = (screen_w - (cols * book_w + (cols - 1) * gap_x)) / 2;

    // 1. 主背景
    novel_select_screen_obj = lv_obj_create(parent);
    lv_obj_set_size(novel_select_screen_obj, screen_w, 480);
    lv_obj_set_style_bg_color(novel_select_screen_obj, lv_color_hex(0xF8F9FA), 0);
    lv_obj_set_style_border_width(novel_select_screen_obj, 0, 0);
    lv_obj_set_style_pad_all(novel_select_screen_obj, 0, 0);
    lv_obj_clear_flag(novel_select_screen_obj, LV_OBJ_FLAG_SCROLLABLE);

    // 2. 滚动区域
    novel_list_area = lv_obj_create(novel_select_screen_obj);
    lv_obj_set_size(novel_list_area, screen_w, 410);
    lv_obj_set_pos(novel_list_area, 0, 70);
    lv_obj_set_style_bg_opa(novel_list_area, 0, 0);
    lv_obj_set_style_border_width(novel_list_area, 0, 0);
    // 关键：由于内部 item 高度不一，设置顶部和底部内边距防止贴边
    lv_obj_set_style_pad_top(novel_list_area, 20, 0);
    lv_obj_set_style_pad_bottom(novel_list_area, 50, 0);
    lv_obj_set_scrollbar_mode(novel_list_area, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(novel_list_area, LV_OBJ_FLAG_SCROLLABLE);

    // 3. 悬浮标题栏
    novel_header_obj = lv_obj_create(novel_select_screen_obj);
    lv_obj_set_size(novel_header_obj, screen_w, 70);
    lv_obj_set_style_bg_color(novel_header_obj, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_radius(novel_header_obj, 0, 0);
    lv_obj_set_style_border_side(novel_header_obj, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_color(novel_header_obj, lv_color_hex(0xEEEEEE), 0);
    lv_obj_set_style_border_width(novel_header_obj, 1, 0);
    lv_obj_align(novel_header_obj, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_clear_flag(novel_header_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_move_foreground(novel_header_obj);

    // 标题文字
    lv_obj_t * title = lv_label_create(novel_header_obj);
    lv_label_set_text(title, "我的图书");// 我的图书
    lv_obj_set_style_text_color(title, lv_color_hex(0x2C3E50), 0);
    lv_obj_set_style_text_font(title, &_lvgl_font3, 0);
    lv_obj_center(title);

    // 返回按钮
    lv_obj_t * back_btn = lv_btn_create(novel_header_obj);
    lv_obj_set_size(back_btn, 40, 40);
    lv_obj_align(back_btn, LV_ALIGN_RIGHT_MID, -15, 0);
    lv_obj_set_style_bg_opa(back_btn, 0, 0);
    lv_obj_set_style_shadow_width(back_btn, 0, 0);
    lv_obj_t * back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, LV_SYMBOL_CLOSE);
    lv_obj_set_style_text_color(back_label, lv_color_hex(0x2C3E50), 0);
    lv_obj_center(back_label);
    lv_obj_add_event_cb(back_btn, back_btn_event_cb, LV_EVENT_CLICKED, NULL);

    // 4. 循环生成书籍
    LinkList_Item *ListItem;
    novel_info_t *info;
    uint16_t i = 0;
    List_HeadFor(&NovelControl.List, ListItem)
    {
        info = List_ListItemGetData(ListItem);
        
        int row = i / cols;
        int col = i % cols;
        int x_pos = start_x + (col * (book_w + gap_x));
        // 注意：如果书名换行非常严重，row * (固定高度) 仍可能重叠，建议 book_h 给足
        int y_pos = start_y + (row * (cover_h + 50 + gap_y)); 

        // 书籍容器：高度设为 CONTENT
        lv_obj_t * book_item = lv_obj_create(novel_list_area);
        lv_obj_set_size(book_item, book_w, LV_SIZE_CONTENT); 
        lv_obj_set_pos(book_item, x_pos, y_pos);
        lv_obj_set_style_bg_opa(book_item, 0, 0);
        lv_obj_set_style_border_width(book_item, 0, 0);
        lv_obj_set_style_pad_all(book_item, 0, 0); // 清除内边距防止对齐偏移
        lv_obj_clear_flag(book_item, LV_OBJ_FLAG_SCROLLABLE);

        // 书封
        lv_obj_t * cover = lv_obj_create(book_item);
        lv_obj_set_size(cover, book_w, cover_h);
        lv_obj_set_style_bg_color(cover, lv_color_hex(shelf_colors[i % 6]), 0);
        lv_obj_set_style_radius(cover, 8, 0);
        lv_obj_set_style_shadow_width(cover, 12, 0);
        lv_obj_set_style_shadow_opa(cover, 20, 0);
        lv_obj_set_style_shadow_ofs_y(cover, 5, 0);
        lv_obj_align(cover, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_clear_flag(cover, LV_OBJ_FLAG_SCROLLABLE);

        // 书封上的书名
        lv_obj_t * label_on_cover = lv_label_create(cover);
        lv_label_set_text(label_on_cover, info->fileName);
        lv_obj_set_style_text_font(label_on_cover, &_lvgl_font1, 0);
        lv_obj_set_style_text_color(label_on_cover, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_width(label_on_cover, book_w - 15); 
        lv_label_set_long_mode(label_on_cover, LV_LABEL_LONG_WRAP); 
        lv_obj_set_style_text_align(label_on_cover, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(label_on_cover, LV_ALIGN_CENTER, 0, -10);

        // txt 格式文字
        lv_obj_t * label_format = lv_label_create(cover);
        lv_label_set_text(label_format, "txt");
        lv_obj_set_style_text_font(label_format, &_lvgl_font3, 0); 
        lv_obj_set_style_text_color(label_format, lv_color_hex(0xEEEEEE), 0);
        lv_obj_align(label_format, LV_ALIGN_BOTTOM_MID, 0, -5);

        // 逻辑绑定
        lv_obj_add_flag(cover, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(cover, book_click_event_cb, LV_EVENT_CLICKED, info);

        // 书封下方的书名（重点优化处）
        lv_obj_t * name = lv_label_create(book_item);
        lv_label_set_text(name, info->fileName);
        lv_obj_set_style_text_font(name, &_lvgl_font1, 0);
        lv_obj_set_style_text_color(name, lv_color_hex(0x333333), 0);
        lv_obj_set_width(name, book_w);
        // 关键 1：高度设为自适应
        lv_obj_set_height(name, LV_SIZE_CONTENT); 
        // 关键 2：长文本模式设为折行
        lv_label_set_long_mode(name, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_align(name, LV_TEXT_ALIGN_CENTER, 0);
        // 关键 3：相对于 cover 对齐
        lv_obj_align_to(name, cover, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);
        
        i++;
    }
}

void novel_select_screen_deinit(void) 
{
    if (novel_select_screen_obj) {
        lv_obj_del(novel_select_screen_obj);
        novel_select_screen_obj = NULL;
    }
}



/* ----------------------------------------------------------------------------
 * 事件回调函数
 * ---------------------------------------------------------------------------- */

// 书本点击回调
static void book_click_event_cb(lv_event_t * e) 
{
    lv_obj_t * obj = lv_event_get_target(e);
    void * info = lv_event_get_user_data(e);

    if(lv_event_get_code(e) == LV_EVENT_CLICKED) 
	{
       
		NovelControl.curr_info =  (novel_info_t *)info;
//		novel_info_t *i = (novel_info_t *)info;
		//DUMP_NOVEL_INFO(i);
		ui_screen_load(&novel_screen, &UI_SCREEN_LOAD_AMIM);
    }
}

// 返回按钮回调
static void back_btn_event_cb(lv_event_t * e) 
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) 
	{
        printf("APP Log: Back button clicked\n");
        // 这里执行返回逻辑，例如：
		
		
        ui_screen_back();
    }
}


#else

ui_screen_t novel_select_screen = {
    .init = NULL,
    .deinit = NULL,
    .obj = NULL,
};




#endif














