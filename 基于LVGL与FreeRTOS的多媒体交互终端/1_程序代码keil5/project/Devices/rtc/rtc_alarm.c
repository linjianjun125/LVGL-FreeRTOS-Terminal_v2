#include "rtc_alarm.h"
#include "../../system/debug/debug.h"
#include "../../system/delay/delay.h"



/**
 * @breif   设置闹钟时间(按星期闹铃,24小时制)
 * @param   week: 星期几(1~7)
 * @param   hour,min,sec: 小时,分钟,秒钟
 * @retval  无
 */
void rtc_set_alarma(uint8_t week, uint8_t hour, uint8_t min, uint8_t sec)
{
    /* 关闭RTC寄存器写保护 */
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;
    RTC->CR &= ~(1 << 8); /* 关闭闹钟A */

    while ((RTC->ISR & 0X01) == 0)
        ; /* 等待闹钟A修改允许 */

    RTC->ALRMAR = 0;                                  /* 清空原来设置 */
    RTC->ALRMAR |= 1 << 30;                           /* 按星期闹铃 */
    RTC->ALRMAR |= 0 << 22;                           /* 24小时制 */
    RTC->ALRMAR |= (uint32_t)rtc_dec2bcd(week) << 24; /* 星期设置 */
    RTC->ALRMAR |= (uint32_t)rtc_dec2bcd(hour) << 16; /* 小时设置 */
    RTC->ALRMAR |= (uint32_t)rtc_dec2bcd(min) << 8;   /* 分钟设置 */
    RTC->ALRMAR |= (uint32_t)rtc_dec2bcd(sec);        /* 秒钟设置 */
    RTC->ALRMASSR = 0;                                /* 不使用SUB SEC */
    RTC->CR |= 1 << 12;                               /* 开启闹钟A中断 */
    RTC->CR |= 1 << 8;                                /* 开启闹钟A */
    RTC->WPR = 0XFF;                                  /* 禁止修改RTC寄存器 */

    RTC->ISR &= ~(1 << 8);                  /* 清除RTC闹钟A的标志 */
    EXTI_D1->PR1 = 1 << 17;                 /* 清除LINE17上的中断标志位 */
    EXTI_D1->IMR1 |= 1 << 17;               /* 开启line17上的中断 */
    EXTI->RTSR1 |= 1 << 17;                 /* line17上事件上升降沿触发 */
    config_nvic_init(2, 2, RTC_Alarm_IRQn); 	/* 抢占2，子优先级2 */
}






/**
 * @breif       周期性唤醒定时器设置
 * @param       wksel
 *   @arg       000,RTC/16;001,RTC/8;010,RTC/4;011,RTC/2;
 *   @arg       10x,ck_spre,1Hz;11x,1Hz,且cnt值增加2^16(即cnt+2^16)
 * @note        注意:RTC就是RTC的时钟频率,即RTCCLK!
 * @param       cnt: 自动重装载值.减到0,产生中断.
 * @retval      无
 */
void rtc_set_wakeup(uint8_t wksel, uint16_t cnt)
{
    /* 关闭RTC寄存器写保护 */
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;
    RTC->CR &= ~(1 << 10); /* 关闭WAKE UP */

    while ((RTC->ISR & 0X04) == 0)
        ; /* 等待WAKE UP修改允许 */

    RTC->CR &= ~(7 << 0);                  /* 清除原来的设置 */
    RTC->CR |= wksel & 0X07;               /* 设置新的值 */
    RTC->WUTR = cnt;                       /* 设置WAKE UP自动重装载寄存器值 */
    RTC->ISR &= ~(1 << 10);                /* 清除RTC WAKE UP的标志 */
    RTC->CR |= 1 << 14;                    /* 开启WAKE UP 定时器中断 */
    RTC->CR |= 1 << 10;                    /* 开启WAKE UP 定时器 */
    RTC->WPR = 0XFF;                       /* 禁止修改RTC寄存器 */
    EXTI_D1->PR1 = 1 << 19;                /* 清除LINE19上的中断标志位 */
    EXTI_D1->IMR1 |= 1 << 19;              /* 开启line19上的中断 */
    EXTI->RTSR1 |= 1 << 19;                /* line19上事件上升降沿触发 */
    config_nvic_init(2, 2, RTC_WKUP_IRQn);    /* 抢占2，子优先级2 */
}






/*------------------------------------------------------------
                    中断服务函数
--------------------------------------------------------------*/

void RTC_Alarm_IRQHandler(void)
{
    if (RTC->ISR & (1 << 8)) /* ALARM A中断? */
    {
        RTC->ISR &= ~(1 << 8); /* 清除中断标志 */
        printf("ALARM A!\r\n");
    }
	
    EXTI_D1->PR1 |= 1 << 17; /* 清除中断线17的中断标志 */
}


/**
 * @breif       RTC WAKE UP中断服务函数
 * @param       无
 * @retval      无
 */
void RTC_WKUP_IRQHandler(void)
{
    if (RTC->ISR & (1 << 10)) /* WK_UP中断? */
    {
        RTC->ISR &= ~(1 << 10); /* 清除中断标志 */
        // LED1_TOGGLE();
    }
    EXTI_D1->PR1 |= 1 << 19; /* 清除中断线19的中断标志 */
}
