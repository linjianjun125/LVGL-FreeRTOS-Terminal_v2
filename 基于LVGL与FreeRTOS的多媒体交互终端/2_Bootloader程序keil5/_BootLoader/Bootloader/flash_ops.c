#include "flash_ops.h"
#include <string.h>

/*
 * 内部静态函数：Flash 解锁
 */
static HAL_StatusTypeDef flash_internal_unlock(void)
{
	return HAL_FLASH_Unlock();
}

/*
 * 内部静态函数：Flash 上锁
 */
static HAL_StatusTypeDef flash_internal_lock(void)
{
	return HAL_FLASH_Lock();
}

/*
 * 原子擦除函数实现
 */
HAL_StatusTypeDef flash_atomic_erase(uint32_t bank, uint32_t sector, uint32_t nb)
{
	FLASH_EraseInitTypeDef erase_init;
	uint32_t sector_error;
	HAL_StatusTypeDef status;

	status = flash_internal_unlock();
	if (status != HAL_OK)
	{
		return status;
	}

	erase_init.TypeErase = FLASH_TYPEERASE_SECTORS;
	erase_init.Banks = bank;
	erase_init.Sector = sector;
	erase_init.NbSectors = nb;
	erase_init.VoltageRange = FLASH_VOLTAGE_RANGE_3;

	status = HAL_FLASHEx_Erase(&erase_init, &sector_error);

	flash_internal_lock();
	return status;
}

/*
 * 原子写入函数实现
 */
HAL_StatusTypeDef flash_atomic_write(uint32_t dest_addr, const uint8_t *data, uint32_t size)
{
	HAL_StatusTypeDef status;
	uint32_t i;

	status = flash_internal_unlock();
	if (status != HAL_OK)
	{
		return status;
	}

	for (i = 0; i < size; i += FLASH_WORD_SIZE)
	{
		status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, dest_addr + i, (uint32_t)&data[i]);
		if (status != HAL_OK)
		{
			break;
		}
	}

	flash_internal_lock();
	return status;
}

/*
 * 原始数据读取
 */
void flash_read_raw(uint32_t src_addr, uint8_t *dest_buf, uint32_t size)
{
	memcpy(dest_buf, (void *)src_addr, size);
}