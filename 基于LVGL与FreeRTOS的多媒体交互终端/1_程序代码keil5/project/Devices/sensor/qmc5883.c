#include "qmc5883.h"
#include "sensor_iic.h"
#include "sensor_iic_port.h"
#include "../../system/debug/debug.h"


uint8_t qmc5883_init(void)
{
	uint8_t buf[4];
	
	buf[0] = 0x06;
	qmc5883_write_regs(0x29, buf,1);
	buf[0] = 0x08;
	qmc5883_write_regs(0x0b, buf, 1);
	buf[0] = 0xcd;
	qmc5883_write_regs(0x0a, buf, 1);
	
	qmc5883_read_regs(0x00, buf, 1);
	if(buf[0] == 0x80)
	{
		printf("[info] qmc5883 初始化成功\n");	
	}
	else
	{
		printf("[error] qmc5883 初始化失败\n");
		return 1;
	}
	
	return 0;
}

uint8_t qmc5883_get_data(int16_t *x, int16_t *y, int16_t *z)
{
	uint8_t buf[6];
	qmc5883_read_regs(QMC5883P_DATA_XL_REG, buf, 6);

	*x = (buf[1] << 8) | buf[0];
	*y = (buf[3] << 8) | buf[2];
	*z = (buf[5] << 8) | buf[4];

	return 0;
}











