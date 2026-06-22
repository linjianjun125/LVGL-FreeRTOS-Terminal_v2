#ifndef _SYS_H
#define _SYS_H

#include "stm32h7xx.h"
#include "core_cm7.h"
#include "stm32h7xx_hal.h"
#include "../drivers/system/usart/usart.h"
#include "../drivers/system/delay/delay.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// 内部辅助宏：接收三个参数，仅返回前两个
#define _CFG_EXTRACT_PORT_PIN(port, pin, af)    port, pin

// 用户调用宏：将包含逗号的参数包展开并提取
#define CFG_GET_PORT_PIN(params)    _CFG_EXTRACT_PORT_PIN(params)

void config_gpio_init(GPIO_TypeDef *GPIOx, uint32_t Pin, uint32_t Alternate, uint32_t Mode, uint32_t Pull, uint32_t Speed);


void config_usart_enable(USART_TypeDef *USARTx);



























#define      ON      1
#define      OFF     0
#define      Write_Through()    do{ *(__IO uint32_t*)0XE000EF9C = 1UL << 2; }while(0)     /* Cache透写模式 */


uint8_t get_icahce_sta(void);
uint8_t get_dcahce_sta(void);
void sys_nvic_set_vector_table(uint32_t baseaddr, uint32_t offset);                       /* 设置中断偏移量 */
void sys_cache_enable(void);                                                              /* 使能STM32H7的L1-Cahce */
uint8_t sys_stm32_clock_init(uint32_t plln, uint32_t pllm, uint32_t pllp, uint32_t pllq); /* 配置系统时钟 */

/* 以下为汇编函数 */
void sys_wfi_set(void);             /* 执行WFI指令 */
void sys_intx_disable(void);        /* 关闭所有中断 */
void sys_intx_enable(void);         /* 开启所有中断 */
void sys_msr_msp(uint32_t addr);    /* 设置栈顶地址 */

#endif

