#ifndef __NOVEL_SELECT_SCREEN_H
#define __NOVEL_SELECT_SCREEN_H

#include "../ui/screen.h"

#if USE_UI_NOVEL_SELECT_SCREEN
/**
 * @brief 小说选择列表屏幕初始化
 * @param parent: 父容器对象（通常是全屏容器）
 */
void novel_select_screen_init(lv_obj_t * parent);

/**
 * @brief 小说选择列表屏幕销毁
 */
void novel_select_screen_deinit(void);
#endif


#endif
