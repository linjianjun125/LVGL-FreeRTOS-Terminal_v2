#include "sensor_spi.h"


/**
 * @brief       SPI初始化代码
 *   @note      主机模式,8位数据,禁止硬件片选
 * @param       无
 * @retval      无
 */
void sensor_spi_init(void)
{
    uint32_t tempreg = 0;

    SENSOR_SPI_ENABLE();

    config_gpio_init(SENSOR_SPI_SCLK_GPIO_PIN, ConfigIO_Mode_AF, ConfigIO_Omode_PP, ConfigIO_Speed_HIG, ConfigIO_Pupd_PU); 
    config_gpio_init(SENSOR_SPI_MISO_GPIO_PIN, ConfigIO_Mode_AF, ConfigIO_Omode_PP, ConfigIO_Speed_HIG, ConfigIO_Pupd_PU);  
    config_gpio_init(SENSOR_SPI_MOSI_GPIO_PIN, ConfigIO_Mode_AF, ConfigIO_Omode_PP, ConfigIO_Speed_HIG, ConfigIO_Pupd_PU);
    config_afio_init(SENSOR_SPI_SCLK_GPIO_PIN, SENSOR_SPI_SCLK_AF);   
    config_afio_init(SENSOR_SPI_MISO_GPIO_PIN, SENSOR_SPI_MISO_AF); 
    config_afio_init(SENSOR_SPI_MOSI_GPIO_PIN, SENSOR_SPI_MOSI_AF); 



    /* 配置SPI1/2/3的时钟源, 选择pll1_q_ck作为时钟源, 200Mhz */
    RCC->D2CCIP1R &= ~(7 << 12);    		/* SPI123SEL[2:0]=0,清除原来的设置 */
    RCC->D2CCIP1R |= 0 << 12;       		/* SPI123SEL[2:0]=1,选择pll1_q_ck作为SPI1/2/3的时钟源,一般为200Mhz */
 
    /* 这里只针对SPI口初始化 */
    RCC->APB1LRSTR |= 1 << 14;      		/* 复位SENSOR_SPI */
    RCC->APB1LRSTR &= ~(1 << 14);   		/* 停止复位SENSOR_SPI */

    SENSOR_SPI->CR1 |= 1 << 12;       		/* SSI=1,设置内部SS信号为高电平 */
    SENSOR_SPI->CFG1 = 7 << 28;       		/* MBR[2:0]=7,设置spi_ker_ck为256分频. */
    SENSOR_SPI->CFG1 |= 7 << 0;       		/* DSIZE[4:0]=7,设置SPI帧格式为8位,即字节传输 */
    tempreg = (uint32_t)1 << 31;    		/* AFCNTR=1,SPI保持对IO口的控制 */
    tempreg |= 0 << 29;             		/* SSOE=0,禁止硬件NSS输出 */
    tempreg |= 1 << 26;             		/* SSM=1,软件管理NSS脚 */
    tempreg &= ~(1 << 25);             		/* CPOL=0,空闲状态下,SCK为高电平 */
    tempreg &= ~(1 << 24);             		/* CPHA=1,数据采样从第2个时间边沿开始 */
    tempreg |= 0 << 23;             		/* LSBFRST=0,MSB先传输 */
    tempreg |= 1 << 22;             		/* MASTER=1,主机模式 */
    tempreg |= 0 << 19;             		/* SP[2:0]=0,摩托罗拉格式 */
    tempreg |= 0 << 17;             		/* COMM[1:0]=0,全双工通信 */
    SENSOR_SPI->CFG2 = tempreg;       		/* 设置CFG2寄存器 */
    SENSOR_SPI->I2SCFGR &= ~(1 << 0); 		/* 选择SPI模式 */
    SENSOR_SPI->CR1 |= 1 << 0;        		/* SPE=1,使能SENSOR_SPI */

    sensor_spi_read_write_byte(0xff);     	/* 启动传输 */
}



/**
 * @brief       SENSOR_SPI读写一个字节数据
 * @param       txdata: 要发送的数据(1字节)
 * @retval      接收到的数据(1字节)
 */
uint8_t sensor_spi_read_write_byte(uint8_t txdata)
{
    uint8_t rxdata = 0;
    SENSOR_SPI->CR1 |= 1 << 0;    /* SPE=1,使能SENSOR_SPI */
    SENSOR_SPI->CR1 |= 1 << 9;    /* CSTART=1,启动传输 */

    while ((SENSOR_SPI->SR & 1 << 1) == 0);   /* 等待发送区空 */

    /* 发送一个byte,以传输长度访问TXDR寄存器 */
    *(volatile uint8_t *)&SENSOR_SPI->TXDR = txdata;  


    while ((SENSOR_SPI->SR & 1 << 0) == 0);   /* 等待接收完一个byte */

    /* 接收一个byte,以传输长度访问RXDR寄存器 */
    rxdata = *(volatile uint8_t *)&SENSOR_SPI->RXDR;  

    SENSOR_SPI->IFCR |= 3 << 3;   /* EOTC和TXTFC置1,清除EOT和TXTFC位 */
    SENSOR_SPI->CR1 &= ~(1 << 0); /* SPE=0,关闭SENSOR_SPI,会执行状态机复位/FIFO重置等操作 */
    return rxdata;              /* 返回收到的数据 */
}






