#include "diskio.h"
#include "sys.h"
#include "devices_include.h"
#include "middle_include.h"

#define SD_CARD                 0   
    
#if SYS_USE_RTOS
	#define USE_SDMMC_SEMAPHORE		1
	#include "app_include.h"
#endif



/**
 * @brief  获得磁盘状态
 * @param  pdrv: 磁盘编号0~9
 * @retval 无
 */
DSTATUS disk_status (BYTE pdrv)
{
    return RES_OK;
}

/**
 * @brief 	初始化磁盘
 * @param   pdrv : 磁盘编号0~9
 * @retval  无
 */
DSTATUS disk_initialize (BYTE pdrv)
{
    uint8_t res = 0;
// sd卡
    if(pdrv == SD_CARD)
    {
	#if USE_SDMMC_SEMAPHORE
		xSemaphoreTake(sdmmc_Semaphore, portMAX_DELAY);
	#endif
	res = sd_init();
	#if USE_SDMMC_SEMAPHORE
		xSemaphoreGive(sdmmc_Semaphore);
	#endif
    }
    if (res)	return STA_NOINIT;	// 失败
    else 		return 0;			// 初始化成功
}

/**
 * @brief 	读扇区
 * @param   pdrv: 磁盘编号0~9
 * @param   buff: 数据接收缓冲首地址
 * @param   sector: 扇区地址
 * @param   count: 需要读取的扇区数
 * @retval  无
 */
DRESULT disk_read (BYTE pdrv,BYTE *buff,DWORD sector,UINT count)
{
    uint8_t res = 0;

    if (!count)
    {
        return RES_PARERR;
    }

    if (pdrv == SD_CARD)
    {
		#if USE_SDMMC_SEMAPHORE
			xSemaphoreTake(sdmmc_Semaphore, portMAX_DELAY);
		#endif
        res = sd_read_disk(buff, sector, count);
		#if USE_SDMMC_SEMAPHORE
			xSemaphoreGive(sdmmc_Semaphore);
		#endif

        while (res)
        {
            printf("sd rd error:%d\r\n", res);
			#if USE_SDMMC_SEMAPHORE
				xSemaphoreTake(sdmmc_Semaphore, portMAX_DELAY);
			#endif
            sd_init(); 
            res = sd_read_disk(buff, sector, count);
			#if USE_SDMMC_SEMAPHORE
				xSemaphoreGive(sdmmc_Semaphore);
			#endif
        }
    }


    if (res == 0x00) return RES_OK;
    else return RES_ERROR; 
}

/**
 * @brief       写扇区
 * @param       pdrv   : 磁盘编号0~9
 * @param       buff   : 发送数据缓存区首地址
 * @param       sector : 扇区地址
 * @param       count  : 需要写入的扇区数
 * @retval      无
 */
DRESULT disk_write (BYTE pdrv,const BYTE *buff, DWORD sector,UINT count)
{
    uint8_t res = 0;
    if (!count)
    {
        return RES_PARERR;
    }

    if (pdrv == SD_CARD) 
    {
		#if USE_SDMMC_SEMAPHORE
			xSemaphoreTake(sdmmc_Semaphore, portMAX_DELAY);
		#endif
        res = sd_write_disk((uint8_t *)buff, sector, count);
		#if USE_SDMMC_SEMAPHORE
			xSemaphoreGive(sdmmc_Semaphore);
		#endif
        while (res)
        {
            printf("sd wr error:%d\r\n", res);
			#if USE_SDMMC_SEMAPHORE
				xSemaphoreTake(sdmmc_Semaphore, portMAX_DELAY);
			#endif
            sd_init();
            res = sd_write_disk((uint8_t *)buff, sector, count);
			#if USE_SDMMC_SEMAPHORE
				xSemaphoreGive(sdmmc_Semaphore);
			#endif
        }
    }

    if (res == 0x00) return RES_OK;
    else return RES_ERROR; 
}


/**
 * @brief       获取其他控制参数
 * @param       pdrv: 磁盘编号0~9
 * @param       ctrl: 控制代码
 * @param       buff: 发送/接收缓冲区指针
 * @retval      无
 */
DRESULT disk_ioctl (BYTE pdrv,BYTE cmd,void *buff)
{
    DRESULT res;

    if (pdrv == SD_CARD)
    {
        switch (cmd)
        {
            case CTRL_SYNC:
                res = RES_OK;
                break;

            case GET_SECTOR_SIZE:
                *(DWORD *)buff = 512;
                res = RES_OK;
                break;

            case GET_BLOCK_SIZE:
                *(WORD *)buff = g_sd_card_info.CardBlockSize;
                res = RES_OK;
                break;

            case GET_SECTOR_COUNT:
                *(DWORD *)buff = g_sd_card_info.CardCapacity/512;
                res = RES_OK;
                break;

            default:
                res = RES_PARERR;
                break;
        }
		return res;
    }

    return RES_ERROR;
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

/**
 * @brief       动态分配内存
 * @param       size : 要分配的内存大小(字节)
 * @retval      分配到的内存首地址.
 */
void *ff_memalloc (UINT size)
{
    return (void*)mymalloc(SRAMIN, size);
}

/**
 * @brief       释放内存
 * @param       mf  : 内存首地址
 * @retval      无
 */
void ff_memfree (void* mf)
{
    myfree(SRAMIN, mf);
}




























