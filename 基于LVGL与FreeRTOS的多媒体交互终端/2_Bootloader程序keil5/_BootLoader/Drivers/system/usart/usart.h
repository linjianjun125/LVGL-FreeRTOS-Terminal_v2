#ifndef _USART_H
#define _USART_H

#include "stdio.h"
#include "./system/sys/sys.h"


// --- 串口与引脚定义 ---
#define USART_TX_GPIO_PARAMS        GPIOA, GPIO_PIN_9, GPIO_AF7_USART1
#define USART_RX_GPIO_PARAMS        GPIOA, GPIO_PIN_10, GPIO_AF7_USART1

#define USART_UX                        USART1
#define USART_UX_IRQn                   USART1_IRQn
#define USART_UX_IRQHandler             USART1_IRQHandler


void usart_init(uint32_t baudrate);  

#endif



