#ifndef __LCDRGB_BLK_H
#define __LCDRGB_BLK_H

#include "config_gpio.h"
#include "sys.h"

#define LCDRGB_GPIO_ENABLE()            	do{ConfigRCC_GPIOD_EN();}while(0)
#define LCDRGB_RES_GPIO_PIN 				GPIOD, ConfigIO_Pin4

#define LCD_RES_Set() 						config_gpio_set(LCDRGB_RES_GPIO_PIN, 1)
#define LCD_RES_Clr() 						config_gpio_set(LCDRGB_RES_GPIO_PIN, 0)




/* TIMX PWM输出定义 默认是针对TIM2~TIM5, TIM9~TIM14.*/
#define LCDRGB_BLK_TIM_ENABLE()             do{ConfigRCC_GPIOA_EN();SYS_RCC_TIM2_EN();}while(0)
#define LCDRGB_BLK_GPIO_PIN                 GPIOA, ConfigIO_Pin15
#define LCDRGB_BLK_GPIO_AF                  1

#define GTIM_TIMX_PWM                       TIM2
#define GTIM_TIMX_PWM_CHY                   1                           			// 通道1-4
#define GTIM_TIMX_PWM_CHY_CCRX              TIM2->CCR1                  			// 通道Y的输出比较寄存器


void lcdrgb_io_init(void);
void lcdrgb_set_pwm_duty_cycle(uint8_t value);

#endif

