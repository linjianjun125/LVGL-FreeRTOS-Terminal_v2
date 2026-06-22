#ifndef __APP_SELECT_SCREEN_H
#define __APP_SELECT_SCREEN_H

#include "../ui/screen.h"

#if USE_UI_APP_SELECT_SCREEN

// 应用选择屏幕初始化
void app_select_screen_init(lv_obj_t * parent);

// 应用选择屏幕销毁
void app_select_screen_deinit(void);

#endif

#endif

