#include "config_rcc.h"

/**
 * @brief   系统时钟初始化函数
 * @param   plln: PLL1倍频系数(PLL倍频), 取值范围: 4~512.
 * @param   pllm: PLL1预分频系数(进PLL之前的分频), 取值范围: 2~63.
 * @param   pllp: PLL1的p分频系数(PLL之后的分频), 分频后作为系统时钟, 取值范围: 2~128.(且必须是2的倍数)
 * @param   pllq: PLL1的q分频系数(PLL之后的分频), 取值范围: 1~128.
 * @retval  无
 */
void config_stm32_clock_init(uint32_t plln, uint32_t pllm, uint32_t pllp, uint32_t pllq)
{
    // RCC清除 //
    RCC->CR = 0x00000001;           // 设置HISON, 开启内部高速RC振荡，其他位全清零
    RCC->CFGR = 0x00000000;         
    RCC->D1CFGR = 0x00000000;       
    RCC->D2CFGR = 0x00000000;       
    RCC->D3CFGR = 0x00000000;       
    RCC->PLLCKSELR = 0x00000000;    
    RCC->PLLCFGR = 0x00000000;      
    RCC->CIER = 0x00000000;         

    GPV->AXI_TARG7_FN_MOD = 0x00000001;     /* 设置AXI SRAM的矩阵读取能力为1 */
    
    config_clock_set(plln, pllm, pllp, pllq);  /* 设置时钟 */
    sys_cache_enable();                     /* 使能L1 Cache */

#ifdef  VECT_TAB_RAM
    sys_nvic_set_vector_table(D1_AXISRAM_BASE, 0x0);
#else
    sys_nvic_set_vector_table(FLASH_BANK1_BASE, 0x0);
#endif
}




/******************************************************************************************
 *
 *                                        时钟配置函数
 *
 ******************************************************************************************/
/**
 * @brief       时钟设置函数
 * @param       plln: PLL1倍频系数(PLL倍频), 取值范围: 4~512.
 * @param       pllm: PLL1预分频系数(进PLL之前的分频), 取值范围: 2~63.
 * @param       pllp: PLL1的p分频系数(PLL之后的分频), 分频后作为系统时钟, 取值范围: 2~128.(且必须是2的倍数)
 * @param       pllq: PLL1的q分频系数(PLL之后的分频), 取值范围: 1~128.
 * @note
 *
 *              Fvco: VCO频率
 *              Fsys: 系统时钟频率, 也是PLL1的p分频输出时钟频率
 *              Fq:   PLL1的q分频输出时钟频率
 *              Fs:   PLL输入时钟频率, 可以是HSI, CSI, HSE等.
 *              Fvco = Fs * (plln / pllm);
 *              Fsys = Fvco / pllp = Fs * (plln / (pllm * pllp));
 *              Fq   = Fvco / pllq = Fs * (plln / (pllm * pllq));
 *
 *              外部晶振为25M的时候, 推荐值: plln = 160, pllm = 5, pllp = 2, pllq = 4.
 *              得到:Fvco = 25 * (160 / 5) = 800Mhz
 *                   Fsys = pll1_p_ck = 800 / 2 = 400Mhz
 *                   Fq   = pll1_q_ck = 800 / 4 = 200Mhz
 *
 *              H743默认需要配置的频率如下:
 *              CPU频率(rcc_c_ck) = sys_d1cpre_ck = 400Mhz
 *              rcc_aclk = rcc_hclk3 = 200Mhz
 *              AHB1/2/3/4(rcc_hclk1/2/3/4) = 200Mhz
 *              APB1/2/3/4(rcc_pclk1/2/3/4) = 100Mhz
 *              pll2_p_ck = (25 / 25) * 440 / 2) = 220Mhz
 *              pll2_r_ck = FMC时钟频率 = ((25 / 25) * 440 / 2) = 220Mhz
 *
 * @retval      错误代码: 0, 成功; 1, HSE错误; 2, PLL1错误; 3, PLL2错误; 4, 切换时钟错误;
 */
uint8_t config_clock_set(uint32_t plln, uint32_t pllm, uint32_t pllp, uint32_t pllq)
{
    uint32_t retry = 0;
    uint8_t retval = 0;
    uint8_t swsval = 0;

    PWR->CR3 &= ~(1 << 2);      /* SCUEN = 0, 锁定LDOEN和BYPASS位的设置 */
    PWR->D3CR |= 3 << 14;       /* VOS = 3, Scale1, 1.2V内核电压,FLASH访问可以得到最高性能 */

    while ((PWR->D3CR & (1 << 13)) == 0);   /* 等待电压稳定 */

    RCC->CR |= 1 << 16; /* HSEON = 1, 开启HSE */

    while (((RCC->CR & (1 << 17)) == 0) && (retry < 0X7FFF))
    {
        retry++;        /* 等待HSE RDY */
    }




    if (retry == 0X7FFF)
    {
        retval = 1;     // HSE无法就绪 
    }
    else
    {
        // PLL1 配置 //
        RCC->PLLCKSELR |= 2 << 0;           // 选择HSE作为PLL的输入时钟源
        RCC->PLLCKSELR |= pllm << 4;        // 设置PLL1的预分频系数DIVM

        //RCC->PLL1DIVR |= 1 << 24;         // 设置PLL1的分频系数DIVR, 设置值需减1
        RCC->PLL1DIVR |= (pllq - 1) << 16;  // 设置PLL1的分频系数DIVQ, 设置值需减1
        RCC->PLL1DIVR |= (pllp - 1) << 9;   // 设置PLL1的分频系数DIVP, 设置值需减1
        RCC->PLL1DIVR |= (plln - 1) << 0;   // 设置PLL1的倍频系数DIVN, 设置值需减1

        RCC->PLLCFGR |= 0 << 1;             // PLL1中的VCO范围--192~836Mhz
        RCC->PLLCFGR |= 2 << 2;             // PLL1输入时钟频率(预分频系数DIVM后)--4~8Mhz之间

        //RCC->PLLCFGR |= 3 << 16;          // 使能DIVR1输出
        RCC->PLLCFGR |= 3 << 17;            // 使能DIVQ1输出
        RCC->PLLCFGR |= 3 << 16;            // 使能DIVP1输出

        RCC->CR |= 1 << 24;                 // 使能PLL1

        retry = 0;
        while ((RCC->CR & (1 << 25)) == 0)   // 等待PLL1就绪
        {
            retry++;

            if (retry > 0X1FFFFF)
            {
                retval = 2; //失败
                break;
            }
        }


        /* 设置PLL2的R分频输出, 为220Mhz, 后续做TFTLCD时钟, 可得到110M的fmc_ker_ck时钟频率 */
        RCC->PLLCKSELR |= 25 << 12;         /* DIVM2[5:0] = 25, 设置PLL2的预分频系数 */
        RCC->PLL2DIVR |= (440 - 1) << 0;    /* DIVN2[8:0] = 440 - 1, 设置PLL2的倍频系数, 设置值需减1 */
        RCC->PLL2DIVR |= (2 - 1) << 9;      /* DIVP2[6:0] = 2 - 1, 设置PLL2的p分频系数, 设置值需减1 */
        RCC->PLL2DIVR |= (2 - 1) << 24;     /* DIVR2[6:0] = 2 - 1, 设置PLL2的r分频系数, 设置值需减1 */
        RCC->PLLCFGR |= 0 << 6;             /* PLL2RGE[1:0] = 0, PLL2输入时钟频率在1~2Mhz之间(25/25 = 1Mhz) */
        RCC->PLLCFGR |= 0 << 5;             /* PLL2VCOSEL = 0, PLL2宽的VCO范围, 192~836Mhz */
        RCC->PLLCFGR |= 1 << 19;            /* DIVP2EN = 1, 使能pll2_p_ck */
        RCC->PLLCFGR |= 1 << 21;            /* DIVR2EN = 1, 使能pll2_r_ck */
        RCC->D1CCIPR &= ~(3 << 0);          /* 清除FMCSEL[1:0]原来的设置 */
        RCC->D1CCIPR |= 2 << 0;             /* FMCSEL[1:0] = 2, FMC时钟来自于pll2_r_ck */
        RCC->CR |= 1 << 26;                 /* PLL2ON = 1, 使能PLL2 */


        retry = 0;
        while ((RCC->CR & (1 << 27)) == 0)  /* PLL2RDY = 1?, 等待PLL2准备好 */
        {
            retry++;

            if (retry > 0X1FFFFF)
            {
                retval = 3; /* PLL2无法就绪 */
                break;
            }
        }


        RCC->D1CFGR |= 8 << 0;              /* HREF[3:0] = 8, rcc_hclk1/2/3/4  =  sys_d1cpre_ck / 2 = 400 / 2 = 200Mhz, 即AHB1/2/3/4 = 200Mhz */
        RCC->D1CFGR |= 0 << 8;              /* D1CPRE[2:0] = 0, sys_d1cpre_ck = sys_clk/1 = 400 / 1 = 400Mhz, 即CPU时钟 = 400Mhz */
        RCC->CFGR |= 3 << 0;                /* SW[2:0] = 3, 系统时钟(sys_clk)选择来自pll1_p_ck, 即400Mhz */

        retry = 0;
        while (swsval != 3)                 /* 等待成功将系统时钟源切换为pll1_p_ck */
        {
            swsval = (RCC->CFGR & (7 << 3)) >> 3;   /* 获取SWS[2:0]的状态, 判断是否切换成功 */
            retry++;

            if (retry > 0X1FFFFF)
            {
                retval = 4; /* 无法切换时钟 */
                break;
            }
        }


        FLASH->ACR |= 2 << 0;               /* LATENCY[2:0] = 2, 2个CPU等待周期(@VOS1 Level, maxclock = 210Mhz) */
        FLASH->ACR |= 2 << 4;               /* WRHIGHFREQ[1:0] = 2, flash访问频率<285Mhz */

        RCC->D1CFGR |= 4 << 4;              /* D1PPRE[2:0] = 4,  rcc_pclk3 = rcc_hclk3/2 = 100Mhz, 即APB3 = 100Mhz */
        RCC->D2CFGR |= 4 << 4;              /* D2PPRE1[2:0] = 4, rcc_pclk1 = rcc_hclk1/2 = 100Mhz, 即APB1 = 100Mhz */
        RCC->D2CFGR |= 4 << 8;              /* D2PPRE2[2:0] = 4, rcc_pclk2 = rcc_hclk1/2 = 100Mhz, 即APB2 = 100Mhz */
        RCC->D3CFGR |= 4 << 4;              /* D3PPRE[2:0] = 4,  rcc_pclk4 = rcc_hclk4/2 = 100Mhz, 即APB4 = 100Mhz */
        
        RCC->CR |= 1 << 7;                  /* CSION = 1, 使能CSI, 为IO补偿单元提供时钟 */
        RCC->APB4ENR |= 1 << 1;             /* SYSCFGEN = 1, 使能SYSCFG时钟 */
        SYSCFG->CCCSR |= 1 << 0;            /* EN = 1, 使能IO补偿单元 */
    }

    return retval;
}











