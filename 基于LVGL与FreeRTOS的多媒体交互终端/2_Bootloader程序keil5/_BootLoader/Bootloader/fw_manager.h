#ifndef __FW_MANAGER_H
#define __FW_MANAGER_H

#include "stm32h7xx_hal.h"
#include "flash_ops.h"

/* 固件状态枚举 */
typedef enum
{
    FW_STATE_IDLE = 0,
    FW_STATE_UPDATING,
    FW_STATE_READY,   /* APP2 准备就绪 */
    FW_STATE_VALID,   /* APP1 有效 */
    FW_STATE_INVALID
} fw_state_t;

#define FW_INFO_MAGIC           0x55AA55AA
#define FW_VERSION_STR_LEN      12

/* 地址定义 */
#define ADDR_APP1_START         0x08020000
#define ADDR_APP2_START         0x08100000
#define ADDR_VERSION_SECTOR     0x081E0000
#define APP_MAX_SIZE            (896 * 1024)

#define SLOT_APP1               1
#define SLOT_APP2               0

#pragma pack(push, 4)
typedef struct
{
    uint32_t magic;
    char version_str[FW_VERSION_STR_LEN];
    uint32_t version_num;
    uint32_t file_size;
    uint32_t crc32;
    uint8_t  state;
    uint8_t  update_count;
    uint16_t rollback_mark;
    uint32_t timestamp;
    uint32_t header_crc;
} firmware_info_t;
#pragma pack(pop)

/* 计算 32 字节对齐的步进 */
#define FW_INFO_SLOT_STEP       ((uint32_t)(((sizeof(firmware_info_t) + 31) / 32) * 32))

HAL_StatusTypeDef fw_slot_get_info(uint8_t slot_idx, firmware_info_t *info);
HAL_StatusTypeDef fw_slot_set_info(uint8_t slot_idx, firmware_info_t *info);
HAL_StatusTypeDef fw_sync_backup_to_main(void);

#endif /* __FW_MANAGER_H */
