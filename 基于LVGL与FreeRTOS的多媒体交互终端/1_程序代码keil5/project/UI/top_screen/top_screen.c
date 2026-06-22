#include "top_screen.h"
#include "../../devices/key/key.h"
#include "../../app/service/at24cxx_data.h"
#include "../../devices/lcdrgb/lcdrgb.h"
/*-------------------------------------------------------------
                        关机提示消息
--------------------------------------------------------------*/
static void shutdown_msgbox_event(lv_event_t * e);
lv_obj_t *top_screen_create_shutdown_msgbox(void* use_data)
{
	static const char * btns[] = {"取消", "确定", ""};
	lv_obj_t * power_msgbox = lv_msgbox_create(lv_layer_top(), "关机", "是否关机？", btns, false);
	lv_obj_set_style_text_font(power_msgbox, &_lvgl_font2, LV_STATE_DEFAULT);
	lv_obj_center(power_msgbox);
	lv_obj_add_event_cb(power_msgbox, shutdown_msgbox_event, LV_EVENT_VALUE_CHANGED, use_data);
	return power_msgbox;
}


static void shutdown_msgbox_event(lv_event_t * e)
{
    lv_obj_t * mbox = lv_event_get_current_target(e);
    const char * btn_txt = lv_msgbox_get_active_btn_text(mbox);
    if(btn_txt)
    {
		if(strcmp(btn_txt, "确定") == 0) 
		{
			lcdrgb_set_pwm_duty_cycle(0);
			at24cxx_set_volume_manager_info(&g_active_vol_cfg);
			 at24cxx_set_backlight_manager_info(&g_active_bl_cfg);
			at24cxx_set_novel_manager_info(&NovelControl);
			//at24cxx_set_alarm_manager_info(&AlarmControl.config);
			at24cxx_save_music_list(&MusicControl.g_music_list);
			___POWER_DISABLE();
		}
    }
	lv_msgbox_close(mbox);
}








