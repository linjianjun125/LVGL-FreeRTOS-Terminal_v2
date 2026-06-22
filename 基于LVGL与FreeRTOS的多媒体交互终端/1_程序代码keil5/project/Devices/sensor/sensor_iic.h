#ifndef __SENSOR_IIC_H
#define __SENSOR_IIC_H

#include "sys.h"

/*!< 宏定义 >!*/
#define SENSOR_IIC_ENABLE()       do{ConfigRCC_GPIOH_EN();}while(0)
#define SENSOR_SCL_GPIO           GPIOH
#define SENSOR_SDA_GPIO           GPIOH
#define SENSOR_SCL_PIN            ConfigIO_Pin4
#define SENSOR_SDA_PIN            ConfigIO_Pin5

#define sensor_scl_write(x)       config_gpio_set(SENSOR_SCL_GPIO, SENSOR_SCL_PIN, x)
#define sensor_sda_write(x)       config_gpio_set(SENSOR_SDA_GPIO, SENSOR_SDA_PIN, x)
#define sensor_scl_read()         config_gpio_get(SENSOR_SCL_GPIO, SENSOR_SCL_PIN)
#define sensor_sda_read()         config_gpio_get(SENSOR_SDA_GPIO, SENSOR_SDA_PIN)


void sensor_iic_init(void);	
void sensor_iic_start(void);				// 发送IIC开始信号
void sensor_iic_stop(void);				 	// 发送IIC停止信号
void sensor_iic_ack(void);				 	// IIC发送ACK信号
void sensor_iic_nack(void);				 	// IIC不发送ACK信号
uint8_t sensor_iic_wait_ack(void);		 	// IIC等待ACK信号
void sensor_iic_send_byte(uint8_t data);	// IIC发送一个字节
uint8_t sensor_iic_read_byte(uint8_t ack); 	// IIC读取一个字节

#endif
