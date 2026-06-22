#ifndef __MAX17048_H
#define __MAX17048_H
#include "sys.h"

void max17048_init(void);
float max17048_get_voltage(void);
float max17048_get_quantity(void);
bool max17048_is_charging(void);
bool max17048_has_bat(void);

#endif
