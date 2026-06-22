#include "clock_add_screen.h"


#if USE_UI_CLOCK_SCREEN

lv_obj_t *clock_add_screen_obj;
ui_screen_t clock_add_screen =
{
	.init = clock_add_screen_init,
	.deinit = clock_add_screen_deinit,
	.obj = &clock_add_screen_obj,
};

static lv_obj_t *clock_rolleer1;
static lv_obj_t *clock_rolleer2;
static lv_obj_t *clock_rolleer3;
static void clock_add_screen_btn_clicked_event(lv_event_t * e);
void clock_create_add_clock_msgbox(void);
static void clock_add_msgbox_event(lv_event_t * e);
static void clock_failure_msgbox_event(lv_event_t * e);
/*-------------------------------------------------------------
                        页面初始化/销毁函数
--------------------------------------------------------------*/
void clock_add_screen_init(lv_obj_t *parent)
{
    clock_add_screen_obj = lv_obj_create(parent);
    lv_obj_set_size(clock_add_screen_obj, lv_pct(100), lv_pct(100));
    lv_obj_clear_flag(clock_add_screen_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(clock_add_screen_obj, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(clock_add_screen_obj, LV_OPA_100, LV_STATE_DEFAULT);

    // clock_close_btn
    lv_obj_t *clock_close_btn = lv_obj_create(clock_add_screen_obj);
    lv_obj_remove_style_all(clock_close_btn);
    lv_obj_set_size(clock_close_btn, 80, 50);
    lv_obj_align(clock_close_btn, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_t *label = lv_label_create(clock_close_btn);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(label, LV_SYMBOL_CLOSE);
    lv_obj_set_style_text_color(label, lv_color_hex(0x313131), LV_STATE_DEFAULT);
    //lv_obj_set_style_text_font(label, &lv_font_montserrat_20, LV_STATE_DEFAULT);
    lv_obj_add_event_cb(clock_close_btn, clock_add_screen_btn_clicked_event, LV_EVENT_CLICKED, (void*)1);

    // clock_ok_btn
    lv_obj_t *clock_ok_btn = lv_obj_create(clock_add_screen_obj);
    lv_obj_remove_style_all(clock_ok_btn);
    lv_obj_set_size(clock_ok_btn, 80, 50);
    lv_obj_align(clock_ok_btn, LV_ALIGN_TOP_RIGHT, -5, 5);
    label = lv_label_create(clock_ok_btn);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(label, LV_SYMBOL_OK);
    lv_obj_set_style_text_color(label, lv_color_hex(0x313131), LV_STATE_DEFAULT);
   // lv_obj_set_style_text_font(label, &lv_font_montserrat_20, LV_STATE_DEFAULT);
    lv_obj_add_event_cb(clock_ok_btn, clock_add_screen_btn_clicked_event, LV_EVENT_CLICKED, (void*)2);

    // Line1
    lv_obj_t *line1 = lv_line_create(clock_add_screen_obj);
    static lv_point_t points1[] = {{170, 140}, {170, 340}};
    lv_line_set_points(line1, points1, 2);
    lv_obj_set_style_line_width(line1, 1, LV_PART_MAIN);
    lv_obj_set_style_line_color(line1, lv_color_hex(0xE6E6E6), LV_STATE_DEFAULT);

    // Line2
    lv_obj_t *line2 = lv_line_create(clock_add_screen_obj);
    static lv_point_t points2[] = {{320, 140}, {320, 340}};
    lv_line_set_points(line2, points2, 2);
    lv_obj_set_style_line_width(line2, 1, LV_PART_MAIN);
    lv_obj_set_style_line_color(line2, lv_color_hex(0xE6E6E6), LV_STATE_DEFAULT);

    static lv_style_t style_roller_main;
    lv_style_init(&style_roller_main);
    lv_style_set_text_color(&style_roller_main, lv_color_hex(0xBABABA));
    lv_style_set_text_font(&style_roller_main, &_lvgl_font3);
    lv_style_set_bg_color(&style_roller_main, lv_color_hex(0xFFFFFF));
    lv_style_set_border_width(&style_roller_main, 0);

    static lv_style_t style_roller_select;
    lv_style_init(&style_roller_select);
    lv_style_set_text_color(&style_roller_select, lv_color_hex(0x010101));
    lv_style_set_text_font(&style_roller_select, &_lvgl_font4);
    lv_style_set_bg_color(&style_roller_select, lv_color_hex(0xFFFFFF));

    // Roller1
    clock_rolleer1 = lv_roller_create(clock_add_screen_obj);
    lv_obj_align(clock_rolleer1, LV_ALIGN_CENTER, -140, 0);
    lv_roller_set_options(clock_rolleer1, "AM\nPM", LV_ROLLER_MODE_NORMAL);
    lv_obj_add_style(clock_rolleer1, &style_roller_main, LV_PART_MAIN);
    lv_obj_add_style(clock_rolleer1, &style_roller_select, LV_PART_SELECTED);

    // Roller2
    clock_rolleer2 = lv_roller_create(clock_add_screen_obj);
    lv_obj_align(clock_rolleer2, LV_ALIGN_CENTER, 0, 0);
    lv_roller_set_options(clock_rolleer2, "01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12", LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(clock_rolleer2, 7);
    lv_obj_add_style(clock_rolleer2, &style_roller_main, LV_PART_MAIN);
    lv_obj_add_style(clock_rolleer2, &style_roller_select, LV_PART_SELECTED);

    // Roller3
    clock_rolleer3 = lv_roller_create(clock_add_screen_obj);
    lv_obj_align(clock_rolleer3, LV_ALIGN_CENTER, 140, 0);
    lv_roller_set_options(clock_rolleer3, "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12", LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(clock_rolleer3, 7);
    lv_obj_add_style(clock_rolleer3, &style_roller_main, LV_PART_MAIN);
    lv_obj_add_style(clock_rolleer3, &style_roller_select, LV_PART_SELECTED);


    // Label H
    lv_obj_t *clock_h = lv_label_create(clock_add_screen_obj);
    lv_obj_align(clock_h, LV_ALIGN_CENTER, 30, 0);
    lv_label_set_text(clock_h, "H");
    lv_obj_set_style_text_color(clock_h, lv_color_hex(0X010101), LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(clock_h, &_lvgl_font2, LV_STATE_DEFAULT);

    // Label M
    lv_obj_t *clock_m = lv_label_create(clock_add_screen_obj);
    lv_obj_align(clock_m, LV_ALIGN_CENTER, 170, 0);
    lv_label_set_text(clock_m, "M");
    lv_obj_set_style_text_color(clock_m, lv_color_hex(0X010101), LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(clock_m, &_lvgl_font2, LV_STATE_DEFAULT);

}

void clock_add_screen_deinit(void)
{
    if (clock_add_screen_obj)
    {
        lv_obj_del(clock_add_screen_obj);
        clock_add_screen_obj = NULL;
    }
}





/*-------------------------------------------------------------
                        额外的创建函数
--------------------------------------------------------------*/
void clock_create_add_clock_msgbox(void) 
{
    static const char * btns[] = {"确定", "取消", ""};
    lv_obj_t * mbox = lv_msgbox_create(lv_layer_top(), "提醒", "是否添加新的闹钟？", btns, false);
	lv_obj_set_style_text_font(mbox, &_lvgl_font2, LV_STATE_DEFAULT);
    lv_obj_center(mbox);
    lv_obj_add_event_cb(mbox, clock_add_msgbox_event, LV_EVENT_VALUE_CHANGED, NULL);
}

void clock_create_add_failure_msgbox(void) 
{
    static const char * btns[] = {"确定", ""};
    lv_obj_t * mbox = lv_msgbox_create(lv_layer_top(), "错误", "添加闹钟失败！", btns, false);
	lv_obj_set_style_text_font(mbox, &_lvgl_font2, LV_STATE_DEFAULT);
	
    lv_obj_center(mbox);
    lv_obj_add_event_cb(mbox, clock_failure_msgbox_event, LV_EVENT_VALUE_CHANGED, NULL);
}

/*-------------------------------------------------------------
                        事件/定时器回调函数
--------------------------------------------------------------*/
static void clock_add_screen_btn_clicked_event(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    int type = (int)lv_event_get_user_data(e);
    if(event_code == LV_EVENT_CLICKED)
    {
        if(type == 1)
        {
            ui_screen_back();
        }
        else if(type == 2)
        {
			clock_create_add_clock_msgbox();
        }
    }
}


static void clock_add_msgbox_event(lv_event_t * e)
{
    lv_obj_t * mbox = lv_event_get_current_target(e);
    const char * btn_txt = lv_msgbox_get_active_btn_text(mbox);
	lv_msgbox_close(mbox);
	
    if(btn_txt)
    {
        if(strcmp(btn_txt, "确定") == 0) 
        {
            int ampm = lv_roller_get_selected(clock_rolleer1);
            int hour = lv_roller_get_selected(clock_rolleer2);
            int min = lv_roller_get_selected(clock_rolleer3);
			hour = (ampm == 1) ? hour + 13 : hour + 1;
			
			if(!AlarmControl.add(hour, min, 0))
			{
				clock_create_add_failure_msgbox();
			}
        }
    }
}


static void clock_failure_msgbox_event(lv_event_t * e)
{
	lv_obj_t * mbox = lv_event_get_current_target(e);
	lv_msgbox_close(mbox);
}


#else

ui_screen_t clock_add_screen =
{
	.init = NULL,
	.deinit = NULL,
	.obj = NULL,
};


#endif





