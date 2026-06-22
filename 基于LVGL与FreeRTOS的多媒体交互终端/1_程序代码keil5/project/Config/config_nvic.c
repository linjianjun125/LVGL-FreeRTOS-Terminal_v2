#include "config_nvic.h"

static uint8_t nvic_priority_group = 2;// 默认使用组别2



/**
 * @brief   设置NVIC(包括分组/抢占优先级/子优先级等)
 * @param   pprio: 抢占优先级(PreemptionPriority)
 * @param   sprio: 子优先级(SubPriority)
 * @param   ch: 中断编号(Channel)
 */
void config_nvic_init(uint8_t pprio, uint8_t sprio, uint8_t ch)
{
    uint32_t temp;

    temp = pprio << (4 - nvic_priority_group);
    temp |= sprio & (0x0f >> nvic_priority_group);
    temp &= 0xf;                            /* 取低四位 */

    NVIC->ISER[ch / 32] |= 1 << (ch % 32);  /* 使能中断位(要清除的话,设置ICER对应位为1即可) */
    NVIC->IP[ch] |= temp << 4;              /* 设置响应优先级和抢断优先级 */
}

/**
 * @brief   设置NVIC分组
 * @param   group: 0~4,共5组
 *   @arg   0, 组0: 0位抢占优先级, 4位子优先级
 *   @arg   1, 组1: 1位抢占优先级, 3位子优先级
 *   @arg   2, 组2: 2位抢占优先级, 2位子优先级
 *   @arg   3, 组3: 3位抢占优先级, 1位子优先级
 *   @arg   4, 组4: 4位抢占优先级, 0位子优先级
 * @retval  无
 */
void sys_nvic_priority_group_config(uint8_t group)
{
    uint32_t temp, temp1;
	
	nvic_priority_group = group;

    temp1 = (~group) & 0x07;
    temp1 <<= 8;

    temp = SCB->AIRCR;     
    temp &= 0X0000F8FF;   
    temp |= 0X05FA0000; 
    temp |= temp1;

    SCB->AIRCR = temp;   
}

/**
 * @brief       外部中断配置函数, 只针对GPIOA~GPIOK
 * @note        该函数会自动开启对应中断, 以及屏蔽线
 * @param       p_gpiox: GPIOA~GPIOK, GPIO指针
 * @param       pinx: 0X0000~0XFFFF, 引脚位置, 每个位代表一个IO, 第0位代表Px0, 第1位代表Px1, 依次类推. 比如0X0101, 代表同时设置Px0和Px8.
 *   @arg       SYS_GPIO_PIN0~SYS_GPIO_PIN15, 1<<0 ~ 1<<15
 * @param       tmode: 1~3, 触发模式
 *   @arg       SYS_GPIO_FTIR, 1, 下降沿触发
 *   @arg       SYS_GPIO_RTIR, 2, 上升沿触发
 *   @arg       SYS_GPIO_BTIR, 3, 任意电平触发
 * @retval      无
 */
void sys_nvic_ex_config(GPIO_TypeDef *p_gpiox, uint16_t pinx, uint8_t tmode)
{
    uint8_t offset;
    uint32_t gpio_num = 0;      /* gpio编号, 0~10, 代表GPIOA~GPIOK */
    uint32_t pinpos = 0, pos = 0, curpin = 0;

    gpio_num = ((uint32_t)p_gpiox - (uint32_t)GPIOA) / 0X400 ;/* 得到gpio编号 */
    RCC->APB4ENR |= 1 << 1;     /* SYSCFGEN = 1,使能SYSCFG时钟 */

    for (pinpos = 0; pinpos < 16; pinpos++)
    {
        pos = 1 << pinpos;      /* 一个个位检查 */
        curpin = pinx & pos;    /* 检查引脚是否要设置 */

        if (curpin == pos)      /* 需要设置 */
        {
            offset = (pinpos % 4) * 4;
            SYSCFG->EXTICR[pinpos / 4] &= ~(0x000F << offset);  /* 清除原来设置！！！ */
            SYSCFG->EXTICR[pinpos / 4] |= gpio_num << offset;   /* EXTI.BITx映射到gpiox.bitx */

            EXTI_D1->IMR1 |= 1 << pinpos;   /* 开启line BITx上的中断(如果要禁止中断，则反操作即可) */

            if (tmode & 0x01) EXTI->FTSR1 |= 1 << pinpos;       /* line bitx上事件下降沿触发 */
            if (tmode & 0x02) EXTI->RTSR1 |= 1 << pinpos;       /* line bitx上事件上升降沿触发 */
        }
    }
}


/**
 * @brief   设置中断向量表偏移地址
 * @param   baseaddr: 基址
 * @param   offset: 偏移量
 * @retval  无
 */
void sys_nvic_set_vector_table(uint32_t baseaddr, uint32_t offset)
{
    /* 设置NVIC的向量表偏移寄存器,VTOR低9位保留,即[8:0]保留 */
    SCB->VTOR = baseaddr | (offset & (uint32_t)0xFFFFFE00);
}

