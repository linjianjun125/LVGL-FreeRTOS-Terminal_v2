#include "audio_es8388_iic.h"
#include "../../system/delay/delay.h"
#include "../../system/debug/debug.h"

#if SYS_USE_RTOS
	#include "app_include.h"
#endif


/**
 * @brief       初始化IIC
 * @param       无
 * @retval      无
 */
void audio_iic_init(void)
{
    AUDIO_IIC_ENABLE();

	config_gpio_init(AUDIO_SCL_GPIO, AUDIO_SCL_PIN, ConfigIO_Mode_OUT, ConfigIO_Omode_PP, ConfigIO_Speed_MID, ConfigIO_Pupd_PU);
    config_gpio_init(AUDIO_SDA_GPIO, AUDIO_SDA_PIN, ConfigIO_Mode_OUT, ConfigIO_Omode_OD, ConfigIO_Speed_MID, ConfigIO_Pupd_PU);

    audio_iic_stop(); 
	
#if SYS_USE_RTOS
	xSemaphoreGive(es8388_Semaphore);
#endif
}



uint8_t es8388_addr_scan(void)
{
	uint8_t addr;
	for(addr = 0x00; addr < 0x80; addr++)
	{
		audio_iic_start();
		audio_iic_send_byte(addr);
		if(!audio_iic_wait_ack())
		{
			//printf("[INFO] es8388_addr = %x",addr >> 1);
			audio_iic_stop();
			return addr >> 1;
		}
		audio_iic_stop();
	}
	return 0xff;
}





// 写寄存器
uint8_t es8388_write_reg(uint8_t reg, uint8_t val)
{

#if SYS_USE_RTOS
	xSemaphoreTake(es8388_Semaphore, portMAX_DELAY);
#endif

    audio_iic_start();

    audio_iic_send_byte((ES8388_ADDR << 1) | 0);
    if (audio_iic_wait_ack()){
        return 1;                           
    }
	

    audio_iic_send_byte(reg);                 
    if (audio_iic_wait_ack()){
        return 2;                 
    }

    audio_iic_send_byte(val & 0xFF);       
    if (audio_iic_wait_ack()){
        return 3;         
    }

    audio_iic_stop();

#if SYS_USE_RTOS
	xSemaphoreGive(es8388_Semaphore);
#endif

    return 0;
}

// 读寄存器
uint8_t es8388_read_reg(uint8_t reg)
{

#if SYS_USE_RTOS
	xSemaphoreTake(es8388_Semaphore, portMAX_DELAY);
#endif

    uint8_t temp = 0;

    audio_iic_start();
    audio_iic_send_byte((ES8388_ADDR << 1) | 0); 
    if (audio_iic_wait_ack()){
        return 1;             
    }

    audio_iic_send_byte(reg);       
    if (audio_iic_wait_ack()){
        return 1;           
    }

    audio_iic_start();
    audio_iic_send_byte((ES8388_ADDR << 1) | 1);  
    if (audio_iic_wait_ack()){
        return 1;          
    }

    temp = audio_iic_read_byte(0);
    audio_iic_stop();

#if SYS_USE_RTOS
	xSemaphoreGive(es8388_Semaphore);
#endif

    return temp;
}




/****************************************************************************************************
											IIC
******************************************************************************************************/
/**
 * @brief       产生IIC起始信号
 * @param       无
 * @retval      无
 */
void audio_iic_start(void)
{
    audio_sda_write(1);
    audio_scl_write(1);
    sys_delay_us(2);
    audio_sda_write(0);     /* START信号: 当SCL为高时, SDA从高变成低, 表示起始信号 */
    sys_delay_us(2);
    audio_scl_write(0);     /* 钳住I2C总线，准备发送或接收数据 */
    sys_delay_us(2);
}

/**
 * @brief       产生IIC停止信号
 * @param       无
 * @retval      无
 */
void audio_iic_stop(void)
{
    audio_sda_write(0);     /* STOP信号: 当SCL为高时, SDA从低变成高, 表示停止信号 */
    sys_delay_us(2);
    audio_scl_write(1);
    sys_delay_us(2);
    audio_sda_write(1);     /* 发送I2C总线结束信号 */
    sys_delay_us(2);
}

/**
 * @brief       等待应答信号到来
 * @param       无
 * @retval      1，接收应答失败
 *              0，接收应答成功
 */
uint8_t audio_iic_wait_ack(void)
{
    uint8_t waittime = 0;
    uint8_t rack = 0;

    audio_sda_write(1);             /* 主机释放SDA线(此时外部器件可以拉低SDA线) */
    sys_delay_us(2);
    audio_scl_write(1);             /* SCL=1, 此时从机可以返回ACK */
    sys_delay_us(2);

    while (audio_sda_read())    /* 等待应答 */
    {
        waittime++;

        if (waittime > 250)
        {
            audio_iic_stop();
            rack = 1;
            break;
        }
    }

    audio_scl_write(0);             /* SCL=0, 结束ACK检查 */
    sys_delay_us(2);
    return rack;
}

/**
 * @brief       产生ACK应答
 * @param       无
 * @retval      无
 */
void audio_iic_ack(void)
{
    audio_sda_write(0);     /* SCL 0 -> 1 时 SDA = 0,表示应答 */
    sys_delay_us(2);
    audio_scl_write(1);     /* 产生一个时钟 */
    sys_delay_us(2);
    audio_scl_write(0);
    sys_delay_us(2);
    audio_sda_write(1);     /* 主机释放SDA线 */
    sys_delay_us(2);
}

/**
 * @brief       不产生ACK应答
 * @param       无
 * @retval      无
 */
void audio_iic_nack(void)
{
    audio_sda_write(1);     /* SCL 0 -> 1  时 SDA = 1,表示不应答 */
    sys_delay_us(2);
    audio_scl_write(1);     /* 产生一个时钟 */
    sys_delay_us(2);
    audio_scl_write(0);
    sys_delay_us(2);
}

/**
 * @brief       IIC发送一个字节
 * @param       data: 要发送的数据
 * @retval      无
 */
void audio_iic_send_byte(uint8_t data)
{
    uint8_t t;
    
    for (t = 0; t < 8; t++)
    {
        audio_sda_write((data & 0x80) >> 7);    /* 高位先发送 */
        sys_delay_us(2);
        audio_scl_write(1);
        sys_delay_us(2);
        audio_scl_write(0);
        data <<= 1;     		/* 左移1位,用于下一次发送 */
    }
    audio_sda_write(1);         /* 发送完成, 主机释放SDA线 */
}

/**
 * @brief       IIC读取一个字节
 * @param       ack:  ack=1时，发送ack; ack=0时，发送nack
 * @retval      接收到的数据
 */
uint8_t audio_iic_read_byte(uint8_t ack)
{
    uint8_t i, receive = 0;

    for (i = 0; i < 8; i++ )    /* 接收1个字节数据 */
    {
        receive <<= 1;          /* 高位先输出,所以先收到的数据位要左移 */
        audio_scl_write(1);
        sys_delay_us(2);

        if (audio_sda_read()){
            receive++;
        }
        
        audio_scl_write(0);
        sys_delay_us(2);
    }


    if (!ack){
        audio_iic_nack();             /* 发送nACK */
    }
    else{
        audio_iic_ack();              /* 发送ACK */
    }

    return receive;
}


