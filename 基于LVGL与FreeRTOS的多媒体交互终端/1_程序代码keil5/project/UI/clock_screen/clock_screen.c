#include "clock_screen.h"


#if USE_UI_CLOCK_SCREEN
lv_obj_t *clock_screen_obj;
ui_screen_t clock_screen =
{
	.init = clock_screen_init,
	.deinit = clock_screen_deinit,
	.obj = &clock_screen_obj,
};


static void clock_screen_quit_event(lv_event_t * e);
static void clock_screen_add_clock_event(lv_event_t * e);
static void clock_screen_clock_switch_event(lv_event_t * e);
static void clock_screen_clock_click_event(lv_event_t * e);
static void clock_del_msgbox_event(lv_event_t * e);
lv_obj_t *clock_item_obj[10];
lv_obj_t *clock_list_obj;
void clock_screen_updata_list(void);
/*-------------------------------------------------------------
                        页面初始化/销毁函数
--------------------------------------------------------------*/
void clock_screen_init(lv_obj_t *parent)
{
    clock_screen_obj = lv_obj_create(parent);
    lv_obj_set_size(clock_screen_obj, lv_pct(100), lv_pct(100));
    lv_obj_clear_flag(clock_screen_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(clock_screen_obj, lv_color_hex(0xF7F7F7), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(clock_screen_obj, LV_OPA_100, LV_STATE_DEFAULT);

    // CLOCK
    lv_obj_t *clock_label_obj = lv_obj_create(clock_screen_obj);
    lv_obj_remove_style_all(clock_label_obj);
    lv_obj_align(clock_label_obj, LV_ALIGN_TOP_LEFT, 25, 0);
    lv_obj_set_size(clock_label_obj, 80, 60);
    lv_obj_t *label_obj = lv_label_create(clock_label_obj);
    lv_obj_align(label_obj, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(label_obj, "\xE9\x80\x80\xE5\x87\xBA"/*返回*/);//退出
    lv_obj_set_style_text_color(label_obj, lv_color_hex(0x313131), LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label_obj, &_lvgl_font2, LV_STATE_DEFAULT);
    lv_obj_add_event_cb(clock_label_obj, clock_screen_quit_event, LV_EVENT_CLICKED, NULL);

    // add btn
    lv_obj_t *clock_add_btn = lv_btn_create(clock_screen_obj);
    lv_obj_align(clock_add_btn, LV_ALIGN_TOP_RIGHT, -25, 5);
    lv_obj_set_size(clock_add_btn, 60, 40);
    label_obj = lv_label_create(clock_add_btn);
    lv_obj_align(label_obj, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(label_obj, "add");
    lv_obj_add_event_cb(clock_add_btn, clock_screen_add_clock_event, LV_EVENT_CLICKED, NULL);

	clock_screen_updata_list();
}



void clock_screen_updata_list(void)
{
    // CLOCK LIST
    clock_list_obj = lv_obj_create(clock_screen_obj);
    lv_obj_remove_style_all(clock_list_obj);
    lv_obj_align(clock_list_obj, LV_ALIGN_TOP_LEFT, 0, 60);
    lv_obj_set_size(clock_list_obj, lv_pct(100), 400);
	if(AlarmControl.get_count() == 0)
	{
		lv_obj_t *label = lv_label_create(clock_list_obj);
		lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
		lv_label_set_text(label, "\xE8\xBF\x98\xE6\xB2\xA1\xE6\x9C\x89\xE6\xB7\xBB\xE5\x8A\xA0\xE9\x97\xB9\xE9\x92\x9F");// 还没有添加闹钟
		lv_obj_set_style_text_font(label, &_lvgl_font3, LV_STATE_DEFAULT);
	}
	else
	{
		for(int i = 0; i < AlarmControl.get_count(); i++)
		{
			time_alarm_t alarm = AlarmControl.get_alarm(i);
			
			clock_item_obj[i] = lv_obj_create(clock_list_obj);
			lv_obj_remove_style_all(clock_item_obj[i]);
			lv_obj_clear_flag(clock_item_obj[i], LV_OBJ_FLAG_SCROLLABLE);
			lv_obj_add_flag(clock_item_obj[i],LV_OBJ_FLAG_CLICKABLE);
			lv_obj_set_size(clock_item_obj[i], lv_pct(90), 80);
			lv_obj_align(clock_item_obj[i], LV_ALIGN_TOP_MID, 0, 85*i);
			lv_obj_set_style_bg_color(clock_item_obj[i], lv_color_hex(0xffffff), LV_STATE_DEFAULT);
			lv_obj_set_style_bg_opa(clock_item_obj[i], LV_OPA_100, LV_STATE_DEFAULT);
			lv_obj_set_style_radius(clock_item_obj[i], 10, LV_STATE_DEFAULT);
			lv_obj_add_event_cb(clock_item_obj[i], clock_screen_clock_click_event, LV_EVENT_LONG_PRESSED, (void*)i);
			
			
			lv_obj_t *label = lv_label_create(clock_item_obj[i]);
			lv_obj_align(label, LV_ALIGN_TOP_LEFT, 5, 15);
			lv_label_set_text_fmt(label, "%02d:%02d", alarm.hour, alarm.minute);
			lv_obj_set_style_text_color(label, lv_color_hex(0x989898), LV_STATE_DEFAULT);
			lv_obj_set_style_text_font(label, &_lvgl_font3, LV_STATE_DEFAULT);

			lv_obj_t *btn = lv_switch_create(clock_item_obj[i]);
			lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -15, 0);
			lv_obj_add_event_cb(btn, clock_screen_clock_switch_event, LV_EVENT_CLICKED, (void*)i);
			if(alarm.enable) lv_obj_add_state(btn, LV_STATE_CHECKED);
			else lv_obj_clear_state(btn, LV_STATE_CHECKED);
		}
	}

}


void clock_screen_deinit(void)
{
	if (clock_screen_obj)
    {
        lv_obj_del(clock_screen_obj);
        clock_screen_obj = NULL;
    }
}






/*-------------------------------------------------------------
                        额外的消息框
--------------------------------------------------------------*/

void clock_create_del_msgbox(int i) 
{
    static const char * btns[] = {"纭畾", "鍙栨秷"};
    lv_obj_t * mbox = lv_msgbox_create(lv_layer_top(), "鎻愮ず", "鏄惁鍒犻櫎闂归挓", btns, false);
	lv_obj_set_style_text_font(mbox, &_lvgl_font2, LV_STATE_DEFAULT);
	
    lv_obj_center(mbox);
    lv_obj_add_event_cb(mbox, clock_del_msgbox_event, LV_EVENT_VALUE_CHANGED, (void*)i);
}


/*-------------------------------------------------------------
                        事件/定时器回调函数
--------------------------------------------------------------*/

static void  clock_screen_quit_event(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);

    if(event_code == LV_EVENT_CLICKED)
    {
        ui_screen_back();
    }
}


static void clock_screen_add_clock_event(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);

    if(event_code == LV_EVENT_CLICKED)
    {
        ui_screen_load(&clock_add_screen, &UI_SCREEN_LOAD_AMIM);
    }
}


static void clock_screen_clock_switch_event(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    int index = (int)lv_event_get_user_data(e);

    if(event_code == LV_EVENT_CLICKED)
    {
        bool state = lv_obj_has_state(target, LV_STATE_CHECKED);
        if(state == true)
        {
            lv_obj_add_state(target, LV_STATE_CHECKED);
			AlarmControl.enable(index, true);
            printf("kaii");
        }
        else
        {
            lv_obj_clear_state(target, LV_STATE_CHECKED);
			AlarmControl.enable(index, false);
            printf("guan");
        }
    }
}


static void clock_screen_clock_click_event(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    int index = (int)lv_event_get_user_data(e);

    if(event_code == LV_EVENT_LONG_PRESSED)
    {
		printf("SSS");
		clock_create_del_msgbox(index);
	}
}

static void clock_del_msgbox_event(lv_event_t * e)
{
    lv_obj_t * mbox = lv_event_get_current_target(e);
    const char * btn_txt = lv_msgbox_get_active_btn_text(mbox);
	int index = (int)lv_event_get_user_data(e);
	
    if(btn_txt)
    {
		if(strcmp(btn_txt, "纭畾") == 0) 
		{
			AlarmControl.del(index);
			if(clock_list_obj) lv_obj_del(clock_list_obj);
			clock_screen_updata_list();
		}
    }
	lv_msgbox_close(mbox);
}



#else


ui_screen_t clock_screen =
{
	.init = NULL,
	.deinit = NULL,
	.obj = NULL,
};


#endif


