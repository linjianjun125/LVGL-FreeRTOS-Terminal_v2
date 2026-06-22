#ifndef __MAX17048_IIC_H
#define __MAX17048_IIC_H

#include "sys.h"

/*!< 宏定义 >!*/
#define MAX17048_IIC_ENABLE()       do{ConfigRCC_GPIOC_EN();}while(0)
#define MAX17048_SCL_GPIO           GPIOC
#define MAX17048_SDA_GPIO           GPIOC
#define MAX17048_SCL_PIN            ConfigIO_Pin7
#define MAX17048_SDA_PIN            ConfigIO_Pin6

#define max17048_scl_write(x)       config_gpio_set(MAX17048_SCL_GPIO, MAX17048_SCL_PIN, x)
#define max17048_sda_write(x)       config_gpio_set(MAX17048_SDA_GPIO, MAX17048_SDA_PIN, x)
#define max17048_scl_read()         config_gpio_get(MAX17048_SCL_GPIO, MAX17048_SCL_PIN)
#define max17048_sda_read()         config_gpio_get(MAX17048_SDA_GPIO, MAX17048_SDA_PIN)

uint8_t max17048_addr_scan(uint8_t *buf, uint8_t length);
void max17048_iic_init(void);	
void max17048_iic_start(void);				// 发送IIC开始信号
void max17048_iic_stop(void);				 	// 发送IIC停止信号
void max17048_iic_ack(void);				 	// IIC发送ACK信号
void max17048_iic_nack(void);				 	// IIC不发送ACK信号
uint8_t max17048_iic_wait_ack(void);		 	// IIC等待ACK信号
void max17048_iic_send_byte(uint8_t data);	// IIC发送一个字节
uint8_t max17048_iic_read_byte(uint8_t ack); 	// IIC读取一个字节

#endif
