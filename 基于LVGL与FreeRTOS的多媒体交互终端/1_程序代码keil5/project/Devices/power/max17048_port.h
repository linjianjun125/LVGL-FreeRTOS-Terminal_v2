#ifndef __MAX17048_PORT_H
#define __MAX17048_PORT_H

#include "sys.h"


#define MAX17048_IIC_ADDR   0X36	
#define MAX17048_IIC_W      0X00	
#define MAX17048_IIC_R      0X01	

/*!< API >!*/			
uint8_t max17048_write_reg(uint8_t reg, uint8_t data);
uint8_t max17048_write_regs(uint8_t reg, uint8_t *buf, uint8_t len);
uint8_t max17048_read_reg(uint8_t reg);
void max17048_read_regs(uint8_t reg, uint8_t *buf, uint8_t len);


#endif

