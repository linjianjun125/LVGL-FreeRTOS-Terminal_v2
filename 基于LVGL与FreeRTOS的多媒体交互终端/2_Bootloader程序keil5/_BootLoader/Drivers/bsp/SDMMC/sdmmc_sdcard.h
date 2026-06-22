#ifndef __SDMMC_SDCARD_H
#define __SDMMC_SDCARD_H

#include "sys.h"

// --- SDMMC1 引脚参数包 (端口, 引脚, 复用功能) ---
#define SD1_D0_GPIO_PARAMS      GPIOC, GPIO_PIN_8,  GPIO_AF12_SDIO1
#define SD1_D1_GPIO_PARAMS      GPIOC, GPIO_PIN_9,  GPIO_AF12_SDIO1
#define SD1_D2_GPIO_PARAMS      GPIOC, GPIO_PIN_10, GPIO_AF12_SDIO1
#define SD1_D3_GPIO_PARAMS      GPIOC, GPIO_PIN_11, GPIO_AF12_SDIO1
#define SD1_CLK_GPIO_PARAMS     GPIOC, GPIO_PIN_12, GPIO_AF12_SDIO1
#define SD1_CMD_GPIO_PARAMS     GPIOD, GPIO_PIN_2,  GPIO_AF12_SDIO1

// 实例定义
#define SD_UX                   SDMMC1

/******************************************************************************************/


// 仅负责打印已获取的 SD 卡信息
#define SD_CARD_INFO_PRINT(info) do { \
    printf("\r\n[SD INFO]\r\n"); \
    printf("Type    : %s\r\n", (info).CardType == CARD_SDHC_SDXC ? "SDHC/SDXC" : "SDSC"); \
    printf("Version : V%d\r\n", (int)(info).CardVersion); \
    printf("Block   : %lu * %d Bytes\r\n", (info).LogBlockNbr, (int)(info).LogBlockSize); \
    printf("Capacity: %.2f GB\r\n", (float)((uint64_t)(info).LogBlockNbr * (info).LogBlockSize) / (1024 * 1024 * 1024)); \
    printf("RCA     : 0x%04X\r\n", (int)(info).RelCardAdd); \
} while(0)









#define SD_TIMEOUT             ((uint32_t)100000000)                                   /* 超时时间 */
#define SD_TRANSFER_OK         ((uint8_t)0x00)                                         /* 传输完成 */
#define SD_TRANSFER_BUSY       ((uint8_t)0x01)                                         /* 卡正忙 */
















/* 根据 SD_HandleTypeDef 定义的宏，用于快速计算容量 */
#define SD_TOTAL_SIZE_BYTE(__Handle__)  (((uint64_t)((__Handle__)->SdCard.LogBlockNbr)*((__Handle__)->SdCard.LogBlockSize))>>0)
#define SD_TOTAL_SIZE_KB(__Handle__)    (((uint64_t)((__Handle__)->SdCard.LogBlockNbr)*((__Handle__)->SdCard.LogBlockSize))>>10)
#define SD_TOTAL_SIZE_MB(__Handle__)    (((uint64_t)((__Handle__)->SdCard.LogBlockNbr)*((__Handle__)->SdCard.LogBlockSize))>>20)
#define SD_TOTAL_SIZE_GB(__Handle__)    (((uint64_t)((__Handle__)->SdCard.LogBlockNbr)*((__Handle__)->SdCard.LogBlockSize))>>30)

extern SD_HandleTypeDef        g_sd_handle;                                            /* SD卡句柄 */
extern HAL_SD_CardInfoTypeDef  g_sd_card_info_handle;                                  /* SD卡信息结构体 */

/******************************************************************************************/

uint8_t sd_init(void);
uint8_t get_sd_card_info(HAL_SD_CardInfoTypeDef *cardinfo);
uint8_t get_sd_card_state(void);
uint8_t sd_read_disk(uint8_t *pbuf, uint32_t saddr, uint32_t cnt);
uint8_t sd_write_disk(uint8_t *pbuf, uint32_t saddr, uint32_t cnt);

#endif
