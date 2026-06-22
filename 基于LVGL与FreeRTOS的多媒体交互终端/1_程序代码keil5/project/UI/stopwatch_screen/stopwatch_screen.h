#ifndef __UI_APP_STOPWATCH_SCREEN_H
#define __UI_APP_STOPWATCH_SCREEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../ui/screen.h"

#if USE_UI_STOPWATCH_SCREEN
/**
 * @brief 秒表界面全局对象
 * 供 UI 管理器统一调用加载
 */
extern ui_screen_t stopwatch_screen;

/**
 * @brief 初始化秒表界面
 * 创建定时器、UI 布局及交互事件
 * @param parent 指向父容器对象的指针
 */
void stopwatch_screen_init(lv_obj_t *parent);

/**
 * @brief 释放秒表界面资源
 * 必须在此处停止并删除 10ms 高频定时器以节省 CPU 资源
 */
void stopwatch_screen_deinit(void);

#endif


#ifdef __cplusplus
}
#endif

#endif /* __UI_APP_STOPWATCH_SCREEN_H */
