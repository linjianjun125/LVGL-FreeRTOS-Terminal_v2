#ifndef __SYSTEM_SCREEN_EX_H
#define __SYSTEM_SCREEN_EX_H

#include "../ui/screen.h"

#if  USE_UI_SYSTEM_SCREEN
// 定义函数指针类型
typedef lv_obj_t* (*system_screen_menu_create_fn_t)(lv_obj_t *parent);
typedef void (*system_screen_menu_del_fn_t)(lv_obj_t *menu_obj);

// 定义枚举索引
typedef enum
{
    SYSTEM_MENU_SOUND = 0,
    SYSTEM_MENU_TIME,
    SYSTEM_MENU_BRIGHTNESS,
    SYSTEM_MENU_ABOUT,
    SYSTEM_MENU_ABOUT_,
    SYSTEM_MENU_MAX
} system_menu_index_t;


typedef enum {
    BL_TIMEOUT_15S    = 150,   			// 150 * 100ms = 15s
    BL_TIMEOUT_30S    = 300,   			// 300 * 100ms = 30s
    BL_TIMEOUT_1M     = 600,   			// 600 * 100ms = 60s
    BL_TIMEOUT_2M     = 1200,  			// 1200 * 100ms = 120s
    BL_TIMEOUT_NEVER  = 0      			// 永不熄屏
} bl_timeout_val_t;





lv_obj_t * system_menu_switch_create(lv_obj_t *parent_obj, system_menu_index_t index);
void system_menu_switch_del(lv_obj_t *menu_obj, system_menu_index_t index);


#endif


#endif
