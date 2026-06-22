#ifndef __QMC5883_H
#define __QMC5883_H

#include "sys.h"

#define QMC5883P_DATA_XL_REG		(0x01)
#define QMC5883P_DATA_XH_REG		(0x02)
#define QMC5883P_DATA_YL_REG		(0x03)
#define QMC5883P_DATA_YH_REG		(0x04)
#define QMC5883P_DATA_ZL_REG		(0x05)
#define QMC5883P_DATA_ZH_REG		(0x06)

uint8_t qmc5883_init(void);
uint8_t qmc5883_get_data(int16_t *x, int16_t *y, int16_t *z);








#endif

