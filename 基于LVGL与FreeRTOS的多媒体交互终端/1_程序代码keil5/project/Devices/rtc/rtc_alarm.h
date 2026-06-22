#ifndef __RTC_ALARM_H
#define __RTC_ALARM_H

#include "sys.h"
#include "rtc_init.h"
                                                  
void rtc_set_wakeup(uint8_t wksel, uint16_t cnt);                          // 设置周期性唤醒
uint8_t rtc_get_week(uint16_t year, uint8_t month, uint8_t day);           // 获取星期
void rtc_set_alarma(uint8_t week, uint8_t hour, uint8_t min, uint8_t sec); // 设置闹钟

#endif
