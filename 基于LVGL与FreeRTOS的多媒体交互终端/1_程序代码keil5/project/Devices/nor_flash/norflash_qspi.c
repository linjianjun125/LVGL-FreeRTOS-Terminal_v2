#include "norflash_qspi.h"


/**
 * @brief       等待状态标志
 * @param       flag : 需要等待的标志位
 * @param       sta  : 需要等待的状态
 * @param       wtime: 等待时间
 * @retval      0, 等待成功; 1, 等待失败.
 */
uint8_t qspi_wait_flag(uint32_t flag, uint8_t sta, uint32_t wtime)
{
    uint8_t flagsta = 0;

    while (wtime)
    {
        flagsta = (QUADSPI->SR & flag) ? 1 : 0; /* 获取状态标志 */

        if (flagsta == sta)break;

        wtime--;
    }

    if (wtime)return 0;
    else return 1;
}

/**
 * @brief       初始化QSPI接口
 * @param       无
 * @retval      0, 成功; 1, 失败.
 */
uint8_t qspi_init(void)
{
    
	/*!< RCC 使能 >!*/
	NORFLASH_QSPI_ENABLE();

	/*!< GPIO 配置 >!*/
	config_gpio_init(NORFLASH_QSPI_IO0_GPIO, NORFLASH_QSPI_IO0_PIN,2, 0, 2, 1);   
	config_gpio_init(NORFLASH_QSPI_IO1_GPIO, NORFLASH_QSPI_IO1_PIN,2, 0, 2, 1);   
	config_gpio_init(NORFLASH_QSPI_IO2_GPIO, NORFLASH_QSPI_IO2_PIN,2, 0, 2, 1);   
	config_gpio_init(NORFLASH_QSPI_IO3_GPIO, NORFLASH_QSPI_IO3_PIN,2, 0, 2, 1);   
	config_gpio_init(NORFLASH_QSPI_CLK_GPIO, NORFLASH_QSPI_CLK_PIN,2, 0, 2, 1);   
	config_gpio_init(NORFLASH_QSPI_NCS_GPIO, NORFLASH_QSPI_NCS_PIN,2, 0, 2, 1);   

	/*!< AFIO 配置 >!*/
	config_afio_init(NORFLASH_QSPI_IO0_GPIO, NORFLASH_QSPI_IO0_PIN, NORFLASH_QSPI_IO0_AF);
	config_afio_init(NORFLASH_QSPI_IO1_GPIO, NORFLASH_QSPI_IO1_PIN, NORFLASH_QSPI_IO1_AF);
	config_afio_init(NORFLASH_QSPI_IO2_GPIO, NORFLASH_QSPI_IO2_PIN, NORFLASH_QSPI_IO2_AF);
	config_afio_init(NORFLASH_QSPI_IO3_GPIO, NORFLASH_QSPI_IO3_PIN, NORFLASH_QSPI_IO3_AF);
	config_afio_init(NORFLASH_QSPI_CLK_GPIO, NORFLASH_QSPI_CLK_PIN, NORFLASH_QSPI_CLK_AF);
	config_afio_init(NORFLASH_QSPI_NCS_GPIO, NORFLASH_QSPI_NCS_PIN, NORFLASH_QSPI_NCS_AF);

	/*!< QSPI 初始化 >!*/
    RCC->AHB3RSTR |= 1 << 14;       
    RCC->AHB3RSTR &= ~(1 << 14);    
    if (qspi_wait_flag(1 << 5, 0, 0XFFFF) == 0) /* 等待BUSY空闲 */
    {
		uint32_t tempreg = 0;
        tempreg = (2 - 1) << 24;    /* 设置QSPI时钟默认为AHB3时钟，我们设置使用2分频, 即200M / 2 = 100Mhz,10ns */
        tempreg |= (4 - 1) << 8;    /* 设置FIFO阈值为4个字节(最大为31,表示32个字节) */
        tempreg |= 0 << 7;          /* 选择FLASH1 */
        tempreg |= 0 << 6;          /* 禁止双闪存模式 */
        tempreg |= 1 << 4;          /* 采样移位半个周期(DDR模式下,必须设置为0) */
        QUADSPI->CR = tempreg;      /* 设置CR寄存器 */
        tempreg = (25 - 1) << 16;   /* 设置FLASH大小为2^25=32MB(为了适配sys.c里面的内存映射设置，实际上这里设置16M也是可以的) */
        tempreg |= (3 - 1) << 8;    /* 片选高电平时间为3个时钟(10*3=30ns),即手册里面的tSHSL参数 */
        tempreg |= 1 << 0;          /* Mode3,空闲时CLK为高电平 */
        QUADSPI->DCR = tempreg;     /* 设置DCR寄存器 */
        QUADSPI->CR |= 1 << 0;      /* 使能QSPI */
    }
    else
    {
        return 1;   /* 不成功 */
    }
    
    return 0;
}

/**
 * @brief       QSPI发送命令
 * @param       cmd : 要发送的指令
 * @param       addr: 发送到的目的地址
 * @param       mode: 模式,详细位定义如下:
 *   @arg       mode[1:0]: 指令模式; 00,无指令;  01,单线传输指令; 10,双线传输指令; 11,四线传输指令.
 *   @arg       mode[3:2]: 地址模式; 00,无地址;  01,单线传输地址; 10,双线传输地址; 11,四线传输地址.
 *   @arg       mode[5:4]: 地址长度; 00,8位地址; 01,16位地址;     10,24位地址;     11,32位地址.
 *   @arg       mode[7:6]: 数据模式; 00,无数据;  01,单线传输数据; 10,双线传输数据; 11,四线传输数据.
 * @param       dmcycle: 空指令周期数
 * @retval      无
 */
void qspi_send_cmd(uint8_t cmd, uint32_t addr, uint8_t mode, uint8_t dmcycle)
{
    uint32_t tempreg = 0;
    uint8_t status;

    if (qspi_wait_flag(1 << 5, 0, 0XFFFF) == 0) /* 等待BUSY空闲 */
    {
        tempreg = 0 << 31;                      /* 禁止DDR模式 */
        tempreg |= 0 << 28;                     /* 每次都发送指令 */
        tempreg |= 0 << 26;                     /* 间接写模式 */
        tempreg |= ((uint32_t)mode >> 6) << 24; /* 设置数据模式 */
        tempreg |= (uint32_t)dmcycle << 18;     /* 设置空指令周期数 */
        tempreg |= ((uint32_t)(mode >> 4) & 0X03) << 12;    /* 设置地址长度 */
        tempreg |= ((uint32_t)(mode >> 2) & 0X03) << 10;    /* 设置地址模式 */
        tempreg |= ((uint32_t)(mode >> 0) & 0X03) << 8;     /* 设置指令模式 */ 
        tempreg |= cmd;                     /* 设置指令 */
        QUADSPI->CCR = tempreg;             /* 设置CCR寄存器 */

        if (mode & 0X0C)                    /* 有指令+地址要发送 */
        {
            QUADSPI->AR = addr;             /* 设置地址寄存器 */
        }

        if ((mode & 0XC0) == 0)             /* 无数据传输,等待指令发送完成 */
        {
            status = qspi_wait_flag(1 << 1, 1, 0XFFFF); /* 等待TCF,即传输完成 */

            if (status == 0)
            {
                QUADSPI->FCR |= 1 << 1;     /* 清除TCF标志位 */
            }
        }
    }
}

/**
 * @brief       QSPI接收指定长度的数据
 * @param       buf     : 接收数据缓冲区首地址
 * @param       datalen : 要传输的数据长度
 * @retval      0, 成功; 其他, 错误代码.
 */
uint8_t qspi_receive(uint8_t *buf, uint32_t datalen)
{
    uint32_t tempreg = QUADSPI->CCR;
    uint32_t addrreg = QUADSPI->AR;
    uint8_t status;
    volatile uint32_t *data_reg = &QUADSPI->DR;
    
    QUADSPI->DLR = datalen - 1; /* 设置数据传输长度 */
    tempreg &= ~(3 << 26);      /* 清除FMODE原来的设置 */
    tempreg |= 1 << 26;         /* 设置FMODE为间接读取模式 */
    QUADSPI->FCR |= 1 << 1;     /* 清除TCF标志位 */
    QUADSPI->CCR = tempreg;     /* 回写CCR寄存器 */
    QUADSPI->AR = addrreg;      /* 回写AR寄存器,触发传输 */

    while (datalen)
    {
        status = qspi_wait_flag(3 << 1, 1, 0XFFFF); /* 等到FTF和TCF,即接收到了数据 */

        if (status == 0)        /* 等待成功 */
        {
            *buf++ = *(volatile uint8_t *)data_reg;
            datalen--;
        }
        else
        {
            break;
        }
    }

    if (status == 0)
    {
        QUADSPI->CR |= 1 << 2;  /* 终止传输 */
        status = qspi_wait_flag(1 << 1, 1, 0XFFFF); /* 等待TCF,即数据传输完成 */

        if (status == 0)
        {
            QUADSPI->FCR |= 1 << 1; /* 清除TCF标志位 */
            status = qspi_wait_flag(1 << 5, 0, 0XFFFF); /* 等待BUSY位清零 */
        }
    }

    return status;
}

/**
 * @brief       QSPI发送指定长度的数据
 * @param       buf     : 发送数据缓冲区首地址
 * @param       datalen : 要传输的数据长度
 * @retval      0, 成功; 其他, 错误代码.
 */
uint8_t qspi_transmit(uint8_t *buf, uint32_t datalen)
{
    uint32_t tempreg = QUADSPI->CCR;
    uint32_t addrreg = QUADSPI->AR;
    uint8_t status;
    volatile uint32_t *data_reg = &QUADSPI->DR;
    
    QUADSPI->DLR = datalen - 1; /* 设置数据传输长度 */
    tempreg &= ~(3 << 26);      /* 清除FMODE原来的设置 */
    tempreg |= 0 << 26;         /* 设置FMODE为间接写入模式 */
    QUADSPI->FCR |= 1 << 1;     /* 清除TCF标志位 */
    QUADSPI->CCR = tempreg;     /* 回写CCR寄存器 */

    while (datalen)
    {
        status = qspi_wait_flag(1 << 2, 1, 0XFFFF); /* 等到FTF */

        if (status != 0)        /* 等待成功 */
        {
            break;
        }

        *(volatile uint8_t *)data_reg = *buf++;
        datalen--;
    }

    if (status == 0)
    {
        QUADSPI->CR |= 1 << 2;      /* 终止传输 */
        status = qspi_wait_flag(1 << 1, 1, 0XFFFF);     /* 等待TCF,即数据传输完成 */

        if (status == 0)
        {
            QUADSPI->FCR |= 1 << 1; /* 清除TCF标志位 */
            status = qspi_wait_flag(1 << 5, 0, 0XFFFF); /* 等待BUSY位清零 */
        }
    }

    return status;
}




















