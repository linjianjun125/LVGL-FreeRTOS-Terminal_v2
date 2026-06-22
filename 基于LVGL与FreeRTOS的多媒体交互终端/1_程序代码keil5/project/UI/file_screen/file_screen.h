#ifndef __FILE_SCREEN_H
#define __FILE_SCREEN_H

#include "../ui/screen.h"

#if USE_UI_FILE_SCREEN
void file_screen_init(lv_obj_t *parent);
void file_screen_deinit(void);
#endif

#endif
