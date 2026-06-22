#include "firmware_service.h"

#include <time.h>
#include <string.h>
#include <time.h>



// 定义全局变量并赋予初始默认值
fw_global_info_t g_sys_fw_info = 
{
    .device_name = "Multi-Music Player",
    .version     = "V0.0.0",
    .mcu_type    = "STM32H743IIT6",
    .build_time  = "2026-01-01 00:00:00",
    .memory_info = "2.0MB Flash"
};

/**
 * @brief 从 Flash 物理地址加载固件信息并更新全局变量
 */
void fw_global_info_init(void)
{
    // 获取 Flash 原始结构体指针
    firmware_info_t *p_flash = (firmware_info_t *)ADDR_VERSION_SECTOR;

    // 针对带 Cache 的 MCU (如 H7) 必须刷新
    // SCB_InvalidateDCache_by_Addr((uint32_t *)ADDR_VERSION_SECTOR, sizeof(firmware_info_t));

    // 校验 Magic Number
    if (p_flash->magic == FW_INFO_MAGIC)
    {
        // 1. 更新版本号
        strncpy(g_sys_fw_info.version, p_flash->version_str, FW_VERSION_STR_LEN);
        g_sys_fw_info.version[FW_VERSION_STR_LEN - 1] = '\0';

        // 2. 转换并更新编译时间
        time_t raw_time = (time_t)p_flash->timestamp;
        struct tm *time_ptr = localtime(&raw_time);
        if (time_ptr) {
            strftime(g_sys_fw_info.build_time, FW_TIME_STR_LEN, "%Y-%m-%d %H:%M:%S", time_ptr);
        }

        // 3. 其他字段如果在 Flash 中有定义也可以在此更新
        // 例如：g_sys_fw_info.device_name = p_flash->device_name;
    }
    // 如果校验失败，则保留定义时的默认值
}


/**
 * @brief 直接访问 Flash 寄存器地址读取 APP1 全部信息
 */
void fw_app_read_current_info(void)
{
    /* 1. 获取指向 APP1 管理槽位的指针 (物理地址 0x081E0000) */
    firmware_info_t *p_app = (firmware_info_t *)ADDR_VERSION_SECTOR;

    /* 2. 刷新 Cache (关键)
     * 确保 CPU 丢弃 D-Cache 中的旧数据，直接从 Flash 存储介质获取最新值
     */
    SCB_InvalidateDCache_by_Addr((uint32_t *)ADDR_VERSION_SECTOR, sizeof(firmware_info_t));

    /* 3. 校验 Magic Number 确保数据存在 */
    if (p_app->magic != FW_INFO_MAGIC)
    {
        printf("[SYSTEM] No valid firmware info at 0x%08X\r\n", ADDR_VERSION_SECTOR);
        return;
    }

    /* 4. 直接访问成员变量打印全部信息 */
    printf("\r\n>>>> CURRENT FIRMWARE INFO (Direct Read) <<<<\r\n");
    printf("Magic        : 0x%08X\r\n", (unsigned int)p_app->magic);
    printf("Version      : %s\r\n",      p_app->version_str);
    printf("Ver Number   : %lu\r\n",     p_app->version_num);
    printf("Payload Size : %lu Bytes\r\n", p_app->file_size);
    printf("Payload CRC  : 0x%08X\r\n", (unsigned int)p_app->crc32);
    printf("State        : %u\r\n",      p_app->state);
    printf("Update Count : %u\r\n",      p_app->update_count);
    printf("Build Time   : %u\r\n",      p_app->timestamp);
	
    /* 格式化时间戳 */
    time_t raw_time = (time_t)p_app->timestamp;
    printf("Build Time   : %s",          ctime(&raw_time));
    
    printf("Header CRC   : 0x%08X\r\n", (unsigned int)p_app->header_crc);
    printf("==============================================\r\n");
}
