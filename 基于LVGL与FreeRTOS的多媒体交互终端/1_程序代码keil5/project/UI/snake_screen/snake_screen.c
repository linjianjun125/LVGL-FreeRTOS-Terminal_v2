#include "snake_screen.h"

#if USE_UI_SNAKE_SCREEN
#include "ex/snake.h"
#include <string.h>

/*--- 1. 游戏区域网格与尺寸宏定义 ---*/
#define GRID_COLS          SNAKE_WIDTH   
#define GRID_ROWS          SNAKE_HEIGHT  
#define CELL_SIZE          20  

// 宽度和高度分开计算
#define GAME_AREA_W        (GRID_COLS * CELL_SIZE)
#define GAME_AREA_H        (GRID_ROWS * CELL_SIZE)

/* 引用外部公用 SDRAM 缓冲区 (与贪吃蛇一致) */
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
    extern uint8_t game_common_buffer[480 * 480 * 2];
#else
    extern uint8_t game_common_buffer[480 * 480 * 2];
#endif

/*--- 3. 全局变量与前向声明 ---*/
lv_obj_t *snake_screen_obj;
static lv_obj_t *snake_canvas;
static lv_timer_t *snake_timer;
static lv_obj_t *snake_stop_label;

Snake snake;
LV_FONT_DECLARE(_lvgl_font2)

static void snake_updata(void);
static void snake_timer_cb(lv_timer_t * timer);
static void snake_event_handler(lv_event_t * e);

ui_screen_t snake_screen = {
    .init = snake_screen_init,
    .deinit = snake_screen_deinit,
    .obj = &snake_screen_obj,
};

/*-------------------------------------------------------------
                        页面主入口
--------------------------------------------------------------*/

void snake_screen_init(lv_obj_t *parent)
{
    snake_initSnake(&snake);

    // 1. 整体大背景：改为明亮的浅灰色
    snake_screen_obj = lv_obj_create(parent);
    lv_obj_set_size(snake_screen_obj, 480, 480);
    lv_obj_clear_flag(snake_screen_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(snake_screen_obj, lv_color_hex(0xF4F7F6), 0); 
    lv_obj_set_style_border_width(snake_screen_obj, 0, 0);

    // 2. 在 SDRAM 上创建画布：背景纯白
    snake_canvas = lv_canvas_create(snake_screen_obj);
    lv_canvas_set_buffer(snake_canvas, game_common_buffer, GAME_AREA_W, GAME_AREA_H, LV_IMG_CF_TRUE_COLOR);
    lv_obj_align(snake_canvas, LV_ALIGN_CENTER, 0, 30);
    lv_obj_set_style_outline_width(snake_canvas, 2, 0);
    lv_obj_set_style_outline_color(snake_canvas, lv_color_hex(0xE0E0E0), 0); // 给画布一个浅色边框

    // 3. 顶部按钮栏 (重开 | 暂停 | 退出)
    // 重新开始按钮
    lv_obj_t *btn_restart = lv_btn_create(snake_screen_obj);
    lv_obj_set_size(btn_restart, 75, 40);
    lv_obj_align(btn_restart, LV_ALIGN_TOP_LEFT, 20, 15);
    lv_obj_set_style_bg_color(btn_restart, lv_color_hex(0x3498DB), 0); // 蓝色
    lv_obj_t *l_res = lv_label_create(btn_restart);
    lv_label_set_text(l_res, "重开");
    lv_obj_set_style_text_font(l_res, &_lvgl_font2, 0);
    lv_obj_center(l_res);
    lv_obj_add_event_cb(btn_restart, snake_event_handler, LV_EVENT_CLICKED, (void*)"RESTART");

    // 暂停/继续按钮
    lv_obj_t *btn_stop = lv_btn_create(snake_screen_obj);
    lv_obj_set_size(btn_stop, 75, 40);
    lv_obj_align(btn_stop, LV_ALIGN_TOP_MID, 0, 15);
    lv_obj_set_style_bg_color(btn_stop, lv_color_hex(0x95A5A6), 0); // 灰色
    snake_stop_label = lv_label_create(btn_stop);
    lv_label_set_text(snake_stop_label, "暂停");
    lv_obj_set_style_text_font(snake_stop_label, &_lvgl_font2, 0);
    lv_obj_center(snake_stop_label);
    lv_obj_add_event_cb(btn_stop, snake_event_handler, LV_EVENT_CLICKED, (void*)"STOP");

    // 退出按钮
    lv_obj_t *btn_exit = lv_btn_create(snake_screen_obj);
    lv_obj_set_size(btn_exit, 75, 40);
    lv_obj_align(btn_exit, LV_ALIGN_TOP_RIGHT, -20, 15);
    lv_obj_set_style_bg_color(btn_exit, lv_color_hex(0xE74C3C), 0); // 红色
    lv_obj_t *l_exit = lv_label_create(btn_exit);
    lv_label_set_text(l_exit, "退出");
    lv_obj_set_style_text_font(l_exit, &_lvgl_font2, 0);
    lv_obj_center(l_exit);
    lv_obj_add_event_cb(btn_exit, snake_event_handler, LV_EVENT_CLICKED, (void*)"EXIT");

    // 4. 事件与定时器
    lv_obj_add_event_cb(snake_screen_obj, snake_event_handler, LV_EVENT_GESTURE, NULL);
    snake_timer = lv_timer_create(snake_timer_cb, 280, NULL);
    
    snake_updata();
}

void snake_screen_deinit(void)
{
    if(snake_timer) {
        lv_timer_del(snake_timer);
        snake_timer = NULL;
    }
    if (snake_screen_obj) {
        lv_obj_del(snake_screen_obj);
        snake_screen_obj = NULL;
    }
}

/*-------------------------------------------------------------
                        静态函数实现
--------------------------------------------------------------*/

static void snake_updata(void)
{
    // 画布背景：白色
    lv_canvas_fill_bg(snake_canvas, lv_color_hex(0xFFFFFF), LV_OPA_COVER);

    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.bg_opa = LV_OPA_COVER;

    // 绘制蛇身：清新绿
    for (int k = 0; k < snake.length; k++) {
        // 蛇头深绿，蛇身浅绿
        rect_dsc.bg_color = (k == 0) ? lv_color_hex(0x27AE60) : lv_color_hex(0x2ECC71);
        rect_dsc.radius = 3;
        lv_canvas_draw_rect(snake_canvas, 
                            snake.x[k] * CELL_SIZE + 1, 
                            snake.y[k] * CELL_SIZE + 1, 
                            CELL_SIZE - 2, 
                            CELL_SIZE - 2, 
                            &rect_dsc);
    }

    // 绘制食物：亮红色
    rect_dsc.bg_color = lv_color_hex(0xE74C3C);
    rect_dsc.radius = LV_RADIUS_CIRCLE;
    lv_canvas_draw_rect(snake_canvas, 
                        snake.food.x * CELL_SIZE + 3, 
                        snake.food.y * CELL_SIZE + 3, 
                        CELL_SIZE - 6, 
                        CELL_SIZE - 6, 
                        &rect_dsc);
}

static void snake_timer_cb(lv_timer_t * timer)
{
    if(snake.stop || snake_isGameOver(&snake)) return;
    snake_move(&snake);
    snake_updata();
}

static void snake_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    void * user_data = lv_event_get_user_data(e);

    if(code == LV_EVENT_CLICKED) {
        if(strcmp((char*)user_data, "EXIT") == 0) {
            ui_screen_back();
        }
        else if(strcmp((char*)user_data, "STOP") == 0) {
            snake.stop = !snake.stop;
            lv_label_set_text(snake_stop_label, snake.stop ? "继续" : "暂停");
        }
        else if(strcmp((char*)user_data, "RESTART") == 0) {
            snake_initSnake(&snake);
            snake_updata();
        }
    }
    else if(code == LV_EVENT_GESTURE) {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if(dir == LV_DIR_BOTTOM)      snake_input(&snake, SNAKE_DOWN);
        else if(dir == LV_DIR_TOP)    snake_input(&snake, SNAKE_UP);
        else if(dir == LV_DIR_LEFT)   snake_input(&snake, SNAKE_LEFT);
        else if(dir == LV_DIR_RIGHT)  snake_input(&snake, SNAKE_RIGHT);
    }
}

#endif
