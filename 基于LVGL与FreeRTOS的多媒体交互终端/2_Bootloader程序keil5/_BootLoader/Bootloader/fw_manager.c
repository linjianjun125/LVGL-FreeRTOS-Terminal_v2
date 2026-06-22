#include "fw_manager.h"
#include "flash_ops.h"
#include "crc32.h"
#include <string.h>

/*
 * 读取槽位信息 (处理 H7 Cache 刷新)
 */
HAL_StatusTypeDef fw_slot_get_info(uint8_t slot_idx, firmware_info_t *info)
{
	uint32_t addr;

	if (!info)
	{
		return HAL_ERROR;
	}

	addr = ADDR_VERSION_SECTOR + (slot_idx == SLOT_APP1 ? 0 : FW_INFO_SLOT_STEP);

	/* 失效 D-Cache 确保从 Flash 介质读取 */
	SCB_InvalidateDCache_by_Addr((uint32_t *)addr, sizeof(firmware_info_t));
	flash_read_raw(addr, (uint8_t *)info, sizeof(firmware_info_t));

	return (info->magic == FW_INFO_MAGIC) ? HAL_OK : HAL_ERROR;
}

/*
 * 更新槽位信息 (原子事务)
 */
HAL_StatusTypeDef fw_slot_set_info(uint8_t slot_idx, firmware_info_t *info)
{
	static uint8_t sector_cache[FW_INFO_SLOT_STEP * 2] __attribute__((aligned(32)));
	uint32_t offset;

	if (!info)
	{
		return HAL_ERROR;
	}

	/* 1. 读取整个管理区 */
	flash_read_raw(ADDR_VERSION_SECTOR, sector_cache, sizeof(sector_cache));

	/* 2. 在 RAM 缓存中修改对应槽位 */
	offset = (slot_idx == SLOT_APP1 ? 0 : FW_INFO_SLOT_STEP);
	memcpy(&sector_cache[offset], info, sizeof(firmware_info_t));

	/* 3. 原子擦除管理扇区 (Sector 7, Bank 2) */
	if (flash_atomic_erase(FLASH_BANK_2, 7, 1) != HAL_OK)
	{
		return HAL_ERROR;
	}

	/* 4. 原子写入更新后的数据 */
	return flash_atomic_write(ADDR_VERSION_SECTOR, sector_cache, sizeof(sector_cache));
}

/*
 * 固件同步 (从 APP2 备份区到 APP1 运行区)
 */
HAL_StatusTypeDef fw_sync_backup_to_main(void)
{
	firmware_info_t app2_info;
	HAL_StatusTypeDef status;

	/* 校验备份区固件是否就绪 */
	if (fw_slot_get_info(SLOT_APP2, &app2_info) != HAL_OK || 
		app2_info.state != FW_STATE_READY)
	{
		return HAL_ERROR;
	}

	/* 1. 原子擦除运行区 (Bank 1 Sector 1-7) */
	status = flash_atomic_erase(FLASH_BANK_1, 1, 7);
	if (status != HAL_OK)
	{
		return status;
	}

	/* 2. 原子搬运固件载荷 */
	status = flash_atomic_write(ADDR_APP1_START, (uint8_t *)ADDR_APP2_START, app2_info.file_size);
	if (status != HAL_OK)
	{
		return status;
	}

	/* 3. 事务完成后更新槽位状态信息 */
	app2_info.state = FW_STATE_VALID;
	fw_slot_set_info(SLOT_APP1, &app2_info);
	
	app2_info.state = FW_STATE_IDLE;
	fw_slot_set_info(SLOT_APP2, &app2_info);

	return HAL_OK;
}
