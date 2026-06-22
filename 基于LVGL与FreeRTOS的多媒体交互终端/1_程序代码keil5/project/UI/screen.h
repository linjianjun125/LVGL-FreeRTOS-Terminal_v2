#ifndef __UI_SCREEN_H
#define __UI_SCREEN_H

#include "sys.h"
#include "devices_include.h"
#include "middle_include.h"
#include "app_include.h"
#include "ui_include.h"

// ---music screen---
#define USE_UI_MUSIC_SCREEN				1

// ---app screen---
#define USE_UI_APP_SELECT_SCREEN		1

#define USE_UI_GAME_SELECT_SCREEN		1
#if USE_UI_GAME_SELECT_SCREEN
	#define USE_UI_SNAKE_SCREEN			1
	#define USE_UI_TETRIS_SCREEN		1
	#define USE_UI_GAME2048_SCREEN		1
#endif

#define USE_UI_TOOL_SELECT_SCREEN		1
#if USE_UI_TOOL_SELECT_SCREEN
	#define USE_UI_CALCULATOR_SCREEN	1
	#define USE_UI_LEVEL_SCREEN			1
	#define USE_UI_COMPASS_SCREEN		1
	#define USE_UI_STOPWATCH_SCREEN		1
	#define USE_UI_BATTERY_SCREEN		1
#endif

#define USE_UI_CLOCK_SCREEN				0
#define USE_UI_FILE_SCREEN				1
#define USE_UI_SYSTEM_SCREEN			1

#define USE_UI_NOVEL_SELECT_SCREEN		1
#if USE_UI_NOVEL_SELECT_SCREEN
	#define USE_UI_NOVEL_SCREEN			1
#endif

#define USE_UI_CALENDAR_SCREEN			1
#define USE_UI_TRANSFER_SCREEN			0


#define USE_UI_WEATHER_SCREEN			1


LV_FONT_DECLARE(_lvgl_font1)
LV_FONT_DECLARE(_lvgl_font2)
LV_FONT_DECLARE(_lvgl_font3)
LV_FONT_DECLARE(_lvgl_font4)


typedef struct
{
    lv_scr_load_anim_t load_type;
    lv_scr_load_anim_t break_type;
    uint32_t load_time;
    uint32_t break_time;
}ui_screen_load_anim_t;


typedef struct
{
    lv_obj_t** obj;
    void (*init)(lv_obj_t *parent);
    void (*deinit)(void);
	ui_screen_load_anim_t anim;
}ui_screen_t;


extern ui_screen_load_anim_t UI_SCREEN_LOAD_AMIM;

// ---music screen---
extern ui_screen_t music_screen;

// ---app screen---
extern ui_screen_t app_select_screen;

extern ui_screen_t game_select_screen;
extern ui_screen_t tetris_screen;
extern ui_screen_t snake_screen;
extern ui_screen_t game2048_screen;

extern ui_screen_t tool_select_screen;
extern ui_screen_t level_screen;
extern ui_screen_t calculator_screen;
extern ui_screen_t stopwatch_screen;
extern ui_screen_t battery_screen;

extern ui_screen_t clock_screen;
extern ui_screen_t clock_add_screen;
extern ui_screen_t file_screen;
extern ui_screen_t system_screen;
extern ui_screen_t novel_screen;
extern ui_screen_t novel_select_screen;
extern ui_screen_t calendar_screen;
extern ui_screen_t transfer_screen;

// ---weather screen---
extern ui_screen_t weather_screen;




void ui_screen_init(uint8_t size, ui_screen_t* load_screen);
uint8_t ui_screen_load(ui_screen_t* load_screen, ui_screen_load_anim_t *screen_load_anim);
uint8_t ui_screen_back(void);
uint32_t ui_screen_default_bg_color(void);


#endif 
