#include "tetris_screen.h"

#if USE_UI_TETRIS_SCREEN
#include "ex/tetris.h"
#include <string.h>

/*--- 1. 游戏区域尺寸与位置定义 ---*/
#define TETRIS_CELL_SIZE    22  
#define TETRIS_CANVAS_W     (TETRIS_WIDTH * TETRIS_CELL_SIZE)
#define TETRIS_CANVAS_H     (TETRIS_HEIGHT * TETRIS_CELL_SIZE)

/*--- 2. UI 专属颜色查找表 (解耦合) ---*/
static uint32_t get_tetris_color(int type) {
    static const uint32_t colors[] = {
        0x3498DB, // 0: I (天蓝)
        0xE67E22, // 1: J (橙色)
        0xF1C40F, // 2: L (黄色)
        0x9B59B6, // 3: O (紫色)
        0x2ECC71, // 4: S (绿色)
        0x1ABC9C, // 5: T (青色)
        0xE74C3C, // 6: Z (红色)
        0xFFFFFF  // 7: 背景 (白色)
    };
    if (type >= 0 && type <= 7) return colors[type];
    return colors[7];
}

/*--- 3. 变量声明 ---*/
lv_obj_t *tetris_screen_obj;
static lv_obj_t *tetris_canvas;
static lv_timer_t *tetris_timer;
static lv_obj_t *tetris_stop_label;

Tetris tetris;
LV_FONT_DECLARE(_lvgl_font2)

/* 引用外部公用 SDRAM 缓冲区 (与贪吃蛇一致) */
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
    extern uint8_t game_common_buffer[480 * 480 * 2];
#else
    extern uint8_t game_common_buffer[480 * 480 * 2];
#endif

static void tetris_updata(void);
static void tetris_timer_cb(lv_timer_t * timer);
static void tetris_event_handler(lv_event_t * e);

ui_screen_t tetris_screen = {
    .init = tetris_screen_init,
    .deinit = tetris_screen_deinit,
    .obj = &tetris_screen_obj,
};

/*-------------------------------------------------------------
                        页面初始化
--------------------------------------------------------------*/

void tetris_screen_init(lv_obj_t *parent)
{
    tetris_initGame(&tetris);

    // 1. 整体大背景
    tetris_screen_obj = lv_obj_create(parent);
    lv_obj_set_size(tetris_screen_obj, 480, 480);
    lv_obj_clear_flag(tetris_screen_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(tetris_screen_obj, lv_color_hex(0xF4F7F6), 0);
    lv_obj_set_style_border_width(tetris_screen_obj, 0, 0);

    // 2. 创建画布：靠左显示
    tetris_canvas = lv_canvas_create(tetris_screen_obj);
    lv_canvas_set_buffer(tetris_canvas, game_common_buffer, TETRIS_CANVAS_W, TETRIS_CANVAS_H, LV_IMG_CF_TRUE_COLOR);
    lv_obj_align(tetris_canvas, LV_ALIGN_LEFT_MID, 40, 40); 
    lv_obj_set_style_outline_width(tetris_canvas, 2, 0);
    lv_obj_set_style_outline_color(tetris_canvas, lv_color_hex(0xDDDDDD), 0);

    // 3. 顶部按钮 (实现居中分布)
    const char* btn_types[] = {"RESTART", "STOP", "EXIT"};
    const char* btn_texts[] = {"重开", "暂停", "退出"};
    lv_color_t colors[] = {lv_color_hex(0x3498DB), lv_color_hex(0x95A5A6), lv_color_hex(0xE74C3C)};
    
    for(int i=0; i<3; i++) {
        lv_obj_t *btn = lv_btn_create(tetris_screen_obj);
        lv_obj_set_size(btn, 85, 45);
        // 使用水平偏移实现三按钮均匀居中分布：-120, 0, 120
        lv_obj_align(btn, LV_ALIGN_TOP_MID, (i - 1) * 135, 20);
        lv_obj_set_style_bg_color(btn, colors[i], 0);
        lv_obj_set_style_radius(btn, 8, 0);
        
        lv_obj_t *l = lv_label_create(btn);
        lv_label_set_text(l, btn_texts[i]);
        lv_obj_set_style_text_font(l, &_lvgl_font2, 0);
        lv_obj_center(l);
        
        if(i == 1) tetris_stop_label = l;
        lv_obj_add_event_cb(btn, tetris_event_handler, LV_EVENT_CLICKED, (void*)btn_types[i]);
    }

    // 4. 右侧方向控制键
// 5. 右侧方向控制键 (增加“上”键，旋转居中)
    const char* dir_types[] = {"UP", "ROTATE", "LEFT", "RIGHT", "DOWN"};
    const char* dir_icons[] = {"上", "旋", "左", "右", "下"}; 
    
    for(int i=0; i<5; i++) {
        lv_obj_t *btn = lv_btn_create(tetris_screen_obj);
        lv_obj_set_size(btn, 60, 60); // 稍微调小一点，防止 5 个键太拥挤
        
        if(i==0) lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -70, -170); // 上 (无效键)
        if(i==1) lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -70, -105); // 旋 (中心)
        if(i==2) lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -135, -105); // 左
        if(i==3) lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -5, -105);   // 右
        if(i==4) lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -70, -40);   // 下 (距离底部40)
        
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x2C3E50), 0);
        lv_obj_set_style_radius(btn, 12, 0);
        
        // 如果是无效的“上”键，可以调淡一点颜色作为区分
        if(i == 0) lv_obj_set_style_bg_opa(btn, LV_OPA_40, 0);

        lv_obj_t *l = lv_label_create(btn);
        lv_label_set_text(l, dir_icons[i]);
        lv_obj_set_style_text_font(l, &_lvgl_font2, 0);
        lv_obj_center(l);
        
        lv_obj_add_event_cb(btn, tetris_event_handler, LV_EVENT_CLICKED, (void*)dir_types[i]);
    }

    // 5. 定时器
    tetris_timer = lv_timer_create(tetris_timer_cb, 550, NULL);
    tetris_updata();
}

/*-------------------------------------------------------------
                        渲染与逻辑
--------------------------------------------------------------*/

static void tetris_updata(void)
{
    // 1. 画布背景 (纯白)
    lv_canvas_fill_bg(tetris_canvas, lv_color_hex(0xFFFFFF), LV_OPA_COVER);

    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.bg_opa = LV_OPA_COVER;
    rect_dsc.radius = 2;

    // 2. 画出面板已固定的块
    for (int i = 0; i < TETRIS_HEIGHT; i++) {
        for (int j = 0; j < TETRIS_WIDTH; j++) {
            if(tetris.board[j][i] != Tetris_Board_None) {
                rect_dsc.bg_color = lv_color_hex(get_tetris_color(tetris.board[j][i]));
                lv_canvas_draw_rect(tetris_canvas, j*TETRIS_CELL_SIZE+1, i*TETRIS_CELL_SIZE+1, 
                                    TETRIS_CELL_SIZE-2, TETRIS_CELL_SIZE-2, &rect_dsc);
            }
        }
    }

    // 3. 画出当前下落的方块
    rect_dsc.bg_color = lv_color_hex(get_tetris_color(tetris.current.type));
    for (int k = 0; k < 4; k++) {
        for (int l = 0; l < 4; l++) {
            if (shapes[tetris.current.type][tetris.current.rotation][k][l] == 1) {
                int draw_x = tetris.current.x + k;
                int draw_y = tetris.current.y + l;
                if(draw_x >= 0 && draw_x < TETRIS_WIDTH && draw_y >= 0 && draw_y < TETRIS_HEIGHT) {
                    lv_canvas_draw_rect(tetris_canvas, draw_x*TETRIS_CELL_SIZE+1, draw_y*TETRIS_CELL_SIZE+1, 
                                        TETRIS_CELL_SIZE-2, TETRIS_CELL_SIZE-2, &rect_dsc);
                }
            }
        }
    }
}

static void tetris_event_handler(lv_event_t * e)
{
    char* type = (char*)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);

    if(code != LV_EVENT_CLICKED) return;

    // 退出处理：防止卡死，先停定时器
    if(strcmp(type, "EXIT") == 0) {
        if(tetris_timer) lv_timer_pause(tetris_timer);
        ui_screen_back();
        return;
    }
    
    // 暂停处理
    if(strcmp(type, "STOP") == 0) {
        tetris.stop = !tetris.stop;
        lv_label_set_text(tetris_stop_label, tetris.stop ? "继续" : "暂停");
    }
    // 重新开始
    else if(strcmp(type, "RESTART") == 0) {
        tetris_initGame(&tetris);
        if(tetris_timer) lv_timer_resume(tetris_timer);
        lv_label_set_text(tetris_stop_label, "暂停");
    }
    // 方向操作
    else {
        if(tetris.stop || tetris.isover) return;
        if(strcmp(type, "ROTATE") == 0)      tetris_rotate(&tetris);
        else if(strcmp(type, "LEFT") == 0)   tetris_moveLeft(&tetris);
        else if(strcmp(type, "RIGHT") == 0)  tetris_moveRight(&tetris);
        else if(strcmp(type, "DOWN") == 0)   tetris_moveDown(&tetris);
    }
    
    tetris_updata();
}

static void tetris_timer_cb(lv_timer_t * timer)
{
    if(tetris.stop || tetris.isover) return;
    tetris_moveDown(&tetris);
    tetris_updata();
}

void tetris_screen_deinit(void)
{
    if(tetris_timer) { 
        lv_timer_del(tetris_timer); 
        tetris_timer = NULL; 
    }
    if (tetris_screen_obj) { 
        lv_obj_del(tetris_screen_obj); 
        tetris_screen_obj = NULL; 
    }
}

#endif
