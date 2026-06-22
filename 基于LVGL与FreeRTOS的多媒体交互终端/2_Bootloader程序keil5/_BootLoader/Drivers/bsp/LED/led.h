#ifndef __LED_H
#define __LED_H

#include "sys.h"


// 引脚定义：包含 端口, 引脚号, 复用值 (0表示普通GPIO) 
#define LED0_GPIO_PARAMS      GPIOH, GPIO_PIN_8, 0
#define LED1_GPIO_PARAMS      GPIOH, GPIO_PIN_5, 0



// LED 专用初始化
void led_init(void);

// LED0 相关接口
void led0(uint8_t x);
void led0_toggle(void);

// LED1 相关接口
void led1(uint8_t x);
void led1_toggle(void);  

#endif


