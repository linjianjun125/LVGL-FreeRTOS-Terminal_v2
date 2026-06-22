#include "tool_select_screen.h"


#if USE_UI_TOOL_SELECT_SCREEN
#include <stdlib.h>
#include <stdio.h>

lv_obj_t * tool_select_screen_obj = NULL;
ui_screen_t tool_select_screen = 
{
    .init = tool_select_screen_init,
    .deinit = tool_select_screen_deinit,
    .obj = &tool_select_screen_obj,
};


/* --- 配置参数 --- */
#define SCREEN_W          480
#define SCREEN_H          480
#define CARD_W            420
#define CARD_H            140
#define CARD_GAP          25
#define STACK_TOP_Y       60
#define STACK_VISIBLE_H   35      // 堆叠后可见的页签高度
#define ITEM_STEP_Y       (CARD_H + CARD_GAP)
#define DOUBLE_CLICK_TIME 300     // 双击判定的毫秒阈值

#define ITEM_CNT          4

uint32_t tool_scrren_card_colors[ITEM_CNT] = {0x6366F1, 0x10B981, /*0xF59E0B,*/ 0xEF4444, 0x8B5CF6};
const char * tool_screeen_card_names[ITEM_CNT] = {"CALCULATOR", "LEVEL", /*"COMPASS",*/ "STOPWATCH", "BATTERY"};
ui_screen_t *tool_screen_list[ITEM_CNT] = {&calculator_screen, &level_screen, /*NULL, */&stopwatch_screen, &battery_screen};
static lv_obj_t * tool_select_screen_scroll_cont = NULL;
static lv_style_t tool_select_screen_style_card;
static void tool_select_screen_scroll_event_cb(lv_event_t * e);
static void tool_screen_card_click_event(lv_event_t * e);
static void tool_screen_quit_event(lv_event_t * e);


/*-------------------------------------------------------------
                        页面初始化/销毁函数
--------------------------------------------------------------*/
void tool_select_screen_init(lv_obj_t * parent)
{
    // 创建主屏幕 (纯白背景)
    tool_select_screen_obj = lv_obj_create(parent);
    lv_obj_set_size(tool_select_screen_obj, SCREEN_W, SCREEN_H);
    lv_obj_set_style_bg_color(tool_select_screen_obj, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_border_width(tool_select_screen_obj, 0, 0);
    lv_obj_clear_flag(tool_select_screen_obj, LV_OBJ_FLAG_SCROLLABLE);

    // 初始化卡片样式
    lv_style_init(&tool_select_screen_style_card);
    lv_style_set_radius(&tool_select_screen_style_card, 16);
    lv_style_set_shadow_width(&tool_select_screen_style_card, 20);
    lv_style_set_shadow_color(&tool_select_screen_style_card, lv_color_hex(0x000000));
    lv_style_set_shadow_opa(&tool_select_screen_style_card, LV_OPA_30);

    // 创建滚动容器
    tool_select_screen_scroll_cont = lv_obj_create(tool_select_screen_obj);
    lv_obj_set_size(tool_select_screen_scroll_cont, SCREEN_W, SCREEN_H);
    lv_obj_set_style_bg_opa(tool_select_screen_scroll_cont, 0, 0);
    lv_obj_set_style_border_width(tool_select_screen_scroll_cont, 0, 0);
    lv_obj_set_scrollbar_mode(tool_select_screen_scroll_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_bottom(tool_select_screen_scroll_cont, SCREEN_H - 100, 0);


    // 循环创建卡片
    for(int i = 0; i < ITEM_CNT; i++)
    {
        lv_obj_t * card = lv_obj_create(tool_select_screen_scroll_cont);
        lv_obj_set_size(card, CARD_W, CARD_H);
        lv_obj_set_pos(card, (SCREEN_W - CARD_W) / 2, i * ITEM_STEP_Y + 70);

        lv_obj_add_style(card, &tool_select_screen_style_card, 0);
        lv_obj_set_style_bg_color(card, lv_color_hex(tool_scrren_card_colors[i % 5]), 0);
        lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_add_flag(card, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(card, tool_screen_card_click_event, LV_EVENT_CLICKED, (void*)(uintptr_t)i);

        // 左上角标题
        lv_obj_t * label = lv_label_create(card);
        lv_label_set_text(label, tool_screeen_card_names[i]);
        lv_obj_set_style_text_font(label, &_lvgl_font2, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, -5);

        // 卡片内容大文字
        lv_obj_t * info = lv_label_create(card);
        lv_label_set_text(info, tool_screeen_card_names[i]);
        lv_obj_set_style_text_font(info, &_lvgl_font3, 0);
        lv_obj_set_style_text_color(info, lv_color_hex(0xFFFFFF), 0);
        lv_obj_align(info, LV_ALIGN_CENTER, 0, 10);
    }

    // 返回按钮
    lv_obj_t * back_btn = lv_btn_create(tool_select_screen_obj);
    lv_obj_set_size(back_btn, 60, 60);
    lv_obj_set_style_radius(back_btn, 10, 0);
    lv_obj_align(back_btn, LV_ALIGN_TOP_RIGHT, -15, 15);
    lv_obj_set_style_bg_color(back_btn, lv_color_hex(0xeeeeee), 0);
    lv_obj_set_style_shadow_width(back_btn, 0, 0);
    lv_obj_add_event_cb(back_btn, tool_screen_quit_event, LV_EVENT_CLICKED, NULL);

    lv_obj_t * btn_lbl = lv_label_create(back_btn);
    lv_label_set_text(btn_lbl, LV_SYMBOL_CLOSE);
    lv_obj_set_style_text_color(btn_lbl, lv_color_hex(0x333333), 0);
    lv_obj_center(btn_lbl);

    lv_obj_add_event_cb(tool_select_screen_scroll_cont, tool_select_screen_scroll_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_update_layout(tool_select_screen_scroll_cont);
    lv_event_send(tool_select_screen_scroll_cont, LV_EVENT_SCROLL, NULL);
}


void tool_select_screen_deinit(void)
{
    if (tool_select_screen_obj != NULL) 
	{
        lv_obj_del(tool_select_screen_obj);
        tool_select_screen_obj = NULL;
    }
    lv_style_reset(&tool_select_screen_style_card);
}


/*-------------------------------------------------------------
                        事件/定时器回调函数
--------------------------------------------------------------*/
// 返回逻辑
static void tool_screen_quit_event(lv_event_t * e)
{
	if(lv_event_get_code(e) == LV_EVENT_CLICKED) 
	{
		ui_screen_back();
	}
}



static void tool_select_screen_scroll_event_cb(lv_event_t * e)
{
    lv_obj_t * cont = lv_event_get_target(e);
    lv_coord_t scroll_y = lv_obj_get_scroll_y(cont);
    uint32_t cnt = lv_obj_get_child_cnt(cont);

    for(uint32_t i = 0; i < cnt; i++) {
        lv_obj_t * child = lv_obj_get_child(cont, i);
        lv_coord_t real_ori_y = lv_obj_get_y_aligned(child) - scroll_y;
        lv_coord_t target_stack_y = STACK_TOP_Y + (i * STACK_VISIBLE_H);

        lv_obj_t * title_label = lv_obj_get_child(child, 0);
        lv_obj_t * center_info = lv_obj_get_child(child, 1);

        if(real_ori_y < target_stack_y) {
            lv_coord_t diff = target_stack_y - real_ori_y;
            lv_obj_set_style_translate_y(child, diff, 0);

            if(diff > 10) {
                lv_obj_clear_flag(child, LV_OBJ_FLAG_CLICKABLE);
            } else {
                lv_obj_add_flag(child, LV_OBJ_FLAG_CLICKABLE);
            }

            lv_obj_set_style_text_opa(title_label, LV_OPA_COVER, 0);
            lv_opa_t opa = (diff > 40) ? LV_OPA_TRANSP : (255 - (diff * 6));
            lv_obj_set_style_text_opa(center_info, opa, 0);
        }
        else {
            lv_obj_set_style_translate_y(child, 0, 0);
            lv_obj_add_flag(child, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_text_opa(title_label, LV_OPA_COVER, 0);
            lv_obj_set_style_text_opa(center_info, LV_OPA_COVER, 0);
        }
    }
}

static void tool_screen_card_click_event(lv_event_t * e)
 {
    static uint32_t last_tick = 0;
    static int last_index = -1;

    uint32_t current_tick = lv_tick_get();
    int current_index = (int)(uintptr_t)lv_event_get_user_data(e);

    // 如果点击的是同一个卡片，且两次点击间隔小于设定阈值
    if (current_index == last_index && (current_tick - last_tick) < DOUBLE_CLICK_TIME)
    {
        printf("Card %d Double Clicked!\n", current_index + 1);
        last_tick = 0; // 重置防止三击连续触发
		
		// 加载APP
		if(current_index >= 0 && current_index < ITEM_CNT)
		{
			ui_screen_load(tool_screen_list[current_index], &UI_SCREEN_LOAD_AMIM);
		}
		
    } else
    {
        printf("Card %d Single Clicked!\n", current_index + 1);
        last_tick = current_tick;
        last_index = current_index;
    }
}


#else

ui_screen_t tool_select_screen = 
{
    .init = NULL,
    .deinit = NULL,
    .obj = NULL,
};



#endif
