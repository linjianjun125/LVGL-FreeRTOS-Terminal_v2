#include "pwr.h"



void pwr_wkup_key_init(void);


/**
 * @brief   进入CPU睡眠模式
 * @param   无
 * @retval  无
 */
void pwr_enter_sleep(void)
{
	pwr_wkup_key_init();
	
	/* 清除WKUP上的中断标志位 */
    EXTI_D1->PR1 = PWR_WKUP_GPIO_PIN;   
    
    /* 确认所有指令都已完成 */
    __DSB ();
    __ISB ();
    
	/* 执行WFI指令, 进入待机模式 */
    sys_wfi_set();                      
}

/**
 * @brief   进入停止模式
 * @param   无
 * @retval  无
 */
void pwr_enter_stop(void)
{
	pwr_wkup_key_init();
	/* 清除WKUP上的中断标志位 */
    EXTI_D1->PR1 = PWR_WKUP_GPIO_PIN;   

    PWR->CR1 |= 1 << 0;             /* LPDS=1, 停止模式下电压调节器处于低功耗模式 */ 
    SCB->SCR |= 1 << 2;             /* 使能SLEEPDEEP位 */
    PWR->CPUCR &= ~(7 << 0);        /* PDDS_D1/D2/D3 = 0, 保持D1/D2/D3进入深度睡眠后,进入停止模式(PDDS=0) */

    /* 确认所有指令都已完成 */
    __DSB ();
    __ISB ();
    
	/* 执行WFI指令, 进入停止模式 */
    sys_wfi_set();                  
    
	/* 关闭SLEEPDEEP位 */
    SCB->SCR &= ~(1 << 2);          
}

/**
 * @brief   进入待机模式
 * @param   无
 * @retval  无
 */
void pwr_enter_standby(void)
{
	pwr_wkup_key_init();
    uint32_t tempreg;   /* 零时存储寄存器值用 */

    EXTI_D1->PR1 = PWR_WKUP_GPIO_PIN;   /* 清除WKUP上的中断标志位 */
    
    /* STM32F4/F7/H7,当开启了RTC相关中断后,必须先关闭RTC中断,再清中断标志位,然后重新设置
     * RTC中断,再进入待机模式才可以正常唤醒,否则会有问题.
     */
    PWR->CR1 |= 1 << 8;         		/* 后备区域写使能 */
    
    /* 关闭RTC寄存器写保护 */
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;
    tempreg = RTC->CR & (0X0F << 12);   /* 记录原来的RTC中断设置 */
    RTC->CR &= ~(0XF << 12);    		/* 关闭RTC所有中断 */
    RTC->ISR &= ~(0X3F << 8);   		/* 清除所有RTC中断标志. */
    RTC->CR |= tempreg;         		/* 重新设置RTC中断(如果还是被RTC周期性唤醒,屏蔽此行代码即可) */
    RTC->WPR = 0xFF;            		/* 使能RTC寄存器写保护 */
    
    sys_standby();  					/* 进入待机模式 */
}





/**
 * @brief       低功耗模式下的按键初始化(用于唤醒睡眠模式/停止模式/待机模式)
 * @param       无
 * @retval      无
 */
void pwr_wkup_key_init(void)
{
    PWR_WKUP_GPIO_CLK_ENABLE();     /* WKUP时钟使能 */

    config_gpio_init(PWR_WKUP_GPIO_PORT, PWR_WKUP_GPIO_PIN, 0, 0, 0, 2); 

    sys_nvic_ex_config(PWR_WKUP_GPIO_PORT, PWR_WKUP_GPIO_PIN, SYS_GPIO_RTIR);   /* WKUP配置为上升沿触发中断 */ 
    config_nvic_init( 2, 2, PWR_WKUP_INT_IRQn);
}

/**
 * @brief       WK_UP按键 外部中断服务程序
 * @param       无
 * @retval      无
 */
void PWR_WKUP_INT_IRQHandler(void)
{
    EXTI_D1->PR1 = PWR_WKUP_GPIO_PIN;   /* 清除WKUP所在中断线 的中断标志位 */
}



























