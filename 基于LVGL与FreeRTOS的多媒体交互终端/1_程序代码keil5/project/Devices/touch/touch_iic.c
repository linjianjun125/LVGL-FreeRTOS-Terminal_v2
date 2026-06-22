#include "touch_iic.h"
#include "../../system/delay/delay.h"




/**
 * @brief       初始化IIC
 * @param       无
 * @retval      无
 */
void lcd_touch_iic_init(void)
{
    LCD_TOUCH_IIC_ENABLE();

    config_gpio_init(LCD_TOUCH_SCL_GPIO, LCD_TOUCH_SCL_PIN, ConfigIO_Mode_OUT, ConfigIO_Omode_PP, ConfigIO_Speed_MID, ConfigIO_Pupd_PU);
    config_gpio_init(LCD_TOUCH_SDA_GPIO, LCD_TOUCH_SDA_PIN, ConfigIO_Mode_OUT, ConfigIO_Omode_OD, ConfigIO_Speed_MID, ConfigIO_Pupd_PU);
    
    lcd_touch_iic_stop(); 
}


/**
 * @brief       向触摸屏写入一次数据
 * @param       reg : 起始寄存器地址
 * @param       buf : 数据缓缓存区
 * @param       len : 写数据长度
 * @retval      0, 成功; 1, 失败;
 */
uint8_t lcd_touch_write_reg(uint16_t reg, uint8_t data)
{
    uint8_t ret = 0;
    lcd_touch_iic_start();
    lcd_touch_iic_send_byte(TOUCH_CMD_WR);    	/* 发送写命令 */
    lcd_touch_iic_wait_ack();
    lcd_touch_iic_send_byte(reg);         		/* 发送8位地址 */
    lcd_touch_iic_wait_ack();

	lcd_touch_iic_send_byte(data);       		/* 发数据 */
	ret = lcd_touch_iic_wait_ack();
    lcd_touch_iic_stop();                     /* 产生一个停止条件 */
    return ret;
}

/**
 * @brief       从触摸屏读出一次数据
 * @param       reg : 起始寄存器地址
 * @param       buf : 数据缓缓存区
 * @param       len : 读数据长度
 * @retval      无
 */
void lcd_touch_read_reg(uint16_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t i;
    lcd_touch_iic_start();
    lcd_touch_iic_send_byte(TOUCH_CMD_WR);                        	/* 发送写命令 */
    lcd_touch_iic_wait_ack();
    lcd_touch_iic_send_byte(reg);                             		/* 发送8位地址 */
    lcd_touch_iic_wait_ack();
	
    lcd_touch_iic_start();
    lcd_touch_iic_send_byte(TOUCH_CMD_RD);                        	/* 发送读命令 */
    lcd_touch_iic_wait_ack();
    for (i = 0; i < len; i++)
    {
        buf[i] = lcd_touch_iic_read_byte(i == (len - 1) ? 0 : 1);  /* 读取数据 */
    }

    lcd_touch_iic_stop();                                          /* 产生一个停止条件 */
}






/**
 * @brief       IIC写入一次数据
 * @param       reg : 起始寄存器地址
 * @param       buf : 数据缓缓存区
 * @param       len : 写数据长度
 * @retval      0, 成功; 1, 失败;
 */
uint8_t lcdmcu_touch_write_reg(uint16_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t i;
    uint8_t ret = 0;
    lcd_touch_iic_start();
    lcd_touch_iic_send_byte(TOUCH_CMD_WR);    /* 发送写命令 */
    lcd_touch_iic_wait_ack();
    lcd_touch_iic_send_byte(reg >> 8);         /* 发送高8位地址 */
    lcd_touch_iic_wait_ack();
    lcd_touch_iic_send_byte(reg & 0XFF);       /* 发送低8位地址 */
    lcd_touch_iic_wait_ack();

    for (i = 0; i < len; i++)
    {
        lcd_touch_iic_send_byte(buf[i]);       /* 发数据 */
        ret = lcd_touch_iic_wait_ack();
        if (ret)
            break;
    }
    lcd_touch_iic_stop();                     /* 产生一个停止条件 */
    return ret;
}

/**
 * @brief       IIC读出一次数据
 * @param       reg : 起始寄存器地址
 * @param       buf : 数据缓缓存区
 * @param       len : 读数据长度
 * @retval      无
 */
void lcdmcu_touch_read_reg(uint16_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t i;
    lcd_touch_iic_start();
    lcd_touch_iic_send_byte(TOUCH_CMD_WR);                        /* 发送写命令 */
    lcd_touch_iic_wait_ack();
    lcd_touch_iic_send_byte(reg >> 8);                             /* 发送高8位地址 */
    lcd_touch_iic_wait_ack();
    lcd_touch_iic_send_byte(reg & 0XFF);                           /* 发送低8位地址 */
    lcd_touch_iic_wait_ack();
    lcd_touch_iic_start();
    lcd_touch_iic_send_byte(TOUCH_CMD_RD);                        /* 发送读命令 */
    lcd_touch_iic_wait_ack();

    for (i = 0; i < len; i++)
    {
        buf[i] = lcd_touch_iic_read_byte(i == (len - 1) ? 0 : 1);  /* 读取数据 */
    }

    lcd_touch_iic_stop();                                          /* 产生一个停止条件 */
}






/**
 * @brief       产生IIC起始信号
 * @param       无
 * @retval      无
 */
void lcd_touch_iic_start(void)
{
    lcd_touch_sda_write(1);
    lcd_touch_scl_write(1);
    sys_delay_us(2);
    lcd_touch_sda_write(0);     /* START信号: 当SCL为高时, SDA从高变成低, 表示起始信号 */
    sys_delay_us(2);
    lcd_touch_scl_write(0);     /* 钳住I2C总线，准备发送或接收数据 */
    sys_delay_us(2);
}

/**
 * @brief       产生IIC停止信号
 * @param       无
 * @retval      无
 */
void lcd_touch_iic_stop(void)
{
    lcd_touch_sda_write(0);     /* STOP信号: 当SCL为高时, SDA从低变成高, 表示停止信号 */
    sys_delay_us(2);
    lcd_touch_scl_write(1);
    sys_delay_us(2);
    lcd_touch_sda_write(1);     /* 发送I2C总线结束信号 */
    sys_delay_us(2);
}

/**
 * @brief       等待应答信号到来
 * @param       无
 * @retval      1，接收应答失败
 *              0，接收应答成功
 */
uint8_t lcd_touch_iic_wait_ack(void)
{
    uint8_t waittime = 0;
    uint8_t rack = 0;

    lcd_touch_sda_write(1);             /* 主机释放SDA线(此时外部器件可以拉低SDA线) */
    sys_delay_us(2);
    lcd_touch_scl_write(1);             /* SCL=1, 此时从机可以返回ACK */
    sys_delay_us(2);

    while (lcd_touch_sda_read())    /* 等待应答 */
    {
        waittime++;

        if (waittime > 250)
        {
            lcd_touch_iic_stop();
            rack = 1;
            break;
        }
    }

    lcd_touch_scl_write(0);             /* SCL=0, 结束ACK检查 */
    sys_delay_us(2);
    return rack;
}

/**
 * @brief       产生ACK应答
 * @param       无
 * @retval      无
 */
void lcd_touch_iic_ack(void)
{
    lcd_touch_sda_write(0);     /* SCL 0 -> 1 时 SDA = 0,表示应答 */
    sys_delay_us(2);
    lcd_touch_scl_write(1);     /* 产生一个时钟 */
    sys_delay_us(2);
    lcd_touch_scl_write(0);
    sys_delay_us(2);
    lcd_touch_sda_write(1);     /* 主机释放SDA线 */
    sys_delay_us(2);
}

/**
 * @brief       不产生ACK应答
 * @param       无
 * @retval      无
 */
void lcd_touch_iic_nack(void)
{
    lcd_touch_sda_write(1);     /* SCL 0 -> 1  时 SDA = 1,表示不应答 */
    sys_delay_us(2);
   lcd_touch_scl_write(1);     /* 产生一个时钟 */
    sys_delay_us(2);
   lcd_touch_scl_write(0);
    sys_delay_us(2);
}

/**
 * @brief       IIC发送一个字节
 * @param       data: 要发送的数据
 * @retval      无
 */
void lcd_touch_iic_send_byte(uint8_t data)
{
    uint8_t t;
    
    for (t = 0; t < 8; t++)
    {
        lcd_touch_sda_write((data & 0x80) >> 7);    /* 高位先发送 */
        sys_delay_us(2);
        lcd_touch_scl_write(1);
        sys_delay_us(2);
        lcd_touch_scl_write(0);
        data <<= 1;     		/* 左移1位,用于下一次发送 */
    }
    lcd_touch_sda_write(1);         /* 发送完成, 主机释放SDA线 */
}

/**
 * @brief       IIC读取一个字节
 * @param       ack:  ack=1时，发送ack; ack=0时，发送nack
 * @retval      接收到的数据
 */
uint8_t lcd_touch_iic_read_byte(uint8_t ack)
{
    uint8_t i, receive = 0;

    for (i = 0; i < 8; i++ )    /* 接收1个字节数据 */
    {
        receive <<= 1;          /* 高位先输出,所以先收到的数据位要左移 */
        lcd_touch_scl_write(1);
        sys_delay_us(2);

        if (lcd_touch_sda_read())
        {
            receive++;
        }
        
        lcd_touch_scl_write(0);
        sys_delay_us(2);
    }

    if (!ack)
    {
        lcd_touch_iic_nack();             /* 发送nACK */
    }
    else
    {
        lcd_touch_iic_ack();              /* 发送ACK */
    }

    return receive;
}


