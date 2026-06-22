#ifndef __AT24CXX_IIC_H
#define __AT24CXX_IIC_H

#include "sys.h"

/*!< 宏定义 >!*/
#define AT24CXX_IIC_ENABLE()       	do{ConfigRCC_GPIOC_EN();}while(0)
#define AT24CXX_SCL_GPIO_PIN       	GPIOH, ConfigIO_Pin2
#define AT24CXX_SDA_GPIO_PIN       	GPIOH, ConfigIO_Pin3     

#define at24cxx_scl_write(x)       config_gpio_set(AT24CXX_SCL_GPIO_PIN, x)
#define at24cxx_sda_write(x)       config_gpio_set(AT24CXX_SDA_GPIO_PIN, x)
#define at24cxx_scl_read()         config_gpio_get(AT24CXX_SCL_GPIO_PIN)
#define at24cxx_sda_read()         config_gpio_get(AT24CXX_SDA_GPIO_PIN)


void at24cxx_iic_init(void);	
void at24cxx_iic_start(void);				// 发送IIC开始信号
void at24cxx_iic_stop(void);				 	// 发送IIC停止信号
void at24cxx_iic_ack(void);				 	// IIC发送ACK信号
void at24cxx_iic_nack(void);				 	// IIC不发送ACK信号
uint8_t at24cxx_iic_wait_ack(void);		 	// IIC等待ACK信号
void at24cxx_iic_send_byte(uint8_t data);	// IIC发送一个字节
uint8_t at24cxx_iic_read_byte(uint8_t ack); 	// IIC读取一个字节

#endif

