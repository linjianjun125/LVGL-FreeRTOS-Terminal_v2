#ifndef __CONFIG_NVIC_H
#define __CONFIG_NVIC_H

#include "stm32h7xx.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>




#define SYS_GPIO_FTIR           1       /* 下降沿触发 */
#define SYS_GPIO_RTIR           2       /* 上升沿触发 */
#define SYS_GPIO_BTIR           3       /* 任意边沿触发 */

void config_nvic_init(uint8_t pprio, uint8_t sprio, uint8_t ch);            		// 设置NVIC
void sys_nvic_priority_group_config(uint8_t group);								// 设置NVIC分组
void sys_nvic_set_vector_table(uint32_t baseaddr, uint32_t offset);             // 设置中断偏移量
void sys_nvic_ex_config(GPIO_TypeDef *p_gpiox, uint16_t pinx, uint8_t tmode);   // 外部中断配置函数,只针对GPIOA~GPIOK










#endif

