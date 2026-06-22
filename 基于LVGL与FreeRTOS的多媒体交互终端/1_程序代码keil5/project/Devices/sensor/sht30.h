#ifndef __SHT307_H
#define __SHT30_H


#include "sys.h"



uint8_t sht30_init(void);
void sht30_get_temperature_humidity(float *t, float *h) ;






#endif

