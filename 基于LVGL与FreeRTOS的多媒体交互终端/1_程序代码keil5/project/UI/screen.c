#include "screen.h"

typedef struct {
    ui_screen_t** screen;
    int8_t top;
    uint8_t size;
} screen_stack_t;

screen_stack_t screen_stack;
ui_screen_load_anim_t UI_SCREEN_LOAD_AMIM = {LV_SCR_LOAD_ANIM_NONE, LV_SCR_LOAD_ANIM_NONE, 300, 300};

uint32_t ui_screen_default_bg_color(void)
{
	return 0xEDEDED;
}

void ui_screen_init(uint8_t size, ui_screen_t* load_screen)
{
    screen_stack.screen = lv_mem_alloc(size * sizeof(ui_screen_t*));
    screen_stack.top = 0;
    screen_stack.size = size;

    load_screen->init(NULL);
    lv_disp_load_scr(*(load_screen->obj));
    screen_stack.screen[screen_stack.top] = load_screen;
}


uint8_t ui_screen_load(ui_screen_t* load_screen, ui_screen_load_anim_t *screen_load_anim)
{
	// 获取上一个界面
    ui_screen_t* prev_screen = NULL;
	prev_screen = screen_stack.screen[screen_stack.top];
	
    // 判断页面是否为空
    if(prev_screen == NULL || prev_screen->init == NULL || prev_screen->deinit == NULL)
    {
        LV_LOG_WARN("prev_screen == NULL || prev_screen->deinit == NULL");
        return 1;
    }
    if(load_screen == NULL || load_screen->init == NULL || load_screen->deinit == NULL){
        LV_LOG_WARN("load_screen->init == NULL || load_screen->init == NULL");
        return 1;
    }
	
	// 更新界面
    load_screen->init(NULL);
	lv_disp_load_scr(*load_screen->obj);
	prev_screen->deinit();
	//if(*prev_screen->obj != NULL)	 lv_obj_del(*prev_screen->obj);
	
	
	
	memcpy(&load_screen->anim, screen_load_anim, sizeof(ui_screen_load_anim_t));
    screen_stack.top ++;
    screen_stack.screen[screen_stack.top] = load_screen;
    if(screen_stack.top > screen_stack.size)
    {
        LV_LOG_WARN("screen_stack.top > size");
        return 1;
    }

    return 0;
}




uint8_t ui_screen_back(void)
{
    ui_screen_t* prev_screen;
    ui_screen_t* load_screen;

	// 获取当前界面和上一个界面
    prev_screen = screen_stack.screen[screen_stack.top];
    load_screen = screen_stack.screen[screen_stack.top-1];
	
	// 判断页面是否为空
    if(prev_screen == NULL || prev_screen->init == NULL || prev_screen->deinit == NULL)
    {
        LV_LOG_WARN("prev_screen == NULL || prev_screen->deinit == NULL");
        return 1;
    }
    if(load_screen == NULL || load_screen->init == NULL || load_screen->deinit == NULL){
        LV_LOG_WARN("load_screen->init == NULL || load_screen->init == NULL");
        return 1;
    }
    
	
	// 更新界面
    load_screen->init(NULL);
	lv_disp_load_scr(*load_screen->obj);
	prev_screen->deinit();
	screen_stack.top--;
	
    return 0;
}












