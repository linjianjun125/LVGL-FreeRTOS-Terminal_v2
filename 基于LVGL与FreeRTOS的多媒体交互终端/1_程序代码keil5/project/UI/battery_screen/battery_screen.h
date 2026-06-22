#ifndef __UI_APP_BATTERY_SCREEN_H
#define __UI_APP_BATTERY_SCREEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../ui/screen.h"

#if USE_UI_BATTERY_SCREEN

void battery_screen_init(lv_obj_t *parent);
void battery_screen_deinit(void);

void battery_update_info(uint8_t percent, float voltage, bool is_charging);

#endif



#ifdef __cplusplus
}
#endif

#endif

