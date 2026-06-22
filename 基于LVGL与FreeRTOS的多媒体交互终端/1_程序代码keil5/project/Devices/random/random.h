#ifndef __SYS_RNG_H
#define __SYS_RNG_H 

#include "sys.h"


uint8_t sys_random_init(void);             		// RNG初始化
uint32_t sys_random_number(void);  				// 得到随机数
int sys_random_number_range(int min,int max);  	// 得到属于某个范围内的随机数


#endif

















