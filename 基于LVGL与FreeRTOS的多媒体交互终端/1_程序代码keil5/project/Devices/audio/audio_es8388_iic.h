#ifndef __AUDIO_ES8388_IIC_H
#define __AUDIO_ES8388_IIC_H

#include "sys.h"


/*!< 宏定义 >!*/
#define AUDIO_IIC_ENABLE()       do{ConfigRCC_GPIOC_EN();ConfigRCC_GPIOI_EN();}while(0)
#define AUDIO_SCL_GPIO           GPIOI
#define AUDIO_SDA_GPIO           GPIOC
#define AUDIO_SCL_PIN            ConfigIO_Pin8
#define AUDIO_SDA_PIN            ConfigIO_Pin13

#define audio_scl_write(x)       config_gpio_set(AUDIO_SCL_GPIO, AUDIO_SCL_PIN, x)
#define audio_sda_write(x)       config_gpio_set(AUDIO_SDA_GPIO, AUDIO_SDA_PIN, x)
#define audio_scl_read()         config_gpio_get(AUDIO_SCL_GPIO, AUDIO_SCL_PIN)
#define audio_sda_read()         config_gpio_get(AUDIO_SDA_GPIO, AUDIO_SDA_PIN)


#define ES8388_ADDR     0x10 	

void audio_iic_init(void); 
uint8_t es8388_addr_scan(void);
uint8_t es8388_read_reg(uint8_t reg);
uint8_t es8388_write_reg(uint8_t reg, uint8_t val);

void audio_iic_start(void);				 /* 发送IIC开始信号 */
void audio_iic_stop(void);				 /* 发送IIC停止信号 */
void audio_iic_ack(void);				 /* IIC发送ACK信号  */
void audio_iic_nack(void);				 /* IIC不发送ACK信号 */
uint8_t audio_iic_wait_ack(void);		 /* IIC等待ACK信号  */
void audio_iic_send_byte(uint8_t data);	 /* IIC发送一个字节 */
uint8_t audio_iic_read_byte(uint8_t ack); /* IIC读取一个字节 */

#endif


