#include "audio_sai.h"
#include "../../devices/lcdrgb/lcdrgb_ltdc.h"




/* mode    : 00,主发送器;01,主接收器;10,从发送器;11,从接收器
 * cpol    : 0,时钟下降沿选通;1,时钟上升沿选通
 * datalen : 2,8位;3,10位;4,16位;5,20位;6,24位;7,32位.
 */
void audio_saia_init(uint8_t mode, uint8_t cpol, uint8_t datalen)
{
    uint32_t tempreg = 0;
	
	/*!< RCC 使能 >!*/
    AUDIO_SAI_ENABLE();
	
	/*!< GPIO 配置 >!*/
    config_gpio_init(AUDIO_SAI_FS_GPIO, AUDIO_SAI_FS_PIN, 2, 0, 2, 1);
    config_gpio_init(AUDIO_SAI_SCK_GPIO, AUDIO_SAI_SCK_PIN, 2, 0, 2, 1);
    config_gpio_init(AUDIO_SAI_MCLK_GPIO, AUDIO_SAI_MCLK_PIN, 2, 0, 2, 1);
    config_gpio_init(AUDIO_SAI_SDA_GPIO, AUDIO_SAI_SDA_PIN, 2, 0, 2, 1);
    config_gpio_init(AUDIO_SAI_SDB_GPIO, AUDIO_SAI_SDB_PIN, 2, 0, 2, 1);


	/*!< AFIO 配置 >!*/
    config_afio_init(AUDIO_SAI_FS_GPIO, AUDIO_SAI_FS_PIN, AUDIO_SAI_FS_AF);
    config_afio_init(AUDIO_SAI_SCK_GPIO, AUDIO_SAI_SCK_PIN, AUDIO_SAI_SCK_AF);
    config_afio_init(AUDIO_SAI_MCLK_GPIO, AUDIO_SAI_MCLK_PIN, AUDIO_SAI_MCLK_AF);
    config_afio_init(AUDIO_SAI_SDA_GPIO, AUDIO_SAI_SDA_PIN, AUDIO_SAI_SDA_AF);
    config_afio_init(AUDIO_SAI_SDB_GPIO, AUDIO_SAI_SDB_PIN, AUDIO_SAI_SDB_AF);


	/*!< SAI 初始化 >!*/
    RCC->D2CCIP1R &= ~(7 << 0);         /* SAI1SEL[2:0]清零 */
    RCC->D2CCIP1R |= 2 << 0;            /* SAI1SEL[2:0]=2,选择pll3_p_ck作为SAI1的时钟源 */
    RCC->APB2RSTR |= 1 << 22;           /* 复位SAI1 */
    RCC->APB2RSTR &= ~(1 << 22);        /* 结束复位 */
	
    tempreg |= mode << 0;           /* 设置SAI1工作模式 */
    tempreg |= 0 << 2;              /* 设置SAI1协议为:自由协议(支持I2S/LSB/MSB/TDM/PCM/DSP等协议) */
    tempreg |= datalen << 5;     	/* 设置数据大小 */
    tempreg |= 0 << 8;              /* 数据MSB位优先 */
    tempreg |= (uint16_t)cpol << 9;              /* 数据在时钟的上升/下降沿选通 */
    tempreg |= 0 << 10;             /* 音频模块异步 */
    tempreg |= 0 << 12;             /* 立体声模式 */
    tempreg |= 1 << 13;             /* 立即驱动音频模块输出 */
    tempreg |= 0 << 19;             /* 使能主时钟分频器(MCKDIV) */
    AUDIO_SAI_A->CR1 = tempreg;       /* 设置CR1寄存器 */

    tempreg = (64 - 1) << 0;        /* 设置帧长度为64,左通道32个SCK,右通道32个SCK */
    tempreg |= (32 - 1) << 8;       /* 设置帧同步有效电平长度,在I2S模式下=1/2帧长 */
    tempreg |= 1 << 16;             /* FS信号为SOF信号+通道识别信号 */
    tempreg |= 0 << 17;             /* FS低电平有效(下降沿) */
    tempreg |= 1 << 18;             /* 在slot0的第一位的前一位使能FS,以匹配飞利浦标准 */
    AUDIO_SAI_A->FRCR = tempreg;

    tempreg = 0 << 0;               /* slot偏移(FBOFF)为0 */
    tempreg |= 2 << 6;              /* slot大小为32位 */
    tempreg |= (2 - 1) << 8;        /* slot数为2个 */
    tempreg |= (1 << 17) | (1 << 16);   /* 使能slot0和slot1 */
    AUDIO_SAI_A->SLOTR = tempreg;     /* 设置slot */

    AUDIO_SAI_A->CR2 = 1 << 0;        /* 设置FIFO阀值:1/4 FIFO */
    AUDIO_SAI_A->CR2 |= 1 << 3;       /* FIFO刷新 */
}



void audio_saib_init(uint8_t mode, uint8_t cpol, uint8_t datalen)
{
    uint32_t tempreg = 0;

    tempreg |= mode << 0;           /* 设置SAI1工作模式 */
    tempreg |= 0 << 2;              /* 设置SAI1协议为:自由协议(支持I2S/LSB/MSB/TDM/PCM/DSP等协议) */
    tempreg |= datalen << 5;        /* 设置数据大小 */
    tempreg |= 0 << 8;              /* 数据MSB位优先 */
    tempreg |= (uint16_t)cpol << 9; /* 数据在时钟的上升/下降沿选通 */
    tempreg |= 1 << 10;             /* 使能同步模式 */
    tempreg |= 0 << 12;             /* 立体声模式 */
    tempreg |= 1 << 13;             /* 立即驱动音频模块输出 */
    AUDIO_SAI_B->CR1 = tempreg;    /* 设置CR1寄存器 */

    tempreg = (64 - 1) << 0;        /* 设置帧长度为64,左通道32个SCK,右通道32个SCK */
    tempreg |= (32 - 1) << 8;       /* 设置帧同步有效电平长度,在I2S模式下=1/2帧长 */
    tempreg |= 1 << 16;             /* FS信号为SOF信号+通道识别信号 */
    tempreg |= 0 << 17;             /* FS低电平有效(下降沿) */
    tempreg |= 1 << 18;             /* 在slot0的第一位的前一位使能FS,以匹配飞利浦标准 */
    AUDIO_SAI_B->FRCR = tempreg;

    tempreg = 0 << 0;               /* slot偏移(FBOFF)为0 */
    tempreg |= 2 << 6;              /* slot大小为32位 */
    tempreg |= (2 - 1) << 8;        /* slot数为2个 */
    tempreg |= (1 << 17) | (1 << 16);   /* 使能slot0和slot1 */
    AUDIO_SAI_B->SLOTR = tempreg;  /* 设置slot */

    AUDIO_SAI_B->CR2 = 1 << 0;     /* 设置FIFO阀值:1/4 FIFO */
    AUDIO_SAI_B->CR2 |= 1 << 3;    /* FIFO刷新 */
    AUDIO_SAI_B->CR1 |= 1 << 17;   /* 使能DMA */
    AUDIO_SAI_B->CR1 |= 1 << 16;   /* 使能SAI1 Block B */
}





/**
 * @brief       专门针对LTDC的一个时钟设置函数
 * @param       pll3n: 当前的pll3倍频系数
 * @retval      无
 */
void sai_ltdc_clkset(uint16_t pll3n)
{
    uint8_t pll3r = 0;
	uint8_t RGB_TLDC_CLK = 25; 	// RGB屏幕的LTDC时钟频率=25M
    pll3r = pll3n / RGB_TLDC_CLK;   

    if (pll3n > (pll3r * RGB_TLDC_CLK))
    {
        pll3r += 1;
    }
    
    ltdc_clk_set(pll3n, 25, pll3r); 
}




const uint16_t SAI_PSC_TBL[][5] =
{
    {800, 256, 5, 25},      /* 8Khz采样率 */
    {1102, 302, 107, 0},    /* 11.025Khz采样率 */
    {1600, 426, 2, 52},     /* 16Khz采样率 */
    {2205, 429, 38, 2},     /* 22.05Khz采样率 */
    {3200, 426, 1, 52},     /* 32Khz采样率 */
    {4410, 429, 1, 38},     /* 44.1Khz采样率 */
    {4800, 467, 1, 38},     /* 48Khz采样率 */
    {8820, 429, 1, 19},     /* 88.2Khz采样率 */
    {9600, 467, 1, 19},     /* 96Khz采样率 */
    {17640, 271, 1, 6},     /* 176.4Khz采样率 */
    {19200, 295, 6, 0},     /* 192Khz采样率 */
};


/**
 * @brief   设置SAI1的采样率
 * @param   samplerate: 采样率, 单位:Hz
 * @retval  0,设置成功 / 1,无法设置
 */
uint8_t audio_sai_samplerate(uint32_t samplerate)
{
    uint16_t retry = 0;
    uint8_t i = 0;
    uint32_t tempreg = 0;
    samplerate /= 10; 

    for (i = 0; i < (sizeof(SAI_PSC_TBL) / 10); i++)
    {
        if (samplerate == SAI_PSC_TBL[i][0])
        break;
    }

    RCC->CR &= ~(1 << 28);                             

    if (i == (sizeof(SAI_PSC_TBL) / 10))
        return 1;    

    while (((RCC->CR & (1 << 29))) && (retry < 0X1FFF)) 
    {
        retry++; 
    }
    
    RCC->PLLCKSELR &= ~(0X3F << 20);                    /* 清除DIVM3[5:0]原来的设置 */
    RCC->PLLCKSELR |= 25 << 20;                         /* DIVM3[5:0]=25,设置PLL3的预分频系数 */
    tempreg = RCC->PLL3DIVR;                            /* 读取PLL3DIVR的值 */
    tempreg &= 0XFFFF0000;                              /* 清除DIVN和PLL3DIVP原来的设置 */
    tempreg |= (uint32_t)(SAI_PSC_TBL[i][1] - 1) << 0;  /* 设置DIVN[8:0] */
    tempreg |= (uint32_t)(SAI_PSC_TBL[i][2] - 1) << 9;  /* 设置DIVP[6:0] */
    RCC->PLL3DIVR = tempreg;                            /* 设置PLL3DIVR寄存器 */
    RCC->PLLCFGR |= 1 << 22;                            /* DIVP3EN=1,使能pll3_p_ck */
    RCC->CR |= 1 << 28;                                 /* 开启PLL3 */
    while ((RCC->CR & 1 << 29) == 0);                   /* 等待PLL3开启成功 */


    tempreg = AUDIO_SAI_A->CR1;
    tempreg &= ~(0X3F << 20);                           /* 清除MCKDIV[5:0]设置 */
    tempreg |= (uint32_t)SAI_PSC_TBL[i][3] << 20;       /* 设置MCKDIV[5:0] */
    tempreg |= 1 << 16;                                 /* 使能SAI1 Block A */
    tempreg |= 1 << 17;                                 /* 使能DMA */
    AUDIO_SAI_A->CR1 = tempreg;                        /* 配置MCKDIV[5:0],同时使能SAI1 Block A */
    
	sai_ltdc_clkset(SAI_PSC_TBL[i][1]);  
	
    return 0;
}



void audio_saia_close(void)
{
	AUDIO_SAI_A->CR1 &= ~(1 << 16);
	AUDIO_SAI_A->CR1 &= ~(1 << 17);
}











