#include "game_select_screen.h"

#if USE_UI_GAME_SELECT_SCREEN
#include <stdlib.h>
#include <stdio.h>

lv_obj_t *game_select_screen_obj;
ui_screen_t game_select_screen =
{
	.init = game_select_screen_init,
	.deinit = game_select_screen_deinit,
	.obj = &game_select_screen_obj,
};

static void game_select_icon_click_event(lv_event_t * e);
static void game_select_quit_event(lv_event_t * e);


/*-------------------------------------------------------------
                        页面初始化/销毁函数
--------------------------------------------------------------*/

void game_select_screen_init(lv_obj_t *parent)
{
	// 布局基础参数
	const int TOP_MARGIN = 90;

	// 主容器：白色背景
	game_select_screen_obj = lv_obj_create(parent);
	lv_obj_set_size(game_select_screen_obj, 480, 480);
	lv_obj_clear_flag(game_select_screen_obj, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_pad_all(game_select_screen_obj, 0, 0);
	lv_obj_set_style_bg_color(game_select_screen_obj, lv_color_hex(0xFFFFFF), 0);
	lv_obj_set_style_border_width(game_select_screen_obj, 0, 0);

	// 退出按钮
	lv_obj_t * btn_quit = lv_btn_create(game_select_screen_obj);
	lv_obj_set_size(btn_quit, 60, 60);
	lv_obj_align(btn_quit, LV_ALIGN_TOP_RIGHT, -15, 15);
	lv_obj_set_style_bg_color(btn_quit, lv_color_hex(0xF2F2F2), 0);
	lv_obj_set_style_text_color(btn_quit, lv_color_hex(0x888888), 0);
	lv_obj_set_style_radius(btn_quit, 10, 0);
	lv_obj_set_style_shadow_width(btn_quit, 0, 0);
	lv_obj_add_event_cb(btn_quit, game_select_quit_event, LV_EVENT_CLICKED, NULL);
	lv_obj_t * lab_quit = lv_label_create(btn_quit);
	lv_label_set_text(lab_quit, LV_SYMBOL_CLOSE);
	lv_obj_center(lab_quit);

    // 初始化属性
	static lv_style_t style_card;
	lv_style_init(&style_card);
	lv_style_set_radius(&style_card, 12);
	lv_style_set_border_width(&style_card, 0);
	lv_style_set_shadow_width(&style_card, 12);
	lv_style_set_shadow_opa(&style_card, 30);
	lv_style_set_shadow_ofs_y(&style_card, 5);
	lv_style_set_text_color(&style_card, lv_color_hex(0xFFFFFF));


	// 游戏1
	lv_obj_t * btn1 = lv_btn_create(game_select_screen_obj);
	lv_obj_set_size(btn1, 210, 160);
	lv_obj_set_pos(btn1, 20, TOP_MARGIN);
	lv_obj_add_style(btn1, &style_card, 0);
	lv_obj_set_style_bg_color(btn1, lv_color_hex(0x91AEC1), 0);
	lv_obj_add_event_cb(btn1, game_select_icon_click_event, LV_EVENT_CLICKED, (void*)1);
	lv_label_set_text(lv_label_create(btn1), "tetris");
	lv_obj_center(lv_obj_get_child(btn1, 0));
	lv_obj_set_style_text_font(lv_obj_get_child(btn1, 0), &_lvgl_font3, 0);


	// 游戏2
	lv_obj_t * btn2 = lv_btn_create(game_select_screen_obj);
	lv_obj_set_size(btn2, 210, 160);
	lv_obj_set_pos(btn2, 250, TOP_MARGIN);
	lv_obj_add_style(btn2, &style_card, 0);
	lv_obj_set_style_bg_color(btn2, lv_color_hex(0xA9BA9D), 0);
	lv_obj_add_event_cb(btn2, game_select_icon_click_event, LV_EVENT_CLICKED, (void*)2);
	lv_label_set_text(lv_label_create(btn2), "snake");
	lv_obj_center(lv_obj_get_child(btn2, 0));
	lv_obj_set_style_text_font(lv_obj_get_child(btn2, 0), &_lvgl_font3, 0);

	// 游戏3
	lv_obj_t * btn3 = lv_btn_create(game_select_screen_obj);
	lv_obj_set_size(btn3, 440, 190);
	lv_obj_set_pos(btn3, 20, 270);
	lv_obj_add_style(btn3, &style_card, 0);
	lv_obj_set_style_bg_color(btn3, lv_color_hex(0xD6C5B0), 0);
	lv_obj_add_event_cb(btn3, game_select_icon_click_event, LV_EVENT_CLICKED, (void*)3);
	lv_label_set_text(lv_label_create(btn3), "2048");
	lv_obj_center(lv_obj_get_child(btn3, 0));
	lv_obj_set_style_text_font(lv_obj_get_child(btn3, 0), &_lvgl_font3, 0);
}

void game_select_screen_deinit(void)
{
	if (game_select_screen_obj != NULL) {
		lv_obj_del(game_select_screen_obj);
		game_select_screen_obj = NULL;
	}
}


/*-------------------------------------------------------------
                        事件/定时器回调函数
--------------------------------------------------------------*/
// 返回主菜单的逻辑
static void game_select_quit_event(lv_event_t * e)
{
	if(lv_event_get_code(e) == LV_EVENT_CLICKED) 
	{
		ui_screen_back();
	}
}

// 具体的游戏跳转逻辑
static void game_select_icon_click_event(lv_event_t * e)
{
	if(lv_event_get_code(e) == LV_EVENT_CLICKED) 
	{
		uint32_t game_id = (uint32_t)(uintptr_t)lv_event_get_user_data(e);
		ui_screen_t *game_screen[] = {&tetris_screen, &snake_screen, &game2048_screen};
		if(game_id >=1 && game_id <= 3)
		{
			ui_screen_load(game_screen[game_id - 1], &UI_SCREEN_LOAD_AMIM);
			
		}
	}
}



#else

ui_screen_t game_select_screen =
{
	.init = NULL,
	.deinit = NULL,
	.obj = NULL,
};


#endif


