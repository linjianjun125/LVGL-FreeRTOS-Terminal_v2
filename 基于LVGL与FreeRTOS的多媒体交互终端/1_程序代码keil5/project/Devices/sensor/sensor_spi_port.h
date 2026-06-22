#ifndef __SENSOR_SPI_PORT_H
#define __SENSOR_SPI_PORT_H

#include "sys.h"


/*-------------------------------------------------------------
                        六轴传感器
--------------------------------------------------------------*/
#define ICM42688_SPI_CSN_GPIO_PIN           GPIOA, ConfigIO_Pin4
#define ICM42688_SPI_CSN(x)     			config_gpio_set(ICM42688_SPI_CSN_GPIO_PIN, x)


#define ICM42688_WRTE_OPA		(0X00)	// 写操作
#define ICM42688_READ_OPA		(0X80)	// 读操作

void icm42688_spi_cs_init(void);
uint8_t icm42688_write_regs(uint8_t reg, uint8_t *pbuf, uint8_t uint8_ts);   /* 写数据区 */
uint8_t icm42688_read_regs(uint8_t reg, uint8_t *pbuf, uint8_t uint8_ts);    /* 读数据区 */






#endif
