#ifndef __SENSOR_H
#define __SENSOR_H

#include "sys.h"

#include "qmc5883.h"
#include "icm42688.h"
#include "sht30.h"
#include "bh1750.h"


void sensor_init(void);

uint8_t sensor_get_imu(float *pitch, float *roll, float *yaw);
uint8_t sensor_get_light(float *light);
uint8_t sensor_get_compass(float *g);
uint8_t sensor_get_temperature_humidity(float *temperature, float *humidity);






#endif

