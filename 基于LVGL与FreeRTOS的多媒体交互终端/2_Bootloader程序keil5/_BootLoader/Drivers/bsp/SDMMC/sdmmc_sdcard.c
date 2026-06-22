#include "string.h"
#include "stdio.h"
#include "sdmmc_sdcard.h"

// 变量定义
SD_HandleTypeDef g_sd_handle;                 // SD卡句柄
HAL_SD_CardInfoTypeDef g_sd_card_info_handle; // SD卡信息结构体

// 512字节对齐缓冲区，用于单块读写或非4字节对齐地址的缓冲
__ALIGNED(4) uint8_t g_sd_data_buffer[512];

/**
 * @brief       初始化SD卡
 * @param       无
 * @retval      0: 初始化成功; 1: 初始化失败
 */
uint8_t sd_init(void)
{
    // 1. SDMMC 句柄参数配置
    g_sd_handle.Instance = SDMMC1;
    g_sd_handle.Init.ClockEdge           = SDMMC_CLOCK_EDGE_RISING;           // 上升沿采样
    g_sd_handle.Init.ClockPowerSave      = SDMMC_CLOCK_POWER_SAVE_DISABLE;    // 空闲不关闭时钟
    g_sd_handle.Init.BusWide             = SDMMC_BUS_WIDE_4B;                 // 使用4位数据线
    g_sd_handle.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE; // 禁用硬件流控
    g_sd_handle.Init.ClockDiv            = SDMMC_NSpeed_CLK_DIV;              // 设置分频系数

    // 2. 调用 HAL 库底层初始化
    if (HAL_SD_Init(&g_sd_handle) != HAL_OK)
    {
        return 1;
    }

    // 3. 获取并显示卡信息
    if (HAL_SD_GetCardInfo(&g_sd_handle, &g_sd_card_info_handle) == HAL_OK)
    {
        // 调用之前定义的打印宏
        SD_CARD_INFO_PRINT(g_sd_card_info_handle);
    }

    return 0;
}


/**
 * @brief       SDMMC底层初始化回调 (由HAL_SD_Init内部触发)
 * @note        由于逻辑已合入sd_init，此处保持为空以符合规范
 */
void HAL_SD_MspInit(SD_HandleTypeDef *hsd)
{
    if (hsd->Instance == SDMMC1)
    {
        // 1. 使能 SDMMC1 外设时钟
        __HAL_RCC_SDMMC1_CLK_ENABLE();

        // 2. 使能所有相关 GPIO 端口时钟
        config_gpio_init(SD1_D0_GPIO_PARAMS,  GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH);
        config_gpio_init(SD1_D1_GPIO_PARAMS,  GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH);
        config_gpio_init(SD1_D2_GPIO_PARAMS,  GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH);
        config_gpio_init(SD1_D3_GPIO_PARAMS,  GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH);
        config_gpio_init(SD1_CLK_GPIO_PARAMS, GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH);
        config_gpio_init(SD1_CMD_GPIO_PARAMS, GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH);
        
        // 如果后续需要使用中断或 DMA，配置代码也写在这里
    }

}

/**
 * @brief       获取当前卡状态
 * @retval      SD_TRANSFER_OK (0): 可进行下次传输; SD_TRANSFER_BUSY (1): 正忙
 */
uint8_t get_sd_card_state(void)
{
    return ((HAL_SD_GetCardState(&g_sd_handle) == HAL_SD_CARD_TRANSFER) ? SD_TRANSFER_OK : SD_TRANSFER_BUSY);
}

/**
 * @brief       读SD卡 (轮询模式)
 * @param       pbuf  : 数据缓存区
 * @param       saddr : 扇区地址
 * @param       cnt   : 扇区个数
 * @retval      0: 成功; 其他: 失败
 */
uint8_t sd_read_disk(uint8_t *pbuf, uint32_t saddr, uint32_t cnt)
{
    uint8_t sta = HAL_OK;
    uint32_t timeout = SD_TIMEOUT;
    
    // 轮询模式下关闭中断，防止被打断导致 FIFO 溢出
    sys_intx_disable(); 
    
    sta = HAL_SD_ReadBlocks(&g_sd_handle, pbuf, (uint32_t)saddr, cnt, SD_TIMEOUT);

    // 等待传输完成
    while (get_sd_card_state() != SD_TRANSFER_OK)
    {
        if (timeout-- == 0)
        {
            sta = SD_TRANSFER_BUSY;
            break;
        }
    }

    sys_intx_enable();
    return sta;
}

/**
 * @brief       写SD卡 (轮询模式)
 * @param       pbuf  : 数据缓存区
 * @param       saddr : 扇区地址
 * @param       cnt   : 扇区个数
 * @retval      0: 成功; 其他: 失败
 */
uint8_t sd_write_disk(uint8_t *pbuf, uint32_t saddr, uint32_t cnt)
{
    uint8_t sta = HAL_OK;
    uint32_t timeout = SD_TIMEOUT;

    sys_intx_disable();
    
    sta = HAL_SD_WriteBlocks(&g_sd_handle, pbuf, (uint32_t)saddr, cnt, SD_TIMEOUT);

    // 等待写入完成
    while (get_sd_card_state() != SD_TRANSFER_OK)
    {
        if (timeout-- == 0)
        {
            sta = SD_TRANSFER_BUSY;
            break;
        }
    }

    sys_intx_enable();
    return sta;
}
