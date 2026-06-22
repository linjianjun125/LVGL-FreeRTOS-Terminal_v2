#ifndef __KEY_H
#define __KEY_H

#include "sys.h"

// RCC 使能
#define KEY_ENABLE()        		do{ConfigRCC_GPIOB_EN();ConfigRCC_GPIOC_EN();ConfigRCC_GPIOD_EN();}while(0)

// GPIO 引脚定义
#define KEY_UP_GPIO_PIN           	GPIOB, ConfigIO_Pin0
#define KEY_DOWM_GPIO_PIN        	GPIOC, ConfigIO_Pin5
#define KEY_POWER_GPIO_PIN          GPIOD, ConfigIO_Pin13


// GPIO 读写定义 
#define key_up_read()         		config_gpio_get(KEY_UP_GPIO_PIN)
#define key_down_read()         	config_gpio_get(KEY_DOWM_GPIO_PIN)
#define key_power_read()       		config_gpio_get(KEY_POWER_GPIO_PIN)


// 按键按下的值
#define KEY_UP    			1            
#define KEY_DOWN    		2              
#define KEY_POWER    		3              

// 按键模式 >!*/
#define KEY_MODE_SINGLE		0
#define KEY_MODE_CONTIN		1





void power_enable(void);
void power_disable(void);

#define ___POWER_ENABLE()		{extern void power_enable(void); power_enable();}
#define ___POWER_DISABLE()		{extern void power_disable(void); power_disable();}


void key_init(void);                
uint8_t key_scan(uint8_t mode);  

#endif


















