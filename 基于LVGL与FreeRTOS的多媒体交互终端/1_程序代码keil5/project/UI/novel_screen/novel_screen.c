#include "novel_screen.h"

#if USE_UI_NOVEL_SCREEN

#include "ex/novel.h"
#include <stdio.h>
#include <string.h>

#define ITEM_HEIGHT 		50
#define NOVEL_TOTAL_ITEMS 	20

lv_obj_t *novel_screen_obj = NULL;
ui_screen_t novel_screen =
{
    .init = novel_screen_init,
    .deinit = novel_screen_deinit,
    .obj = &novel_screen_obj
};


LV_FONT_DECLARE(_lvgl_font3)
static const lv_font_t* novel_fonts = &_lvgl_font3;

static lv_obj_t *novel_label_content;
static lv_obj_t *novel_page_info;
static lv_obj_t *novel_bottom_menu;
static lv_obj_t *novel_chapter_slider;
static lv_obj_t *nivel_list_panel;
static lv_obj_t *novel_list_mask;
static bool is_night_mode = false;
static int current_page = 1;

static void novel_event_handler(lv_event_t * e);
static void menu_toggle_anim(bool show);
static void list_panel_toggle(bool show);
static void menu_common_event_cb(lv_event_t * e);
static void update_ui_content(NovelOp_t opa);
static void list_mask_event_cb(lv_event_t * e);
static void chapter_item_event_cb(lv_event_t * e);
static void list_scroll_event_cb(lv_event_t * e);


NovelReader reader = { 0 };

/*-------------------------------------------------------------
                        页面初始化/销毁函数
--------------------------------------------------------------*/

void novel_screen_init(lv_obj_t *parent)
{
	// ---------小说初始化------
	novel_info_t *info = NovelControl.curr_info;
    novel_init(&reader, NovelControl.Dir, info->fileName);
    novel_set_display_params(&reader, info->page_lines, info->line_max_chars);
	novel_paginate(&reader,info->chapter_idx);
    novel_load_page_data(&reader, info->cur_page_idx);
//	for (int i = 0; i < reader.total_chapters; i++)
//	{
//		printf("%s\r\n", reader.all_chapters[i].title);
//	}
	// ---------小说初始化------

    novel_screen_obj = lv_obj_create(parent);
    lv_obj_remove_style_all(novel_screen_obj);
    lv_obj_set_size(novel_screen_obj, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(novel_screen_obj, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(novel_screen_obj, lv_color_hex(0xFCF5E5), 0);
    lv_obj_clear_flag(novel_screen_obj, LV_OBJ_FLAG_SCROLLABLE);

    // 文本层
    novel_label_content = lv_label_create(novel_screen_obj);
    lv_obj_set_size(novel_label_content, lv_pct(90), lv_pct(80));
    lv_obj_align(novel_label_content, LV_ALIGN_TOP_MID, 0, 35);
    lv_label_set_long_mode(novel_label_content, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(novel_label_content, lv_color_hex(0x1A1A1A), 0);
    lv_obj_set_style_text_line_space(novel_label_content, 8, 0);
	lv_obj_set_style_text_font(novel_label_content, novel_fonts, LV_STATE_DEFAULT);

    // 页码信息
    novel_page_info = lv_label_create(novel_screen_obj);
    lv_obj_align(novel_page_info, LV_ALIGN_BOTTOM_MID, 0, -8);
    lv_obj_set_style_text_color(novel_page_info, lv_color_hex(0x888888), 0);
	lv_obj_set_style_text_font(novel_page_info, &_lvgl_font2, LV_STATE_DEFAULT);
	
    

    // 底部弹出菜单
    novel_bottom_menu = lv_obj_create(novel_screen_obj);
    lv_obj_remove_style_all(novel_bottom_menu);
    lv_obj_set_size(novel_bottom_menu, lv_pct(100), lv_pct(40));
    lv_obj_set_pos(novel_bottom_menu, 0, lv_pct(100));
    lv_obj_set_style_bg_opa(novel_bottom_menu, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(novel_bottom_menu, lv_color_hex(0x2D2D2D), 0);

    // --- 滑块组件 ---
    novel_chapter_slider = lv_slider_create(novel_bottom_menu);
    lv_obj_set_size(novel_chapter_slider, lv_pct(55), 6);
    lv_obj_align(novel_chapter_slider, LV_ALIGN_TOP_MID, 0, 35);
    lv_slider_set_range(novel_chapter_slider, 1, 100);
    lv_slider_set_value(novel_chapter_slider, current_page, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(novel_chapter_slider, lv_color_hex(0x555555), LV_PART_MAIN);
    lv_obj_set_style_bg_color(novel_chapter_slider, lv_color_hex(0x00AABB), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(novel_chapter_slider, lv_color_hex(0xFFFFFF), LV_PART_KNOB);
    lv_obj_set_style_pad_all(novel_chapter_slider, 3, LV_PART_KNOB);
    lv_obj_add_event_cb(novel_chapter_slider, menu_common_event_cb, LV_EVENT_VALUE_CHANGED, (void*)(uintptr_t)6);

    // --- 按钮组件 ---
    const char * btn_texts[] = {"上一章", "下一章", "返回", "夜间", "返回"/*返回*/};
    lv_coord_t btn_w[] = {85, 85, 95, 95, 95};
    lv_coord_t btn_h[] = {45, 45, 50, 50, 50};
    lv_obj_t * btns[5];
    for(int i = 0; i < 5; i++)
    {
        btns[i] = lv_obj_create(novel_bottom_menu);
        lv_obj_remove_style_all(btns[i]);
        lv_obj_set_size(btns[i], btn_w[i], btn_h[i]);
        lv_obj_set_style_bg_opa(btns[i], LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(btns[i], lv_color_hex(0x2D2D2D), 0);
        lv_obj_set_style_radius(btns[i], 10, 0);
        lv_obj_add_flag(btns[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_color(btns[i], lv_color_hex(0x444444), LV_STATE_PRESSED);

        lv_obj_t * lbl = lv_label_create(btns[i]);
        lv_label_set_text(lbl, btn_texts[i]);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xE0E0E0), 0);
		lv_obj_set_style_text_font(lbl, &_lvgl_font2, LV_STATE_DEFAULT);
        lv_obj_center(lbl);
        lv_obj_add_event_cb(btns[i], menu_common_event_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)i);
    }
    lv_obj_align_to(btns[0], novel_chapter_slider, LV_ALIGN_OUT_LEFT_MID, -15, 0);
    lv_obj_align_to(btns[1], novel_chapter_slider, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
    lv_obj_align(btns[2], LV_ALIGN_BOTTOM_LEFT, 25, -20);
    lv_obj_align(btns[3], LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_align(btns[4], LV_ALIGN_BOTTOM_RIGHT, -25, -20);

    // 章节列表逻辑
    novel_list_mask = lv_obj_create(novel_screen_obj);
    lv_obj_set_size(novel_list_mask, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(novel_list_mask, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(novel_list_mask, LV_OPA_0, 0);
    lv_obj_add_flag(novel_list_mask, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(novel_list_mask, list_mask_event_cb, LV_EVENT_CLICKED, NULL);

    nivel_list_panel = lv_obj_create(novel_screen_obj);
    lv_obj_set_size(nivel_list_panel, lv_pct(70), lv_pct(100));
    lv_obj_set_pos(nivel_list_panel, -lv_pct(70), 0);
    lv_obj_set_style_bg_color(nivel_list_panel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(nivel_list_panel, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(nivel_list_panel, 0, 0);
    lv_obj_set_style_border_width(nivel_list_panel, 0, 0);
    lv_obj_set_style_pad_all(nivel_list_panel, 0, 0);
    lv_obj_set_scrollbar_mode(nivel_list_panel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(nivel_list_panel, list_scroll_event_cb, LV_EVENT_SCROLL, NULL);

    for(int i = 0; i < NOVEL_TOTAL_ITEMS; i++)
    {
        lv_obj_t * item = lv_obj_create(nivel_list_panel);
        lv_obj_set_size(item, lv_pct(100), ITEM_HEIGHT);
        lv_obj_set_pos(item, 0, i * ITEM_HEIGHT);
        lv_obj_set_style_bg_opa(item, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(item, 1, 0);
        lv_obj_set_style_border_side(item, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_color(item, lv_color_hex(0xEEEEEE), 0);
        lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE);
		lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(item, lv_color_hex(0xDDDDDD), LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(item, LV_OPA_COVER, LV_STATE_PRESSED);
        lv_obj_set_user_data(item, (void*)(uintptr_t)(i + 200));

        lv_obj_t * item_lbl = lv_label_create(item);
        lv_label_set_text_fmt(item_lbl, "Chapter %d", i + 200);
        lv_obj_set_style_text_color(item_lbl, lv_color_hex(0x333333), 0);
        lv_obj_align(item_lbl, LV_ALIGN_LEFT_MID, 20, 0);

        lv_obj_add_event_cb(item, chapter_item_event_cb, LV_EVENT_CLICKED, NULL);
    }

    lv_obj_add_event_cb(novel_screen_obj, novel_event_handler, LV_EVENT_ALL, NULL);
	
	update_ui_content(NOVEL_OP_RELOAD);
}



void novel_screen_deinit(void)
{
    if (novel_screen_obj) {
        lv_obj_del(novel_screen_obj);
        novel_screen_obj = NULL;
    }
	
	novel_info_t *info = NovelControl.curr_info;
	
	info->page_lines = reader.active_chap.page_lines;
	info->line_max_chars = reader.active_chap.line_max_chars;
	info->chapter_idx = reader.active_chap.chapter_idx;
	info->cur_page_idx = reader.active_chap.cur_page_idx;
	
	novel_deinit(&reader);
}





/*-------------------------------------------------------------
                        事件/定时器回调函数
--------------------------------------------------------------*/

// 章节列表--无限滚动 回调函数
static void list_scroll_event_cb(lv_event_t * e)
{

}

// 章节列表-章节点击 回调函数
static void chapter_item_event_cb(lv_event_t * e)
{
    lv_obj_t * target = lv_event_get_target(e);
    int chapter = (int)(uintptr_t)lv_obj_get_user_data(target);
    current_page = chapter;
    update_ui_content(NOVEL_OP_RELOAD);
 //   lv_slider_set_value(novel_chapter_slider, current_page % 100, LV_ANIM_ON);
    list_panel_toggle(false);
}

// 章节列表--点击退出 回调函数
static void list_mask_event_cb(lv_event_t * e)
{
    list_panel_toggle(false);
}



// 菜单选择 回调函数
static void menu_common_event_cb(lv_event_t * e)
{
    int index = (int)(uintptr_t)lv_event_get_user_data(e);
    switch(index)
    {
        case 0://上一章节
        {
			update_ui_content(NOVEL_OP_PREV_CHAPTER);
            break;
        }
        case 1:// 下一章节
        {
			update_ui_content(NOVEL_OP_NEXT_CHAPTER);
            break;
        }
        case 2: // 目录
        {
            //menu_toggle_anim(false);
            //list_panel_toggle(true);
			ui_screen_back();
            break;
        }
        case 3: // 主题夜间
        {
            is_night_mode = !is_night_mode;
            uint32_t bg = is_night_mode ? 0x121212 : 0xFCF5E5;
            uint32_t txt = is_night_mode ? 0x888888 : 0x1A1A1A;
            lv_obj_set_style_bg_color(novel_screen_obj, lv_color_hex(bg), 0);
            lv_obj_set_style_text_color(novel_label_content, lv_color_hex(txt), 0);
            break;
        }
        case 4://返回
        {
			ui_screen_back();
            break;
        }
        case 6://进度条
        {
			lv_obj_t * slider = lv_event_get_target(e);
			int slider_val = (int)lv_slider_get_value(slider); // 获取 0-100 的值

			// 1. 映射计算：将 0-100 映射到 0 到 total_chapters-1
			// 注意：total_chapters 必须 > 0
			if (reader.total_chapters > 0) 
			{
				int target_chap = (slider_val * (reader.total_chapters - 1)) / 100;

				// 2. 调用之前封装的控制逻辑
				if (target_chap < reader.total_chapters && target_chap >= 0)
				{
					// 切换对应章节
					novel_paginate(&reader, target_chap);
					novel_load_page_data(&reader, 0);
				}
				// 3. 刷新 UI 显示
				update_ui_content(NOVEL_OP_RELOAD);
			}
        }
    }
}


// 页面滑动回调函数
static void novel_event_handler(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_GESTURE)
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if(dir == LV_DIR_TOP)
        {
            menu_toggle_anim(true);
        }
        else if(dir == LV_DIR_BOTTOM)
        {
            menu_toggle_anim(false);
        }
        else if(dir == LV_DIR_RIGHT)
        {
			//printf("LV_DIR_RIGHT\r\n");
			update_ui_content(NOVEL_OP_PREV_PAGE);
        }
        else if(dir == LV_DIR_LEFT)
        {
			//printf("LV_DIR_LEFT\r\n");
			update_ui_content(NOVEL_OP_NEXT_PAGE);
        }
    }
}

/*-------------------------------------------------------------
                        static函数
--------------------------------------------------------------*/
// 更新页面内容
static void update_ui_content(NovelOp_t opa)
{
	switch(opa)
	{
		case NOVEL_OP_RELOAD:	// 加载数据
			break;
		case NOVEL_OP_PREV_PAGE://上一页
		{
            if (reader.active_chap.cur_page_idx > 0)
            {
                reader.active_chap.cur_page_idx--;
                novel_load_page_data(&reader, reader.active_chap.cur_page_idx);
            }
            else if (reader.active_chap.chapter_idx > 0)
            {
                // 切换到上一章：分页 + 设置为末页索引 + 手动加载
                novel_paginate(&reader, reader.active_chap.chapter_idx - 1);
                reader.active_chap.cur_page_idx = reader.active_chap.total_pages - 1;
                novel_load_page_data(&reader, reader.active_chap.cur_page_idx);
            }
			break;
		}
		case NOVEL_OP_PREV_CHAPTER: // 上一章
		{
            if (reader.active_chap.chapter_idx > 0)
            {
                // 切换到上一章：分页 + 设置为末页索引 + 手动加载
                novel_paginate(&reader, reader.active_chap.chapter_idx - 1);
                reader.active_chap.cur_page_idx = 0;
                novel_load_page_data(&reader, reader.active_chap.cur_page_idx);
            }
			break;
		}
		case NOVEL_OP_NEXT_PAGE:// 下一页
		{
            if (reader.active_chap.cur_page_idx + 1 < reader.active_chap.total_pages)
            {
                reader.active_chap.cur_page_idx++;
                novel_load_page_data(&reader, reader.active_chap.cur_page_idx);
            }
            else if (reader.active_chap.chapter_idx + 1 < reader.total_chapters)
            {
                // 切换到下一章：分页 + 手动加载
                novel_paginate(&reader, reader.active_chap.chapter_idx + 1);
                novel_load_page_data(&reader, 0);
            }
			break;
		}
		case NOVEL_OP_NEXT_CHAPTER:// 下一章
		{
            if (reader.active_chap.chapter_idx + 1 < reader.total_chapters)
            {
                // 切换到下一章：分页 + 手动加载
                novel_paginate(&reader, reader.active_chap.chapter_idx + 1);
                novel_load_page_data(&reader, 0);
            }
			break;
		}
	}
	
    lv_label_set_text(novel_label_content, reader.active_chap.cur_page_content);
	//printf("%s", reader.active_chap.cur_page_content);
    lv_label_set_text_fmt(novel_page_info, "第%d章 / 共%d章", reader.active_chap.chapter_idx + 1, reader.total_chapters);
	
	// 1. 获取总章节数和当前章节索引
	uint32_t total = reader.total_chapters;
	uint32_t current = reader.active_chap.chapter_idx;

	// 2. 计算百分比 (注意防止除以0)
	uint16_t slider_val = 0;
	if (total > 1) { slider_val = (uint16_t)(current * 100 / (total - 1));}
	else if (total == 1){slider_val = 100;} // 只有一章时直接满进度

	// 3. 设置滑块数值 (假设滑块范围已设为 0-100)
	lv_slider_set_value(novel_chapter_slider, slider_val, LV_ANIM_ON);
	
}


static void menu_toggle_anim(bool show)
{
    lv_obj_set_y(novel_bottom_menu, show ? lv_pct(60) : lv_pct(100));
}

static void list_panel_toggle(bool show)
{
    if(show) {
        lv_obj_clear_flag(novel_list_mask, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_bg_opa(novel_list_mask, LV_OPA_40, 0);
        lv_obj_set_x(nivel_list_panel, 0);
    } else {
        lv_obj_add_flag(novel_list_mask, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_x(nivel_list_panel, -lv_pct(70));
    }
}

#else
ui_screen_t novel_screen =
{
    .init = NULL,
    .deinit = NULL,
    .obj = NULL
};



#endif

