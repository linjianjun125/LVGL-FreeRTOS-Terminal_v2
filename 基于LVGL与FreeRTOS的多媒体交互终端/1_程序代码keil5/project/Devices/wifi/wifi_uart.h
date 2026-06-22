#ifndef __WIFI_USART_H
#define __WIFI_USART_H


#include "sys.h"
#include "sys.h"


#define WIFI_USART_ENABLE()			do{ConfigRCC_GPIOA_EN();ConfigRCC_GPIOB_EN();SYS_RCC_UART4_EN();}while(0)
#define WIFI_USART_TX_GPIO         	GPIOA
#define WIFI_USART_RX_GPIO         	GPIOA
#define WIFI_USART_TX_PIN          	ConfigIO_Pin0
#define WIFI_USART_RX_PIN          	ConfigIO_Pin1
#define WIFI_USART_TX_AF           	8
#define WIFI_USART_RX_AF           	8
#define WIFI_USART			        UART4
#define WIFI_IRQn                  	UART4_IRQn
#define WIFI_USART_IRQHandler       UART4_IRQHandler


void wifi_uart_init(uint32_t pclk2, uint32_t bound);

void uart_send_string(char *str);


// 定义回调函数：参数为数据指针和数据长度
typedef void (*wifi_uart_callback_t)(const uint8_t *data, uint16_t len);

void wifi_uart_set_callback(wifi_uart_callback_t cb);









#endif  
















