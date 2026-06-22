#include "game2048_screen.h"

#if USE_UI_GAME2048_SCREEN
#include "ex/game2048.h"

lv_obj_t *game2048_screen_obj;
ui_screen_t game2048_screen =
{
	.init = game2048_screen_init,
	.deinit = game2048_screen_deinit,
	.obj = &game2048_screen_obj,
};

LV_FONT_DECLARE(_lvgl_font2)


static int game2048_board[GAME2048_SIZE][GAME2048_SIZE];

static lv_obj_t* game2048_map_obj[GAME2048_SIZE][GAME2048_SIZE];
static lv_obj_t* game2048_map_label[GAME2048_SIZE][GAME2048_SIZE];
static lv_obj_t* game2048_return_button;
static lv_obj_t* game2048_restart_button;
static void game2048_updata_gesture_event(lv_event_t * e);
static void game2048_screen_break_event(lv_event_t * e);
static void game2048_restart_button_clicked_event(lv_event_t * e);
static void game2048_updata(void);


/*-------------------------------------------------------------
                        页面初始化/销毁函数
--------------------------------------------------------------*/

void game2048_screen_init(lv_obj_t *parent)
{
    lv_obj_t *label;
    game2048_initBoard(game2048_board);

    // 创建界面
    game2048_screen_obj = lv_obj_create(parent);
    lv_obj_set_size(game2048_screen_obj, lv_pct(100), lv_pct(100));
    lv_obj_clear_flag(game2048_screen_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(game2048_screen_obj, lv_color_hex(ui_screen_default_bg_color()), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(game2048_screen_obj, LV_OPA_100, LV_STATE_DEFAULT);
	
    // 返回按钮
    game2048_return_button = lv_btn_create(game2048_screen_obj);
    lv_obj_set_size(game2048_return_button, 60, 50);
    lv_obj_align(game2048_return_button, LV_ALIGN_TOP_RIGHT, -60, 20);
    label = lv_label_create(game2048_return_button);
    lv_label_set_text(label,"退出");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_text_font(label, &_lvgl_font2, LV_STATE_DEFAULT);

    // 重新开始按钮
    game2048_restart_button = lv_btn_create(game2048_screen_obj);
    lv_obj_set_size(game2048_restart_button, 60, 50);
    lv_obj_align(game2048_restart_button, LV_ALIGN_TOP_LEFT, 60, 20);
    label = lv_label_create(game2048_restart_button);
    lv_label_set_text(label,"重新");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_text_font(label, &_lvgl_font2, LV_STATE_DEFAULT);

    // 创建游戏地图
    lv_obj_t *game2048_map = lv_obj_create(game2048_screen_obj);
    lv_obj_set_size(game2048_map, 80*GAME2048_SIZE + 42, 80*GAME2048_SIZE + 42);
    lv_obj_align(game2048_map, LV_ALIGN_CENTER, 0, 20);
    lv_obj_clear_flag(game2048_map, LV_OBJ_FLAG_SCROLLABLE);
    for (int i = 0; i < GAME2048_SIZE; i++)
    {
        for (int j = 0; j < GAME2048_SIZE; j++)
        {
            game2048_map_obj[i][j] = lv_obj_create(game2048_map);
            lv_obj_clear_flag(game2048_map_obj[i][j], LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_size(game2048_map_obj[i][j], 75, 75);
            lv_obj_align(game2048_map_obj[i][j], LV_ALIGN_TOP_LEFT, 80 * j + 3, 80 * i + 3);
            lv_obj_set_style_bg_color(game2048_map_obj[i][j], lv_color_hex(0xd9d9d9), 0);
            lv_obj_set_style_bg_opa(game2048_map_obj[i][j], LV_OPA_100, 0);

            game2048_map_label[i][j] = lv_label_create(game2048_map_obj[i][j]);
            lv_label_set_text(game2048_map_label[i][j]," ");
            lv_obj_align(game2048_map_label[i][j], LV_ALIGN_CENTER, 0, 0);
            lv_label_set_long_mode(game2048_map_label[i][j], LV_LABEL_LONG_CLIP);
            lv_obj_set_style_text_color(game2048_map_label[i][j], lv_color_hex(0xffffff), LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(game2048_map_label[i][j], &_lvgl_font2, LV_STATE_DEFAULT);
		}
	}
	game2048_updata();

	// event
	lv_obj_add_event_cb(game2048_screen_obj, game2048_updata_gesture_event, LV_EVENT_GESTURE, NULL);
    lv_obj_add_event_cb(game2048_restart_button, game2048_restart_button_clicked_event, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(game2048_return_button, game2048_screen_break_event, LV_EVENT_CLICKED, NULL);
}

void game2048_screen_deinit(void)
{
    if (game2048_screen_obj)
    {
        lv_obj_del(game2048_screen_obj);
        game2048_screen_obj = NULL;
    }
}


/*-------------------------------------------------------------
                        事件/定时器回调函数
--------------------------------------------------------------*/
static void game2048_screen_break_event(lv_event_t * e)
{
	ui_screen_back();
}


static void game2048_restart_button_clicked_event(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);

    if(event_code == LV_EVENT_CLICKED)
    {
        game2048_initBoard(game2048_board);
        game2048_updata();
    }
}




static void game2048_updata_gesture_event(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    if(event_code == LV_EVENT_GESTURE)
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if(dir == LV_DIR_TOP)
        {
            if (game2048_moveUp(game2048_board)){
                game2048_generateNewNumber(game2048_board);
            }
        }
        else if(dir == LV_DIR_BOTTOM)
        {
            if (game2048_moveDown(game2048_board)){
                game2048_generateNewNumber(game2048_board);
            }
        }
        else if(dir == LV_DIR_LEFT)
        {
            if (game2048_moveLeft(game2048_board)){
                game2048_generateNewNumber(game2048_board);
            }
        }
        else if(dir == LV_DIR_RIGHT)
        {
            if (game2048_moveRight(game2048_board)){
                game2048_generateNewNumber(game2048_board);
            }
        }
        game2048_updata();
    }
}




const char* game2048_texts[32] =
{
    " ",     "2",     "4",    "8",   "16",
    "32",    "64",    "128",  "256", "512",
    "1024",  "2048",  "4096", "8192"
};

uint32_t game2048_colors[32] =
{
    0xd9d9d9, 0x725280, 0x560320, 0xa12345, 0x83f393,
    0x277f63, 0x13d663, 0x228320, 0xa12345, 0xc36393,
    0x2f7663, 0x287633, 0xa12080, 0x123345
};

static void game2048_updata(void)
{
    for (int i = 0; i < GAME2048_SIZE; i++)
    {
        for (int j = 0; j < GAME2048_SIZE; j++)
        {
            int index = game2048_getIndex(game2048_board[i][j]);
            const lv_font_t* font = (index < 10) ? &_lvgl_font2 : &_lvgl_font1;

            lv_obj_set_style_bg_color(game2048_map_obj[i][j], lv_color_hex(game2048_colors[index]), 0);
            lv_obj_set_style_text_font(game2048_map_label[i][j], font, LV_STATE_DEFAULT);
            lv_label_set_text(game2048_map_label[i][j],game2048_texts[index]);
        }
    }
}

#else

ui_screen_t game2048_screen =
{
	.init = NULL,
	.deinit = NULL,
	.obj = NULL,
};



#endif


