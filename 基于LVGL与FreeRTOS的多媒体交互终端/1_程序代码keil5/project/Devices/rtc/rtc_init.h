#ifndef __RTC_INIT_H
#define __RTC_INIT_H

#include "sys.h"

uint8_t rtc_init(void); 
uint8_t rtc_wait_synchro(void);
uint8_t rtc_init_mode(void);

uint8_t rtc_dec2bcd(uint8_t val);
uint8_t rtc_bcd2dec(uint8_t val);

#endif
