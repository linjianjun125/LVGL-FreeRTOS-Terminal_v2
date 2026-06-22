#include "fonts.h"
#include <string.h>
#include "../../system/debug/debug.h"
#include "../../system/delay/delay.h"
#include "../../middle/malloc/malloc.h"
#include "../../middle/fatfs/ff.h"
#include "../../devices/nor_flash/norflash.h"

#define font0_size			(182*1024) 
#define font1_size			(1106*1024) 
#define font2_size			(1775*1024) 
#define font3_size			(3058*1024) 
#define font4_size			(4300*1024) 

#define font0_addr			(FONTINFOADDR) 
#define font1_addr			(font0_addr   + font0_size) 
#define font2_addr			(font1_addr   + font1_size) 
#define font3_addr			(font2_addr   + font2_size) 
#define font4_addr			(font3_addr   + font3_size) 


fontInfo_t 	fontInfo[] = 
{
	{.path = "0:/SYSTEM/FONT/UNIGBK.BIN", 	   .mem_addr = font0_addr, .addr = 0},
	{.path = "0:/SYSTEM/FONT/_lvgl_font1.bin", .mem_addr = font1_addr, .addr = 0},
	{.path = "0:/SYSTEM/FONT/_lvgl_font2.bin", .mem_addr = font2_addr, .addr = 0},
	{.path = "0:/SYSTEM/FONT/_lvgl_font3.bin", .mem_addr = font3_addr, .addr = 0},
	{.path = "0:/SYSTEM/FONT/_lvgl_font4.bin", .mem_addr = font4_addr, .addr = 0},
};


uint8_t fonts_init(void)
{
	fontInfo_t 	temp;
	
	for(int i = 0; i < sizeof(fontInfo) / sizeof(fontInfo[0]) ; i++)
	{
		norflash_read_data((void*)(&temp), fontInfo[i].mem_addr, sizeof(fontInfo_t));
		
		//printf("%x\n\r\n", temp.status);
		if(temp.status != FONT_OK)
		{
			printf("[info] 正在更新字库%s\n", fontInfo[i].path);
			if(font_updata(&fontInfo[i]) == SYS_RET_ERR)
			{
				printf("字库%s下载失败\n", fontInfo[i].path);
			}
		}
		else
		{
			memcpy(&fontInfo[i], &temp, sizeof(fontInfo_t));
		}
	}
	printf("[info] 字库初始化成功\r\n");
	return SYS_RET_OK;
}




uint8_t font_updata(fontInfo_t *font)
{
	FIL *fs;
	uint8_t res, Ret = SYS_RET_ERR;	
	uint32_t offx = 0, br;
	uint8_t *buf;
	int progress = 0;
	int temp = 0;

	// 申请内存
	fs = (FIL *)mymalloc(SRAMIN, sizeof(FIL));
	buf = (uint8_t*)mymalloc(SRAMIN, 4096);
    if (fs == NULL || buf == NULL)	
	{	
		printf("[error] font_updata 申请内存失败\n");
		Ret = SYS_RET_ERR;
        goto error;
	}	

	// 打开文件	
	res = f_open(fs, (const TCHAR *)font->path, FA_READ);
	if (res != FR_OK)
	{
		printf("[error] font_updata 打开%s文件失败\n", font->path);
		Ret = SYS_RET_ERR;
		goto error;
	}
	
	// 擦除扇区
	font->size = fs->obj.objsize;
	font->addr = font->mem_addr + sizeof(fontInfo_t);
	norflash_erase_data(font->addr, font->size);
	
	// 更新字库
	while (res == FR_OK)  
	{
		res = f_read(fs, buf, 4096, (UINT *)&br); 
		if (res != FR_OK){
			printf("[error] font_updata 文件读取失败\n");
			goto error;
		}     
		norflash_write_data(buf, offx + font->addr, br); 
		
		temp = offx *100 / fs->obj.objsize;
		if(temp > progress)
		{
			progress = temp;
			printf("[info] font_updata 更新进度%d\n", progress);
		}
		offx += br;
		if (br < 4096)	
			break;
	}
	
	
	font->status = FONT_OK;
	norflash_write_data((void*)font, font->mem_addr, sizeof(fontInfo_t)); 
	
	
	Ret = SYS_RET_OK;

error:
	myfree(SRAMIN, fs);
	myfree(SRAMIN, buf);
	return Ret;
}






