#include "lcdrgb.h"
#include "lcdrgb_ltdc.h"
#include "lcdrgb_dma2d.h"
#include "lcdrgb_init.h"
#include "lcdrgb_blk.h"
#include "../../system/delay/delay.h"
#include "../../system/debug/debug.h"

lcdrgb_color_t ltdc_lcd_framebuf[LTDC_WIDTH][LTDC_HEIGHT] __attribute__((at((0XC0000000 + 1024*1024*1))));


lcdrgb_dev_t lcdrgb_dev = 
{
    .width = LTDC_WIDTH,
    .height = LTDC_HEIGHT,
    .dir = LCDRGB_DIR,
    .pixsize = LTDC_PIXFORMAT_BYTE,
    .buff = (uint32_t*)&ltdc_lcd_framebuf,
};

void lcdrgb_init(void)
{

	lcdrgb_io_init();
	lcdrgb_set_pwm_duty_cycle(40);
	
	LCD_RES_Set();
	delay_ms(20);
	LCD_RES_Clr();
	delay_ms(20);
	LCD_RES_Set();
	delay_ms(120);
	

	lcdrgb_param_init();
	lcdrgb_ltdc_init();
	
	// 原代码：
	// lcdrgb_fill_color(0, 0, lcdrgb_dev.width - 1, lcdrgb_dev.height - 1, 0xffff);

	// 修改为画点函数实现：
	int x, y;
	for (y = 0; y < lcdrgb_dev.height; y++) 
	{
		for (x = 0; x < lcdrgb_dev.width; x++) 
		{
			lcdrgb_draw_point(x, y, 0xffff);
		}
	}
}



// 画点函数
void lcdrgb_draw_point(int x, int y, lcdrgb_color_t color)
{
    if (lcdrgb_dev.dir == LCDRGB_DIR_HOR)
        *(lcdrgb_color_t *)((uint32_t)lcdrgb_dev.buff + lcdrgb_dev.pixsize * (lcdrgb_dev.width * y + x)) = color;
    else
        *(lcdrgb_color_t *)((uint32_t)lcdrgb_dev.buff + lcdrgb_dev.pixsize * (lcdrgb_dev.width * (lcdrgb_dev.height - x - 1) + y)) = color;
}


// 读点函数
lcdrgb_color_t lcdrgb_readPoint(int x, int y)
{
    if (lcdrgb_dev.dir == LCDRGB_DIR_HOR)
        return *(lcdrgb_color_t *)((uint32_t)lcdrgb_dev.buff + lcdrgb_dev.pixsize * (lcdrgb_dev.width * y + x));
    else
        return *(lcdrgb_color_t *)((uint32_t)lcdrgb_dev.buff + lcdrgb_dev.pixsize * (lcdrgb_dev.width * (lcdrgb_dev.height - x - 1) + y));
}

