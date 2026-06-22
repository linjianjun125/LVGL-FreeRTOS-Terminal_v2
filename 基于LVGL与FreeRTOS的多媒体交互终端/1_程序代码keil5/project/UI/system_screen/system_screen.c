#include "system_screen.h"

#if USE_UI_SYSTEM_SCREEN
#include "system_screen_ex.h"
#include <stdio.h>

lv_obj_t *system_screen_obj = NULL;
ui_screen_t system_screen =
{
    .init = system_screen_init,
    .deinit = system_screen_deinit,
    .obj = &system_screen_obj,
};


#define SYSTEM_SCREEEN_ICON_SOUND       LV_SYMBOL_VOLUME_MID"  Sound"
#define SYSTEM_SCREEEN_ICON_TIME        LV_SYMBOL_SETTINGS"  Time"
#define SYSTEM_SCREEEN_ICON_BRIGHTNESS  LV_SYMBOL_SETTINGS"  Brightness"
#define SYSTEM_SCREEEN_ICON_ABOUT       LV_SYMBOL_HOME"  About"
#define SYSTEM_SCREEEN_ICON_ABOUT_      LV_SYMBOL_HOME"  About"


lv_obj_t *system_current_menu_obj = NULL;
lv_obj_t *system_current_select_obj = NULL;
system_menu_index_t system_current_select_index = SYSTEM_MENU_SOUND;
lv_obj_t *system_select_obj[10];
lv_obj_t *menu_parent_obj = NULL;

const char* system_screen_icon[5] =
{
    SYSTEM_SCREEEN_ICON_SOUND, SYSTEM_SCREEEN_ICON_TIME, SYSTEM_SCREEEN_ICON_BRIGHTNESS,
    SYSTEM_SCREEEN_ICON_ABOUT, SYSTEM_SCREEEN_ICON_ABOUT_,
};

static void system_screen_icon_click_event(lv_event_t * e);
static void system_screen_quit_event(lv_event_t * e);


/*-------------------------------------------------------------
                        页面初始化/销毁函数
--------------------------------------------------------------*/
void system_screen_init(lv_obj_t *parent)
{
    system_screen_obj = lv_obj_create(parent);
    lv_obj_set_size(system_screen_obj, lv_pct(100), lv_pct(100));
    lv_obj_clear_flag(system_screen_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(system_screen_obj, lv_color_hex(0xFFFFFF), 0); // 纯白背景

    // Settings 标题
    lv_obj_t *set_label_obj = lv_label_create(system_screen_obj);
    lv_obj_align(set_label_obj, LV_ALIGN_TOP_LEFT, 15, 12);
    lv_label_set_text(set_label_obj, "Settings");
    lv_obj_set_style_text_color(set_label_obj, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_text_font(set_label_obj, &lv_font_montserrat_14, 0);

    // Setting 容器
    lv_obj_t *set_obj = lv_obj_create(system_screen_obj);
    lv_obj_remove_style_all(set_obj);
    lv_obj_set_size(set_obj, 140, 108);
    lv_obj_align(set_obj, LV_ALIGN_TOP_LEFT, 10, 45);
    lv_obj_set_style_bg_color(set_obj, lv_color_hex(0xF2F2F7), 0);
    lv_obj_set_style_bg_opa(set_obj, LV_OPA_100, 0);
    lv_obj_set_style_radius(set_obj, 10, 0);
    lv_obj_set_style_clip_corner(set_obj, true, 0);

    for(int i = 0; i < 3; i++)
    {
        system_select_obj[i] = lv_obj_create(set_obj);
        lv_obj_remove_style_all(system_select_obj[i]);
        lv_obj_set_size(system_select_obj[i], 140, 36);
        lv_obj_align(system_select_obj[i], LV_ALIGN_TOP_MID, 0, 36 * i);
        lv_obj_set_style_radius(system_select_obj[i], 10, 0);

        lv_obj_t *label = lv_label_create(system_select_obj[i]);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 12, 0);
        lv_label_set_text(label, system_screen_icon[i]);
        lv_obj_set_style_text_color(label, lv_color_hex(0x3A3A3C), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);

        lv_obj_add_event_cb(system_select_obj[i], system_screen_icon_click_event, LV_EVENT_CLICKED, NULL);
    }

    // Info 标题
    lv_obj_t *info_label_obj = lv_label_create(system_screen_obj);
    lv_obj_align(info_label_obj, LV_ALIGN_TOP_LEFT, 15, 185);
    lv_label_set_text(info_label_obj, "Info");
    lv_obj_set_style_text_color(info_label_obj, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_text_font(info_label_obj, &lv_font_montserrat_14, 0);

    // Info 容器
    lv_obj_t *info_obj = lv_obj_create(system_screen_obj);
    lv_obj_remove_style_all(info_obj);
    lv_obj_set_size(info_obj, 140, 72);
    lv_obj_align(info_obj, LV_ALIGN_TOP_LEFT, 10, 215);
    lv_obj_set_style_bg_color(info_obj, lv_color_hex(0xF2F2F7), 0);
    lv_obj_set_style_bg_opa(info_obj, LV_OPA_100, 0);
    lv_obj_set_style_radius(info_obj, 10, 0);
    lv_obj_set_style_clip_corner(info_obj, true, 0);

    for(int i = 3; i < 5; i++)
    {
        system_select_obj[i] = lv_obj_create(info_obj);
        lv_obj_remove_style_all(system_select_obj[i]);
        lv_obj_set_size(system_select_obj[i], 140, 36);
        lv_obj_align(system_select_obj[i], LV_ALIGN_TOP_MID, 0, 36 * (i - 3));
        lv_obj_set_style_radius(system_select_obj[i], 10, 0);

        lv_obj_t *label = lv_label_create(system_select_obj[i]);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 12, 0);
        lv_label_set_text(label, system_screen_icon[i]);
        lv_obj_set_style_text_color(label, lv_color_hex(0x3A3A3C), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);

        lv_obj_add_event_cb(system_select_obj[i], system_screen_icon_click_event, LV_EVENT_CLICKED, NULL);
    }

	// 退出按钮
	lv_obj_t * btn_quit = lv_btn_create(system_screen_obj);
	lv_obj_set_size(btn_quit, 60, 60);
	lv_obj_align(btn_quit, LV_ALIGN_BOTTOM_LEFT, 15, -15);
	lv_obj_set_style_bg_color(btn_quit, lv_color_hex(0xF2F2F2), 0);
	lv_obj_set_style_text_color(btn_quit, lv_color_hex(0x888888), 0);
	lv_obj_set_style_radius(btn_quit, 10, 0);
	lv_obj_set_style_shadow_width(btn_quit, 0, 0);
	
	lv_obj_t * lab_quit = lv_label_create(btn_quit);
	lv_label_set_text(lab_quit, LV_SYMBOL_CLOSE);
	lv_obj_center(lab_quit);
	lv_obj_add_event_cb(btn_quit, system_screen_quit_event, LV_EVENT_CLICKED, NULL);


    // 右侧内容区
    menu_parent_obj = lv_obj_create(system_screen_obj);
    lv_obj_remove_style_all(menu_parent_obj);
    lv_obj_set_size(menu_parent_obj, 320, 480);
    lv_obj_align(menu_parent_obj, LV_ALIGN_TOP_RIGHT, 0, 0);

    // 初始化默认选中状态
    system_current_menu_obj = system_menu_switch_create(menu_parent_obj, (system_menu_index_t)0);
    system_current_select_obj = system_select_obj[0];
	system_current_select_index = (system_menu_index_t)0;

    lv_obj_set_style_bg_color(system_current_select_obj, lv_color_hex(0x007AFF), 0); // 蓝色高亮
    lv_obj_set_style_bg_opa(system_current_select_obj, LV_OPA_100, 0);
    lv_obj_t *first_label = lv_obj_get_child(system_current_select_obj, 0);
    lv_obj_set_style_text_color(first_label, lv_color_hex(0xFFFFFF), 0);
}

void system_screen_deinit(void)
{
    if (system_screen_obj != NULL) {
        lv_obj_del(system_screen_obj);
        system_screen_obj = NULL;
    }
}



/*-------------------------------------------------------------
                        事件/定时器回调函数
--------------------------------------------------------------*/
// 返回主菜单的逻辑
static void system_screen_quit_event(lv_event_t * e)
{
	if(lv_event_get_code(e) == LV_EVENT_CLICKED) 
	{
		ui_screen_back();
	}
}



static void system_screen_icon_click_event(lv_event_t * e)
{
    lv_obj_t * target = lv_event_get_target(e);

    if(lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        for(int i = 0; i < 5; i++)
        {
            if(system_select_obj[i] == target)
            {
                // 恢复旧项为透明背景，深灰文字
                lv_obj_set_style_bg_opa(system_current_select_obj, LV_OPA_0, 0);
                lv_obj_set_style_text_color(lv_obj_get_child(system_current_select_obj, 0), lv_color_hex(0x3A3A3C), 0);

                // 更新新项为蓝色背景，白色文字
                system_current_select_obj = target;
                lv_obj_set_style_bg_color(system_current_select_obj, lv_color_hex(0x007AFF), 0);
                lv_obj_set_style_bg_opa(system_current_select_obj, LV_OPA_100, 0);
                lv_obj_set_style_text_color(lv_obj_get_child(system_current_select_obj, 0), lv_color_hex(0xFFFFFF), 0);

                // 切换菜单
				if(i >= 0 && i < SYSTEM_MENU_MAX)
				{
					if(system_current_menu_obj) system_menu_switch_del(system_current_menu_obj, system_current_select_index);
					system_current_menu_obj = system_menu_switch_create(menu_parent_obj, (system_menu_index_t)i);
					system_current_select_index = (system_menu_index_t)i;
					break;
				}
            }
        }
    }
}

#else

ui_screen_t system_screen =
{
    .init = NULL,
    .deinit = NULL,
    .obj = NULL,
};

#endif

