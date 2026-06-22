#include "weather_screen.h"

#if USE_UI_WEATHER_SCREEN





lv_obj_t *weather_screen_obj = NULL;
ui_screen_t weather_screen =
{
    .init = weather_screen_init,
    .deinit = weather_screen_deinit,
    .obj = &weather_screen_obj
};

LV_FONT_DECLARE(_lvgL_big_number_font)

static lv_obj_t * weather_time_label = NULL;
static lv_obj_t * weather_date_label = NULL;
static lv_obj_t * weather_temp_label = NULL;
static lv_obj_t * weather_temp_bar   = NULL;
static lv_obj_t * weather_humi_label = NULL;
static lv_obj_t * weather_humi_bar   = NULL;
static lv_obj_t * weather_icon       = NULL;
static lv_obj_t * weather_temp       = NULL;

static void weather_status_update_cb(sensor_id_t id, const uint8_t* data);
static void weather_screen_gesture_event_cb(lv_event_t * e);
static void weather_menu_item_event_cb(lv_event_t * e);

/*-------------------------------------------------------------
                        页面初始化/销毁函数
--------------------------------------------------------------*/

void weather_screen_init(lv_obj_t *parent)
{
	weather_time_label = NULL;
    weather_date_label = NULL;
    weather_temp_label = NULL;
    weather_humi_label = NULL;
    // 主背景创建
    weather_screen_obj = lv_obj_create(parent);
    lv_obj_set_size(weather_screen_obj, lv_pct(100), lv_pct(100));
    lv_obj_clear_flag(weather_screen_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(weather_screen_obj, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_width(weather_screen_obj, 0, 0);


    // --- 顶部左侧：时间 ---
    lv_obj_t *time_cont = lv_obj_create(weather_screen_obj);
    lv_obj_remove_style_all(time_cont);
    lv_obj_set_size(time_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(time_cont, LV_ALIGN_TOP_LEFT, 30, 30);

    weather_time_label = lv_label_create(time_cont);
    lv_obj_set_style_text_font(weather_time_label, &_lvgl_font4, 0);
    lv_obj_set_style_text_color(weather_time_label, lv_color_hex(0x2C3E50), 0);

    weather_date_label = lv_label_create(time_cont);
    lv_obj_align_to(weather_date_label, weather_time_label, LV_ALIGN_OUT_BOTTOM_LEFT, 5, 5);
    lv_obj_set_style_text_font(weather_date_label, &_lvgl_font2, 0);
    lv_obj_set_style_text_color(weather_date_label, lv_color_hex(0x95A5A6), 0);

    // --- 顶部右侧：天气---
    weather_icon = lv_label_create(weather_screen_obj);
    lv_label_set_text(weather_icon, " ");
	lv_obj_set_style_text_font(weather_icon, &_lvgl_font4, 0);
	lv_obj_align(weather_icon, LV_ALIGN_TOP_RIGHT, -45, 45);
    lv_obj_set_style_text_font(weather_icon, &_lvgl_font4, 0);
    lv_obj_set_style_text_color(weather_icon, lv_color_hex(0xF1C40F), 0);

    weather_temp = lv_label_create(weather_screen_obj);
    lv_label_set_text(weather_temp, " ");
    lv_obj_set_style_text_font(weather_temp, &_lvgl_font1, 0);
    lv_obj_set_style_text_color(weather_temp, lv_color_hex(0x2C3E50), 0);
    lv_obj_align(weather_temp, LV_ALIGN_TOP_RIGHT, -45, 90);

    // --- 中间：温湿度 ---
    lv_obj_t *env_cont = lv_obj_create(weather_screen_obj);
    lv_obj_remove_style_all(env_cont);
    lv_obj_set_size(env_cont, 240, 120);
    lv_obj_align(env_cont, LV_ALIGN_LEFT_MID, 35, -10);
    weather_temp_label = lv_label_create(env_cont);
    lv_obj_set_style_text_font(weather_temp_label, &_lvgl_font2, 0);

    weather_temp_bar = lv_bar_create(env_cont);
    lv_obj_set_size(weather_temp_bar, 160, 8);
    lv_obj_align_to(weather_temp_bar, weather_temp_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 6);
    lv_bar_set_range(weather_temp_bar, 0, 50);
    lv_obj_set_style_bg_color(weather_temp_bar, lv_color_hex(0xE67E22), LV_PART_INDICATOR);

    weather_humi_label = lv_label_create(env_cont);
    lv_obj_set_style_text_font(weather_humi_label, &_lvgl_font2, 0);
    lv_obj_align_to(weather_humi_label, weather_temp_bar, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);

    weather_humi_bar = lv_bar_create(env_cont);
    lv_obj_set_size(weather_humi_bar, 160, 8);
    lv_obj_align_to(weather_humi_bar, weather_humi_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 6);
    lv_bar_set_range(weather_humi_bar, 0, 100);
    lv_obj_set_style_bg_color(weather_humi_bar, lv_color_hex(0x3498DB), LV_PART_INDICATOR);

    // --- 底部菜单 ---
    lv_obj_t *weather_menu = lv_obj_create(weather_screen_obj);
    lv_obj_remove_style_all(weather_menu);
    lv_obj_set_size(weather_menu, lv_pct(100), 120);
    lv_obj_align(weather_menu, LV_ALIGN_BOTTOM_MID, 0, -10);

    for(int i = 0; i < 2; i++)
    {
        lv_obj_t *menu_item = lv_obj_create(weather_menu);
        lv_obj_set_user_data(menu_item, (void*)(uintptr_t)i);
        lv_obj_set_size(menu_item, 85, 85);
        lv_obj_align(menu_item, LV_ALIGN_TOP_LEFT, 25 + (113 * i), 10);
        lv_obj_set_style_radius(menu_item, 15, 0);
        lv_obj_set_style_bg_color(menu_item, lv_color_hex(0xF7F9F9), 0);
        lv_obj_set_style_bg_color(menu_item, lv_color_hex(0x3498DB), LV_STATE_CHECKED);

        lv_obj_add_flag(menu_item, LV_OBJ_FLAG_CHECKABLE);
		ledControl.status[i] ? lv_obj_add_state(menu_item, LV_STATE_CHECKED) 
                  : lv_obj_clear_state(menu_item, LV_STATE_CHECKED);
		
		lv_obj_clear_flag(menu_item, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_event_cb(menu_item, weather_menu_item_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

        lv_obj_t *icon_label = lv_label_create(menu_item);
        lv_label_set_text(icon_label, "LED");
        lv_obj_align(icon_label, LV_ALIGN_CENTER, 0, 0);
    }

    // 添加手势事件
    lv_obj_add_event_cb(weather_screen_obj, weather_screen_gesture_event_cb, LV_EVENT_GESTURE, NULL);

    // --- 订阅 SensorHub 消息 ---
	SensorHub.subscribe(SENSOR_RTC, weather_status_update_cb);
	SensorHub.subscribe(SENSOR_ENV, weather_status_update_cb);
	SensorHub.subscribe(SENSOR_WEATHER_DATA, weather_status_update_cb);
}

void weather_screen_deinit(void)
{
    // 1. 退订消息
    SensorHub.unsubscribe(SENSOR_RTC, weather_status_update_cb);
    SensorHub.unsubscribe(SENSOR_ENV, weather_status_update_cb);
	SensorHub.unsubscribe(SENSOR_WEATHER_DATA, weather_status_update_cb);

    // 2. 销毁 UI
    if (weather_screen_obj != NULL)
    {
        lv_obj_del(weather_screen_obj);
        weather_screen_obj = NULL;
    }

    // 3. 重置所有 Label 句柄，防止回调误触
    weather_time_label = NULL;
    weather_date_label = NULL;
    weather_temp_label = NULL;
    weather_temp_bar   = NULL;
    weather_humi_label = NULL;
    weather_humi_bar   = NULL;
}

/*-------------------------------------------------------------
                        事件回调与消息解析
--------------------------------------------------------------*/
static void weather_status_update_cb(sensor_id_t id, const uint8_t* data)
{
    // 核心安全检查：如果页面已销毁，直接返回
    if (weather_screen_obj == NULL) return;

    if (id == SENSOR_RTC)
    {
        sensor_rtc_data_t* rtc = (sensor_rtc_data_t*)data;
        if (weather_time_label)
		{
            lv_label_set_text_fmt(weather_time_label, "%02d:%02d", rtc->hour, rtc->minute);
        }
        if (weather_date_label) {
            // 这里可以根据实际需求构建日期字符串，或者使用原来的获取函数
            lv_label_set_text_fmt(weather_date_label, "20%02d-%02d-%02d", rtc->year, rtc->month, rtc->day);
        }
    }
    else if (id == SENSOR_ENV)
    {
        sensor_env_data_t* env = (sensor_env_data_t*)data;
        
        // 更新温度
        if (weather_temp_label) 
		{
            lv_label_set_text_fmt(weather_temp_label, "温度: %d度", (int)env->temperature);
        }
        if (weather_temp_bar) 
		{
            lv_bar_set_value(weather_temp_bar, (int)env->temperature, LV_ANIM_ON);
        }

        // 更新湿度
        if (weather_humi_label) {
            lv_label_set_text_fmt(weather_humi_label, "湿度: %d%%", (int)env->humidity);
        }
        if (weather_humi_bar) {
            lv_bar_set_value(weather_humi_bar, (int)env->humidity, LV_ANIM_ON);
        }
    }
	
	else if (id == SENSOR_WEATHER_DATA)
    {
		sensor_weather_data_t  *weather = (sensor_weather_data_t*)data;
        if (weather_icon) 
		{
             lv_label_set_text(weather_icon, weather->info);
        }
		
		if(weather_temp && weather->temp != -200)
		{
			 lv_label_set_text_fmt(weather_temp, "%d 度", weather->temp);
		}
	}
}

static void weather_screen_gesture_event_cb(lv_event_t * e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if(dir == LV_DIR_BOTTOM)
    {
        ui_screen_back();
    }
}

static void weather_menu_item_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
	
    if(code == LV_EVENT_VALUE_CHANGED)
    {
        bool is_checked = lv_obj_has_state(obj, LV_STATE_CHECKED);
        uintptr_t id = (uintptr_t)lv_obj_get_user_data(obj);
        //printf("App %d: %d\n", (int)id, is_checked);
		
		if(is_checked) ledControl.open(id);
		else ledControl.close(id);
		
		
    }
}

#else
ui_screen_t weather_screen = { .init = NULL, .deinit = NULL, .obj = NULL };
#endif




