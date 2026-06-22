#ifndef __FIRMWARE_SERRVICE_H
#define __FIRMWARE_SERRVICE_H


#include <stdint.h>
#include <stdbool.h>
#include "sys.h"

/* 固件信息常量定义 */
#define FW_INFO_MAGIC           	0x55AA55AA
#define FW_INFO_SIZE            	sizeof(firmware_info_t)  
#define FW_VERSION_STR_LEN      	12
#define ADDR_VERSION_SECTOR 		0x081E0000

#pragma pack(push, 4) /* 确保结构体在跨平台时不产生多余空隙 */
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



/**
 * @brief 固件信息全局结构体类型
 */
#define FW_VERSION_STR_LEN    12
#define FW_TIME_STR_LEN       20
#define FW_DEVICE_NAME_LEN    32
#define FW_MCU_TYPE_LEN       32
#define FW_MCU_MEN_LEN       12

typedef struct
{
    char device_name[FW_DEVICE_NAME_LEN]; // 设备名称
    char version[FW_VERSION_STR_LEN];     // 版本号
    char mcu_type[FW_MCU_TYPE_LEN];       // MCU型号
    char build_time[FW_TIME_STR_LEN];     // 编译时间
	char memory_info[FW_MCU_MEN_LEN];		
} fw_global_info_t;

/* --- 全局变量定义 --- */
extern fw_global_info_t g_sys_fw_info;


void fw_app_read_current_info(void);
void fw_global_info_init(void);


#endif



