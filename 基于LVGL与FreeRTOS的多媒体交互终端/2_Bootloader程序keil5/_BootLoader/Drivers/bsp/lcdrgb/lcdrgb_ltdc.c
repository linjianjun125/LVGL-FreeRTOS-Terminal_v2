#include "lcdrgb_ltdc.h"
#include "lcdrgb.h"
#include "../../system/debug/debug.h"

void lcdrgb_ltdc_init(void)
{
    LCDRGB_LTDC_ENABLE();

    _config_gpio_init(LCDRGB_LTDC_HSYNC_GPIO, LCDRGB_LTDC_HSYNC_PIN, 2, 0, 2, 1);
    _config_gpio_init(LCDRGB_LTDC_VSYNC_GPIO, LCDRGB_LTDC_VSYNC_PIN, 2, 0, 2, 1);
    _config_gpio_init(LCDRGB_LTDC_DE_GPIO, LCDRGB_LTDC_DE_PIN, 2, 0, 2, 1);
    _config_gpio_init(LCDRGB_LTDC_CLK_GPIO, LCDRGB_LTDC_CLK_PIN, 2, 0, 2, 1);
    _config_gpio_init(GPIOG, ConfigIO_Pin6 | ConfigIO_Pin11, 2, 0, 2, 1);   
    _config_gpio_init(GPIOH, 0X7F << 9,2, 0, 2, 1);   						
    _config_gpio_init(GPIOI, 7 << 0 | 0XF << 4,2, 0, 2, 1);   				
	
    config_afio_init(LCDRGB_LTDC_HSYNC_GPIO, LCDRGB_LTDC_HSYNC_PIN, LCDRGB_LTDC_HSYNC_AF);
    config_afio_init(LCDRGB_LTDC_VSYNC_GPIO, LCDRGB_LTDC_VSYNC_PIN, LCDRGB_LTDC_VSYNC_AF);
    config_afio_init(LCDRGB_LTDC_DE_GPIO, LCDRGB_LTDC_DE_PIN, LCDRGB_LTDC_DE_AF);
    config_afio_init(LCDRGB_LTDC_CLK_GPIO, LCDRGB_LTDC_CLK_PIN, LCDRGB_LTDC_CLK_AF);
    config_afio_init(GPIOG, ConfigIO_Pin6 | ConfigIO_Pin11, 14);     		
    config_afio_init(GPIOH, 0X7F << 9, 14);                          		
    config_afio_init(GPIOI, 7 << 0 | 0XF << 4, 14);                  		

    ltdc_clk_set(250, 25, 10);       												// 设置像素时钟

    LTDC->GCR = 1<< 28;                  											// 像素时钟极性:不反向 
    LTDC->GCR |= 0 << 29;                 											// 数据使能极性:低电平有效
    LTDC->GCR |= 0 << 30;                 											// 垂直同步极性:低电平有效
    LTDC->GCR |= 0 << 31;                 											// 水平同步极性:低电平有效

    LTDC->SSCR = (LTDC_VSW - 1) << 0;  												// 垂直脉宽-1
    LTDC->SSCR |= (LTDC_HSW - 1) << 16; 											// 水平脉宽-1     

    LTDC->BPCR = (LTDC_VSW + LTDC_VBP - 1) << 0;     								// 累加垂直后沿=垂直脉宽+垂直后沿-1
    LTDC->BPCR |= (LTDC_HSW + LTDC_HBP - 1) << 16;   								// 累加水平后沿=水平脉宽+水平后沿-1
                         				
    LTDC->AWCR = (LTDC_VSW + LTDC_VBP + LTDC_HEIGHT - 1) << 0;  					// 累加有效高度=垂直脉宽+垂直后沿+垂直分辨率-1 
    LTDC->AWCR |= (LTDC_HSW + LTDC_HBP + LTDC_WIDTH - 1) << 16; 					// 累加有效宽度=水平脉宽+水平后沿+水平分辨率-1 
                                          		
    LTDC->TWCR  = (LTDC_VSW + LTDC_VBP + LTDC_HEIGHT + LTDC_VFP - 1) << 0;  		// 总高度=垂直脉宽+垂直后沿+垂直分辨率+垂直前廊-1
    LTDC->TWCR |= (LTDC_HSW + LTDC_HBP + LTDC_WIDTH  + LTDC_HFP - 1) << 16; 		// 总宽度=水平脉宽+水平后沿+水平分辨率+水平前廊-1
	
    LTDC->BCCR = LTDC_BACKLAYERCOLOR; 												// 背景颜色
    LTDC->GCR |= 1 << 0;															// 打开LTDC                   	
	
    ltdc_layer_parameter_config(LTDC_USE_LAYER, (uint32_t)lcdrgb_dev.buff, LTDC_PIXFORMAT_RGB565, 255, 0, 6, 7, 0X000000);

    ltdc_layer_window_config(LTDC_USE_LAYER, 0, 0, LTDC_WIDTH, LTDC_HEIGHT);   

}





/**
 * @brief   LTDC层基本参数设置
 *  @note   此函数,必须在ltdc_layer_window_config之前设置.
 * @param   layerx: 0,第一层; 1,第二层;
 * @param   bufaddr: 层颜色帧缓存起始地址
 * @param   pixformat: 颜色格式. 0,ARGB8888; 1,RGB888; 2,RGB565; 3,ARGB1555; 4,ARGB4444; 5,L8; 6;AL44; 7;AL88
 * @param   alpha: 层颜色Alpha值, 0,全透明;255,不透明
 * @param   alpha0: 默认颜色Alpha值, 0,全透明;255,不透明
 * @param   bfac1: 混合系数1, 4(100),恒定的Alpha; 6(101),像素Alpha*恒定Alpha
 * @param   bfac2: 混合系数2, 5(101),恒定的Alpha; 7(111),像素Alpha*恒定Alpha
 * @param   bkcolor: 层默认颜色,32位,低24位有效,RGB888格式
 * @retval  无
 */
void ltdc_layer_parameter_config(uint8_t layerx, uint32_t bufaddr, uint8_t pixformat, uint8_t alpha, uint8_t alpha0, uint8_t bfac1, uint8_t bfac2, uint32_t bkcolor)
{
	LTDC_Layer_TypeDef* LTDC_Layer = LTDC_Layer1;//(layerx == 1 ? LTDC_Layer1 : LTDC_Layer2);
	LTDC_Layer->CFBAR = bufaddr;                           // 设置层颜色帧缓存起始地址
	LTDC_Layer->PFCR = pixformat;                          // 设置层颜色格式
	LTDC_Layer->CACR = alpha;                              // 设置层颜色Alpha值,255分频;设置255,则不透明
	LTDC_Layer->DCCR = ((uint32_t)alpha0 << 24) | bkcolor; // 设置默认颜色Alpha值,以及默认颜色
	LTDC_Layer->BFCR = ((uint32_t)bfac1 << 8) | bfac2;     // 设置层混合系数
}


/**
 * @brief   LTDC层窗口设置, 窗口以LCD面板坐标系为基准
 * @note    此函数必须在ltdc_layer_parameter_config之后再设置.另外,当设置的窗口值不等于面板的尺
 *          寸时,GRAM的操作(读/写点函数),也要根据窗口的宽高来进行修改,否则显示不正常(本例程就未做修改).
 * @param   layerx      : 0,第一层; 1,第二层;
 * @param   sx, sy      : 起始坐标
 * @param   width,height: 宽度和高度
 * @retval  无
 */
void ltdc_layer_window_config(uint8_t layerx, uint16_t sx, uint16_t sy, uint16_t width, uint16_t height)
{
	uint32_t ex = (sx + width + ((LTDC->BPCR & 0X0FFF0000) >> 16)) << 16;
	uint32_t ey = (sy + height + (LTDC->BPCR & 0X7FF)) << 16;
	
	LTDC_Layer_TypeDef* LTDC_Layer = LTDC_Layer1;//layerx == 1 ? LTDC_Layer1 : LTDC_Layer2;
	
	LTDC_Layer->WHPCR = (sx + ((LTDC->BPCR & 0X0FFF0000) >> 16) + 1) | ex; 							// 设置行起始和结束位置
	LTDC_Layer->WVPCR = (sy + (LTDC->BPCR & 0X7FF) + 1) | ey; 										// 设置列起始和结束位置
	LTDC_Layer->CFBLR = (width * LTDC_PIXFORMAT_BYTE << 16) | (width * LTDC_PIXFORMAT_BYTE + 3); 	// 帧缓冲区长度设置(字节为单位)
	LTDC_Layer->CFBLNR = height;          															// 帧缓冲区行数设置
    LTDC_Layer->CR |= 1 << 0;																		// 层使能
	LTDC->SRCR 	|= 1 << 0; 																			// 重新加载
}












/**
 * @brief       LTDC时钟(Fdclk)设置函数
 * @param       pll3n     : PLL3倍频系数(PLL倍频),           取值范围:4~512.
 * @param       pll3m     : PLL3预分频系数(进PLL之前的分频),  取值范围:2~63.
 * @param       pll3r     : PLL3的r分频系数(PLL之后的分频),   取值范围:1~128.
 *
 *  @note       Fvco = Fs * (pll3n / pll3m);
 *              Fr = Fvco / pll3r = Fs * (pll3n / (pll3m * pll3r));
 *              Fdclk = Fr;
 *              其中:
 *              Fvco: VCO频率
 *              Fr: PLL3的r分频输出时钟频率
 *              Fs: PLL3输入时钟频率,可以是HSI,CSI,HSE等.
 *
 *              因为LTDC和SAI共用pll3,所以,当使用SAI的时候,会切换不同的pll3n(一般会
 *              大于300),导致LTDC的plln也变化了,因此这里设置为300,得到的像素时钟在
 *              后面SAI修改pll3n的时候,只会大,不会小.不会降低LCD的刷新频率.
 *              因此,LTDC,我们一般设置pll3m = 25, pll3n = 300, 这样,可以得到Fvco = 300Mhz
 *              然后,只 需要通过修改pll3r, 来匹配不同的液晶屏时序即可.

 *              假设:外部晶振为25M, pllm = 25 的时候, Fs = 25Mhz， pllm分频后频率 为1Mhz.
 *              例如: 要得到33M的LTDC时钟, 则可以设置: pll3n = 300, pllm = 25, pll3r = 9
 *              Fdclk= ((25 / 25) * 300) / 9 = 33 Mhz
 * @retval      0, 成功;
 *              其他, 失败;
 */
uint8_t ltdc_clk_set(uint32_t pll3n, uint32_t pll3m, uint32_t pll3r)
{
    uint16_t retry = 0;
    uint8_t status = 0;

    RCC->CR &= ~(1 << 28); /* 关闭PLL3时钟 */

    while (((RCC->CR & (1 << 29))) && (retry < 0X1FFF))
        retry++; /* 等待PLL3时钟失锁 */

    if (retry == 0X1FFF)
        status = 1; /* LTDC时钟关闭失败 */
    else
    {
        RCC->PLLCKSELR &= ~(0X3F << 20);    /* 清除DIVM3[5:0]原来的设置 */
        RCC->PLLCKSELR |= pll3m << 20;      /* DIVM3[5:0] = 25,设置PLL3的预分频系数 */
        RCC->PLL3DIVR &= ~(0X1FF << 0);     /* 清除DIVN3[8:0]原来的设置 */
        RCC->PLL3DIVR |= (pll3n - 1) << 0;  /* DIVN3[8:0] = pll3n - 1, 设置PLL3的倍频系数,设置值需减1 */
        RCC->PLL3DIVR &= ~(0X7F << 24);     /* 清除DIVR2[6:0]原来的设置 */
        RCC->PLL3DIVR |= (pll3r - 1) << 24; /* DIVR3[8:0] = pll3r - 1,设置PLL3的倍频系数,设置值需减1 */

        RCC->PLLCFGR &= ~(0X0F << 8); /* 清除PLL3RGE[1:0]/PLL3VCOSEL/PLL3FRACEN的设置 */
        RCC->PLLCFGR |= 0 << 10;      /* PLL3RGE[1:0] = 0,PLL3输入时钟频率在1~2Mhz之间(25 / 25 = 1Mhz) */
        RCC->PLLCFGR |= 0 << 9;       /* PLL3VCOSEL = 0, PLL3的VCO范围,192~836Mhz */
        RCC->PLLCFGR |= 1 << 24;      /* DIVR3EN = 1, 使能pll3_r_ck */
        RCC->CR |= 1 << 28;           /* PLL3ON = 1, 使能PLL3 */

        while (((RCC->CR & (1 << 29)) == 0) && (retry < 0X1FFF))
            retry++; /* 等待PLL3时钟锁定 */

        if (retry == 0X1FFF)
            status = 2;
    }

    return status;
}




