#include "lcdrgb.h"
#include "lcdrgb_ltdc.h"
#include "lcdrgb_dma2d.h"
#include "lcdrgb_init.h"
#include "lcdrgb_blk.h"
#include "../../system/delay/delay.h"
#include "../../system/debug/debug.h"

lcdrgb_color_t ltdc_lcd_framebuf[LTDC_WIDTH][LTDC_HEIGHT] __attribute__((at(SYS_SRAMEX_RGBLCD_START_ADDR)));


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
	
	LCD_RES_Set();
	sys_delay_ms(20);
	LCD_RES_Clr();
	sys_delay_ms(20);
	LCD_RES_Set();
	sys_delay_ms(120);
	
lcdrgb_dma2d_init();
	lcdrgb_param_init();
	lcdrgb_ltdc_init();
	
	lcdrgb_fill_color(0, 0, lcdrgb_dev.width - 1, lcdrgb_dev.height - 1, 0xffff);
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


// 填充颜色
void lcdrgb_fill_color(int sx, int sy, int ex, int ey, lcdrgb_color_t color)
{
    uint32_t psx, psy, pex, pey; 
    uint16_t offline;
    uint32_t addr;

    int width = LTDC_WIDTH, height = LTDC_HEIGHT;


    if (lcdrgb_dev.dir == LCDRGB_DIR_HOR)  
    {
        psx = sx;
        psy = sy;
        pex = ex;
        pey = ey;
    }
    else if (lcdrgb_dev.dir == LCDRGB_DIR_VER)  
    {
        if(ex >= height)
            ex = height - 1;  
        if(sx >= height)
            sx = height - 1;  
        
        psx = sy;
        psy = height - ex - 1;
        pex = ey;
        pey = height - sx - 1;
    }

    offline = width - (pex - psx + 1);
    addr = ((uint32_t)lcdrgb_dev.buff + lcdrgb_dev.pixsize * (width * psy + psx));

    lcdrgb_dma2d_fill(pex - psx + 1, pey - psy + 1, addr, offline, color);


}


// 填充颜色buff
void lcdrgb_fill_colors(int sx, int sy, int ex, int ey, lcdrgb_color_t* color)
{
    uint32_t psx, psy, pex, pey; 
    uint16_t offline;
    uint32_t addr;

    int width = LTDC_WIDTH, height = LTDC_HEIGHT;

    if (lcdrgb_dev.dir == LCDRGB_DIR_HOR)  
    {
        psx = sx;
        psy = sy;
        pex = ex;
        pey = ey;
    }
    else if (lcdrgb_dev.dir == LCDRGB_DIR_VER)  
    {
        if(ex >= height)
            ex = height - 1;  
        if(sx >= height)
            sx = height - 1;  
        
        psx = sy;
        psy = height - ex - 1;
        pex = ey;
        pey = height - sx - 1;
    }

    offline = width - (pex - psx + 1);
    addr = ((uint32_t)lcdrgb_dev.buff + lcdrgb_dev.pixsize * (width * psy + psx));

    lcdrgb_dma2d_colorFill(pex - psx + 1, pey - psy + 1, addr, offline, color);
}



void lcdrgb_refresh(void)
{


}
