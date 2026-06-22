#ifndef NOVEL_SCREEN_H
#define NOVEL_SCREEN_H

#include "../ui/screen.h"


#if USE_UI_NOVEL_SCREEN



typedef enum {
    NOVEL_OP_NEXT_PAGE = 0,    // 下一页
    NOVEL_OP_PREV_PAGE,        // 上一页
    NOVEL_OP_NEXT_CHAPTER,     // 下一章
    NOVEL_OP_PREV_CHAPTER,     // 上一章
    NOVEL_OP_RELOAD            // 加载/刷新当前页
} NovelOp_t;



/* 初始化小说界面 */
void novel_screen_init(lv_obj_t *parent);

/* 销毁/释放小说界面 */
void novel_screen_deinit(void);

#endif

#endif

