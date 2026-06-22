#include "calculator_screen.h"

#if USE_UI_CALCULATOR_SCREEN
#include "ex/tinyexpr.h"
#include <string.h>

lv_obj_t *calculator_srceen_obj;
ui_screen_t calculator_screen =
{
	.init = calculator_screen_init,
	.deinit = calculator_screen_deinit,
	.obj = &calculator_srceen_obj
};


static lv_obj_t * calculor_btnmat_obj;
static lv_obj_t * calculator_textarea_obj;
#define  calculator_delete 		"DEL"
static const char * ui_ComPageBtnmap[] =
{
    "log",      "ln",       "pow",      "exp",      "C",    calculator_delete,  "\n",
    "sin",      "cos",     "tan",       "sqrt",     "(",    ")",                "\n",
    "asin",     "acos",     "atan",     "abs",      "^",    ",",                "\n",
    "e",        "7",        "8",        "9",        "+",    "-",                "\n",
    "pi",       "4",        "5",        "6",        "*",    "/",                "\n",
    "0",        "1",        "2",        "3",        ".",    "=",                ""
};

static void calculator_screen_btnmat_clicked_event(lv_event_t* e);
static void calculator_screen_updata_gesture_event(lv_event_t * e);



/*-------------------------------------------------------------
                        页面初始化/销毁函数
--------------------------------------------------------------*/

void calculator_screen_init(lv_obj_t *parent)
{
    calculator_srceen_obj = lv_obj_create(parent);
    lv_obj_set_size(calculator_srceen_obj, lv_pct(100), lv_pct(100));
    lv_obj_clear_flag(calculator_srceen_obj,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(calculator_srceen_obj, lv_color_hex(ui_screen_default_bg_color()), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(calculator_srceen_obj, LV_OPA_100, LV_STATE_DEFAULT);
	
	// 显示部分
    calculator_textarea_obj = lv_textarea_create(calculator_srceen_obj);
    lv_obj_set_size(calculator_textarea_obj, lv_pct(93),  lv_pct(17));
    lv_obj_align(calculator_textarea_obj, LV_ALIGN_TOP_MID, 0, lv_pct(3));
    lv_obj_add_state(calculator_textarea_obj, LV_STATE_FOCUSED);
    lv_obj_clear_flag(calculator_textarea_obj,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_radius(calculator_textarea_obj, 0, 0);
    lv_obj_set_style_border_width(calculator_textarea_obj, 0, 0);
    lv_obj_set_style_text_font(calculator_textarea_obj, &_lvgl_font2, LV_STATE_DEFAULT);
    lv_textarea_set_align(calculator_textarea_obj, LV_TEXT_ALIGN_RIGHT);

	// 按键部分
    calculor_btnmat_obj = lv_btnmatrix_create(calculator_srceen_obj);
    lv_obj_clear_flag(calculor_btnmat_obj, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_set_size(calculor_btnmat_obj,lv_pct(100),  lv_pct(80));
    lv_obj_set_align(calculor_btnmat_obj,LV_ALIGN_BOTTOM_MID);
    lv_obj_set_style_border_width(calculor_btnmat_obj,0,0);
    lv_obj_set_style_bg_opa(calculor_btnmat_obj,0,0);
    lv_obj_set_style_text_font(calculor_btnmat_obj, &_lvgl_font2, LV_STATE_DEFAULT);
    lv_btnmatrix_set_map(calculor_btnmat_obj, ui_ComPageBtnmap);
    lv_btnmatrix_set_one_checked(calculor_btnmat_obj,true);
	lv_btnmatrix_set_btn_ctrl(calculor_btnmat_obj, 15, LV_BTNMATRIX_CTRL_RECOLOR);
	
	// event
    lv_obj_add_event_cb(calculator_srceen_obj, calculator_screen_updata_gesture_event, LV_EVENT_GESTURE, NULL);
    lv_obj_add_event_cb(calculor_btnmat_obj, calculator_screen_btnmat_clicked_event, LV_EVENT_PRESSED, calculator_textarea_obj);

}


void calculator_screen_deinit(void)
{
    if (calculator_srceen_obj)
    {
        lv_obj_del(calculator_srceen_obj);
        calculator_srceen_obj = NULL;
    }
}

/*-------------------------------------------------------------
                        事件/定时器回调函数
--------------------------------------------------------------*/
extern ui_screen_t music_screen;
static void calculator_screen_updata_gesture_event(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    if(event_code == LV_EVENT_GESTURE)
    {
         lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
         //if(dir == LV_DIR_LEFT)
         {
               // printf("[info] LV_DIR_LEFT\n");
                ui_screen_back();
         }
    }
}


void calculator_screen_btnmat_clicked_event(lv_event_t* e)
{
    const char *text;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t *show = lv_event_get_user_data(e);

    if (code == LV_EVENT_PRESSED)
    {
        text = lv_btnmatrix_get_btn_text(target, lv_btnmatrix_get_selected_btn(target));
		//printf("%s\n", text);

        // 计算结果
        if (strcmp(text, "=") == 0)
        {
            char result_string[128];
            const char *expression_string = lv_textarea_get_text(show);
            double r = te_interp(expression_string, 0);
            snprintf(result_string, sizeof(result_string), "%s = %3f", expression_string, r);
            lv_textarea_set_text(show, result_string);
        }
        // 清空全部
        else if (strcmp(text, "C") == 0)
        {
            lv_textarea_set_text(show, "");
        }
        // 删除一个字符
        else if (strcmp(text, calculator_delete) == 0)
        {
            lv_textarea_del_char(show);
        }
        // 追加字符
        else
        {
            lv_textarea_add_text(show, text);
        }
    }
}

#else

ui_screen_t calculator_screen =
{
	.init = NULL,
	.deinit = NULL,
	.obj = NULL
};

#endif



