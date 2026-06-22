#ifndef __LCDRGB_LTDC_H
#define __LCDRGB_LTDC_H

#include "sys.h"


// 外设配置
#define LCDRGB_LTDC_ENABLE()        do{ConfigRCC_GPIOI_EN(); ConfigRCC_GPIOF_EN();ConfigRCC_GPIOG_EN(); ConfigRCC_GPIOH_EN();SYS_RCC_LTDC_EN();}while(0)
#define LCDRGB_LTDC_HSYNC_GPIO      GPIOI
#define LCDRGB_LTDC_VSYNC_GPIO      GPIOI
#define LCDRGB_LTDC_DE_GPIO         GPIOF
#define LCDRGB_LTDC_CLK_GPIO        GPIOG
#define LCDRGB_LTDC_HSYNC_PIN       ConfigIO_Pin10
#define LCDRGB_LTDC_VSYNC_PIN       ConfigIO_Pin9
#define LCDRGB_LTDC_DE_PIN          ConfigIO_Pin10
#define LCDRGB_LTDC_CLK_PIN         ConfigIO_Pin7
#define LCDRGB_LTDC_HSYNC_AF        14
#define LCDRGB_LTDC_VSYNC_AF        14
#define LCDRGB_LTDC_DE_AF           14
#define LCDRGB_LTDC_CLK_AF          14

// 参数设置
#define LTDC_WIDTH					(480)
#define LTDC_HEIGHT					(480)
#define LTDC_HSW					(25)
#define LTDC_HBP					(30)
#define LTDC_HFP					(4)

#define LTDC_VSW					(8)
#define LTDC_VBP					(20)
#define LTDC_VFP					(15)



// 背景颜色
#define LTDC_BACKLAYERCOLOR          0X00000000

// 颜色格式
#define LTDC_PIXFORMAT_ARGB8888      0X00       			// ARGB8888格式
#define LTDC_PIXFORMAT_RGB888        0X01       			// RGB888格式
#define LTDC_PIXFORMAT_RGB565        0X02       			// RGB565格式
#define LTDC_PIXFORMAT               LTDC_PIXFORMAT_RGB565	// 颜色格式
#define LTDC_PIXFORMAT_BYTE          (2)					// 一个像素点的字节数

// 使用层
#define LTDC_LAYER1					1
#define LTDC_LAYER2					2
#define LTDC_USE_LAYER				LTDC_LAYER1


void lcdrgb_ltdc_init(void);
void ltdc_switch(uint8_t sw);
void ltdc_layer_switch(uint8_t layerx, uint8_t sw);
uint8_t ltdc_clk_set(uint32_t pllsain, uint32_t pllsair, uint32_t pllsaidivr);
void ltdc_layer_window_config(uint8_t layerx, uint16_t sx, uint16_t sy, uint16_t width, uint16_t height);
void ltdc_layer_parameter_config(uint8_t layerx, uint32_t bufaddr, uint8_t pixformat, uint8_t alpha, uint8_t alpha0, uint8_t bfac1, uint8_t bfac2, uint32_t bkcolor);






#endif
