#include "sensor_iic.h"
#include "../../system/delay/delay.h"

void sensor_iic_delay(uint8_t ns)
{
	sys_delay_us(2);
}


/**
 * @brief       初始化IIC
 * @param       无
 * @retval      无
 */
void sensor_iic_init(void)
{
    SENSOR_IIC_ENABLE();

    config_gpio_init(SENSOR_SCL_GPIO, SENSOR_SCL_PIN, ConfigIO_Mode_OUT, ConfigIO_Omode_PP, ConfigIO_Speed_MID, ConfigIO_Pupd_PU);
    config_gpio_init(SENSOR_SDA_GPIO, SENSOR_SDA_PIN, ConfigIO_Mode_OUT, ConfigIO_Omode_OD, ConfigIO_Speed_MID, ConfigIO_Pupd_PU);
    
    sensor_iic_stop(); 
}






/**
 * @brief       产生IIC起始信号
 * @param       无
 * @retval      无
 */
void sensor_iic_start(void)
{
    sensor_sda_write(1);
    sensor_scl_write(1);
    sensor_iic_delay(2);
    sensor_sda_write(0);     /* START信号: 当SCL为高时, SDA从高变成低, 表示起始信号 */
    sensor_iic_delay(2);
    sensor_scl_write(0);     /* 钳住I2C总线，准备发送或接收数据 */
    sensor_iic_delay(2);
}

/**
 * @brief       产生IIC停止信号
 * @param       无
 * @retval      无
 */
void sensor_iic_stop(void)
{
    sensor_sda_write(0);     /* STOP信号: 当SCL为高时, SDA从低变成高, 表示停止信号 */
    sensor_iic_delay(2);
    sensor_scl_write(1);
    sensor_iic_delay(2);
    sensor_sda_write(1);     /* 发送I2C总线结束信号 */
    sensor_iic_delay(2);
}

/**
 * @brief       等待应答信号到来
 * @param       无
 * @retval      1，接收应答失败
 *              0，接收应答成功
 */
uint8_t sensor_iic_wait_ack(void)
{
    uint8_t waittime = 0;
    uint8_t rack = 0;

    sensor_sda_write(1);             /* 主机释放SDA线(此时外部器件可以拉低SDA线) */
    sensor_iic_delay(2);
    sensor_scl_write(1);             /* SCL=1, 此时从机可以返回ACK */
    sensor_iic_delay(2);

    while (sensor_sda_read())    /* 等待应答 */
    {
        waittime++;

        if (waittime > 250)
        {
            sensor_iic_stop();
            rack = 1;
            break;
        }
    }

    sensor_scl_write(0);             /* SCL=0, 结束ACK检查 */
    sensor_iic_delay(2);
    return rack;
}

/**
 * @brief       产生ACK应答
 * @param       无
 * @retval      无
 */
void sensor_iic_ack(void)
{
    sensor_sda_write(0);     /* SCL 0 -> 1 时 SDA = 0,表示应答 */
    sensor_iic_delay(2);
    sensor_scl_write(1);     /* 产生一个时钟 */
    sensor_iic_delay(2);
    sensor_scl_write(0);
    sensor_iic_delay(2);
    sensor_sda_write(1);     /* 主机释放SDA线 */
    sensor_iic_delay(2);
}

/**
 * @brief       不产生ACK应答
 * @param       无
 * @retval      无
 */
void sensor_iic_nack(void)
{
    sensor_sda_write(1);     /* SCL 0 -> 1  时 SDA = 1,表示不应答 */
    sensor_iic_delay(2);
   sensor_scl_write(1);     /* 产生一个时钟 */
    sensor_iic_delay(2);
   sensor_scl_write(0);
    sensor_iic_delay(2);
}

/**
 * @brief       IIC发送一个字节
 * @param       data: 要发送的数据
 * @retval      无
 */
void sensor_iic_send_byte(uint8_t data)
{
    uint8_t t;
    
    for (t = 0; t < 8; t++)
    {
        sensor_sda_write((data & 0x80) >> 7);    /* 高位先发送 */
        sensor_iic_delay(2);
        sensor_scl_write(1);
        sensor_iic_delay(2);
        sensor_scl_write(0);
        data <<= 1;     		/* 左移1位,用于下一次发送 */
    }
    sensor_sda_write(1);         /* 发送完成, 主机释放SDA线 */
}

/**
 * @brief       IIC读取一个字节
 * @param       ack:  ack=1时，发送ack; ack=0时，发送nack
 * @retval      接收到的数据
 */
uint8_t sensor_iic_read_byte(uint8_t ack)
{
    uint8_t i, receive = 0;

    for (i = 0; i < 8; i++ )    /* 接收1个字节数据 */
    {
        receive <<= 1;          /* 高位先输出,所以先收到的数据位要左移 */
        sensor_scl_write(1);
        sensor_iic_delay(2);

        if (sensor_sda_read())
        {
            receive++;
        }
        
        sensor_scl_write(0);
        sensor_iic_delay(2);
    }

    if (!ack)
    {
        sensor_iic_nack();             /* 发送nACK */
    }
    else
    {
        sensor_iic_ack();              /* 发送ACK */
    }

    return receive;
}


