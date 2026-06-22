#ifndef __DEVICES_INCLUDE_H
#define __DEVICES_INCLUDE_H


/* 基础外设驱动 */
#include "led/led.h"
#include "key/key.h"
#include "power/max17048.h"
#include "sensor/sensor.h"
#include "lcdrgb/lcdrgb.h"
#include "touch/touch.h"
#include "sdram/sdram.h"
#include "sdmmc/sdmmc_sdcard.h"
#include "nor_flash/norflash.h"
#include "at24cxx/at24cxx.h"
#include "wifi/wifi_uart.h"

/* 音频相关 */
#include "audio/audio_es8388.h"
#include "audio/decode/decode.h"

/* 系统与底层 */
#include "random/random.h"
#include "mpu/mpu.h"
#include "rtc/rtc_init.h"
#include "rtc/rtc_time.h"

#endif
