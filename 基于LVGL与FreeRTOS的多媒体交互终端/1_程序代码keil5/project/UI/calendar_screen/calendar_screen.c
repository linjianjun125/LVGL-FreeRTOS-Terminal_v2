#include "calendar_screen.h"
#include <stdio.h>

#if USE_UI_CALENDAR_SCREEN

lv_obj_t *calendar_screen_obj; 
ui_screen_t calendar_screen = {
    .init = calendar_screen_init,
    .deinit = calendar_screen_deinit,
    .obj = &calendar_screen_obj,
};


// --- 私有全局变量 ---
static lv_obj_t * calendar_inst = NULL;    // 日历组件实例

static void btn_exit_event_handler(lv_event_t * e);
static void calendar_status_update_cb(sensor_id_t id, const uint8_t* data);
static bool first_init_status = true;
/*-------------------------------------------------------------
                        页面初始化/销毁函数
--------------------------------------------------------------*/

void calendar_screen_init(lv_obj_t *parent)
{
    // 1. 设置主屏幕容器：采用极淡的薄荷绿背景 (Mint White)
    calendar_screen_obj = lv_obj_create(parent);
    lv_obj_set_size(calendar_screen_obj, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(calendar_screen_obj, lv_color_hex(0xF0F9F4), 0);
    lv_obj_set_style_border_width(calendar_screen_obj, 0, 0);
    lv_obj_clear_flag(calendar_screen_obj, LV_OBJ_FLAG_SCROLLABLE);

    // 2. 顶部标题栏设计 (Header)
    lv_obj_t * header = lv_obj_create(calendar_screen_obj);
    lv_obj_set_size(header, lv_pct(100), 45);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_70, 0);   // 毛玻璃半透明效果
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_border_side(header, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_color(header, lv_color_hex(0xE2F0E9), 0); // 淡绿色分割线
    lv_obj_set_style_border_width(header, 1, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    // 2.1 退出按钮：采用森林绿图标
    lv_obj_t * btn_exit = lv_btn_create(header);
    lv_obj_set_size(btn_exit, 60, 60);
    lv_obj_align(btn_exit, LV_ALIGN_LEFT_MID, -8, 0);
    lv_obj_set_style_bg_opa(btn_exit, LV_OPA_TRANSP, 0);
    lv_obj_set_style_shadow_width(btn_exit, 0, 0);

    lv_obj_t * label_exit = lv_label_create(btn_exit);
    lv_label_set_text(label_exit, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(label_exit, lv_color_hex(0x4A7C59), 0); // 森林绿
    lv_obj_center(label_exit);
    lv_obj_add_event_cb(btn_exit, btn_exit_event_handler, LV_EVENT_CLICKED, NULL);

    // 2.2 标题：深色森林绿
    lv_obj_t * title = lv_label_create(header);
    lv_label_set_text(title, "Calendar");
    lv_obj_set_style_text_color(title, lv_color_hex(0x2F4F39), 0);
    lv_obj_set_style_text_font(title, &_lvgl_font2, 0);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);

    // 3. 日历卡片设计 (Main Card)
    calendar_inst = lv_calendar_create(calendar_screen_obj);
    lv_obj_set_size(calendar_inst, lv_pct(92), lv_pct(80));
    lv_obj_align(calendar_inst, LV_ALIGN_BOTTOM_MID, 0, -15);

    // 3.1 整体卡片外观：纯白底色 + 圆润边角
    lv_obj_set_style_bg_color(calendar_inst, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_radius(calendar_inst, 18, 0);
    lv_obj_set_style_border_width(calendar_inst, 0, 0);

    // 使用“淡绿色”阴影，增加整体色调一致性
    lv_obj_set_style_shadow_width(calendar_inst, 25, 0);
    lv_obj_set_style_shadow_color(calendar_inst, lv_color_hex(0xDDEEE5), 0);
    lv_obj_set_style_shadow_opa(calendar_inst, LV_OPA_80, 0);
    lv_obj_set_style_shadow_ofs_y(calendar_inst, 8, 0);

    // 3.2 日历内容配色：雅致灰绿
    lv_obj_set_style_text_font(calendar_inst, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(calendar_inst, lv_color_hex(0x6A8D73), 0); // 灰绿色日期

    // 3.3 选中状态：莫兰迪绿背景 + 纯白文字
    lv_obj_set_style_bg_color(calendar_inst, lv_color_hex(0x82C09A), LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(calendar_inst, lv_color_hex(0xFFFFFF), LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_radius(calendar_inst, 10, LV_PART_ITEMS | LV_STATE_CHECKED);

    // 3.4 今天的标记：森林绿细边框
    lv_obj_set_style_border_color(calendar_inst, lv_color_hex(0x82C09A), LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(calendar_inst, 2, LV_PART_ITEMS | LV_STATE_DEFAULT);

    // 4. 创建日历页眉 (月份切换箭头)
    lv_obj_t * header_arrow = lv_calendar_header_arrow_create(calendar_inst);
    lv_obj_set_style_text_color(header_arrow, lv_color_hex(0x4A7C59), 0);

    // 5. 初始化时间并启动定时刷新
    SensorHub.subscribe(SENSOR_RTC, calendar_status_update_cb);
	
	
}

/**
 * @brief 界面卸载：清理资源与定时器
 */
void calendar_screen_deinit(void)
{
	SensorHub.unsubscribe(SENSOR_RTC, calendar_status_update_cb);

    if (calendar_screen_obj != NULL)
    {
        lv_obj_del(calendar_screen_obj);
        calendar_screen_obj = NULL;
    }
	first_init_status = true;
	
}



/*-------------------------------------------------------------
                        事件/定时器回调函数
--------------------------------------------------------------*/
/**
 * @brief 退出按钮点击事件
 */
static void btn_exit_event_handler(lv_event_t * e)
{
    printf("[UI] Calendar Screen: Exit clicked\n");
    // 执行界面切换逻辑 (请对接您的 UI 框架接口)
	ui_screen_back();
}

/**
 * @brief 定时器回调函数：每秒检测并同步系统日期
 * @param timer 关联的定时器对象
 */
static void calendar_status_update_cb(sensor_id_t id, const uint8_t* data)
{
    // 核心安全检查：如果页面已销毁，直接返回
    if (calendar_screen_obj == NULL) return;

    if (id == SENSOR_RTC)
    {
		if (calendar_inst == NULL) return;
		 sensor_rtc_data_t* rtc = (sensor_rtc_data_t*)data;   
		lv_calendar_set_today_date(calendar_inst, rtc->year, rtc->month, rtc->day);
		
		if(first_init_status){
			lv_calendar_set_showed_date(calendar_inst, rtc->year, rtc->month);
			first_init_status = false;
		}
	}
}



#else

ui_screen_t calendar_screen = {
    .init = NULL,
    .deinit = NULL,
    .obj = NULL,
};

#endif





