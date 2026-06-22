#ifndef __USART_H
#define __USART_H


#include "sys.h"
#include "sys.h"


#define DEBUG_USART_ENABLE()			do{ConfigRCC_GPIOA_EN();SYS_RCC_USART1_EN();}while(0)
#define DEBAG_USART_TX_GPIO         	GPIOA
#define DEBAG_USART_RX_GPIO         	GPIOA
#define DEBAG_USART_TX_PIN          	ConfigIO_Pin9
#define DEBAG_USART_RX_PIN          	ConfigIO_Pin10
#define DEBAG_USART_TX_AF           	7
#define DEBAG_USART_RX_AF           	7
#define DEBAG_USART			        	USART1
#define DEBAG_IRQn                  	USART1_IRQn
#define DEBAG_IRQHandler            	USART1_IRQHandler


void sys_debug_init(uint32_t pclk2, uint32_t bound);

















#endif  
















