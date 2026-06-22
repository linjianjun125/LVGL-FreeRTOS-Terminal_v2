#include "sensor_spi_port.h"
#include "sensor_spi.h"




void icm42688_spi_cs_init(void)
{

	sensor_spi_init();
	config_gpio_init(ICM42688_SPI_CSN_GPIO_PIN,  ConfigIO_Mode_OUT, ConfigIO_Omode_PP, ConfigIO_Speed_HIG, ConfigIO_Pupd_PU);	
}



uint8_t icm42688_read_regs(uint8_t reg, uint8_t *pbuf, uint8_t len)
{
    uint8_t status, i;
	
    ICM42688_SPI_CSN(0);    						
    status = sensor_spi_read_write_byte(ICM42688_READ_OPA | reg); 		

    for (i = 0; i < len; i++)
    {
        pbuf[i] = sensor_spi_read_write_byte(0XFF);  
    }
    
    ICM42688_SPI_CSN(1);    					
    return status;      						
}


uint8_t icm42688_write_regs(uint8_t reg, uint8_t *pbuf, uint8_t len)
{
    uint8_t status, i;
	
    ICM42688_SPI_CSN(0);    					
    status = sensor_spi_read_write_byte(ICM42688_WRTE_OPA | reg);	

    for (i = 0; i < len; i++)
    {
        sensor_spi_read_write_byte(*pbuf++); 	
    }
    
    ICM42688_SPI_CSN(1);    						
    return status;      
}


