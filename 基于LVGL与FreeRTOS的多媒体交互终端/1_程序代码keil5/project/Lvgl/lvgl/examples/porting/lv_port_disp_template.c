/**
 * @file lv_port_disp_templ.c
 *
 */

 /*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp_template.h"
#include "../../lvgl.h"
#include "../../Devices/lcdrgb/lcdrgb.h"
#include "../../System/sys.h"

lv_disp_drv_t disp_drv;

#define MY_DISP_HOR_RES     480     // 修改为你的实际水平像素
#define MY_DISP_VER_RES     480     // 修改为你的实际竖直像素
#define LCD_PIXELS          (MY_DISP_HOR_RES * MY_DISP_VER_RES)

/* 定义在 SDRAM 中的两个全屏缓冲区 */
lv_color_t lvgl_buf_3_1[LCD_PIXELS] __attribute__((at(SYS_SRAMEX_UI_START_ADDR)));
lv_color_t lvgl_buf_3_2[LCD_PIXELS] __attribute__((at(SYS_SRAMEX_UI_START_ADDR + LCD_PIXELS * 2)));

static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);

void lv_port_disp_init(void)
{
    lcdrgb_init();  // 确保此时 LTDC 已经初始化并开启
	
	RCC->AHB3ENR |= 1 << 4;     // 使能DM2D时钟
	config_nvic_init(3, 0, DMA2D_IRQn);  
    static lv_disp_draw_buf_t draw_buf;
	
    /* 初始化绘制缓冲区：传入两个全屏 Buffer，大小为像素个数 */
    lv_disp_draw_buf_init(&draw_buf, lvgl_buf_3_1, lvgl_buf_3_2, LCD_PIXELS);
	memset(lvgl_buf_3_1, 0xffffffff, LCD_PIXELS * 2); 
    memset(lvgl_buf_3_2, 0xffffffff, LCD_PIXELS * 2); 
	
    
    lv_disp_drv_init(&disp_drv);
    
    disp_drv.draw_buf = &draw_buf;
    disp_drv.flush_cb = disp_flush;
    disp_drv.hor_res  = MY_DISP_HOR_RES; // 使用宏定义 480
    disp_drv.ver_res  = MY_DISP_VER_RES; // 使用宏定义 480

    lv_disp_drv_register(&disp_drv);
}



static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
	lcdrgb_fill_colors(area->x1, area->y1, area->x2, area->y2, (uint16_t*)color_p);
 //   lv_disp_flush_ready(disp_drv);
}








#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif


