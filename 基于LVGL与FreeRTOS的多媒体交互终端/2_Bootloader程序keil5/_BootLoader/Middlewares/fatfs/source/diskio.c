#include "diskio.h"
#include "../drivers/bsp/sdmmc/sdmmc_sdcard.h"

#define SD_CARD     0       // SD卡, 卷标为0

/**
 * @brief       获得磁盘状态
 * @param       pdrv : 磁盘编号0~9
 * @retval      DSTATUS
 */
DSTATUS disk_status (
    BYTE pdrv
)
{
    return RES_OK;
}

/**
 * @brief       初始化磁盘
 * @param       pdrv : 磁盘编号0~9
 * @retval      DSTATUS
 */
DSTATUS disk_initialize (
    BYTE pdrv
)
{
    return RES_OK;           // 初始化成功
}

/**
 * @brief       读扇区
 * @param       pdrv   : 磁盘编号
 * @param       buff   : 数据接收缓冲
 * @param       sector : 扇区地址
 * @param       count  : 扇区数
 * @retval      DRESULT
 */
DRESULT disk_read (
    BYTE pdrv,
    BYTE *buff,
    DWORD sector,
    UINT count
)
{
    uint8_t res = 0;

    if (pdrv != SD_CARD || !count) return RES_PARERR;

    res = sd_read_disk(buff, sector, count);

    while (res)             // 读出错则重试
    {
        sd_init();          // 重新初始化SD卡
        res = sd_read_disk(buff, sector, count);
    }

    return (res == 0) ? RES_OK : RES_ERROR;
}

/**
 * @brief       写扇区
 * @param       pdrv   : 磁盘编号
 * @param       buff   : 发送数据缓冲
 * @param       sector : 扇区地址
 * @param       count  : 扇区数
 * @retval      DRESULT
 */
DRESULT disk_write (
    BYTE pdrv,
    const BYTE *buff,
    DWORD sector,
    UINT count
)
{
    uint8_t res = 0;

    if (pdrv != SD_CARD || !count) return RES_PARERR;

    res = sd_write_disk((uint8_t *)buff, sector, count);

    while (res)             // 写出错则重试
    {
        sd_init();          // 重新初始化SD卡
        res = sd_write_disk((uint8_t *)buff, sector, count);
    }

    return (res == 0) ? RES_OK : RES_ERROR;
}

/**
 * @brief       获取其他控制参数
 * @param       pdrv : 磁盘编号
 * @param       cmd  : 控制代码
 * @param       buff : 发送/接收缓冲区
 * @retval      DRESULT
 */
DRESULT disk_ioctl (
    BYTE pdrv,
    BYTE cmd,
    void *buff
)
{
    DRESULT res;

    if (pdrv != SD_CARD) return RES_PARERR;

    switch (cmd)
    {
        case CTRL_SYNC:
            res = RES_OK;
            break;

        case GET_SECTOR_SIZE:
            *(WORD *)buff = 512;
            res = RES_OK;
            break;

        case GET_BLOCK_SIZE:
            // 获取 SD 卡块大小 (通常为 512 字节)
            *(WORD *)buff = g_sd_card_info_handle.LogBlockSize;
            res = RES_OK;
            break;

        case GET_SECTOR_COUNT:
            // 获取 SD 卡总扇区数
            *(DWORD *)buff = g_sd_card_info_handle.LogBlockNbr;
            res = RES_OK;
            break;

        default:
            res = RES_PARERR;
            break;
    }

    return res;
}

/**
 * @brief       获得时间
 * @param       mf  : 内存首地址
 * @retval      时间
 * @note        时间编码规则如下:
 *              User defined function to give a current time to fatfs module 
 *              31-25: Year(0-127 org.1980), 24-21: Month(1-12), 20-16: Day(1-31)
 *              15-11: Hour(0-23), 10-5: Minute(0-59), 4-0: Second(0-29 *2) 
 */
DWORD get_fattime (void)
{
    return 0;
}

