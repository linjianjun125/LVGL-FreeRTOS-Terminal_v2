#ifndef __TMP117_PORT_H
#define __TMP117_PORT_H

#include "sys.h"



uint8_t sensor_addr_scan(uint8_t *buf, uint8_t length);






/*-------------------------------------------------------------
                        三轴磁力计传感器
--------------------------------------------------------------*/
#define QMC5883_IIC_ADDR   0X2C	
#define QMC5883_IIC_W      0X00	
#define QMC5883_IIC_R      0X01	
	
uint8_t qmc5883_write_regs(uint8_t reg, uint8_t *buf, uint8_t len);
void qmc5883_read_regs(uint8_t reg, uint8_t *buf, uint8_t len);

/*-------------------------------------------------------------
                        温湿度传感器
--------------------------------------------------------------*/
#define SHT30_IIC_ADDR   0X44	
#define SHT30_IIC_W      0X00	
#define SHT30_IIC_R      0X01			

uint8_t sht30_write_regs(uint8_t reg, uint8_t *buf, uint8_t len);
void sht30_read_regs(uint8_t *buf, uint8_t len);


/*-------------------------------------------------------------
                        光照传感器
--------------------------------------------------------------*/
#define BH1750_IIC_ADDR 0x23

uint8_t bh1750_write_cmd(uint8_t cmd);
uint8_t bh1750_read_data(uint8_t *buf, uint8_t len);



#endif

