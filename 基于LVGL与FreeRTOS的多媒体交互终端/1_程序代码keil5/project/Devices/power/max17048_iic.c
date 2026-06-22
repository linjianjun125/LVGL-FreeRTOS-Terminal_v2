#include "max17048_iic.h"
#include "../../system/delay/delay.h"

void max17048_iic_delay(uint8_t ns)
{
	sys_delay_us(2);
}


/**
 * @brief       初始化IIC
 * @param       无
 * @retval      无
 */
void max17048_iic_init(void)
{
    MAX17048_IIC_ENABLE();

    config_gpio_init(MAX17048_SCL_GPIO, MAX17048_SCL_PIN, ConfigIO_Mode_OUT, ConfigIO_Omode_PP, ConfigIO_Speed_MID, ConfigIO_Pupd_PU);
    config_gpio_init(MAX17048_SDA_GPIO, MAX17048_SDA_PIN, ConfigIO_Mode_OUT, ConfigIO_Omode_OD, ConfigIO_Speed_MID, ConfigIO_Pupd_PU);
    
    max17048_iic_stop(); 
}



uint8_t max17048_addr_scan(uint8_t *buf, uint8_t length)
{
	uint8_t addr;
	uint8_t i = 0;
	uint8_t res;
	
	for(addr = 0x00; addr < 0x80; addr++)
	{
		max17048_iic_start();
		max17048_iic_send_byte(addr);
		if(!max17048_iic_wait_ack())
		{
			buf[i] = addr >> 1;
			i++;
			if(i > length - 1) break;
		}
		max17048_iic_stop();
	}
	
	res = (i == 0 ? 1 : 0);
	return res;
}


/**
 * @brief       产生IIC起始信号
 * @param       无
 * @retval      无
 */
void max17048_iic_start(void)
{
    max17048_sda_write(1);
    max17048_scl_write(1);
    max17048_iic_delay(2);
    max17048_sda_write(0);     /* START信号: 当SCL为高时, SDA从高变成低, 表示起始信号 */
    max17048_iic_delay(2);
    max17048_scl_write(0);     /* 钳住I2C总线，准备发送或接收数据 */
    max17048_iic_delay(2);
}

/**
 * @brief       产生IIC停止信号
 * @param       无
 * @retval      无
 */
void max17048_iic_stop(void)
{
    max17048_sda_write(0);     /* STOP信号: 当SCL为高时, SDA从低变成高, 表示停止信号 */
    max17048_iic_delay(2);
    max17048_scl_write(1);
    max17048_iic_delay(2);
    max17048_sda_write(1);     /* 发送I2C总线结束信号 */
    max17048_iic_delay(2);
}

/**
 * @brief       等待应答信号到来
 * @param       无
 * @retval      1，接收应答失败
 *              0，接收应答成功
 */
uint8_t max17048_iic_wait_ack(void)
{
    uint8_t waittime = 0;
    uint8_t rack = 0;

    max17048_sda_write(1);             /* 主机释放SDA线(此时外部器件可以拉低SDA线) */
    max17048_iic_delay(2);
    max17048_scl_write(1);             /* SCL=1, 此时从机可以返回ACK */
    max17048_iic_delay(2);

    while (max17048_sda_read())    /* 等待应答 */
    {
        waittime++;

        if (waittime > 250)
        {
            max17048_iic_stop();
            rack = 1;
            break;
        }
    }

    max17048_scl_write(0);             /* SCL=0, 结束ACK检查 */
    max17048_iic_delay(2);
    return rack;
}

/**
 * @brief       产生ACK应答
 * @param       无
 * @retval      无
 */
void max17048_iic_ack(void)
{
    max17048_sda_write(0);     /* SCL 0 -> 1 时 SDA = 0,表示应答 */
    max17048_iic_delay(2);
    max17048_scl_write(1);     /* 产生一个时钟 */
    max17048_iic_delay(2);
    max17048_scl_write(0);
    max17048_iic_delay(2);
    max17048_sda_write(1);     /* 主机释放SDA线 */
    max17048_iic_delay(2);
}

/**
 * @brief       不产生ACK应答
 * @param       无
 * @retval      无
 */
void max17048_iic_nack(void)
{
    max17048_sda_write(1);     /* SCL 0 -> 1  时 SDA = 1,表示不应答 */
    max17048_iic_delay(2);
   max17048_scl_write(1);     /* 产生一个时钟 */
    max17048_iic_delay(2);
   max17048_scl_write(0);
    max17048_iic_delay(2);
}

/**
 * @brief       IIC发送一个字节
 * @param       data: 要发送的数据
 * @retval      无
 */
void max17048_iic_send_byte(uint8_t data)
{
    uint8_t t;
    
    for (t = 0; t < 8; t++)
    {
        max17048_sda_write((data & 0x80) >> 7);    /* 高位先发送 */
        max17048_iic_delay(2);
        max17048_scl_write(1);
        max17048_iic_delay(2);
        max17048_scl_write(0);
        data <<= 1;     		/* 左移1位,用于下一次发送 */
    }
    max17048_sda_write(1);         /* 发送完成, 主机释放SDA线 */
}

/**
 * @brief       IIC读取一个字节
 * @param       ack:  ack=1时，发送ack; ack=0时，发送nack
 * @retval      接收到的数据
 */
uint8_t max17048_iic_read_byte(uint8_t ack)
{
    uint8_t i, receive = 0;

    for (i = 0; i < 8; i++ )    /* 接收1个字节数据 */
    {
        receive <<= 1;          /* 高位先输出,所以先收到的数据位要左移 */
        max17048_scl_write(1);
        max17048_iic_delay(2);

        if (max17048_sda_read())
        {
            receive++;
        }
        
        max17048_scl_write(0);
        max17048_iic_delay(2);
    }

    if (!ack)
    {
        max17048_iic_nack();             /* 发送nACK */
    }
    else
    {
        max17048_iic_ack();              /* 发送ACK */
    }

    return receive;
}


