#ifndef __FONTS_H
#define __FONTS_H

#include "sys.h"

#define FONTINFOADDR        (0)  	// 字库起始地址
#define FONT_OK				(0x17770000)
#define FONT_ERR			(0x00004321)



enum
{
	FONT_FATFS_UNIGBK_BIN = 0,
	FONT_LVGL_FONT1,
	FONT_LVGL_FONT2,
	FONT_LVGL_FONT3,
	FONT_LVGL_FONT4,
};





typedef struct
{
	char* path;				// 字库路径
	uint32_t status;		// 字库的状态
	uint32_t mem_addr;   	// 存储的地址
	uint32_t addr;   		// 字库的地址
    uint32_t size;  		// 字库的大小
}fontInfo_t;
extern fontInfo_t 	fontInfo[];

uint8_t fonts_init(void);
uint8_t font_updata(fontInfo_t *font);

#endif

