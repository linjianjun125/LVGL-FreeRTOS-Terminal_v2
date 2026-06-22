#ifndef __RTC_TIME_H
#define __RTC_TIME_H

#include "sys.h"
#include "rtc_init.h"
                                                  
void rtc_get_time(uint8_t *hour, uint8_t *min, uint8_t *sec, uint8_t *ampm);    // 获取时间
void rtc_get_date(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *week); // 获取日期
uint8_t rtc_set_time(uint8_t hour, uint8_t min, uint8_t sec, uint8_t ampm);    	// 设置时间
uint8_t rtc_set_date(uint8_t year, uint8_t month, uint8_t date, uint8_t week); 	// 设置日期





#endif
