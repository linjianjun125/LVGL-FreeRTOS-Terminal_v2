#ifndef __SAVEDATA_MANAGER_H
#define __SAVEDATA_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "volume_service.h"
#include "alarm_service.h"
#include "backlight_service.h"
#include "novel_service.h"
#include "music_service.h"


/*---------------------------------
				音量信息
----------------------------------*/
#define VOLUME_INFO_MAGIC_FLAG    0x55AA5522  // 音量专属魔数
#define VOLUME_AT24CXX_ADDR       (128)      	// 音量起始存储地址

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint16_t data_len;
    uint16_t crc16;
} volume_storage_header_t;

uint8_t at24cxx_set_volume_manager_info(volume_config_t *config);
uint8_t at24cxx_get_volume_manager_info(volume_config_t *config) ;



/*---------------------------------
				闹钟
----------------------------------*/

#define ALARM_INFO_MAGIC_FLAG    0x55AA5533  // 闹钟专属魔数
#define ALARM_AT24CXX_ADDR       (1024)       // 闹钟起始存储地址

typedef struct __attribute__((packed)) {
    uint32_t magic;         // 4字节：魔数 (0x55AA6677)
    uint8_t  alarm_count;   // 1字节：有效闹钟数量
    uint8_t  reserved[1];   // 1字节：预留
    uint16_t crc16;         // 2字节：对后面 alarms 数组数据的校验和
} alarm_storage_header_t;   // 正好 8 字节，完美对齐

uint8_t at24cxx_set_alarm_manager_info(alarm_config_t *config);
uint8_t at24cxx_get_alarm_manager_info(alarm_config_t *config);


/*---------------------------------
				背光信息
----------------------------------*/
#define BACKLIGHT_INFO_MAGIC_FLAG    0x11225522  // 背光专属魔数
#define BACKLIGHT_AT24CXX_ADDR       (3072)      // 存储起始地址

typedef struct __attribute__((packed)) {
    uint32_t magic;      // 魔数校验
    uint16_t data_len;   // 数据长度（用于版本兼容）
    uint16_t crc16;      // 对 config 数据的 CRC 校验
} backlight_storage_header_t; // 正好 8 字节对齐

// 函数声明调整为直接接收 config 指针
uint8_t at24cxx_set_backlight_manager_info(backlight_config_t *config);
uint8_t at24cxx_get_backlight_manager_info(backlight_config_t *config);



/*---------------------------------
				小说
----------------------------------*/
#define NOVEL_INFO_MAGIC_FLAG    0x55AA7755
#define NOVEL_AT24CXX_ADDR		 (5120)
uint8_t at24cxx_get_novel_manager_info(novel_control_t *novel);
uint8_t at24cxx_set_novel_manager_info(novel_control_t *novel);



/*---------------------------------
              音乐列表存储
----------------------------------*/
#define MUSIC_LIST_MAGIC_FLAG    0x55AA7788  
#define MUSIC_LIST_AT24CXX_ADDR  (12800)       // 存储起始地址

// 1. 单条歌曲的持久化格式 (约 74 字节)
typedef struct __attribute__((packed)) {
    char name[MAX_NAME_LEN];    // 文件名
    uint16_t dir_idx;           // 目录池索引
    uint32_t type;              // 音乐格式
    uint32_t flag;              // 业务标记 (如：是否属于列表1)
} music_storage_item_t;

// 2. 存储控制头部
typedef struct __attribute__((packed)) {
    uint32_t magic;             // 魔数，用于判断 EEPROM 是否初始化过
    uint16_t music_count;       // 歌曲总数
    uint16_t dir_count;         // 目录池数量
	uint16_t dir_file_count[MAX_DIR_COUNT];
    uint16_t crc16;             // 整个数据区的校验码
} music_storage_header_t;

uint8_t at24cxx_save_music_list(music_list_t *mlist);
uint8_t at24cxx_load_music_list(music_list_t *mlist);


#endif

