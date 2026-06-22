#ifndef __MY_CRC32_H
#define __MY_CRC32_H


#include "stm32h7xx_hal.h"

HAL_StatusTypeDef fw_verify_integrity(uint32_t start_addr, uint32_t size, uint32_t expected_crc);




#endif
