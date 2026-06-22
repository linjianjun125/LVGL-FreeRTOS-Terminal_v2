#ifndef __CLOCK_SCREEN_H
#define __CLOCK_SCREEN_H



#include "../ui/screen.h"

#if USE_UI_CLOCK_SCREEN
void clock_screen_init(lv_obj_t *parent);
void clock_screen_deinit(void);

#endif

#endif
