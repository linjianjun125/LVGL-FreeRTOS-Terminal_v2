#include "bootloader_core.h"
#include "flash_ops.h"
#include <string.h>
#include "crc32.h"
#include "../drivers/system/usart/usart.h"

static HAL_StatusTypeDef fw_download_to_backup(const char *path, firmware_info_t *sd_fw);
static HAL_StatusTypeDef fw_read_sd_header(const char *path, firmware_info_t *sd_fw);

/*
 * 完整升级调度任务
 */
HAL_StatusTypeDef fw_process_upgrade_flow(const char *sd_path)
{
    firmware_info_t sd_fw;
    firmware_info_t app1_fw;
    firmware_info_t app2_info;

    /* --- 步骤 A: 检查 SD 卡是否有新固件 --- */
    if (fw_read_sd_header(sd_path, &sd_fw) == HAL_OK)
    {
        /* 获取当前运行版本 */
        HAL_StatusTypeDef app1_stat = fw_slot_get_info(SLOT_APP1, &app1_fw);

        /* 逻辑判断：如果 APP1 无效，或者 SD 版本更高 */
        if (app1_stat != HAL_OK || sd_fw.version_num > app1_fw.version_num)
        {
            printf("[info] Found new version: %s. Start download...\n", sd_fw.version_str);

            /* 1. 先将 APP2 标记为 UPDATING 状态，防止中途断电误启动 */
            sd_fw.state = FW_STATE_UPDATING;
            fw_slot_set_info(SLOT_APP2, &sd_fw);

            /* 2. 执行下载与烧录 */
            if (fw_download_to_backup(sd_path, &sd_fw) == HAL_OK)
            {
                /* 3. 烧录成功且校验通过，标记 APP2 为 READY，准备同步 */
                sd_fw.state = FW_STATE_READY;
                fw_slot_set_info(SLOT_APP2, &sd_fw);
                printf("[info] APP2 download successful.\n");
            }
            else
            {
                printf("[error] APP2 download failed.\n");
            }
        }
    }

    /* --- 步骤 B: 检查 APP2 状态是否需要同步到 APP1 --- */
    if (fw_slot_get_info(SLOT_APP2, &app2_info) == HAL_OK)
    {
        if (app2_info.state == FW_STATE_READY)
        {
            printf("[info] New firmware ready in backup. Syncing to APP1...\n");
            if (fw_sync_backup_to_main() == HAL_OK)
            {
                printf("[info] System sync completed.\n");
            }
            else
            {
                printf("[error] Sync failed!\n");
                return HAL_ERROR;
            }
        }
    }

    /* --- 步骤 C: 最终检查 APP1 是否处于有效状态 --- */
    if (fw_slot_get_info(SLOT_APP1, &app1_fw) == HAL_OK)
    {
        if (app1_fw.state == FW_STATE_VALID)
        {
            printf("[success] APP1 is valid. Ready to jump.\n");
            return HAL_OK;
        }
    }

    printf("[critical] No valid firmware found!\n");
    return HAL_ERROR;
}

/*
 * 安全跳转至 APP
 */
void jump_to_app(uint32_t app_addr)
{
    uint32_t msp_val = *(__IO uint32_t*)app_addr;
    uint32_t jump_addr = *(__IO uint32_t*)(app_addr + 4);
    pFunction jump_func;

    if ((msp_val & 0xFF000000) == 0x24000000 || (msp_val & 0xFF000000) == 0x20000000)
    {
        __disable_irq();

        for (uint8_t i = 0; i < 8; i++)
        {
            NVIC->ICER[i] = 0xFFFFFFFF;
            NVIC->ICPR[i] = 0xFFFFFFFF;
        }

        HAL_DeInit();
        HAL_RCC_DeInit();
        SysTick->CTRL = 0;

        /* 关闭 H7 Cache */
        SCB_DisableICache();
        SCB_DisableDCache();
        __DSB();
        __ISB();

        __set_MSP(msp_val);
        jump_func = (pFunction)jump_addr;
        jump_func();
    }
}

/*
 * 从 SD 卡文件尾部读取 Header
 */
static HAL_StatusTypeDef fw_read_sd_header(const char *path, firmware_info_t *sd_fw)
{
    FIL file;
    UINT br;
    FRESULT fr;
    uint32_t file_size;

    if (f_open(&file, path, FA_READ) != FR_OK) return HAL_ERROR;

    file_size = f_size(&file);
    fr = f_lseek(&file, file_size - sizeof(firmware_info_t));
    if (fr == FR_OK)
    {
        fr = f_read(&file, sd_fw, sizeof(firmware_info_t), &br);
    }
    f_close(&file);

    return (fr == FR_OK && sd_fw->magic == FW_INFO_MAGIC) ? HAL_OK : HAL_ERROR;
}


/*
 * 内部静态函数：从 SD 卡文件读取并烧录到 Flash (APP2)
 * 采用你原有的 32 字节对齐逻辑，确保 CRC 绝对正确
 */
static HAL_StatusTypeDef fw_flash_from_sd(const char *path, uint32_t dest_addr, uint32_t payload_size)
{
	FIL file;
	UINT br;
	HAL_StatusTypeDef status = HAL_OK;
	/* 保持你原来的 32 字节对齐缓冲区 */
	static uint8_t buffer[FLASH_WORD_SIZE] __attribute__((aligned(FLASH_WORD_SIZE)));
	uint32_t written = 0;

	if (f_open(&file, path, FA_READ) != FR_OK)
	{
		return HAL_ERROR;
	}

	/* 烧录开始前：解锁一次 */
	HAL_FLASH_Unlock();

	while (written < payload_size)
	{
		/* 核心逻辑：每一轮都清空，确保末尾对齐部分是 0xFF */
		memset(buffer, 0xFF, FLASH_WORD_SIZE);
		
		uint32_t remain = payload_size - written;
		uint32_t to_read = (remain >= FLASH_WORD_SIZE) ? FLASH_WORD_SIZE : remain;
		
		if (f_read(&file, buffer, to_read, &br) != FR_OK)
		{
			status = HAL_ERROR;
			break;
		}

		/* 编程：直接调用 HAL，因为我们在循环外统一加锁 */
		status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, dest_addr + written, (uint32_t)buffer);
		if (status != HAL_OK)
		{
			break;
		}

		written += FLASH_WORD_SIZE;
	}

	/* 烧录结束后：锁定一次 */
	HAL_FLASH_Lock();
	f_close(&file);

	return status;
}

/*
 * 业务层函数：下载流程控制
 */
static HAL_StatusTypeDef fw_download_to_backup(const char *path, firmware_info_t *sd_fw)
{
	HAL_StatusTypeDef status;

	/* 1. 标记状态 */
	sd_fw->state = FW_STATE_UPDATING;
	fw_slot_set_info(SLOT_APP2, sd_fw);

	/* 2. 原子擦除 (事务级) */
	printf("[info] Erasing Bank 2...\n");
	status = flash_atomic_erase(FLASH_BANK_2, 0, 7);
	if (status != HAL_OK)
	{
		return status;
	}

	/* 3. 烧录数据 (使用你最稳定的 32 字节步进逻辑) */
	printf("[info] Flashing...\n");
	status = fw_flash_from_sd(path, ADDR_APP2_START, sd_fw->file_size);
	if (status != HAL_OK)
	{
		return status;
	}

	/* 4. CRC 校验 */
	status = fw_verify_integrity(ADDR_APP2_START, sd_fw->file_size, sd_fw->crc32);
	if (status != HAL_OK)
	{
		printf("[error] CRC mismatch!\n");
		return HAL_ERROR;
	}

	/* 5. 完成标记 */
	sd_fw->state = FW_STATE_READY;
	return fw_slot_set_info(SLOT_APP2, sd_fw);
}



