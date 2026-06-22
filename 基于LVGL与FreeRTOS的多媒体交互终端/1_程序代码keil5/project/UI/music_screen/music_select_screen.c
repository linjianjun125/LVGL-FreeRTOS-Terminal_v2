#include "music_select_screen.h"

#if USE_UI_MUSIC_SCREEN

static lv_obj_t *music_select_screen_obj;
ui_screen_t music_select_screen = 
{
    .init = music_select_screen_init,
    .deinit = music_select_screen_deinit,
    .obj = &music_select_screen_obj
};

/* 静态变量定义 */
static lv_obj_t * music_list_view = NULL;      // List 控件
static lv_obj_t * label_back;
music_info_t * g_current_music_info = NULL;


/* 私有函数声明 */
static void music_list_item_event_cb(lv_event_t * e);
static void music_list_break_clicked_event(lv_event_t * e);
void music_list_refresh_view(void);
void create_music_manage_panel(music_info_t * info);
static void music_select_status_update_cb(sensor_id_t id, const uint8_t* data);
	

/**
 * music_list_create - 创建歌单列表界面
 */
void music_select_screen_init(lv_obj_t* parent)
{
    /* 1. 创建全屏容器 */
    music_select_screen_obj = lv_obj_create(parent);
    lv_obj_remove_style_all(music_select_screen_obj);
    lv_obj_set_size(music_select_screen_obj, 480, 480);

    /* 2. 顶部返回栏 (Header) */
    lv_obj_t * music_list_break = lv_obj_create(music_select_screen_obj);
    lv_obj_set_size(music_list_break, lv_pct(100), lv_pct(10));
    lv_obj_align(music_list_break, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(music_list_break, lv_color_hex(0x2C3E50), 0);
    lv_obj_set_style_bg_opa(music_list_break, LV_OPA_100, 0);
    lv_obj_set_style_radius(music_list_break, 0, 0);
    lv_obj_add_flag(music_list_break, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_clear_flag(music_list_break, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_border_width(music_list_break, 0, 0);
	lv_obj_set_style_pad_all(music_list_break, 0, 0);

    label_back = lv_label_create(music_list_break);
    lv_obj_align(label_back, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(label_back, &_lvgl_font2, 0);
    lv_obj_set_style_text_color(label_back, lv_color_white(), 0);
	lv_obj_set_scrollbar_mode(label_back, LV_SCROLLBAR_MODE_OFF);
    const char * title = "全部音乐";
    music_flag_t f = MusicControl.g_music_list.flag;
    if (f == MUSIC_FLAG_1)      title = "播放列表1";
    else if (f == MUSIC_FLAG_2) title = "播放列表2";
    else if (f == MUSIC_FLAG_3) title = "播放列表3";
    else if (f == MUSIC_FLAG_4) title = "播放列表4";
    else if (f == MUSIC_FLAG_NONE) title = "全部音乐";
    lv_label_set_text_fmt(label_back, "%s", title);
	
    lv_obj_add_event_cb(music_list_break, music_list_break_clicked_event, LV_EVENT_CLICKED, NULL);

    /* 3. 创建标准的 LV_LIST 控件 */
    music_list_view = lv_list_create(music_select_screen_obj);
    lv_obj_set_size(music_list_view, lv_pct(100), lv_pct(90));
    lv_obj_align(music_list_view, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(music_list_view, lv_color_hex(0xF8F9FA), 0);
    lv_obj_set_style_text_font(music_list_view, &_lvgl_font2, 0); 

    /* 4. 调用刷新函数填充初始内容 */
	music_list_refresh_view();
	
	SensorHub.subscribe(SENSOR_MUSIC_INFO, music_select_status_update_cb);
}


void music_select_screen_deinit(void) 
{
	SensorHub.unsubscribe(SENSOR_MUSIC_INFO, music_select_status_update_cb);
    if (music_select_screen_obj) {
        lv_obj_del(music_select_screen_obj);
        music_select_screen_obj = NULL;
    }
}

/*-------------------------------------------------------------
                        UI辅助函数
--------------------------------------------------------------*/
static void music_select_status_update_cb(sensor_id_t id, const uint8_t* data)
{
	if (music_select_screen_obj == NULL) return;

    if (id == SENSOR_MUSIC_INFO)
    {
        sensor_music_data_t* music = (sensor_music_data_t*)data;
		if (music == NULL || music->music_info == NULL) return;
		
        if (music->music_info != NULL)
		{
			g_current_music_info = music->music_info;
			
			if (music_list_view && lv_obj_is_valid(music_list_view))
			{
                music_list_refresh_view();
            }
		}
	}
}


/**
 * 刷新列表内容（清空旧按钮并重新根据 flag 加载）
 */
void music_list_refresh_view(void)
{
    if (music_list_view == NULL || !lv_obj_is_valid(music_list_view)) return;

    lv_obj_clean(music_list_view);

    music_info_t *item;
    MUSIC_LIST_FOR_EACH(item, &MusicControl.g_music_list)
    {
        lv_obj_t * btn = lv_list_add_btn(music_list_view, LV_SYMBOL_AUDIO, item->name);
        lv_obj_add_event_cb(btn, music_list_item_event_cb, LV_EVENT_ALL, item);

        /* --- 核心修改：如果是全局变量指向的歌曲，修改颜色 --- */
        if (g_current_music_info != NULL && item == g_current_music_info) 
        {
            // 修改文字颜色为蓝色（高亮当前播放）
            lv_obj_set_style_text_color(btn, lv_palette_main(LV_PALETTE_BLUE), 0);
            
            // 可选：加个背景色让它更明显
            lv_obj_set_style_bg_color(btn, lv_palette_lighten(LV_PALETTE_BLUE, 4), 0);
            lv_obj_set_style_bg_opa(btn, LV_OPA_20, 0);
            
            // 可选：自动滚动到这个高亮的按钮
            //lv_obj_scroll_to_view(btn, LV_ANIM_OFF);
        }
		 lv_obj_scroll_to_y(music_list_view, 0, LV_ANIM_OFF);
    }
}


/*-------------------------------------------------------------
                        事件回调处理
--------------------------------------------------------------*/
/**
 * music_list_break_clicked_event - 返回播放主界面
 */
static void music_list_break_clicked_event(lv_event_t * e)
{
	ui_screen_back();
}

/**
 * music_list_item_event_cb - 列表单项事件处理
 */
static void music_list_item_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    music_info_t * info = (music_info_t *)lv_event_get_user_data(e);
    
    static uint32_t prev_tick = 0;
    static void * prev_info = NULL;

    /* A. 双击逻辑: 播放歌曲 */
    if(code == LV_EVENT_CLICKED)
    {
        uint32_t current_tick = lv_tick_get();
        if(lv_tick_elaps(prev_tick) < 500 && prev_info == info)
        {
            printf("[UI] Double Click: %s\n", info->name);
            if(MusicControl.select != NULL)
            {
                MusicControl.select(info);
				
				
                /* 播放后自动回到主界面 */
                music_list_break_clicked_event(NULL);
            }
            prev_tick = 0;
            prev_info = NULL;
        }
        else
        {
            prev_tick = current_tick;
            prev_info = info;
        }
    }

    /* B. 长按逻辑: 弹出管理菜单 */
    if(code == LV_EVENT_LONG_PRESSED)
    {
        create_music_manage_panel(info);
    }
}


static lv_obj_t * mbox_container = NULL;

/**
 * 确定/取消/关闭 按钮通用事件回调
 */
static void confirm_menu_event_cb(lv_event_t * e) 
{
    music_info_t * info = lv_event_get_user_data(e);
    lv_obj_t * btn = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) 
	{
        // 尝试获取按钮内部的 Label 文本
        lv_obj_t * label = lv_obj_get_child(btn, 0);
        const char * txt = (label) ? lv_label_get_text(label) : "";

        // 如果点击的是 "Confirm"
        if(strcmp(txt, "确定") == 0) 
		{
            uint32_t new_flag = 0;
            lv_obj_t * panel = lv_obj_get_parent(btn);
            uint32_t child_cnt = lv_obj_get_child_cnt(panel);

            // 遍历所有子对象，寻找 Checkbox
            for(uint32_t i = 0; i < child_cnt; i++) 
			{
                lv_obj_t * child = lv_obj_get_child(panel, i);
                if(lv_obj_check_type(child, &lv_checkbox_class))
				{
                    if(lv_obj_has_state(child, LV_STATE_CHECKED)) 
					{
                        new_flag |= (uint32_t)lv_obj_get_user_data(child);
                    }
                }
            }
            info->flag = new_flag;
            music_list_refresh_view(); // 刷新背景列表
        }

        // 无论点击 Confirm、Cancel 还是 X，销毁容器
        if(mbox_container) {
            lv_obj_del(mbox_container);
            mbox_container = NULL;
        }
    }
}

/**
 * 创建自定义管理面板
 */
void create_music_manage_panel(music_info_t * info) 
{
    if(mbox_container) return; // 防止重复创建

    /* 1. 背景全屏遮罩 */
    mbox_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(mbox_container, 480, 480);
    lv_obj_set_style_bg_color(mbox_container, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(mbox_container, LV_OPA_50, 0);
    lv_obj_set_style_border_width(mbox_container, 0, 0);
    lv_obj_add_flag(mbox_container, LV_OBJ_FLAG_CLICKABLE);

    /* 2. 中央面板 (Panel) */
    lv_obj_t * panel = lv_obj_create(mbox_container);
    lv_obj_set_size(panel, 320, 360);
    lv_obj_center(panel);
    lv_obj_set_style_radius(panel, 15, 0);
    lv_obj_set_style_clip_corner(panel, true, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    /* 3. 右上角关闭按钮 (X) */
    lv_obj_t * close_btn = lv_btn_create(panel);
    lv_obj_set_size(close_btn, 35, 35);
    lv_obj_align(close_btn, LV_ALIGN_TOP_RIGHT, 5, -5);
    lv_obj_set_style_bg_opa(close_btn, 0, 0); // 按钮背景透明
    lv_obj_set_style_shadow_width(close_btn, 0, 0);
    lv_obj_t * x_label = lv_label_create(close_btn);
    lv_label_set_text(x_label, LV_SYMBOL_CLOSE);
    lv_obj_set_style_text_color(x_label, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_center(x_label);
    lv_obj_add_event_cb(close_btn, confirm_menu_event_cb, LV_EVENT_CLICKED, info);

    /* 4. 标题 (歌名) */
    lv_obj_t * title = lv_label_create(panel);
    lv_label_set_text(title, info->name);
    lv_obj_set_width(title, 240);
    lv_label_set_long_mode(title, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_text_font(title, &_lvgl_font2, 0);
    lv_obj_set_style_text_color(title, lv_palette_main(LV_PALETTE_BLUE), 0);

    /* 5. 复选框列表 (Checkbox) */
    const char * cb_names[] = {"添加到List1", "添加到List2", "添加到List 3", "添加到List4"};
    uint32_t masks[] = {MUSIC_FLAG_1, MUSIC_FLAG_2, MUSIC_FLAG_3, MUSIC_FLAG_4};
    
    for(int i = 0; i < 4; i++) 
	{
        lv_obj_t * cb = lv_checkbox_create(panel);
        lv_checkbox_set_text(cb, cb_names[i]);
        lv_obj_set_pos(cb, 20, 65 + (i * 45));
        lv_obj_set_style_text_font(cb, &_lvgl_font1, 0);
        
        // --- 核心修改：Checkbox 选中后的颜色 ---
        // 修改勾选框图标 (INDICATOR) 选中时的背景色
        lv_obj_set_style_bg_color(cb, lv_palette_main(LV_PALETTE_BLUE), LV_PART_INDICATOR | LV_STATE_CHECKED);
        // 修改文字选中时的颜色
        lv_obj_set_style_text_color(cb, lv_palette_main(LV_PALETTE_BLUE), LV_STATE_CHECKED);

        // 存储位掩码到 user_data
        lv_obj_set_user_data(cb, (void*)masks[i]);
        
        // 根据 info 状态初始化
        if(info->flag & masks[i]) {
            lv_obj_add_state(cb, LV_STATE_CHECKED);
        }
    }

    /* 6. 底部确定按钮 */
    lv_obj_t * btn_confirm = lv_btn_create(panel);
    lv_obj_set_size(btn_confirm, 110, 45);
    lv_obj_align(btn_confirm, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_style_radius(btn_confirm, 8, 0);
    lv_obj_t * l1 = lv_label_create(btn_confirm);
    lv_label_set_text(l1, "确定");
	lv_obj_set_style_text_font(l1, &_lvgl_font2, 0);
    lv_obj_center(l1);
    lv_obj_add_event_cb(btn_confirm, confirm_menu_event_cb, LV_EVENT_CLICKED, info);

    /* 7. 底部取消按钮 */
    lv_obj_t * btn_cancel = lv_btn_create(panel);
    lv_obj_set_size(btn_cancel, 110, 45);
    lv_obj_align(btn_cancel, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_radius(btn_cancel, 8, 0);
    lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0xE0E0E0), 0);
    lv_obj_t * l2 = lv_label_create(btn_cancel);
    lv_label_set_text(l2, "取消");
	lv_obj_set_style_text_font(l2, &_lvgl_font2, 0);
    lv_obj_set_style_text_color(l2, lv_color_black(), 0);
    lv_obj_center(l2);
    lv_obj_add_event_cb(btn_cancel, confirm_menu_event_cb, LV_EVENT_CLICKED, info);
}


#endif
