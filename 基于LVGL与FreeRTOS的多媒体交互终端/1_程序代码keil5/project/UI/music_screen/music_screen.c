#include "music_screen.h"

#if USE_UI_MUSIC_SCREEN



static lv_obj_t *music_screen_obj;
ui_screen_t music_screen = 
{
    .init = music_screen_init,
    .deinit = music_screen_deinit,
    .obj = &music_screen_obj
};

extern const lv_img_dsc_t _lvgl_music_cd_img;

// --- 属性配置 ---
static char* music_mode_text[3] = {LV_SYMBOL_LOOP, LV_SYMBOL_SHUFFLE, LV_SYMBOL_REFRESH};
static char* music_status_text[2] = {LV_SYMBOL_PLAY, LV_SYMBOL_PAUSE}; 
static const lv_style_prop_t trans_props[] = {LV_STYLE_BG_OPA, LV_STYLE_TRANSFORM_ZOOM, LV_STYLE_BG_COLOR, LV_STYLE_PROP_INV};
static lv_style_transition_dsc_t btn_trans;

// UI 句柄
lv_obj_t *music_screen_obj;
static lv_obj_t *cd_img_obj;
static lv_obj_t *mode_label; 
static lv_obj_t *status_label;
static lv_obj_t *progress_obj;

/* 静态变量仅记录 UI 上一次显示的索引 */
static uint16_t last_ui_idx = 0; 
static music_status_t g_music_status = MUSIC_STATUS_STOPPED;
static music_mode_t g_music_mode = MUSIC_MODE_LIST_LOOP;

// 函数声明
static lv_obj_t* create_icon_btn(lv_obj_t* parent, const char* symbol, lv_coord_t x, lv_coord_t y, lv_event_cb_t event_cb);
static void set_cd_rotation(bool en);
static void music_screen_updata_gesture_event(lv_event_t * e);
static void music_screen_mode_obj_clicked_event(lv_event_t * e);
static void music_screen_select_obj_clicked_event(lv_event_t * e);
static void music_screen_next_obj_clicked_event(lv_event_t * e);
static void music_screen_prev_obj_clicked_event(lv_event_t * e);
static void music_screen_status_obj_clicked_event(lv_event_t * e);
static void music_screen_progress_obj_clicked_event(lv_event_t * e);
static void music_playlist_dropdown_event_cb(lv_event_t * e);
static void music_status_update_cb(sensor_id_t id, const uint8_t* data);


/*-------------------------------------------------------------
                        页面初始化/销毁
--------------------------------------------------------------*/
void music_screen_init(lv_obj_t *parent) 
{
    lv_style_transition_dsc_init(&btn_trans, trans_props, lv_anim_path_linear, 100, 0, NULL);

    music_screen_obj = lv_obj_create(parent);
    lv_obj_set_size(music_screen_obj, lv_pct(100), lv_pct(100));
    lv_obj_clear_flag(music_screen_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(music_screen_obj, lv_color_hex(ui_screen_default_bg_color()), 0);

    cd_img_obj = lv_img_create(music_screen_obj);
    lv_obj_align(cd_img_obj, LV_ALIGN_TOP_MID, 0, 60);
    lv_img_set_src(cd_img_obj, &_lvgl_music_cd_img);
    lv_img_set_pivot(cd_img_obj, 100, 100); 

	/* 定义一个下拉列表用于选择播放过滤模式 */
	lv_obj_t * playlist_dropdown = lv_dropdown_create(music_screen_obj);
	lv_obj_set_size(playlist_dropdown, 120, 40);
	lv_obj_align(playlist_dropdown, LV_ALIGN_TOP_RIGHT, -10, 10); // 右上角，偏移10像素
	/* 设置选项内容：对应你的 flag 模式 */
	lv_dropdown_set_options(playlist_dropdown, "全部播放\n播放列表1\n播放列表2\n播放列表3\n播放列表4");
	/* 设置样式使其融入 UI */
	lv_obj_set_style_text_font(playlist_dropdown, &_lvgl_font1, 0);
	lv_obj_set_style_bg_color(playlist_dropdown, lv_color_hex(0xF0F0F0), 0);
	lv_obj_set_style_border_width(playlist_dropdown, 1, 0);
	lv_obj_t * list = lv_dropdown_get_list(playlist_dropdown);
	lv_obj_set_style_text_font(list, &_lvgl_font1, 0);
	lv_dropdown_set_selected(playlist_dropdown, last_ui_idx);
	
	/* 添加事件回调 */
	lv_obj_add_event_cb(playlist_dropdown, music_playlist_dropdown_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    mode_label   = create_icon_btn(music_screen_obj, music_mode_text[0], 50, 310, music_screen_mode_obj_clicked_event);
	status_label = create_icon_btn(music_screen_obj, music_status_text[0], 217, 390, music_screen_status_obj_clicked_event);
    create_icon_btn(music_screen_obj, LV_SYMBOL_LIST, 385, 310, music_screen_select_obj_clicked_event);
    create_icon_btn(music_screen_obj, LV_SYMBOL_PREV, 50, 390, music_screen_prev_obj_clicked_event);
    create_icon_btn(music_screen_obj, LV_SYMBOL_NEXT, 385, 390, music_screen_next_obj_clicked_event);
    

    progress_obj = lv_slider_create(music_screen_obj);
    lv_obj_set_size(progress_obj, 380, 12); // 稍微加粗一点方便点击
    lv_obj_align(progress_obj, LV_ALIGN_TOP_MID, 0, 370);
    lv_obj_set_style_bg_color(progress_obj, lv_palette_main(LV_PALETTE_BLUE), LV_PART_INDICATOR);
    
    // 允许滑块通过点击改变数值
    lv_slider_set_range(progress_obj, 0, 100);
    lv_obj_add_event_cb(progress_obj, music_screen_progress_obj_clicked_event, LV_EVENT_ALL, NULL);

	// 1. 同步播放图标：播放中显示暂停符，停止中显示播放符
	bool is_playing = (g_music_status == MUSIC_STATUS_PLAYING);
	lv_label_set_text(status_label, is_playing ? music_status_text[1] : music_status_text[0]);
	set_cd_rotation(is_playing);

	// 2. 同步播放模式图标
	lv_label_set_text(mode_label, music_mode_text[MusicControl.mode]);

    lv_obj_add_event_cb(music_screen_obj, music_screen_updata_gesture_event, LV_EVENT_GESTURE, NULL);
	SensorHub.subscribe(SENSOR_MUSIC_STATUS, music_status_update_cb);
}

void music_screen_deinit(void) 
{
	SensorHub.unsubscribe(SENSOR_MUSIC_STATUS, music_status_update_cb);
    if (music_screen_obj) {
        lv_obj_del(music_screen_obj);
        music_screen_obj = NULL;
    }
}

/*-------------------------------------------------------------
                        UI辅助函数
--------------------------------------------------------------*/
static void music_status_update_cb(sensor_id_t id, const uint8_t* data)
{
	if (music_screen_obj == NULL) return;

    if (id == SENSOR_MUSIC_STATUS)
    {
        sensor_music_data_t* music = (sensor_music_data_t*)data;
        if (progress_obj)
		{
			//printf("--------%d\n", music->current_percent);
			if(music->current_percent <= 100)
			{
				if(!lv_obj_has_state(progress_obj, LV_STATE_PRESSED)) {
				lv_slider_set_value(progress_obj, music->current_percent, LV_ANIM_OFF);
				 MusicControl.current_percent = music->current_percent;}
			}
		}
		if(status_label)
		{
			bool is_playing = (music->status == 1) ? true : false;
			g_music_status = (music->status == 1) ? MUSIC_STATUS_PLAYING : MUSIC_STATUS_STOPPED;
			set_cd_rotation(is_playing);
			lv_label_set_text(status_label, is_playing ? music_status_text[1] : music_status_text[0]);
		}
	}
}




static lv_obj_t* create_icon_btn(lv_obj_t* parent, const char* symbol, 
				lv_coord_t x, lv_coord_t y, lv_event_cb_t event_cb) 
{
    lv_obj_t* btn_obj = lv_obj_create(parent);
    lv_obj_set_size(btn_obj, 60, 60); // 增大点击区域
    lv_obj_set_pos(btn_obj, x - 5, y - 5);
    
    lv_obj_set_style_bg_opa(btn_obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_radius(btn_obj, LV_RADIUS_CIRCLE, 0); 
    lv_obj_set_style_border_width(btn_obj, 0, 0);
    lv_obj_clear_flag(btn_obj, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(btn_obj, lv_palette_main(LV_PALETTE_GREY), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn_obj, LV_OPA_20, LV_STATE_PRESSED);
    lv_obj_set_style_transform_zoom(btn_obj, 230, LV_STATE_PRESSED); // 稍微放大反馈
    lv_obj_set_style_transition(btn_obj, &btn_trans, 0);

    lv_obj_t* label = lv_label_create(btn_obj);
    lv_label_set_text(label, symbol);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0); // 增大图标字体
    lv_obj_set_style_text_color(label, lv_color_hex(0x2C3E50), 0); 
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    
    lv_obj_add_event_cb(btn_obj, event_cb, LV_EVENT_CLICKED, NULL);
    return label;
}

static void set_cd_rotation(bool en) 
{
    lv_anim_t * old_anim = lv_anim_get(cd_img_obj, (lv_anim_exec_xcb_t)lv_img_set_angle);
    if(en) {
        if(old_anim) return;
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, cd_img_obj);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_img_set_angle);
        lv_anim_set_values(&a, 0, 3600); 
        lv_anim_set_time(&a, 3000);
        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
        lv_anim_start(&a);
    } else {
        if(old_anim) lv_anim_del(cd_img_obj, (lv_anim_exec_xcb_t)lv_img_set_angle);
    }
}



/*-------------------------------------------------------------
                        事件回调处理
--------------------------------------------------------------*/
extern ui_screen_t music_select_screen;
static void music_screen_select_obj_clicked_event(lv_event_t * e) 
{
	SensorHub.unsubscribe(SENSOR_MUSIC_STATUS, music_status_update_cb);
    ui_screen_load(&music_select_screen, &UI_SCREEN_LOAD_AMIM);
}

static void music_screen_updata_gesture_event(lv_event_t * e) 
{
    if(!lv_obj_has_flag(music_screen_obj, LV_OBJ_FLAG_HIDDEN)) 
	{
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if(dir == LV_DIR_BOTTOM)
		{
			SensorHub.unsubscribe(SENSOR_MUSIC_STATUS, music_status_update_cb);
			ui_screen_load(&app_select_screen, &UI_SCREEN_LOAD_AMIM);
		}
        else if(dir == LV_DIR_TOP)
		{
			SensorHub.unsubscribe(SENSOR_MUSIC_STATUS, music_status_update_cb);
			ui_screen_load(&weather_screen, &UI_SCREEN_LOAD_AMIM);
		}
    }
}

static void music_playlist_dropdown_event_cb(lv_event_t * e)
{
    lv_obj_t * dropdown = lv_event_get_target(e);
    uint16_t selected = lv_dropdown_get_selected(dropdown);
    
    music_flag_t target_flag = MUSIC_FLAG_NONE;
    switch(selected) 
	{
        case 0: target_flag = MUSIC_FLAG_NONE; break;
        case 1: target_flag = MUSIC_FLAG_1;    break;
        case 2: target_flag = MUSIC_FLAG_2;    break;
        case 3: target_flag = MUSIC_FLAG_3;    break;
        case 4: target_flag = MUSIC_FLAG_4;    break;
    }

    /* 调用底层：底层现在失败会自动回滚 mlist 内部的状态 */
	music_info_t * curr = music_list_set_flag(&MusicControl.g_music_list, target_flag);
	//printf("MusicControl.g_music_list.flag = %d\n", MusicControl.g_music_list.flag);
    if(curr != NULL) 
	{
        // 切换成功，更新 UI 记录点
        last_ui_idx = selected;
		MusicControl.select(curr);
		
    } else 
	{
        // 切换失败，UI 强行弹回
        lv_dropdown_set_selected(dropdown, last_ui_idx);
        printf("Empty list! UI reverted.\n");
    }
}


// 如果是用户点击或拖动释放进度条事件
static void music_screen_progress_obj_clicked_event(lv_event_t * e) 
{
    lv_obj_t * slider = lv_event_get_target(e);
	lv_event_code_t code = lv_event_get_code(e);
	
	if(code == LV_EVENT_RELEASED || code ==LV_EVENT_PRESS_LOST) {
		int32_t value = lv_slider_get_value(slider);
		//printf("=====%d\n", value);
		MusicControl.set_seek((uint8_t)value);
	}
}
static void music_screen_mode_obj_clicked_event(lv_event_t * e) { MusicControl.cycle_mode();lv_label_set_text(mode_label, music_mode_text[MusicControl.mode]);}
static void music_screen_status_obj_clicked_event(lv_event_t * e) {(g_music_status == MUSIC_STATUS_PLAYING) ? MusicControl.stop() : MusicControl.play();}
static void music_screen_prev_obj_clicked_event(lv_event_t * e) { MusicControl.prev(); }
static void music_screen_next_obj_clicked_event(lv_event_t * e) { MusicControl.next(); }

#else
ui_screen_t music_screen = { .init = NULL, .deinit = NULL, .obj = NULL };
#endif


