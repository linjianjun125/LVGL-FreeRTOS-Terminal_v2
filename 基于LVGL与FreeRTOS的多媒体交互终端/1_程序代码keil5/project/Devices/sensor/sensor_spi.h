#ifndef __SENSOR_SPI_H
#define __SENSOR_SPI_H

#include "sys.h"


#define SENSOR_SPI_ENABLE()					do{SYS_RCC_SPI1_EN();ConfigRCC_GPIOA_EN();}while(0)
#define SENSOR_SPI                    		SPI1

#define SENSOR_SPI_SCLK_GPIO_PIN             GPIOA, ConfigIO_Pin5
#define SENSOR_SPI_MISO_GPIO_PIN             GPIOA, ConfigIO_Pin6
#define SENSOR_SPI_MOSI_GPIO_PIN             GPIOA, ConfigIO_Pin7
#define SENSOR_SPI_MISO_AF               	5
#define SENSOR_SPI_SCLK_AF                	5
#define SENSOR_SPI_MOSI_AF               	5






void sensor_spi_init(void);
uint8_t sensor_spi_read_write_byte(uint8_t txdata);






#endif





