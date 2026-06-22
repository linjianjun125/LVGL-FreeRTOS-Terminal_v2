#include "app_select_screen.h"

#if USE_UI_APP_SELECT_SCREEN


/* --- 尺寸与布局常量 --- */
#define CARD_BASE_W    200
#define CARD_BASE_H    260
#define CARD_MAX_H     300
#define CARD_GAP       40
#define CARD_START_X   140
#define APP_SCREEN_APP_NUMBER 6

/* --- 静态变量 --- */
lv_obj_t * app_select_screen_obj = NULL;
ui_screen_t app_select_screen = 
{
    .init = app_select_screen_init,
    .deinit = app_select_screen_deinit,
    .obj = &app_select_screen_obj,
};

static uint32_t colors[3] = {0x91AEC1, 0xA9BA9D, 0xD6C5B0};
ui_screen_t* app_screen_list[APP_SCREEN_APP_NUMBER] = 
{
	&game_select_screen, &tool_select_screen, &novel_select_screen,
	&file_screen, &system_screen, &calendar_screen,
};

static char* app_select_screen_app_name[APP_SCREEN_APP_NUMBER] = 
{
	"游戏", "工具", "小说",
	"文件", "系统","日历"
};

//static char* app_select_screen_app_name[APP_SCREEN_APP_NUMBER] = 
//{
//	"Games", "Tools", "Books",
//	"Files", "System", "Calendar"
//};

// UI 句柄
static lv_obj_t * app_select_screen_scroll_cont = NULL;
static lv_obj_t * label_time = NULL, * label_date = NULL, * label_vol = NULL, * label_bat = NULL;

static lv_coord_t last_scroll_x = 0;

// 函数声明
static void ui_status_update_cb(sensor_id_t id, const uint8_t* data);
static void scroll_event_cb(lv_event_t * e);
static void card_event_cb(lv_event_t * e);
static void gesture_event_cb(lv_event_t * e);



/*-------------------------------------------------------------
                        页面生命周期
--------------------------------------------------------------*/
void app_select_screen_init(lv_obj_t * parent)
{
    // 1. 基础容器
    app_select_screen_obj = lv_obj_create(parent);
    lv_obj_set_size(app_select_screen_obj, 480, 480);
    lv_obj_set_style_bg_color(app_select_screen_obj, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_width(app_select_screen_obj, 0, 0);
    lv_obj_clear_flag(app_select_screen_obj, LV_OBJ_FLAG_SCROLLABLE);

    // 2. 状态栏创建
    label_date = lv_label_create(app_select_screen_obj);
    lv_obj_align(label_date, LV_ALIGN_TOP_LEFT, 20, 15);
    
    label_time = lv_label_create(app_select_screen_obj);
    lv_obj_align(label_time, LV_ALIGN_TOP_MID, 0, 15);
    lv_obj_set_style_text_font(label_time, &lv_font_montserrat_20, 0);

    label_vol = lv_label_create(app_select_screen_obj);
    lv_obj_align(label_vol, LV_ALIGN_TOP_RIGHT, -110, 15);

    label_bat = lv_label_create(app_select_screen_obj);
    lv_obj_align(label_bat, LV_ALIGN_TOP_RIGHT, -15, 15);

    // 3. 应用滚动容器
    app_select_screen_scroll_cont = lv_obj_create(app_select_screen_obj);
    lv_obj_set_size(app_select_screen_scroll_cont, 480, 400);
    lv_obj_align(app_select_screen_scroll_cont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_opa(app_select_screen_scroll_cont, 0, 0);
    lv_obj_set_style_border_width(app_select_screen_scroll_cont, 0, 0);
    lv_obj_set_scrollbar_mode(app_select_screen_scroll_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_snap_x(app_select_screen_scroll_cont, LV_SCROLL_SNAP_CENTER);

    // 4. 卡片生成
    for(int i = 0; i < APP_SCREEN_APP_NUMBER; i++) {
        lv_obj_t * card = lv_obj_create(app_select_screen_scroll_cont);
        lv_obj_set_size(card, CARD_BASE_W, CARD_BASE_H);
        lv_obj_set_pos(card, CARD_START_X + i * (CARD_BASE_W + CARD_GAP), 50);
        lv_obj_set_style_bg_color(card, lv_color_hex(colors[i % 3]), 0);
        lv_obj_set_style_radius(card, 24, 0);
        lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t * btn_label = lv_label_create(card);
        lv_label_set_text(btn_label, app_select_screen_app_name[i]);
        lv_obj_set_style_text_font(btn_label, &_lvgl_font3, 0); // 确保字体包含中文
        lv_obj_center(btn_label);

        lv_obj_add_event_cb(card, card_event_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
    }

    // 5. 事件订阅与绑定
    SensorHub.subscribe(SENSOR_BATTERY, ui_status_update_cb);
	SensorHub.subscribe(SENSOR_VOLUME, ui_status_update_cb);
    SensorHub.subscribe(SENSOR_RTC, ui_status_update_cb);

    lv_obj_add_event_cb(app_select_screen_scroll_cont, scroll_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_add_event_cb(app_select_screen_obj, gesture_event_cb, LV_EVENT_GESTURE, NULL);

	// 恢复上次离开时的位置 (使用无动画模式立即跳转)
    lv_obj_scroll_to_x(app_select_screen_scroll_cont, last_scroll_x, LV_ANIM_OFF);
    
    // 手动触发一次事件处理，确保卡片的缩放高度在进入第一帧就是正确的
    lv_event_send(app_select_screen_scroll_cont, LV_EVENT_SCROLL, NULL);
}

void app_select_screen_deinit(void)
{
    SensorHub.unsubscribe(SENSOR_BATTERY, ui_status_update_cb);
    SensorHub.unsubscribe(SENSOR_VOLUME, ui_status_update_cb);
    SensorHub.unsubscribe(SENSOR_RTC, ui_status_update_cb);

	if (app_select_screen_scroll_cont) {
        // 在容器被删除前，保存当前的滚动位置
        last_scroll_x = lv_obj_get_scroll_x(app_select_screen_scroll_cont);
    }
    if (app_select_screen_obj) {
        lv_obj_del(app_select_screen_obj);
        app_select_screen_obj = NULL;
    }
}

/*-------------------------------------------------------------
                        SensorHub 回调逻辑
--------------------------------------------------------------*/
static void ui_status_update_cb(sensor_id_t id, const uint8_t* data)
{
    // 基础防御：如果主屏幕对象都不存在，说明 UI 还没准备好或已销毁
    if (app_select_screen_obj == NULL) return;

    if (id == SENSOR_BATTERY) 
    {
        // 确保 label 已经创建
        if (label_bat == NULL) return; 

        sensor_battery_data_t* bat = (sensor_battery_data_t*)data;
		//if(bat->has_bat)
		{
			const char *color = (bat->percentage <= 20) ? "FF0000" : 
								(bat->percentage <= 50  ? "FFC800" : "00FF00");
			const char *icon  = (bat->percentage <= 20) ? LV_SYMBOL_BATTERY_EMPTY : LV_SYMBOL_BATTERY_FULL;
			const char *charge_mark = bat->is_charging ? LV_SYMBOL_CHARGE : "";

			lv_label_set_recolor(label_bat, true);
			lv_label_set_text_fmt(label_bat, "#%s %s%s# %d%%", color, charge_mark, icon, (int)bat->percentage);
		}
//		else
//		{
//			lv_label_set_text(label_bat, "  ");
//		}
    }
    else if (id == SENSOR_VOLUME) 
    {
        if (label_vol == NULL) return;

        volume_config_t* vol = (volume_config_t*)data;
        const char *icon = (vol->current_vol == 0) ? LV_SYMBOL_MUTE : 
                           (vol->current_vol < 60 ? LV_SYMBOL_VOLUME_MID : LV_SYMBOL_VOLUME_MAX);
        lv_label_set_text_fmt(label_vol, "%s %d%%", icon, vol->current_vol);
    }
    else if (id == SENSOR_RTC) 
    {
        // 时间和日期可能不在同一个 Label，分开检查
        sensor_rtc_data_t* rtc = (sensor_rtc_data_t*)data;

        if (label_time != NULL) 
		{
            lv_label_set_text_fmt(label_time, "%02d:%02d:%02d", rtc->hour, rtc->minute, rtc->second);
        }

        if (label_date != NULL) 
		{
            lv_label_set_text_fmt(label_date, "20%02d-%02d-%02d", rtc->year, rtc->month, rtc->day);
        }
    }
}

/*-------------------------------------------------------------
                        交互处理
--------------------------------------------------------------*/
static void scroll_event_cb(lv_event_t * e)
{
    lv_obj_t * cont = lv_event_get_target(e);
    lv_coord_t center_x = cont->coords.x1 + lv_obj_get_width(cont) / 2;
    uint32_t cnt = lv_obj_get_child_cnt(cont);

    for(uint32_t i = 0; i < cnt; i++) {
        lv_obj_t * child = lv_obj_get_child(cont, i);
        lv_coord_t child_center_x = child->coords.x1 + lv_obj_get_width(child) / 2;
        lv_coord_t diff = abs(center_x - child_center_x);

        // 计算缩放比例，距离中心越近越大
        if(diff > 250) diff = 250;
        lv_coord_t new_h = CARD_MAX_H - (diff * (CARD_MAX_H - CARD_BASE_H) / 250);
        
        lv_obj_set_height(child, new_h);
        lv_obj_set_y(child, (400 - new_h) / 2);
        //lv_obj_set_style_opa(child, 255 - (diff * 150 / 250), 0); // 侧边卡片半透明
    }
}

static void card_event_cb(lv_event_t * e)
{
    int index = (int)(uintptr_t)lv_event_get_user_data(e);
    static uint32_t last_click = 0;
    
    // 双击判定进入 APP
    if((lv_tick_get() - last_click) < 300) {
        ui_screen_load(app_screen_list[index], &UI_SCREEN_LOAD_AMIM);
    }
    last_click = lv_tick_get();
}

static void gesture_event_cb(lv_event_t * e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if(dir == LV_DIR_TOP) ui_screen_back();
}

#else

	
ui_screen_t app_select_screen = 
{
	.init = NULL,
	.deinit = NULL,
	.obj = NULL,
};

#endif
