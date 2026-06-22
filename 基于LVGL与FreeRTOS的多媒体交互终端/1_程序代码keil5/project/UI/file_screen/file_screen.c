#include "file_screen.h"



#if USE_UI_FILE_SCREEN
#include <string.h>
#include <stdio.h>
#include "../../middle/fatfs/ff.h" 

lv_obj_t *file_screen_obj;
ui_screen_t file_screen =
{
    .init = file_screen_init,
    .deinit = file_screen_deinit,
    .obj = &file_screen_obj,
};

LV_FONT_DECLARE(_lvgl_font2)
static const lv_font_t *file_screen_font = &_lvgl_font2;
static lv_obj_t *file_list;
static lv_obj_t *file_path_label;

// 定义文件系统根目录及路径缓冲区
#define FILE_ROOT_DIRECTORY         "0:"
static char current_path[256] = FILE_ROOT_DIRECTORY;

// 前置声明私有函数
static void file_item_event_handler(lv_event_t * e);
static void file_screen_refresh_show_list(const char *path);
static void file_screen_quit_event(lv_event_t * e);
static void file_screen_get_parent_path(char *path);


/*-------------------------------------------------------------
                        页面初始化/销毁函数
--------------------------------------------------------------*/
// 屏幕对象初始化
void file_screen_init(lv_obj_t *parent)
{
    // 创建主屏幕容器并配置背景属性
    file_screen_obj = lv_obj_create(parent);
    lv_obj_set_size(file_screen_obj, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(file_screen_obj, lv_color_hex(0xFFFFFF), 0);
    lv_obj_clear_flag(file_screen_obj, LV_OBJ_FLAG_SCROLLABLE);

    // 初始化顶部导航栏
    lv_obj_t * header = lv_obj_create(file_screen_obj);
    lv_obj_set_size(header, lv_pct(100), 45);
    lv_obj_set_style_bg_color(header, lv_color_hex(0xECECF2), 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    // 路径标签
    file_path_label = lv_label_create(header);
    lv_obj_set_width(file_path_label, lv_pct(70));
    lv_label_set_long_mode(file_path_label, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_font(file_path_label, file_screen_font, 0);
    lv_obj_align(file_path_label, LV_ALIGN_LEFT_MID, 10, 0);

    // 右上角退出按钮
    lv_obj_t * exit_btn = lv_btn_create(header);
    lv_obj_set_size(exit_btn, 35, 35);
    lv_obj_align(exit_btn, LV_ALIGN_RIGHT_MID, -5, 0);
    lv_obj_set_style_bg_color(exit_btn, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_set_style_radius(exit_btn, 5, 0);
    lv_obj_add_event_cb(exit_btn, file_screen_quit_event, LV_EVENT_CLICKED, NULL);

    lv_obj_t * exit_label = lv_label_create(exit_btn);
    lv_label_set_text(exit_label, LV_SYMBOL_CLOSE);
    lv_obj_center(exit_label);

    // 初始化列表控件
    file_list = lv_list_create(file_screen_obj);
    lv_obj_set_size(file_list, lv_pct(100), lv_pct(85));
    lv_obj_set_style_border_width(file_list, 0, 0);
    lv_obj_set_style_radius(file_list, 0, 0);
	lv_obj_set_style_text_font(file_list, file_screen_font, 0);
    lv_obj_align(file_list, LV_ALIGN_TOP_MID, 0, 45);
    lv_obj_set_scrollbar_mode(file_list, LV_SCROLLBAR_MODE_AUTO);
	
    file_screen_refresh_show_list(FILE_ROOT_DIRECTORY);
}


void file_screen_deinit(void)
{
    if (file_screen_obj)
    {
        lv_obj_del(file_screen_obj);
        file_screen_obj = NULL;
    }
}



/*-------------------------------------------------------------
                        事件/定时器回调函数
--------------------------------------------------------------*/

// 退出回调
static void file_screen_quit_event(lv_event_t * e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
		// printf("Exit manager\n");
		ui_screen_back();
    }
}


// 点击事件回调
static void file_item_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    intptr_t type = (intptr_t)lv_event_get_user_data(e);

    if (code == LV_EVENT_CLICKED)
    {
        const char * text = lv_list_get_btn_text(file_list, obj);
        if (text == NULL) return;

        if (type == 2) // 返回上级按钮
        {
            file_screen_get_parent_path(current_path);
            file_screen_refresh_show_list(current_path);
        }
        else if (type == 1) // 文件夹
        {
            char next_path[256];
            int len = strlen(current_path);
            const char *sep = (current_path[len-1] == '/' || current_path[len-1] == ':') ? "" : "/";
            snprintf(next_path, sizeof(next_path), "%s%s%s", current_path, sep, text);
            file_screen_refresh_show_list(next_path);
        }
        else // 文件
        {
            //printf("Selected File: %s\n", text);
        }
    }
}


/*-------------------------------------------------------------
                        static函数
--------------------------------------------------------------*/

// 刷新并渲染文件列表内容
static void file_screen_refresh_show_list(const char *path)
{
    FRESULT res;
    DIR dir;
    static FILINFO fno;

    // 更新路径缓存并刷新显示
    strncpy(current_path, path, sizeof(current_path));
    lv_obj_clean(file_list);
    lv_label_set_text_fmt(file_path_label, "Path: %s", current_path);

    // 动态调整对齐逻辑
    lv_obj_update_layout(file_path_label);
    lv_coord_t txt_w = lv_obj_get_self_width(file_path_label);
    lv_coord_t obj_w = lv_obj_get_width(file_path_label);
    lv_obj_set_style_text_align(file_path_label, (txt_w > obj_w) ? LV_TEXT_ALIGN_RIGHT : LV_TEXT_ALIGN_LEFT, 0);

    // 打开 FatFS 目录
    res = f_opendir(&dir, path);
    if (res != FR_OK) 
	{
        lv_list_add_text(file_list, "Failed to open drive");
        return;
    }

    // 返回上级按钮
    if (strcmp(current_path, FILE_ROOT_DIRECTORY) != 0)
    {
        lv_obj_t * btn_back = lv_list_add_btn(file_list, LV_SYMBOL_UP, ".. (back to parent)");
		lv_obj_set_style_bg_color(btn_back, lv_palette_main(LV_PALETTE_RED), 0);
		lv_obj_set_style_bg_opa(btn_back, LV_OPA_COVER, 0);
        lv_obj_add_event_cb(btn_back, file_item_event_handler, LV_EVENT_CLICKED, (void*)2);
    }

    // 渲染目录
    while (f_readdir(&dir, &fno) == FR_OK && fno.fname[0] != 0) 
	{
        if (fno.fattrib & AM_DIR) 
		{
            lv_obj_t * btn = lv_list_add_btn(file_list, NULL, fno.fname);
			//printf("%s\r\n", fno.fname);
            lv_obj_add_event_cb(btn, file_item_event_handler, LV_EVENT_CLICKED, (void*)1);
        }
    }

    // 渲染文件
    f_rewinddir(&dir); 
    while (f_readdir(&dir, &fno) == FR_OK && fno.fname[0] != 0) 
	{
        if (!(fno.fattrib & AM_DIR)) 
		{
            lv_obj_t * btn = lv_list_add_btn(file_list, NULL, fno.fname);
			//printf("%s\r\n", fno.fname);
            lv_obj_add_event_cb(btn, file_item_event_handler, LV_EVENT_CLICKED, (void*)0);
        }
    }

    f_closedir(&dir);
}

// 返回上一级路径处理
static void file_screen_get_parent_path(char *path)
{
    if (strcmp(path, FILE_ROOT_DIRECTORY) == 0) return;

    char *last_slash = strrchr(path, '/');
    if (last_slash != NULL)
    {
        if (last_slash == strchr(path, '/') && *(last_slash - 1) == ':') 
        {
             *(last_slash + 1) = '\0'; 
        } else 
        {
             *last_slash = '\0';
        }
    }
    else
    {
        strcpy(path, FILE_ROOT_DIRECTORY);
    }
}

#else


ui_screen_t file_screen =
{
    .init = NULL,
    .deinit = NULL,
    .obj = NULL,
};






#endif

