#include "random.h"
#include "../../system/delay/delay.h"

/**
 * @brief       初始化RNG
 * @param       无
 * @retval      0,成功;1,失败
 */
uint8_t sys_random_init(void)
{
    uint16_t retry = 0;
    RCC->AHB2ENR = 1 << 6;      /* 开启RNG时钟 */
    RCC->D2CCIP2R &= ~(3 << 8); /* RNGSEL[1:0]清零 */
    RCC->D2CCIP2R |= 1 << 8;    /* RNGSEL[1:0]=1,选择pll1_q_ck作为RNG时钟源(200Mhz) */
    RNG->CR |= 1 << 2;          /* 使能RNG */

    while ((RNG->SR & 0X01) == 0 && retry < 10000)  /* 等待随机数就绪 */
    {
        retry++;
        sys_delay_us(100);
    }

    if (retry >= 10000)return 1;    /* 随机数产生器工作不正常 */

    return 0;
}

/**
 * @brief       得到随机数
 * @param       无
 * @retval      获取到的随机数(32bit)
 */
uint32_t sys_random_number(void)
{
    while ((RNG->SR & 0X01) == 0);  /* 等待随机数就绪 */

    return RNG->DR;
}

/**
 * @brief       得到某个范围内的随机数
 * @param       min,max: 最小,最大值.
 * @retval      得到的随机数(rval),满足:min<=rval<=max
 */
int sys_random_number_range(int min, int max)
{
    return sys_random_number() % (max - min + 1) + min;
}















