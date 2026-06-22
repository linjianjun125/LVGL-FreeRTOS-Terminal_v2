#include "./system/sys/sys.h"
#include "./system/delay/delay.h"

static uint32_t g_fac_us = 0;       // us延时倍乘数

/**
 * @brief     初始化延迟函数
 * @param     sysclk: 系统时钟频率, 即CPU频率(HCLK), 单位 Mhz
 * @retval    无
 */  
void delay_init(uint16_t sysclk)
{
    // H7建议使用 HAL_RCC_GetHCLKFreq() / 1000000 传入
    g_fac_us = sysclk; 
    
    // 配置 SysTick 为系统时钟源，并开启计数器（不开启中断）
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk; 
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;    
}

/**
 * @brief     延时nus
 * @note      使用时钟摘取法做us延时
 * @param     nus: 要延时的us数
 */ 
void delay_us(uint32_t nus)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD;        // LOAD的值
    ticks = nus * g_fac_us;                 // 需要的节拍数

    told = SysTick->VAL;                    // 刚进入时的计数器值
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            // SysTick 是递减计数器
            if (tnow < told)
            {
                tcnt += told - tnow;
            }
            else
            {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks) 
            {
                break;                      // 时间超过/等于要延迟的时间,则退出
            }
        }
    }
} 

/**
 * @brief     延时nms
 * @param     nms: 要延时的ms数
 */
void delay_ms(uint16_t nms)
{
    delay_us((uint32_t)(nms * 1000));
}

/**
 * @brief     重写HAL库内部函数用到的延时
 * @param     Delay : 要延时的毫秒数
 */
void HAL_Delay(uint32_t Delay)
{
     delay_ms(Delay);
}
