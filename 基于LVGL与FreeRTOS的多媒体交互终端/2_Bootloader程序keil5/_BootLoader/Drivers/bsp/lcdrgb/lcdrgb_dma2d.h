#ifndef __LCDRGB_DMA2D_H
#define __LCDRGB_DMA2D_H



#include "sys.h"
#include "lcdrgb_ltdc.h"

#define SYS_USE_DMA2D_IT	1

void lcdrgb_dma2d_init(void);
void lcdrgb_dma2d_fill(uint16_t width, uint16_t height, uint32_t addr, uint16_t offline, uint16_t color);
void lcdrgb_dma2d_colorFill(uint16_t width, uint16_t height, uint32_t addr, uint16_t offline, uint16_t *color);



#endif
