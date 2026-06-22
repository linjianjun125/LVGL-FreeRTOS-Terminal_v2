#include "stopwatch_screen.h"
#include <stdio.h>

#if USE_UI_STOPWATCH_SCREEN
lv_obj_t *stopwatch_screen_obj;
ui_screen_t stopwatch_screen = 
{
    .init = stopwatch_screen_init,
    .deinit = stopwatch_screen_deinit,
    .obj = &stopwatch_screen_obj,
};

extern uint32_t g_sys_ms_tick;
// --- 私有变量 ---
static uint32_t start_tick = 0;  // 记录开始时的系统滴答值
static uint32_t paused_time = 0; // 记录已累积的时间（用于暂停后再开始）
static uint32_t time_counter = 0;
static bool is_running = false;
static lv_timer_t * stopwatch_timer = NULL;

// --- UI 控件句柄 ---
static lv_obj_t *time_label;
static lv_obj_t *btn_start_stop;
static lv_obj_t *btn_start_label;
static lv_obj_t *btn_reset;

static void btn_back_handler(lv_event_t * e);
static void update_time_display(void);
static void stopwatch_timer_cb(lv_timer_t * timer);
static void btn_reset_handler(lv_event_t * e);
static void btn_start_handler(lv_event_t * e);


/*-------------------------------------------------------------
                        页面初始化/销毁函数
--------------------------------------------------------------*/

void stopwatch_screen_init(lv_obj_t *parent)
{
    // 1. 主容器
    stopwatch_screen_obj = lv_obj_create(parent);
    lv_obj_set_size(stopwatch_screen_obj, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(stopwatch_screen_obj, lv_color_hex(0xF0F9F4), 0);
    lv_obj_set_style_border_width(stopwatch_screen_obj, 0, 0);
    lv_obj_clear_flag(stopwatch_screen_obj, LV_OBJ_FLAG_SCROLLABLE);

    // 2. 返回按钮 (左上角)
    lv_obj_t * btn_back = lv_btn_create(stopwatch_screen_obj);
    lv_obj_set_size(btn_back, 50, 50);
    lv_obj_align(btn_back, LV_ALIGN_TOP_LEFT, 15, 15);
    lv_obj_set_style_radius(btn_back, 10, 0);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x82C09A), 0);
    lv_obj_set_style_bg_opa(btn_back, LV_OPA_20, 0); // 浅色背景
    lv_obj_set_style_shadow_width(btn_back, 0, 0);
    lv_obj_add_event_cb(btn_back, btn_back_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t * lbl_back = lv_label_create(btn_back);
    lv_label_set_text(lbl_back, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(lbl_back, lv_color_hex(0x4A7C59), 0);
    lv_obj_center(lbl_back);

    // 3. 装饰性背景环
    lv_obj_t * circle = lv_obj_create(stopwatch_screen_obj);
    lv_obj_set_size(circle, 220, 220);
    lv_obj_center(circle);
    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(circle, 0, 0);
    lv_obj_set_style_border_color(circle, lv_color_hex(0x82C09A), 0);
    lv_obj_set_style_border_width(circle, 2, 0);
    lv_obj_set_style_border_opa(circle, LV_OPA_30, 0);

    // 4. 时间显示标签
    time_label = lv_label_create(stopwatch_screen_obj);
    lv_obj_set_width(time_label, lv_pct(100));
    lv_obj_set_style_text_align(time_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(time_label, &_lvgl_font4, 0);
    lv_obj_set_style_text_color(time_label, lv_color_hex(0x2F4F39), 0);
    lv_obj_center(time_label);
    update_time_display();

    // 5. 底部按钮容器
    lv_obj_t * cont = lv_obj_create(stopwatch_screen_obj);
    lv_obj_set_size(cont, lv_pct(100), 80);
    lv_obj_align(cont, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_obj_set_style_bg_opa(cont, 0, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 重置按钮
    btn_reset = lv_btn_create(cont);
    lv_obj_set_size(btn_reset, 85, 45);
    lv_obj_set_style_bg_color(btn_reset, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_color(btn_reset, lv_color_hex(0x82C09A), 0);
    lv_obj_set_style_border_width(btn_reset, 1, 0);
    lv_obj_set_style_radius(btn_reset, 22, 0);
    lv_obj_add_event_cb(btn_reset, btn_reset_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t * lbl_reset = lv_label_create(btn_reset);
    lv_label_set_text(lbl_reset, "RESET");
    lv_obj_set_style_text_color(lbl_reset, lv_color_hex(0x82C09A), 0);
    lv_obj_center(lbl_reset);

    // 开始/暂停按钮
    btn_start_stop = lv_btn_create(cont);
    lv_obj_set_size(btn_start_stop, 125, 45);
    lv_obj_set_style_bg_color(btn_start_stop, lv_color_hex(0x82C09A), 0);
    lv_obj_set_style_radius(btn_start_stop, 22, 0);
    lv_obj_add_event_cb(btn_start_stop, btn_start_handler, LV_EVENT_CLICKED, NULL);

    btn_start_label = lv_label_create(btn_start_stop);
    lv_label_set_text(btn_start_label, "START");
    lv_obj_set_style_text_color(btn_start_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(btn_start_label);

    stopwatch_timer = lv_timer_create(stopwatch_timer_cb, 10, NULL);
}

void stopwatch_screen_deinit(void)
{
    if (stopwatch_timer != NULL) {
        lv_timer_del(stopwatch_timer);
        stopwatch_timer = NULL;
    }
}


/*-------------------------------------------------------------
                        UI界面函数
--------------------------------------------------------------*/
/**
 * @brief 格式化并更新时间显示
 */
static void update_time_display(void)
{
    // time_counter 现在代表毫秒 (ms)
    uint32_t total_ms = time_counter;
    
    uint32_t ms_part = (total_ms % 1000) / 10; // 取 00-99 的百分秒
    uint32_t sec = (total_ms / 1000) % 60;
    uint32_t min = (total_ms / 60000) % 100;
    
    lv_label_set_text_fmt(time_label, "%02u:%02u.%02u", min, sec, ms_part);
}



/*-------------------------------------------------------------
                        事件/定时器回调函数
--------------------------------------------------------------*/
/**
 * @brief 返回按钮事件：退出当前屏幕
 */
static void btn_back_handler(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) 
	{
        // 假设 ui_screen_break 是你框架中定义的退出/切换函数
        //printf("[UI] Stopwatch exit\n");
		ui_screen_back();
    }
}


static void stopwatch_timer_cb(lv_timer_t * timer)
{
    if (is_running) {
        // 计算当前运行了多少毫秒
        time_counter = (g_sys_ms_tick - start_tick) + paused_time;
        update_time_display();
    }
}

static void btn_start_handler(lv_event_t * e)
{
	    if (is_running) 
	{
        lv_label_set_text(btn_start_label, "PAUSE");
        lv_obj_set_style_bg_color(btn_start_stop, lv_color_hex(0xFFB3BA), 0);
    } else {
        lv_label_set_text(btn_start_label, "START");
        lv_obj_set_style_bg_color(btn_start_stop, lv_color_hex(0x82C09A), 0);
    }
	
	// 伪代码：在你的 btn_start_stop 回调中
	if (!is_running) {
		// 切换到运行状态
		start_tick = g_sys_ms_tick; // 记录按下这一刻的时间点
		is_running = true;
	} else {
		// 切换到停止状态
		paused_time = time_counter; // 保存当前已经走过的时间点
		is_running = false;
	}
}

static void btn_reset_handler(lv_event_t * e)
{
    is_running = false;
    time_counter = 0;
    update_time_display();
    lv_label_set_text(btn_start_label, "START");
    lv_obj_set_style_bg_color(btn_start_stop, lv_color_hex(0x82C09A), 0);
}

#else


ui_screen_t stopwatch_screen = 
{
    .init = NULL,
    .deinit = NULL,
    .obj = NULL,
};


#endif
