#ifndef __LCDRGB_H
#define __LCDRGB_H

#include "sys.h"
#include "lcdrgb_init.h"
#include "lcdrgb_blk.h"
#include "lcdrgb_dma2d.h"


// 显示方向
#define LCDRGB_DIR_HOR  		1       // 水平方向
#define LCDRGB_DIR_VER  		0       // 竖直方向
#define LCDRGB_DIR				(LCDRGB_DIR_HOR)

// 颜色类型
typedef uint16_t lcdrgb_color_t;


// LCD设备结构体
typedef struct 
{
    int width;
    int height;
    int dir;
    int pixsize;
    uint32_t* buff;
}lcdrgb_dev_t;
extern lcdrgb_dev_t lcdrgb_dev;


void lcdrgb_init(void);



void lcdrgb_draw_point(int x, int y, lcdrgb_color_t color);
lcdrgb_color_t lcdrgb_readPoint(int x, int y);
void lcdrgb_fill_color(int sx, int sy, int ex, int ey, lcdrgb_color_t color);
void lcdrgb_fill_colors(int sx, int sy, int ex, int ey, lcdrgb_color_t* color);
void lcdrgb_refresh(void);








#endif
