#ifndef __SYSTEM_SCREEN_H
#define __SYSTEM_SCREEN_H

#include "../ui/screen.h"

#if USE_UI_SYSTEM_SCREEN
void system_screen_init(lv_obj_t *parent);
void system_screen_deinit(void);

#endif

#endif
