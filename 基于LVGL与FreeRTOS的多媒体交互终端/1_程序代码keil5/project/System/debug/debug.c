#include "debug.h"



/**
 * @brief       串口初始化函数
 * @param       sclk: 串口X的时钟源频率(单位: MHz)
 *              串口1,6的时钟源来自: rcc_pclk2 = 100Mhz
 *              串口2,3,4,5,7,8的时钟源来自: rcc_pclk1 = 100Mhz
 * @param       baudrate: 波特率, 根据自己需要设置波特率值
 * @retval      无
 */
void sys_debug_init(uint32_t sclk, uint32_t baudrate)
{
    uint32_t temp;
	
	/*!< RCC 使能 >!*/
    DEBUG_USART_ENABLE();
	
	/*!< GPIO 配置 >!*/
    config_gpio_init(DEBAG_USART_TX_GPIO, DEBAG_USART_TX_PIN, 2, 0, 1, 0);
    config_gpio_init(DEBAG_USART_RX_GPIO, DEBAG_USART_RX_PIN, 2, 0, 1, 0);

	/*!< AFIO 配置 >!*/
    config_afio_init(DEBAG_USART_TX_GPIO, DEBAG_USART_TX_PIN, DEBAG_USART_TX_AF);
    config_afio_init(DEBAG_USART_RX_GPIO, DEBAG_USART_RX_PIN, DEBAG_USART_RX_AF);

	/*!< USART 初始化 >!*/
    temp = (sclk * 1000000 + baudrate / 2) / baudrate;
    DEBAG_USART->BRR = temp;       // 波特率设置
    DEBAG_USART->CR1 = 0;          // 清零CR1寄存器
    DEBAG_USART->CR1 |= 0 << 28;   // 设置M1 = 0
    DEBAG_USART->CR1 |= 0 << 12;   // 选择8位字长
    DEBAG_USART->CR1 |= 0 << 15;   // 16倍过采样
    DEBAG_USART->CR1 |= 1 << 3;    // 串口发送使能
    DEBAG_USART->CR1 |= 1 << 0;    // 串口使能
}





/******************************************************************************************
 *                                          串口发送--重定向printf()
 *****************************************************************************************/
#pragma import(__use_no_semihosting)
struct __FILE
{
	int handle;
};
FILE __stdout; 
int _ttywrch(int ch)
{
    ch = ch;
    return ch;
}

void _sys_exit(int x)
{
    x = x;
}

char *_sys_command_string(char *cmd, int len)
{
    return NULL;
}



// 重定义fputc函数, printf函数最终会通过调用fputc输出字符串到串口
int fputc(int ch, FILE *f)
{
    while ((DEBAG_USART->ISR & 0X40) == 0);
    DEBAG_USART->TDR = (uint8_t)ch;  
    return ch;
}


