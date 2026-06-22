#include "config_gpio.h"


/************************************************************************************** 
	0-输入模式 1-通用输出 2-复用功能 3-模拟模式
	0-推挽输出 1-开漏输出
	0-低速模式 1-中速模式 2-高速模式 3-超高速 
	0-无上下拉 1-上拉模式 2-下拉模式 3-保留
****************************************************************************************/
void _config_gpio_init(GPIO_TypeDef *p_gpiox, uint32_t pinx, uint32_t mode, uint32_t otype, uint32_t ospeed, uint32_t pupd)
{
    uint32_t pinpos = 0, pos = 0, curpin = 0;
    for (pinpos = 0; pinpos < 16; pinpos++)
    {
        pos = 1 << pinpos;      /* 一个个位检查 */
        curpin = pinx & pos;    /* 检查引脚是否要设置 */
        if (curpin == pos)      /* 需要设置 */
        {
            p_gpiox->MODER &= ~(3 << (pinpos * 2)); /* 先清除原来的设置 */
            p_gpiox->MODER |= mode << (pinpos * 2); /* 设置新的模式 */

            if ((mode == 0X01) || (mode == 0X02))   /* 如果是输出模式/复用功能模式 */
            {
                p_gpiox->OSPEEDR &= ~(3 << (pinpos * 2));       /* 清除原来的设置 */
                p_gpiox->OSPEEDR |= (ospeed << (pinpos * 2));   /* 设置新的速度值 */
                p_gpiox->OTYPER &= ~(1 << pinpos) ;             /* 清除原来的设置 */
                p_gpiox->OTYPER |= otype << pinpos;             /* 设置新的输出模式 */
            }

            p_gpiox->PUPDR &= ~(3 << (pinpos * 2)); /* 先清除原来的设置 */
            p_gpiox->PUPDR |= pupd << (pinpos * 2); /* 设置新的上下拉 */
        }
    }
}

void config_afio_init(GPIO_TypeDef *p_gpiox, uint16_t pinx, uint8_t afx)
{
    uint32_t pinpos = 0, pos = 0, curpin = 0;
    for (pinpos = 0; pinpos < 16; pinpos++)
    {
        pos = 1 << pinpos;
        curpin = pinx & pos;
        if (curpin == pos) 
        {
            p_gpiox->AFR[pinpos >> 3] &= ~(0X0F << ((pinpos & 0X07) * 4));
            p_gpiox->AFR[pinpos >> 3] |= (uint32_t)afx << ((pinpos & 0X07) * 4);
        }
    }
}
void config_gpio_set(GPIO_TypeDef *p_gpiox, uint16_t pinx, uint8_t status)
{
    if (status & 0X01)
    {
		// 设置GPIOx的pinx为1
        p_gpiox->BSRR |= pinx;              	
    }
    else
    {
		// 设置GPIOx的pinx为0
        p_gpiox->BSRR |= (uint32_t)pinx << 16;   
    }
}


uint8_t config_gpio_get(GPIO_TypeDef *p_gpiox, uint16_t pinx)
{
    //return (p_gpiox->IDR & pinx) ? 1: 0;
	if (p_gpiox->IDR & pinx)
    {
        return 1;   /* pinx的状态为1 */
    }
    else
    {
        return 0;   /* pinx的状态为0 */
    }
}
