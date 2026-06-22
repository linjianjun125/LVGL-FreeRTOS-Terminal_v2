#include "wifi_uart.h"
#include "../../system/delay/delay.h"
#include "../../system/debug/debug.h"

#define WIFI_RX_BUF_MAX  1024

// 全局变量
uint8_t g_wifi_rx_buf[WIFI_RX_BUF_MAX]; 
uint16_t g_wifi_rx_cnt = 0;
static wifi_uart_callback_t g_uart_rx_callback = NULL;







/**
 * @brief 串口发送字符串 (修正版)
 */
void uart_send_string(char *str)
{
    uint32_t timeout;
    if (str == NULL) return;

    while (*str != '\0')
    {
        timeout = 0xFFFF;
        
        /* 1. 等待 TXE (发送寄存器空, 0x80) */
        /* 注意：这里改成了 0x80，比 0x40 效率更高 */
        while (((WIFI_USART->ISR & 0x80) == 0) && (--timeout > 0));

        if (timeout == 0) break; 

        /* 2. 写入数据到 TDR */
        WIFI_USART->TDR = (uint8_t)(*str);
        str++;
    }
}




/**
 * @brief 注册串口接收完成回调函数
 */
void wifi_uart_set_callback(wifi_uart_callback_t cb) {
    g_uart_rx_callback = cb;
}





void wifi_uart_init(uint32_t sclk, uint32_t baudrate)
{
    uint32_t temp;
	
	// RCC 使能
    WIFI_USART_ENABLE();
	
	// GPIO 配置 
    config_gpio_init(WIFI_USART_TX_GPIO, WIFI_USART_TX_PIN, 2, 0, 1, 0);
    config_gpio_init(WIFI_USART_RX_GPIO, WIFI_USART_RX_PIN, 2, 0, 1, 0);

	// AFIO 配置
    config_afio_init(WIFI_USART_TX_GPIO, WIFI_USART_TX_PIN, WIFI_USART_TX_AF);
    config_afio_init(WIFI_USART_RX_GPIO, WIFI_USART_RX_PIN, WIFI_USART_RX_AF);

	config_gpio_init(GPIOB, ConfigIO_Pin1,  ConfigIO_Mode_OUT, ConfigIO_Omode_PP, ConfigIO_Speed_MID, ConfigIO_Pupd_PU);
	config_gpio_set(GPIOB, ConfigIO_Pin1, 1);

	// USART 初始化
    temp = (sclk * 1000000 + baudrate / 2) / baudrate;
    WIFI_USART->BRR = temp;       // 波特率设置
    WIFI_USART->CR1 = 0;          // 清零CR1寄存器
    WIFI_USART->CR1 |= 0 << 28;   // 设置M1 = 0
    WIFI_USART->CR1 |= 0 << 12;   // 选择8位字长
    WIFI_USART->CR1 |= 0 << 15;   // 16倍过采样
    WIFI_USART->CR1 |= 1 << 3;    // 串口发送使能
   
	
	#if 1  
    WIFI_USART->CR1 |= 1 << 2;    		// 串口接收使能
    WIFI_USART->CR1 |= 1 << 5;    		// 接收缓冲区非空中断使能
	WIFI_USART->CR1 |= 1 << 4;   		// IDLEIE: 使能空闲检测中断
    config_nvic_init(7, 0, WIFI_IRQn);	
	#endif


	WIFI_USART->CR1 |= 1 << 0;    // 串口使能
	sys_delay_ms(200);
}

void WIFI_USART_IRQHandler(void)
{
    // H7 中读取状态寄存器为 ISR
    uint32_t isr = WIFI_USART->ISR; 
    
    // 1. 检查 RXNE (位5): 接收缓冲区非空
    if (isr & (1 << 5))
    {
        // H7 中读取数据寄存器为 RDR
        uint8_t res = (uint8_t)WIFI_USART->RDR; 
        
        if (g_wifi_rx_cnt < WIFI_RX_BUF_MAX)
        {
            g_wifi_rx_buf[g_wifi_rx_cnt++] = res;
        }
		printf("%c", isr);
    }

    // 2. 检查 IDLE (位4): 检测到总线空闲
    if (isr & (1 << 4))
    {
        /* 
         * H7 清除 IDLE 标志位的标准方法：
         * 向 ICR 寄存器的位 4 (IDLECF) 写入 1
         */
        WIFI_USART->ICR = (1 << 4); 

        if (g_wifi_rx_cnt > 0)
        {
            // 执行回调函数
            if (g_uart_rx_callback != NULL) 
            {
                g_uart_rx_callback(g_wifi_rx_buf, g_wifi_rx_cnt);
            }
            
            // 重置计数器
            g_wifi_rx_cnt = 0;
        }
    }

    // 3. 错误处理（可选但建议）：H7 串口如果发生溢出（ORE）不处理会死机
    if (isr & ((1 << 3) | (1 << 1) | (1 << 0))) // ORE, FE, NE 错误
    {
        WIFI_USART->ICR = (1 << 3) | (1 << 1) | (1 << 0); // 清除错误标志
    }
}
