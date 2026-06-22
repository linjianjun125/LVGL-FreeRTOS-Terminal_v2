#include "lcdrgb_dma2d.h"
#include "lcdrgb_ltdc.h"
#include "lvgl.h"



void lcdrgb_dma2d_init(void)
{
	RCC->AHB3ENR |= 1 << 4;     // 使能DM2D时钟

#if SYS_USE_DMA2D_IT
	config_nvic_init(3, 0, DMA2D_IRQn);  
#endif
}



void lcdrgb_dma2d_fill(uint16_t width, uint16_t height, uint32_t addr, uint16_t offline, uint16_t color)
{
    RCC->AHB3ENR |= 1 << 4;         	// 使能DM2D时钟
    DMA2D->CR &= ~(1 << 0);         	// 先停止DMA2D
    DMA2D->CR = 3 << 16;            	// 寄存器到存储器模式
	
    DMA2D->OPFCCR = LTDC_PIXFORMAT;  	// 设置颜色格式
    DMA2D->OOR = offline;           	// 设置行偏移
    DMA2D->OMAR = addr;             	// 输出存储器地址
    DMA2D->NLR = height|(width << 16); 	// 设定行数寄存器
    DMA2D->OCOLR = color;           	// 设定输出颜色寄存器

#if SYS_USE_DMA2D_IT
	DMA2D->CR 	|= (1 << 9);			// 开启中断
	DMA2D->IFCR |= 1 << 1;				// 清除标志位
	DMA2D->CR   |= 1 << 0;              // 启动DMA2D
#else
	DMA2D->CR |= 1 << 0;                // 启动DMA2D
	SYS_WAIT_FLAG_BIT_SET_1(DMA2D->ISR, (1 << 1), ms_to_wait);
	if(ms_to_wait == 0)
	{
		printf("[error] lcdrgb_dma2d_fill 填充失败\r\n");
    }
	lv_disp_flush_ready(&disp_drv);
	return ;
#endif
}



extern lv_disp_drv_t disp_drv;   
void lcdrgb_dma2d_colorFill(uint16_t width, uint16_t height, uint32_t addr, uint16_t offline, uint16_t *color)
{
    RCC->AHB3ENR |= 1 << 4;             // 使能DM2D时钟
    DMA2D->CR &= ~(1 << 0);             // 先停止DMA2D
    DMA2D->CR = 0 << 16;                // 存储器到存储器模式
    DMA2D->FGPFCCR = LTDC_PIXFORMAT;    // 设置颜色格式
    DMA2D->FGOR = 0;                    // 前景层行偏移为0
    DMA2D->OOR = offline;               // 设置行偏移
    DMA2D->FGMAR = (uint32_t)color;     // 源地址
    DMA2D->OMAR = addr;                 // 输出存储器地址
    DMA2D->NLR = height| (width << 16); // 设定行数寄存器

#if SYS_USE_DMA2D_IT
	DMA2D->CR 	|= (1 << 9);			// 开启中断
	DMA2D->IFCR |= 1 << 1;				// 清除标志位
	DMA2D->CR   |= 1 << 0;              // 启动DMA2D
#else
	DMA2D->CR |= 1 << 0;                // 启动DMA2D
	SYS_WAIT_FLAG_BIT_SET_1(DMA2D->ISR, (1 << 1), ms_to_wait);
	if(ms_to_wait == 0)
	{
		printf("[error] lcdrgb_dma2d_fills 填充失败\r\n");
		return;
    }
	lv_disp_flush_ready(&disp_drv);
#endif
}  



#if SYS_USE_DMA2D_IT
void DMA2D_IRQHandler(void)
{
	if ((DMA2D->ISR & (1 << 1)) != 0U)
	{
		DMA2D->IFCR = (1 << 1);			// 清除标志位
		lv_disp_flush_ready(&disp_drv);	// 刷新显示设备
	}
}
#endif

