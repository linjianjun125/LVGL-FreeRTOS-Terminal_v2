#include "battery_screen.h"
#include <stdio.h>

#if USE_UI_BATTERY_SCREEN

lv_obj_t *battery_screen_obj = NULL;
ui_screen_t battery_screen = 
{
    .init = battery_screen_init,
    .deinit = battery_screen_deinit,
    .obj = &battery_screen_obj,
};

// --- 配置参数 ---
#define BATT_BAR_NUM  10    
#define BATT_SHELL_H  120   
#define BATT_SHELL_W  60    

// --- 私有变量 --- 
static lv_anim_t anim_charge_step;

// --- UI 控件句柄 --- 
static lv_obj_t *bar_objs[BATT_BAR_NUM];
static lv_obj_t *percent_label;
static lv_obj_t *status_label;
static lv_obj_t *level_val_label;
static lv_obj_t *volt_val_label;
static lv_obj_t *status_val_label;

// --- 私有函数声明 ---
static void anim_step_cb(void * var, int32_t v);
static void back_btn_event_cb(lv_event_t * e);
static void set_bar_status(int count, lv_color_t color, bool visible);
void battery_update_ui(uint8_t percent, float voltage, bool is_charging);
static void battery_sensor_cb(sensor_id_t id, const uint8_t* data);



/*-------------------------------------------------------------
                        页面初始化/销毁函数
--------------------------------------------------------------*/

void battery_screen_init(lv_obj_t *parent) 
{
    battery_screen_obj = lv_obj_create(parent);
    lv_obj_set_size(battery_screen_obj, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(battery_screen_obj, lv_color_hex(0xF4F7F6), 0);
    lv_obj_set_style_border_width(battery_screen_obj, 0, 0);
    lv_obj_clear_flag(battery_screen_obj, LV_OBJ_FLAG_SCROLLABLE);

    // 1. 返回按钮
    lv_obj_t * back_btn = lv_btn_create(battery_screen_obj);
    lv_obj_set_size(back_btn, 50, 50);
    lv_obj_set_pos(back_btn, 10, 10);
    lv_obj_set_style_radius(back_btn, 12, 0);
    lv_obj_set_style_bg_color(back_btn, lv_color_hex(0xFFFFFF), 0);
    lv_obj_add_event_cb(back_btn, back_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(back_label, lv_color_hex(0x2F4F39), 0);
    lv_obj_center(back_label);

    // 2. 电池外壳与电量格
    lv_obj_t * shell = lv_obj_create(battery_screen_obj);
    lv_obj_set_size(shell, BATT_SHELL_W, BATT_SHELL_H);
    lv_obj_align(shell, LV_ALIGN_CENTER, -65, -40);
    lv_obj_set_style_radius(shell, 8, 0);
    lv_obj_set_style_border_color(shell, lv_color_hex(0xBDC3C7), 0);
    lv_obj_set_style_border_width(shell, 3, 0);
    lv_obj_set_style_pad_all(shell, 5, 0);
    lv_obj_clear_flag(shell, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * cap = lv_obj_create(battery_screen_obj);
    lv_obj_set_size(cap, 20, 7);
    lv_obj_align_to(cap, shell, LV_ALIGN_OUT_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(cap, lv_color_hex(0xBDC3C7), 0);

    for(int i = 0; i < BATT_BAR_NUM; i++) {
        bar_objs[i] = lv_obj_create(shell);
        lv_obj_set_size(bar_objs[i], lv_pct(100), 8);
        lv_obj_align(bar_objs[i], LV_ALIGN_BOTTOM_MID, 0, -(i * 11));
        lv_obj_set_style_bg_color(bar_objs[i], lv_color_hex(0x2ECC71), 0);
        lv_obj_set_style_border_width(bar_objs[i], 0, 0);
        lv_obj_set_style_opa(bar_objs[i], 0, 0);
    }

    // 3. 右侧标签创建 (省略冗余样式代码)
    percent_label = lv_label_create(battery_screen_obj);
    lv_obj_set_style_text_font(percent_label, &_lvgl_font4, 0);
    lv_obj_align_to(percent_label, shell, LV_ALIGN_OUT_RIGHT_MID, 25, -15);

    status_label = lv_label_create(battery_screen_obj);
    lv_obj_set_style_text_font(status_label, &_lvgl_font2, 0);
    lv_obj_align_to(status_label, percent_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    // 4. 详情卡片逻辑 (使用之前定义的 CREATE_ROW)
    lv_obj_t *card = lv_obj_create(battery_screen_obj);
    lv_obj_set_size(card, lv_pct(92), 110);
    lv_obj_align(card, LV_ALIGN_BOTTOM_MID, 0, -15);
    lv_obj_set_style_radius(card, 16, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    #define CREATE_ROW(y_ofs, title, val_ptr) { \
        lv_obj_t * row = lv_obj_create(card); \
        lv_obj_set_size(row, lv_pct(100), 25); \
        lv_obj_align(row, LV_ALIGN_TOP_MID, 0, y_ofs); \
        lv_obj_set_style_bg_opa(row, 0, 0); \
        lv_obj_set_style_border_width(row, 0, 0); \
        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE); \
        lv_obj_t * t = lv_label_create(row); \
        lv_label_set_text(t, title); \
        lv_obj_align(t, LV_ALIGN_LEFT_MID, 0, 0); \
        lv_obj_set_style_text_font(t, &_lvgl_font2, 0); \
        *val_ptr = lv_label_create(row); \
        lv_obj_align(*val_ptr, LV_ALIGN_RIGHT_MID, 0, 0); \
        lv_obj_set_style_text_font(*val_ptr, &_lvgl_font2, 0); \
    }

    CREATE_ROW(0, "Current Level:", &level_val_label);
    CREATE_ROW(30, "Bus Voltage:", &volt_val_label);
    CREATE_ROW(60, "Status:", &status_val_label);

    // 5. 动画配置
    lv_anim_init(&anim_charge_step);
    lv_anim_set_var(&anim_charge_step, NULL);
    lv_anim_set_time(&anim_charge_step, 2000);
    lv_anim_set_repeat_count(&anim_charge_step, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_exec_cb(&anim_charge_step, (lv_anim_exec_xcb_t)anim_step_cb);

    // 6. 重要：通过 SensorHub 订阅电量信息
    // 订阅时，SensorHub 会立即喂入一次快照数据，触发 battery_sensor_cb
    SensorHub.subscribe(SENSOR_BATTERY, battery_sensor_cb);
}

void battery_screen_deinit(void) 
{
	// 取消订阅
	SensorHub.unsubscribe(SENSOR_BATTERY, battery_sensor_cb);
	
    // 停止所有属于此页面的动画
    lv_anim_del(NULL, (lv_anim_exec_xcb_t)anim_step_cb);
    
    lv_obj_del(battery_screen_obj);
}



/*-------------------------------------------------------------
                        UI界面逻辑
--------------------------------------------------------------*/
static void back_btn_event_cb(lv_event_t * e) 
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        ui_screen_back();
    }
}


/**
 * @brief SensorHub 订阅回调函数
 */
static void battery_sensor_cb(sensor_id_t id, const uint8_t* data)
{
    if (id == SENSOR_BATTERY && data != NULL) {
        sensor_battery_data_t* bat = (sensor_battery_data_t*)data;
        battery_update_ui(bat->percentage, bat->voltage, bat->is_charging);
    }
}

void battery_update_ui(uint8_t percent, float voltage, bool is_charging) 
{
    if (!battery_screen_obj) return;

    // 1. 更新数值文本
    lv_label_set_text_fmt(percent_label, "%d%%", percent);
    lv_label_set_text_fmt(level_val_label, "%d %%", percent);
    
    // 电压显示处理
    int int_part = (int)voltage;
    int dec_part = (int)((voltage - int_part) * 100);
    lv_label_set_text_fmt(volt_val_label, "%d.%02d V", int_part, dec_part);

    // 2. 处理充电与非充电显示逻辑
    int active_bars = (percent + 5) / 10;
    if (active_bars > BATT_BAR_NUM) active_bars = BATT_BAR_NUM;

    if (is_charging) {
        lv_label_set_text(status_label, LV_SYMBOL_CHARGE " Charging");
        lv_label_set_text(status_val_label, "Charging");
        lv_obj_set_style_text_color(status_val_label, lv_color_hex(0x2ECC71), 0);

        // 如果动画没运行，则开启
        if(!lv_anim_get(NULL, (lv_anim_exec_xcb_t)anim_step_cb)) {
            lv_anim_set_values(&anim_charge_step, active_bars, BATT_BAR_NUM);
            lv_anim_start(&anim_charge_step);
        }
    } else {
        lv_label_set_text(status_label, "Battery Mode");
        lv_label_set_text(status_val_label, "Discharging");
        lv_obj_set_style_text_color(status_val_label, lv_color_hex(0xE74C3C), 0);

        // 停止动画并根据电量设颜色
        lv_anim_del(NULL, (lv_anim_exec_xcb_t)anim_step_cb);
        lv_color_t color = (percent < 20) ? lv_color_hex(0xE74C3C) : lv_color_hex(0x273C2C);
        set_bar_status(active_bars, color, true);
    }
}

static void set_bar_status(int count, lv_color_t color, bool visible) 
{
    for (int i = 0; i < BATT_BAR_NUM; i++) {
        if (i < count) {
            lv_obj_set_style_bg_color(bar_objs[i], color, 0);
            lv_obj_set_style_opa(bar_objs[i], visible ? 255 : 0, 0);
        } else {
            lv_obj_set_style_opa(bar_objs[i], 0, 0);
        }
    }
}


static void anim_step_cb(void * var, int32_t v) 
{
    set_bar_status(v, lv_color_hex(0x2ECC71), true);
}


#else

ui_screen_t battery_screen = 
{
    .init = NULL,
    .deinit = NULL,
    .obj = NULL,
};

#endif



