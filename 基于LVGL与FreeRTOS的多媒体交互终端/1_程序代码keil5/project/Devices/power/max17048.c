#include "max17048.h"
#include "max17048_iic.h"
#include "max17048_port.h"
#include "../../system/debug/debug.h"



void max17048_init(void)
{
	
	
	
	config_gpio_init(GPIOA, ConfigIO_Pin8, ConfigIO_Mode_IN, ConfigIO_Omode_OD, ConfigIO_Speed_MID, ConfigIO_Pupd_NO);
	config_gpio_init(GPIOD, ConfigIO_Pin11, ConfigIO_Mode_IN, ConfigIO_Omode_OD, ConfigIO_Speed_MID, ConfigIO_Pupd_NO);
	
	uint8_t iic_addr[2];
	max17048_iic_init();
	max17048_addr_scan(iic_addr, 2);
	if(iic_addr[0] == MAX17048_IIC_ADDR)
	{
		printf("[info] max17048 初始化成功\r\n");
	}
	else
	{
		printf("[info] max17048 初始化失败\r\n");
		return ;
	}
}


float max17048_get_voltage(void)
{
	uint8_t data[2];
	
	max17048_read_regs(0x02, data, 2);
	
	uint16_t v = (data[0] << 8) | data[1];
	float d = (float)v * 78.125 / 1000 / 1000;
	
	//printf("%x%x\r\n", data[0], data[1]);
	//printf("[info] voltage = %f\r\n", d);
	return d;
}

float max17048_get_quantity(void)
{
	uint8_t data[2];

	max17048_read_regs(0x04, data, 2);
	
	uint16_t v = (data[0] << 8) | data[1];
	float d = (float)v / 256;
	
	if(d > 100.0f) d = 100.0f;
	if(d < 0.0f)   d = 0.0f;
	//printf("%x%x\r\n", data[0], data[1]);
	//printf("[info] quantity = %f\r\n", d);
	return d;
}

bool max17048_is_charging(void)
{
	uint8_t status = config_gpio_get(GPIOA, ConfigIO_Pin8);
	//printf("%d\r\n", status);
	return (status == 1) ? true : false;
	
}

bool max17048_has_bat(void)
{
	uint8_t status = config_gpio_get(GPIOD, ConfigIO_Pin11);
	//printf("%d\r\n", status);
	return (status == 1) ? true : false;
	
}