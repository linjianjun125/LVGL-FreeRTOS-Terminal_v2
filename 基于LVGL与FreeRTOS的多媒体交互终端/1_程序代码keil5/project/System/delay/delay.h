#ifndef __DELAY_H
#define __DELAY_H

#include "sys.h"
#include "sys.h"



#if SYS_USE_RTOS
    #include "FreeRTOS.h"
    #include "task.h"
#endif



void sys_delay_init(uint16_t sysclk);   /* 初始化延迟函数 */
void sys_delay_ms(uint16_t nms);        /* 延时nms */
void sys_delay_us(uint32_t nus);        /* 延时nus */
uint32_t SYS_GetTick(void);


extern uint32_t g_sys_ms_tick;



#endif





























