#ifndef __SYS_H
#define __SYS_H

#include "config.h"
#include "config_rcc.h"
#include "config_gpio.h"
#include "config_nvic.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "debug/debug.h"
#include "delay/delay.h"




#define SYS_USE_RTOS					1

#define BOOTLOADER_ADDR					(0x8000000)
#define BOOTLOADER_SIZE					(0x20000)

/*< SRAMEX使用 >*/



extern uint8_t game_common_buffer[];



// RGB-LTDC
#define SYS_SRAMEX_RGBLCD_START_ADDR	(0XC0000000 + 1024*1024*1)
#define SYS_SRAMEX_RGBLCD_SIZE			(480*480*2)
// LVGL缓存
#define SYS_SRAMEX_UI_START_ADDR		(SYS_SRAMEX_RGBLCD_START_ADDR + SYS_SRAMEX_RGBLCD_SIZE)
#define SYS_SRAMEX_UI_SIZE				(480*480*2*2)
// MALLOC
#define SYS_SRAMEX_MALLOC_START_ADDR	(SYS_SRAMEX_UI_START_ADDR + SYS_SRAMEX_UI_SIZE)




// 等待flag变为flag_set，超时时间ms_to_wait，超时退出
#define SYS_WAIT_FLAG_BIT_SET_1(flag, flag_wait, ms_to_wait)  		do{ 														\
																	while(((flag) & (flag_wait) == 0) && (ms_to_wait > 0)) 		\
																	{ 															\
																		sys_delay_ms(1); 										\
																		ms_to_wait--; 											\
																	} 															\
																	}while(0)
															
#define SYS_WAIT_FLAG_BIT_SET_0(flag, flag_wait, ms_to_wait)  		do{ 														\
																	while(((flag) & (flag_wait) != 0) && (ms_to_wait > 0)) 		\
																	{ 															\
																		sys_delay_ms(1); 										\
																		ms_to_wait--; 											\
																	} 															\
																	}while(0)															
												 

// 返回值
#define SYS_RET_OK      0
#define SYS_RET_ERR     1
#define Return_pNULL    NULL



// 大小端转化
#define BIG_LITTLE_SWAP16(x)        ( (((*(short int *)&x) & 0xff00) >> 8) | \
                                      (((*(short int *)&x) & 0x00ff) << 8) )
 
#define BIG_LITTLE_SWAP32(x)        ( (((*(long int *)&x) & 0xff000000) >> 24) | \
                                      (((*(long int *)&x) & 0x00ff0000) >> 8) | \
                                      (((*(long int *)&x) & 0x0000ff00) << 8) | \
                                      (((*(long int *)&x) & 0x000000ff) << 24) )
 
#define BIG_LITTLE_SWAP64(x)        ( (((*(long long int *)&x) & 0xff00000000000000) >> 56) | \
                                      (((*(long long int *)&x) & 0x00ff000000000000) >> 40) | \
                                      (((*(long long int *)&x) & 0x0000ff0000000000) >> 24) | \
                                      (((*(long long int *)&x) & 0x000000ff00000000) >> 8) | \
                                      (((*(long long int *)&x) & 0x00000000ff000000) << 8) | \
                                      (((*(long long int *)&x) & 0x0000000000ff0000) << 24) | \
                                      (((*(long long int *)&x) & 0x000000000000ff00) << 40) | \
                                      (((*(long long int *)&x) & 0x00000000000000ff) << 56) )
						  



/*!< 系统函数 >!*/
void sys_wfi_set(void);             // 执行WFI指令
void sys_intx_disable(void);        // 关闭所有中断
void sys_intx_enable(void);         // 开启所有中断
void sys_msr_msp(uint32_t addr);    // 设置栈顶地址 
void sys_standby(void);             // 进入待机模式
void sys_soft_reset(void);          // 系统软复位
void sys_cache_enable(void);        // 使能 Cache










#endif











