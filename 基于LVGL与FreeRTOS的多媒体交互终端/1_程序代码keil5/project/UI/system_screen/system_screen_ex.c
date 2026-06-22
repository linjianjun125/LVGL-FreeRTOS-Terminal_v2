#include "system_screen_ex.h"

#if USE_UI_SYSTEM_SCREEN
/*-----------------------------------------------------------
						sound
-------------------------------------------------------------*/
static lv_obj_t *sound_progress_obj;

// Set 按钮的回调函数
static void system_sound_set_event_cb(lv_event_t * e)
{
    lv_obj_t * btn = lv_event_get_target(e);

    if(lv_event_get_code(e) == LV_EVENT_CLICKED) 
	{
        int value = lv_slider_get_value(sound_progress_obj);
		printf("%d\n", value);
		
		// 这里可以添加实际调节系统音量的代码
		VolumeControl.set_volume(value);
    }
}


static void ui_sound_status_update_cb(sensor_id_t id, const uint8_t* data)
{
    // 基础防御：如果主屏幕对象都不存在，说明 UI 还没准备好或已销毁
    if (sound_progress_obj == NULL) return;

    if (id == SENSOR_VOLUME) 
    {
		volume_config_t* config = (volume_config_t*)data;
		lv_slider_set_value(sound_progress_obj, config->current_vol, LV_ANIM_ON);
		printf("UI set %d\n", config->current_vol);
	}
}


lv_obj_t *system_screen_menu_sound_create(lv_obj_t *parent_obj)
{
    // 创建主卡片容器
    lv_obj_t *obj = lv_obj_create(parent_obj);
    lv_obj_remove_style_all(obj);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(obj, lv_pct(94), 140);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 15);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xF2F2F7), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(obj, 10, 0);

    // 标题文字 "Sound"
    lv_obj_t *label_title = lv_label_create(obj);
    lv_obj_align(label_title, LV_ALIGN_TOP_LEFT, 15, 12);
    lv_label_set_text(label_title, "Sound");
    lv_obj_set_style_text_color(label_title, lv_color_hex(0x1C1C1E), 0);
    lv_obj_set_style_text_font(label_title, &lv_font_montserrat_14, 0);

    // 音量图标
    lv_obj_t *icon = lv_label_create(obj);
    lv_obj_align(icon, LV_ALIGN_TOP_LEFT, 15, 45);
    lv_label_set_text(icon, LV_SYMBOL_VOLUME_MID);
    lv_obj_set_style_text_color(icon, lv_color_hex(0x3A3A3C), 0);

    // 音量滑块
    sound_progress_obj = lv_slider_create(obj);
    lv_obj_set_size(sound_progress_obj, lv_pct(70), 8);
    lv_obj_align(sound_progress_obj, LV_ALIGN_TOP_LEFT, 45, 50);
    lv_obj_set_style_bg_color(sound_progress_obj, lv_color_hex(0x007AFF), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(sound_progress_obj, lv_color_hex(0xFFFFFF), LV_PART_KNOB);

    // 创建 "Set" 按钮
    lv_obj_t *set_btn = lv_btn_create(obj);
    lv_obj_set_size(set_btn, 70, 30);
    lv_obj_align(set_btn, LV_ALIGN_BOTTOM_RIGHT, -15, -12);
    lv_obj_set_style_radius(set_btn, 8, 0);
    lv_obj_set_style_bg_color(set_btn, lv_color_hex(0x007AFF), 0);
    lv_obj_t *btn_label = lv_label_create(set_btn);
    lv_label_set_text(btn_label, "Set");
    lv_obj_center(btn_label);
    lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(btn_label, lv_color_hex(0xFFFFFF), 0);

    // 添加事件回调 (将滑块对象作为用户数据传入)
    lv_obj_add_event_cb(set_btn, system_sound_set_event_cb, LV_EVENT_CLICKED, NULL);
	
	// 数据交互
	SensorHub.subscribe(SENSOR_VOLUME, ui_sound_status_update_cb);

    return obj;
}

void system_screen_menu_sound_del(lv_obj_t *menu_obj)
{
	SensorHub.unsubscribe(SENSOR_VOLUME, ui_sound_status_update_cb);
	
	if(menu_obj == NULL) return;
	 lv_obj_del(menu_obj);
}







/*-----------------------------------------------------------
						  brighting
-------------------------------------------------------------*/
static lv_obj_t *backlight_time_roller = NULL;		// 自动息屏时间
static lv_obj_t *backlight_auto_adjust_sw = NULL;	// 自动调节亮度开关
static lv_obj_t *manual_cont = NULL;				// 手动调节亮度菜单
static lv_obj_t *brightness_slider = NULL;			// 亮度设置滑块

static void ui_backlight_status_update_cb(sensor_id_t id, const uint8_t* data)
{
	if(id == SENSOR_BACKLIGHT)
	{
		backlight_config_t* cfg = (backlight_config_t*)data;
		
		// 初始化自动息屏时间
		uint16_t current_val = cfg->timeout_100ms;
		uint8_t target_index = 1; 
		if (current_val == BL_TIMEOUT_15S)        target_index = 0;
		else if (current_val == BL_TIMEOUT_30S)   target_index = 1;
		else if (current_val == BL_TIMEOUT_1M)    target_index = 2;
		else if (current_val == BL_TIMEOUT_2M)    target_index = 3;
		else if (current_val == BL_TIMEOUT_NEVER) target_index = 4;
		lv_roller_set_selected(backlight_time_roller, target_index, LV_ANIM_ON);
	
		// 亮度调节按钮/亮度调节滑块显示
		if (cfg->auto_adjust) 
		{
			lv_obj_clear_state(backlight_auto_adjust_sw, LV_STATE_CHECKED); // 关闭状态
			lv_obj_add_flag(manual_cont, LV_OBJ_FLAG_HIDDEN);
		} else 
		{
			lv_obj_add_state(backlight_auto_adjust_sw, LV_STATE_CHECKED); // 开启状态
			lv_obj_clear_flag(manual_cont, LV_OBJ_FLAG_HIDDEN);
		}
		
		// 亮度调节滑块
		// 设置范围
		lv_slider_set_range(brightness_slider, cfg->min_level, cfg->max_level);
		lv_slider_set_value(brightness_slider, cfg->target_level, LV_ANIM_ON);
		//printf("--lider: %d\n", cfg->target_level);
	}
}


static void system_brightness_set_event_cb(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
		
		// 自动息屏时间
		int index = lv_roller_get_selected(backlight_time_roller);
		int timeout = 0;
		switch (index) {
			case 0:  timeout = BL_TIMEOUT_15S;   break;
			case 1:  timeout = BL_TIMEOUT_30S;   break;
			case 2:  timeout = BL_TIMEOUT_1M;    break;
			case 3:  timeout = BL_TIMEOUT_2M;    break;
			case 4:  timeout = BL_TIMEOUT_NEVER; break;
			default: timeout = BL_TIMEOUT_30S;   break;
		}
		BacklightControl.set_timeout(timeout);
		
		// 自动亮度调节选择
		if(lv_obj_has_state(backlight_auto_adjust_sw, LV_STATE_CHECKED))
		{
			BacklightControl.set_auto_adjust_status(false);
		} else
		{
			BacklightControl.set_auto_adjust_status(true);
		}
		
		// 手动调节滑动条
		int slider_val = lv_slider_get_value(brightness_slider);
		BacklightControl.set_target(slider_val);
    }
}


static void brightness_mode_switch_event_cb(lv_event_t * e)
{
    if(lv_obj_has_state(backlight_auto_adjust_sw, LV_STATE_CHECKED))
    {
		lv_obj_clear_flag(manual_cont, LV_OBJ_FLAG_HIDDEN);
    } else
    {
		 lv_obj_add_flag(manual_cont, LV_OBJ_FLAG_HIDDEN);
    }
}


lv_obj_t *system_screen_menu_backlight_create(lv_obj_t *parent_obj)
{
    // 主卡片容器
    lv_obj_t *obj = lv_obj_create(parent_obj);
    lv_obj_remove_style_all(obj);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(obj, lv_pct(94), 310);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 15);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xF2F2F7), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(obj, 12, 0);

    // --- 第一部分：Screen Timeout ---
    lv_obj_t *label_st = lv_label_create(obj);
    lv_obj_align(label_st, LV_ALIGN_TOP_LEFT, 15, 20);
    lv_label_set_text(label_st, "Screen Timeout");
    lv_obj_set_style_text_font(label_st, &lv_font_montserrat_14, 0);

    backlight_time_roller = lv_roller_create(obj);
    lv_obj_set_size(backlight_time_roller, 60, 65);
    lv_obj_align(backlight_time_roller, LV_ALIGN_TOP_RIGHT, -15, 15);
    lv_roller_set_options(backlight_time_roller, "15s\n30s\n1min\n2min\nNever", LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(backlight_time_roller, 2);
    lv_obj_set_style_bg_color(backlight_time_roller, lv_color_hex(0x007AFF), LV_PART_SELECTED);
    lv_obj_set_style_text_color(backlight_time_roller, lv_color_hex(0xFFFFFF), LV_PART_SELECTED);
	lv_obj_set_style_text_font(backlight_time_roller, &lv_font_montserrat_14, 0);
    lv_obj_set_style_border_width(backlight_time_roller, 0, 0);

    lv_obj_t *line = lv_obj_create(obj);
    lv_obj_set_size(line, lv_pct(92), 1);
    lv_obj_align(line, LV_ALIGN_TOP_MID, 0, 150);
    lv_obj_set_style_bg_color(line, lv_color_hex(0xD1D1D6), 0);
    lv_obj_set_style_border_width(line, 0, 0);

    // --- 第二部分：Manual Brightness 开关 ---
    lv_obj_t *label_br = lv_label_create(obj);
    lv_obj_align(label_br, LV_ALIGN_TOP_LEFT, 15, 175);
    lv_label_set_text(label_br, "Manual Brightness");
    lv_obj_set_style_text_font(label_br, &lv_font_montserrat_14, 0);
	
    backlight_auto_adjust_sw = lv_switch_create(obj);
    lv_obj_set_size(backlight_auto_adjust_sw, 45, 24);
    lv_obj_align(backlight_auto_adjust_sw, LV_ALIGN_TOP_RIGHT, -15, 175);
    lv_obj_set_style_bg_color(backlight_auto_adjust_sw, lv_color_hex(0x34C759), LV_STATE_CHECKED | LV_PART_INDICATOR);
	

    // --- 第三部分：亮度调节容器 ---
    manual_cont = lv_obj_create(obj);
    lv_obj_remove_style_all(manual_cont);
    lv_obj_set_size(manual_cont, lv_pct(100), 100);
    lv_obj_align(manual_cont, LV_ALIGN_TOP_MID, 0, 210);

    // 亮度图标
    lv_obj_t *icon_br = lv_label_create(manual_cont);
    lv_obj_align(icon_br, LV_ALIGN_TOP_LEFT, 15, 10);
    lv_label_set_text(icon_br, LV_SYMBOL_SETTINGS); // 这里可以换成太阳图标的 Symbol
    lv_obj_set_style_text_color(icon_br, lv_color_hex(0x3A3A3C), 0);

    // 亮度滑块
    brightness_slider = lv_slider_create(manual_cont);
    lv_obj_set_size(brightness_slider, lv_pct(70), 8); 
    lv_obj_align(brightness_slider, LV_ALIGN_TOP_LEFT, 55, 15);
    lv_obj_set_style_bg_color(brightness_slider, lv_color_hex(0x007AFF), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(brightness_slider, lv_color_hex(0xFFFFFF), LV_PART_KNOB);
	
	
	// --- 第四部分：设置开关 ---
    lv_obj_t *set_btn = lv_btn_create(obj);
    lv_obj_set_size(set_btn, 70, 32);
    lv_obj_align(set_btn, LV_ALIGN_BOTTOM_RIGHT, -15, -15);
    lv_obj_set_style_bg_color(set_btn, lv_color_hex(0x007AFF), 0);
    lv_obj_set_style_radius(set_btn, 8, 0);
    lv_obj_t *btn_label = lv_label_create(set_btn);
    lv_label_set_text(btn_label, "set");
    lv_obj_center(btn_label);

    // --- 事件绑定 ---
    lv_obj_add_event_cb(set_btn, system_brightness_set_event_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_add_event_cb(backlight_auto_adjust_sw, brightness_mode_switch_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	
	// 数据交互
	SensorHub.subscribe(SENSOR_BACKLIGHT, ui_backlight_status_update_cb);
	
    return obj;
}



void system_screen_menu_backbright_del(lv_obj_t *menu_obj)
{
	// 数据交互
	SensorHub.unsubscribe(SENSOR_BACKLIGHT, ui_backlight_status_update_cb);
	
	if(menu_obj == NULL) return;
	 lv_obj_del(menu_obj);

}






/*-----------------------------------------------------------
						  about
-------------------------------------------------------------*/
lv_obj_t *system_screen_menu_about_create(lv_obj_t *parent_obj)
{
    // 1. 主卡片容器 (保持不变)
    lv_obj_t *obj = lv_obj_create(parent_obj);
    lv_obj_remove_style_all(obj);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(obj, lv_pct(94), 260); // 稍微调高高度以适应更多信息
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 15);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xF2F2F7), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(obj, 12, 0);

    // 2. 标题 (保持不变)
    lv_obj_t *label_title = lv_label_create(obj);
    lv_obj_align(label_title, LV_ALIGN_TOP_LEFT, 15, 15);
    lv_label_set_text(label_title, "System Information");
    lv_obj_set_style_text_font(label_title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(label_title, lv_color_hex(0x1C1C1E), 0);

    // 3. 分割线 (保持不变)
    lv_obj_t *line = lv_obj_create(obj);
    lv_obj_set_size(line, lv_pct(92), 1);
    lv_obj_align(line, LV_ALIGN_TOP_MID, 0, 45);
    lv_obj_set_style_bg_color(line, lv_color_hex(0xD1D1D6), 0);
    lv_obj_set_style_border_width(line, 0, 0);

    // 4. 信息展示区域容器
    lv_obj_t *info_cont = lv_obj_create(obj);
    lv_obj_remove_style_all(info_cont);
    lv_obj_set_size(info_cont, lv_pct(100), 180);
    lv_obj_align(info_cont, LV_ALIGN_TOP_MID, 0, 55);
    lv_obj_set_flex_flow(info_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(info_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_left(info_cont, 15, 0);
    lv_obj_set_style_pad_right(info_cont, 15, 0);
    lv_obj_set_style_pad_row(info_cont, 8, 0);

    // --- 核心修改部分：数据绑定 ---
    const char *keys[] = {"Device:", "Version:", "MCU:", "Build:", "Memory:"};
    
    // 将全局结构体中的值映射到临时数组中
    const char *vals[5];
    vals[0] = g_sys_fw_info.device_name; // 来自全局变量
    vals[1] = g_sys_fw_info.version;     // 来自全局变量
    vals[2] = g_sys_fw_info.mcu_type;    // 来自全局变量
    vals[3] = g_sys_fw_info.build_time;  // 来自全局变量（之前已转为字符串）
    vals[4] = "2.0MB Flash";             // 仍可保留硬编码或从其它变量获取

    for(int i = 0; i < 5; i++)
    {
        lv_obj_t *item_cont = lv_obj_create(info_cont);
        lv_obj_remove_style_all(item_cont);
        lv_obj_set_size(item_cont, lv_pct(100), 25);

        // 属性名
        lv_obj_t *key_lab = lv_label_create(item_cont);
        lv_label_set_text(key_lab, keys[i]);
        lv_obj_align(key_lab, LV_ALIGN_LEFT_MID, 0, 0);
        lv_obj_set_style_text_color(key_lab, lv_color_hex(0x8E8E93), 0);
        lv_obj_set_style_text_font(key_lab, &lv_font_montserrat_14, 0);

        // 属性值
        lv_obj_t *val_lab = lv_label_create(item_cont);
        // 如果值为空字符串，显示 "N/A"
        lv_label_set_text(val_lab, (vals[i][0] == '\0') ? "Unknown" : vals[i]);
        lv_obj_align(val_lab, LV_ALIGN_RIGHT_MID, 0, 0);
        lv_obj_set_style_text_color(val_lab, lv_color_hex(0x1C1C1E), 0);
        lv_obj_set_style_text_font(val_lab, &lv_font_montserrat_14, 0);
    }

    // 5. 底部版权文字 (保持不变)
    lv_obj_t *label_cp = lv_label_create(obj);
    lv_obj_align(label_cp, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_label_set_text(label_cp, "Copyright 2026. All rights reserved.");
    lv_obj_set_style_text_font(label_cp, &lv_font_montserrat_14, 0); // 确保字体存在
    lv_obj_set_style_text_color(label_cp, lv_color_hex(0xAEAEB2), 0);

    return obj;
}


void system_screen_menu_about_del(lv_obj_t *menu_obj)
{
	if(menu_obj == NULL) return;
	 lv_obj_del(menu_obj);

}



/*-----------------------------------------------------------
						外部调用
-------------------------------------------------------------*/
system_screen_menu_create_fn_t system_screen_menu_create_fn[] =
{
    [SYSTEM_MENU_SOUND]      = system_screen_menu_sound_create,
    [SYSTEM_MENU_TIME]      = system_screen_menu_sound_create,
    [SYSTEM_MENU_BRIGHTNESS] = system_screen_menu_backlight_create,
    [SYSTEM_MENU_ABOUT]     = system_screen_menu_about_create,
    [SYSTEM_MENU_ABOUT_]     = system_screen_menu_about_create,
};



system_screen_menu_del_fn_t system_screen_menu_del_fn[] =
{
    [SYSTEM_MENU_SOUND]      = system_screen_menu_sound_del,
    [SYSTEM_MENU_TIME]      = system_screen_menu_sound_del,
    [SYSTEM_MENU_BRIGHTNESS] = system_screen_menu_backbright_del,
    [SYSTEM_MENU_ABOUT]     = system_screen_menu_about_del,
    [SYSTEM_MENU_ABOUT_]     = system_screen_menu_about_del,
};

// 调用时建议增加安全检查
lv_obj_t * system_menu_switch_create(lv_obj_t *menu_obj, system_menu_index_t index)
{
    lv_obj_t *system_current_menu_obj;
    if (index < SYSTEM_MENU_MAX && system_screen_menu_create_fn[index] != NULL)
    {
        system_current_menu_obj = system_screen_menu_create_fn[index](menu_obj);
        return system_current_menu_obj;
    }
    return NULL;
}


// 调用时建议增加安全检查
void system_menu_switch_del(lv_obj_t *menu_obj, system_menu_index_t index)
{
    if (index < SYSTEM_MENU_MAX && system_screen_menu_del_fn[index] != NULL)
    {
        system_screen_menu_del_fn[index](menu_obj);
    }
}








#endif
