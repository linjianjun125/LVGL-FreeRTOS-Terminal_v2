#include "./system/sys/sys.h"
#include "./system/usart/usart.h"


UART_HandleTypeDef g_uart1_handle; 

#define RXBUFFERSIZE   1  			// 建议设为 1，这样每收到一个字节都会触发一次回调，方便处理不定长数据
uint8_t g_rx_buffer[RXBUFFERSIZE];  // 接收缓冲区

/**
 * @brief       串口初始化函数 (标准协议层配置)
 * @param       baudrate: 波特率
 * @retval      无
 */
void usart_init(uint32_t baudrate)
{
    g_uart1_handle.Instance          = USART_UX;                // 串口实例
    g_uart1_handle.Init.BaudRate     = baudrate;                // 波特率
    g_uart1_handle.Init.WordLength   = UART_WORDLENGTH_8B;      // 8位数据位
    g_uart1_handle.Init.StopBits     = UART_STOPBITS_1;         // 1个停止位
    g_uart1_handle.Init.Parity       = UART_PARITY_NONE;        // 无奇偶校验
    g_uart1_handle.Init.HwFlowCtl     = UART_HWCONTROL_NONE;     // 无硬件流控
    g_uart1_handle.Init.Mode         = UART_MODE_TX_RX;         // 收发模式
    HAL_UART_Init(&g_uart1_handle);
    
#if USART_EN_RX
    // 开启中断接收
    HAL_UART_Receive_IT(&g_uart1_handle, (uint8_t *)g_rx_buffer, RXBUFFERSIZE);
#endif
}


/**
 * @brief       串口底层驱动初始化 (标准硬件层配置)
 * @note        此函数会被 HAL_UART_Init() 自动调用
 * @param       huart: 串口句柄
 * @retval      无
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART_UX)
    {
        // 1. 使能串口外设时钟
        config_usart_enable(USART_UX);

        // 2. GPIO 引脚配置 (TX & RX)
        config_gpio_init(USART_TX_GPIO_PARAMS, GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH);
        config_gpio_init(USART_RX_GPIO_PARAMS, GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH);

#if USART_EN_RX
        // 3. 中断配置
        HAL_NVIC_SetPriority(USART_UX_IRQn, 3, 3);    // 设置优先级
        HAL_NVIC_EnableIRQ(USART_UX_IRQn);            // 使能中断通道
#endif
    }
}



 

 

// 1. 禁用半主机模式 (兼容 AC5 和 AC6)
#if (__ARMCC_VERSION >= 6010050)
    __asm(".global __use_no_semihosting\n\t");
    __asm(".global __ARM_use_no_argv\n\t");
#else
    #pragma import(__use_no_semihosting)
    struct __FILE { int handle; };
#endif

// 2. 实现标准库缺少的桩函数 (防止链接半主机库)
int _ttywrch(int ch) { return ch; }
void _sys_exit(int x) { (void)x; }
char *_sys_command_string(char *cmd, int len) { return NULL; }

// 3. 重定向 printf 到 USART_UX
FILE __stdout;

int fputc(int ch, FILE *f)
{
    // 等待发送缓冲区为空 (TXE)
	while (!(USART_UX->ISR & 0X40)); 

    // 数据写入 TDR 寄存器 (Transmit Data Register)
    USART_UX->TDR = (uint8_t)ch;
    return ch;
}

 

 





