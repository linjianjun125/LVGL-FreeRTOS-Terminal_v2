#include "lcdrgb_blk.h"


void gtim_timx_pwm_chy_init(uint16_t arr, uint16_t psc);


void lcdrgb_io_init(void)
{
    LCDRGB_GPIO_ENABLE();
	config_gpio_init(LCDRGB_RES_GPIO_PIN, ConfigIO_Mode_OUT, ConfigIO_Omode_PP, ConfigIO_Speed_FAS, ConfigIO_Pupd_PU);
	
	gtim_timx_pwm_chy_init(1000 - 1, 200 - 1);
	lcdrgb_set_pwm_duty_cycle(50);
	
}



/**
 * @brief   通用定时器TIMX 通道Y PWM输出 初始化函数（使用PWM模式1）
 * @note	通用定时器的时钟来自APB1,当PPRE1≥2分频的时候
 *          通用定时器的时钟为APB1时钟的2倍, 而APB1为100M, 所以定时器时钟 = 200Mhz
 * @param   arr: 自动重装值。
 * @param  	psc: 时钟预分频数
 * @retval  无
 */
void gtim_timx_pwm_chy_init(uint16_t arr, uint16_t psc)
{
    uint8_t chy = GTIM_TIMX_PWM_CHY;
    
	LCDRGB_BLK_TIM_ENABLE();
	
	config_gpio_init(LCDRGB_BLK_GPIO_PIN, ConfigIO_Mode_AF, ConfigIO_Omode_PP, ConfigIO_Speed_MID, ConfigIO_Pupd_PU);
    config_afio_init(LCDRGB_BLK_GPIO_PIN, LCDRGB_BLK_GPIO_AF);

    GTIM_TIMX_PWM->ARR = arr;       /* 设定计数器自动重装值 */
    GTIM_TIMX_PWM->PSC = psc;       /* 设置预分频器  */
    GTIM_TIMX_PWM->BDTR |= 1 << 15; /* 使能MOE位(仅TIM15/16/17 有此寄存器,必须设置MOE才能输出PWM), 其他通用定时器,
                                     * 这个寄存器是无效的, 所以设置/不设置并不影响结果, 为了兼容这里统一改成设置MOE位*/

    if (chy <= 2)
    {
        GTIM_TIMX_PWM->CCMR1 |= 6 << (4 + 8 * (chy - 1));   /* CH1/2 PWM模式1 */
        GTIM_TIMX_PWM->CCMR1 |= 1 << (3 + 8 * (chy - 1));   /* CH1/2 预装载使能 */
    }
    else if (chy <= 4)
    {
        GTIM_TIMX_PWM->CCMR2 |= 6 << (4 + 8 * (chy - 3));   /* CH3/4 PWM模式1 */
        GTIM_TIMX_PWM->CCMR2 |= 1 << (3 + 8 * (chy - 3));   /* CH3/4 预装载使能 */
    }

    GTIM_TIMX_PWM->CCER |= 1 << (4 * (chy - 1));        /* OCy 输出使能 */
    GTIM_TIMX_PWM->CCER |= 1 << (1 + 4 * (chy - 1));    /* OCy 低电平有效 */
    GTIM_TIMX_PWM->CR1 |= 1 << 7;   					/* ARPE使能 */
    GTIM_TIMX_PWM->CR1 |= 1 << 0;   					/* 使能定时器TIMX */
}


// 映射逻辑：输入 0-100 -> 对应 CCR 寄存器 1000-0
void lcdrgb_set_pwm_duty_cycle(uint8_t value)
{
    if(value > 100) value = 100;
    
    int forward_v = value * 10;
    int v = 1000 - forward_v;
    
   // printf("Input: %d%% -> PWM CCR: %d\n", value, v);
    
    GTIM_TIMX_PWM_CHY_CCRX = v;
}








