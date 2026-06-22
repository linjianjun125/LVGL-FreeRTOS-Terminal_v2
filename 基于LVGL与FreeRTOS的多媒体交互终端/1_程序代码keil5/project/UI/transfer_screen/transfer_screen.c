#include "transfer_screen.h"
#include <stdio.h>

#if USE_UI_TRANSFER_SCREEN

lv_obj_t *transfer_screen_obj;
ui_screen_t transfer_screen =
{
    .init = transfer_screen_init,
    .deinit = transfer_screen_deinit,
    .obj = &transfer_screen_obj,
};


// --- 私有变量与状态 --- 
static int current_progress = 0;
static transfer_status_t current_status = TRANSFER_STATUS_CONNECTING;
static lv_timer_t * transfer_sim_timer = NULL;

// --- UI 控件句柄 ---
static lv_obj_t *status_icon;
static lv_obj_t *status_label;
static lv_obj_t *progress_bar;
static lv_obj_t *info_label;
static lv_obj_t *btn_action;
static lv_obj_t *btn_label;

void transfer_update_ui(transfer_status_t status, uint8_t progress);
static void transfer_simulate_timer_cb(lv_timer_t * timer);
static void btn_back_handler(lv_event_t * e);
static void btn_action_event_handler(lv_event_t * e);


/*-------------------------------------------------------------
                        页面初始化/销毁函数
--------------------------------------------------------------*/

void transfer_screen_init(lv_obj_t *parent)
{
    // 1. 设置主容器
    transfer_screen_obj = lv_obj_create(parent);
    lv_obj_set_size(transfer_screen_obj, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(transfer_screen_obj, lv_color_hex(0xF0F9F4), 0);
    lv_obj_set_style_border_width(transfer_screen_obj, 0, 0);
    lv_obj_clear_flag(transfer_screen_obj, LV_OBJ_FLAG_SCROLLABLE);

    // 2. 返回按钮 (位于左上角)
    lv_obj_t * btn_back = lv_btn_create(transfer_screen_obj);
    lv_obj_set_size(btn_back, 60, 60);
    lv_obj_align(btn_back, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_set_style_radius(btn_back, 10, 0);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x82C09A), 0);
    lv_obj_set_style_bg_opa(btn_back, LV_OPA_20, 0);
    lv_obj_set_style_shadow_width(btn_back, 0, 0);
    lv_obj_add_event_cb(btn_back, btn_back_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t * lbl_back = lv_label_create(btn_back);
    lv_label_set_text(lbl_back, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(lbl_back, lv_color_hex(0x4A7C59), 0);
    lv_obj_center(lbl_back);

    // 3. 状态卡片
    lv_obj_t *card = lv_obj_create(transfer_screen_obj);
    lv_obj_set_size(card, lv_pct(88), lv_pct(75));
    lv_obj_align(card, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_obj_set_style_radius(card, 20, 0);
    lv_obj_set_style_bg_color(card, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_shadow_width(card, 30, 0);
    lv_obj_set_style_shadow_color(card, lv_color_hex(0xDDEEE5), 0);
    lv_obj_set_style_shadow_opa(card, LV_OPA_70, 0);
    lv_obj_set_style_pad_all(card, 0, 0);

    // 4. 状态图标
    status_icon = lv_label_create(card);
    lv_obj_set_width(status_icon, lv_pct(100));
    lv_obj_set_style_text_align(status_icon, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(status_icon, &_lvgl_font4, 0);
    lv_obj_align(status_icon, LV_ALIGN_TOP_MID, 0, 35);

    // 5. 状态标题
    status_label = lv_label_create(card);
    lv_obj_set_width(status_label, lv_pct(100));
    lv_obj_set_style_text_align(status_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(status_label, &_lvgl_font2, 0);
    lv_obj_align_to(status_label, status_icon, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);

    // 6. 进度条
    progress_bar = lv_bar_create(card);
    lv_obj_set_size(progress_bar, lv_pct(80), 10);
    lv_obj_align(progress_bar, LV_ALIGN_CENTER, 0, 45);
    lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0xE2F0E9), LV_PART_MAIN);
    lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0x82C09A), LV_PART_INDICATOR);
    lv_obj_set_style_radius(progress_bar, 5, 0);

    // 7. 详情描述信息
    info_label = lv_label_create(card);
    lv_obj_set_width(info_label, lv_pct(100));
    lv_obj_set_style_text_align(info_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(info_label, lv_color_hex(0x6A8D73), 0);
    lv_obj_set_style_text_font(info_label, &_lvgl_font2, 0);
    lv_obj_align_to(info_label, progress_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 12);

    // 8. 操作按钮
    btn_action = lv_btn_create(card);
    lv_obj_set_size(btn_action, 110, 42);
    lv_obj_align(btn_action, LV_ALIGN_BOTTOM_MID, 0, -25);
    lv_obj_set_style_bg_color(btn_action, lv_color_hex(0x82C09A), 0);
    lv_obj_set_style_radius(btn_action, 21, 0);
    lv_obj_set_style_shadow_width(btn_action, 0, 0);
    lv_obj_add_event_cb(btn_action, btn_action_event_handler, LV_EVENT_CLICKED, NULL);

    btn_label = lv_label_create(btn_action);
    lv_obj_set_width(btn_label, lv_pct(100));
    lv_obj_set_style_text_align(btn_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(btn_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(btn_label);

    // 9. 默认启动状态
    current_progress = 0;
    transfer_update_ui(TRANSFER_STATUS_CONNECTING, 0);
    transfer_sim_timer = lv_timer_create(transfer_simulate_timer_cb, 1000, NULL);
}

void transfer_screen_deinit(void)
{
    if (transfer_sim_timer != NULL) {
        lv_timer_del(transfer_sim_timer);
        transfer_sim_timer = NULL;
    }
}


/*-------------------------------------------------------------
                        UI界面函数
--------------------------------------------------------------*/
/**
 * @brief 更新界面显示状态
 */
void transfer_update_ui(transfer_status_t status, uint8_t progress)
{
    if (!transfer_screen_obj) return;
    current_status = status;

    switch (status) {
        case TRANSFER_STATUS_CONNECTING:
            lv_label_set_text(status_label, "Connecting...");
            lv_obj_set_style_text_color(status_label, lv_color_hex(0x4A7C59), 0);
            lv_label_set_text(status_icon, LV_SYMBOL_LOOP);
            lv_obj_set_style_text_color(status_icon, lv_color_hex(0x82C09A), 0);
            lv_label_set_text(info_label, "Waiting for device...");
            lv_label_set_text(btn_label, "Cancel");
            break;

        case TRANSFER_STATUS_PROCESSING:
            lv_label_set_text(status_label, "Transferring...");
            lv_obj_set_style_text_color(status_label, lv_color_hex(0x2F4F39), 0);
            lv_label_set_text(status_icon, LV_SYMBOL_DOWNLOAD);
            lv_obj_set_style_text_color(status_icon, lv_color_hex(0x82C09A), 0);
            lv_bar_set_value(progress_bar, progress, LV_ANIM_ON);
            lv_label_set_text_fmt(info_label, "Progress: %d%%", progress);
            lv_label_set_text(btn_label, "Stop");
            break;

        case TRANSFER_STATUS_SUCCESS:
            lv_label_set_text(status_label, "Success!");
            lv_obj_set_style_text_color(status_label, lv_color_hex(0x28A745), 0);
            lv_label_set_text(status_icon, LV_SYMBOL_OK);
            lv_obj_set_style_text_color(status_icon, lv_color_hex(0x28A745), 0);
            lv_bar_set_value(progress_bar, 100, LV_ANIM_ON);
            lv_label_set_text(info_label, "Transfer completed.");
            lv_label_set_text(btn_label, "Done");
            break;

        case TRANSFER_STATUS_FAILED:
            lv_label_set_text(status_label, "Transfer Failed");
            lv_obj_set_style_text_color(status_label, lv_color_hex(0xA52A2A), 0);
            lv_label_set_text(status_icon, LV_SYMBOL_WARNING);
            lv_obj_set_style_text_color(status_icon, lv_color_hex(0xFF4C4C), 0);
            lv_label_set_text(info_label, "Error: Timeout occurred.");
            lv_label_set_text(btn_label, "Retry");
            break;
    }
}


/*-------------------------------------------------------------
                        事件/定时器回调函数
--------------------------------------------------------------*/
/**
 * @brief 返回按钮回调
 */
static void btn_back_handler(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED)
	{
        // 调用你框架中的退出/切换函数
        printf("[UI] Back to main menu from transfer\n");
		ui_screen_back();
    }
}



static void btn_action_event_handler(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        if (current_status == TRANSFER_STATUS_FAILED) {
            current_progress = 0;
            transfer_update_ui(TRANSFER_STATUS_CONNECTING, 0);
            if(transfer_sim_timer) lv_timer_resume(transfer_sim_timer);
        } else {
            printf("[UI] Action Button: Done/Stop\n");
        }
    }
}

static void transfer_simulate_timer_cb(lv_timer_t * timer)
{
    if (current_status == TRANSFER_STATUS_CONNECTING) 
	{
        static int wait_count = 0;
        if (++wait_count >= 2) {
            wait_count = 0;
            transfer_update_ui(TRANSFER_STATUS_PROCESSING, 0);
        }
    }
    else if (current_status == TRANSFER_STATUS_PROCESSING) {
        current_progress += 10;
        if (current_progress <= 100) {
            transfer_update_ui(TRANSFER_STATUS_PROCESSING, current_progress);
        } else {
            transfer_update_ui(TRANSFER_STATUS_SUCCESS, 100);
            lv_timer_pause(timer);
        }
    }
}

#else

ui_screen_t transfer_screen =
{
    .init = NULL,
    .deinit = NULL,
    .obj = NULL,
};

#endif
