#include "crc32.h"

CRC_HandleTypeDef hcrc;
/**
 * @brief 使用硬件 CRC32 外设校验 Flash 分区数据的完整性
 * @param start_addr 待校验分区的起始地址
 * @param size 固件的原始字节大小
 * @param expected_crc 预期的 CRC32 值（通常存储在 Header 中）
 * @return HAL_StatusTypeDef HAL_OK 表示校验通过
 */
HAL_StatusTypeDef fw_verify_integrity(uint32_t start_addr, uint32_t size, uint32_t expected_crc)
{
	uint32_t calculated_crc;
	uint32_t word_length;
	
	/* 1. 参数合法性检查 */
	if (start_addr == 0 || size == 0)
	{
		return HAL_ERROR;
	}

	/* 3. 配置并启动硬件 CRC */
	/* 假设已经在初始化阶段调用了 MX_CRC_Init()，此处只需重置 DR 寄存器 */
    // 配置硬件 CRC 适配 Python crcmod (rev=True, xorOut=0xFFFFFFFF)
    __HAL_RCC_CRC_CLK_ENABLE();
    hcrc.Instance = CRC;
    hcrc.Init.DefaultPolynomialUse    = DEFAULT_POLYNOMIAL_ENABLE;
    hcrc.Init.DefaultInitValueUse     = DEFAULT_INIT_VALUE_ENABLE;
    
    // 关键适配修改点：
	hcrc.Init.InputDataInversionMode  = CRC_INPUTDATA_INVERSION_BYTE;   // 输入位反转
    hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_ENABLE; // 输出整体反转
    hcrc.InputDataFormat              = CRC_INPUTDATA_FORMAT_BYTES;     // 字节模式
    
    if (HAL_CRC_Init(&hcrc) != HAL_OK) return HAL_ERROR;
    __HAL_CRC_DR_RESET(&hcrc);

	/* 4. 执行校验计算
	 * H7 的硬件 CRC 直接通过总线读取 Flash，速度非常快。
	 */
	 SCB_InvalidateDCache_by_Addr((void *)start_addr, size);
	calculated_crc = HAL_CRC_Calculate(&hcrc, (uint32_t *)start_addr, size);

	/* 5. 结果比对 */
	if (calculated_crc == expected_crc)
	{
		printf("CRC Check Pass: 0x%08X\n", calculated_crc);
		return HAL_OK;
	}
	else
	{
		printf("CRC Check Fail! Expected: 0x%08X, Got: 0x%08X\n", expected_crc, calculated_crc);
		return HAL_ERROR;
	}
}

