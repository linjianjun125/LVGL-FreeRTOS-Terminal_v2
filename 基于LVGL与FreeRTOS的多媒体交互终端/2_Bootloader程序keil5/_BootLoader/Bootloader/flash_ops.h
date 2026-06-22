#ifndef __FLASH_OPS_H
#define __FLASH_OPS_H

#include "stm32h7xx_hal.h"

#define FLASH_WORD_SIZE    32 

/*
 * 从 Flash 地址读取原始数据 (读取无需加锁)
 */
void flash_read_raw(uint32_t src_addr, uint8_t *dest_buf, uint32_t size);

/*
 * 原子操作：擦除并上锁
 * 内部逻辑：Unlock -> Erase -> Lock
 */
HAL_StatusTypeDef flash_atomic_erase(uint32_t bank, uint32_t sector, uint32_t nb);

/*
 * 原子操作：写入并上锁
 * 内部逻辑：Unlock -> Write Loop -> Lock
 */
HAL_StatusTypeDef flash_atomic_write(uint32_t dest_addr, const uint8_t *data, uint32_t size);

#endif /* __FLASH_OPS_H */
