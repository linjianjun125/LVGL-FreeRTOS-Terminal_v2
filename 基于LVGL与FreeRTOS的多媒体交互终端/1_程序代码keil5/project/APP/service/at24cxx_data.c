#include "at24cxx_data.h"
#include "../devices/at24cxx/at24cxx.h"
#include "../../middle/linklist/linklist.h"
#include <string.h>
#include <stdio.h>


/**
 * @brief 通用查表法/计算法 CRC16 (Modbus)
 * @param data: 指向待校验数据的指针 (如 &config)
 * @param len:  数据长度 (如 sizeof(backlight_config_t))
 * @return uint16_t: 计算出的 16 位校验码
 */
uint16_t manager_calculate_crc(uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF; // 预置 16 位寄存器为十六进制 FFFF
    
    for (uint16_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i]; // 把第一个 8 位数据与 16 位 CRC 寄存器的低 8 位相异或
        
        for (int j = 0; j < 8; j++) { // 循环 8 次
            if (crc & 0x0001) { // 如果最低位为 1
                crc = (crc >> 1) ^ 0xA001; // 右移一位并与多项式 A001 异或
            } else { // 如果最低位为 0
                crc >>= 1; // 右移一位
            }
        }
    }
    return crc;
}

/*-----------------------------------------------------
							音量
-------------------------------------------------------*/
uint8_t at24cxx_set_volume_manager_info(volume_config_t *config) 
{
    if (config == NULL) return 1;
    
    volume_storage_header_t header;
    header.magic = VOLUME_INFO_MAGIC_FLAG;
    header.data_len = sizeof(volume_config_t);
    header.crc16 = manager_calculate_crc((uint8_t *)config, sizeof(volume_config_t));

    at24cxx_write_data(VOLUME_AT24CXX_ADDR, (uint8_t *)&header, sizeof(header));
    at24cxx_write_data(VOLUME_AT24CXX_ADDR + sizeof(header), (uint8_t *)config, sizeof(volume_config_t));
    return 0;
}

uint8_t at24cxx_get_volume_manager_info(volume_config_t *config) 
{
    if (config == NULL) return 1;
    
    volume_storage_header_t header;
    
    // 1. 读取头部并校验
    at24cxx_read_data(VOLUME_AT24CXX_ADDR, (uint8_t *)&header, sizeof(header));

    // 2. 校验魔数
    if (header.magic != VOLUME_INFO_MAGIC_FLAG) {
        printf("[EEPROM] Volume Magic Mismatch! Expected: 0x%08X, Got: 0x%08X\n", 
				VOLUME_INFO_MAGIC_FLAG, (unsigned int)header.magic);
        return 2;
    }

    // 3. 确定读取长度并读取配置数据本体
    uint16_t read_len = (header.data_len < sizeof(volume_config_t)) ? 
                         header.data_len : sizeof(volume_config_t);
                         
    at24cxx_read_data(VOLUME_AT24CXX_ADDR + sizeof(header), (uint8_t *)config, read_len);

    // 4. CRC 完整性校验
    uint16_t current_crc = manager_calculate_crc((uint8_t *)config, read_len);
    if (current_crc != header.crc16) {
        printf("[EEPROM] Volume CRC Error! Expected: 0x%04X, Got: 0x%04X\n",  header.crc16, current_crc);
        return 3;
    }
	
	printf("[info] Success at24cxx_get_volume_manager_info \r\n");

    return 0;
}

/*-----------------------------------------------------
							闹钟
-------------------------------------------------------*/

// --- 保存函数 ---
uint8_t at24cxx_set_alarm_manager_info(alarm_config_t *config)
 {
    if (config == NULL) return 1;

    alarm_storage_header_t header;
    header.magic = ALARM_INFO_MAGIC_FLAG;
    header.alarm_count = (uint8_t)config->count;
    header.reserved[0] = 0x00;

    // 计算当前 alarms 数组中有用部分的 CRC
    uint16_t data_size = sizeof(time_alarm_t) * config->count;
    if (config->count > 0) {
        header.crc16 = manager_calculate_crc((uint8_t *)config->alarms, data_size);
    } else {
        header.crc16 = 0;
    }

    // 1. 写入 Header
    at24cxx_write_data(ALARM_AT24CXX_ADDR, (uint8_t *)&header, sizeof(header));
    
    // 2. 写入数据
    if (config->count > 0) {
        at24cxx_write_data(ALARM_AT24CXX_ADDR + sizeof(header), (uint8_t *)config->alarms, data_size);
    }
    return 0;
}

// --- 读取函数 ---
uint8_t at24cxx_get_alarm_manager_info(alarm_config_t *config) 
{
    if (config == NULL) return 1;

    alarm_storage_header_t header;
    at24cxx_read_data(ALARM_AT24CXX_ADDR, (uint8_t *)&header, sizeof(header));

    // 校验魔数
    if (header.magic != ALARM_INFO_MAGIC_FLAG)
	{	
		printf("[ERROR] No valid alarm data (Magic: 0x%08X mismatch).\n", (unsigned int)header.magic);
		return 2;
	}

    // 读取数据
    config->count = (int)header.alarm_count;
    if (config->count > 0 && config->count <= MAX_ALARMS) 
	{
        uint16_t data_size = sizeof(time_alarm_t) * config->count;
        at24cxx_read_data(ALARM_AT24CXX_ADDR + sizeof(header), (uint8_t *)config->alarms, data_size);

        // CRC 校验检测
        uint16_t current_crc = manager_calculate_crc((uint8_t *)config->alarms, data_size);
        if (current_crc != header.crc16) 
		{
            printf("[ERROR] Alarm CRC Error! Data corrupted.\n");
            config->count = 0; // 校验失败，清空加载的数据
            return 3;
        }
    }
	
	printf("[info] Success at24cxx_get_alarm_manager_info \r\n");
    return 0;
}





/*-----------------------------------------------------
							背光
-------------------------------------------------------*/
/**
 * @brief 保存背光配置到 EEPROM
 */
uint8_t at24cxx_set_backlight_manager_info(backlight_config_t *config)
{
    if (config == NULL) return 1;

    backlight_storage_header_t header;
    header.magic = BACKLIGHT_INFO_MAGIC_FLAG;
    header.data_len = sizeof(backlight_config_t);
    
    // 计算 config 数据的 CRC
    header.crc16 = manager_calculate_crc((uint8_t *)config, sizeof(backlight_config_t));

    // 1. 写入校验头部 (8字节)
    at24cxx_write_data(BACKLIGHT_AT24CXX_ADDR, (uint8_t *)&header, sizeof(header));
    
    // 2. 紧接着写入具体的 config 结构体本体
    uint16_t data_addr = BACKLIGHT_AT24CXX_ADDR + sizeof(header);
    at24cxx_write_data(data_addr, (uint8_t *)config, sizeof(backlight_config_t));

    return 0;
}

/**
 * @brief 从 EEPROM 加载背光配置
 */
uint8_t at24cxx_get_backlight_manager_info(backlight_config_t *config)
{
    if (config == NULL) return 1;

    backlight_storage_header_t header;
    
    // 1. 读取头部并校验
    at24cxx_read_data(BACKLIGHT_AT24CXX_ADDR, (uint8_t *)&header, sizeof(header));

    if (header.magic != BACKLIGHT_INFO_MAGIC_FLAG)
    {
        printf("[ERROR] Backlight Magic Mismatch!\n");
        return 2; 
    }

    // 2. 确定读取长度
    uint16_t data_addr = BACKLIGHT_AT24CXX_ADDR + sizeof(header);
    uint16_t read_len = header.data_len;
                         
    at24cxx_read_data(data_addr, (uint8_t *)config, read_len);

    // 3. CRC 完整性校验
    uint16_t current_crc = manager_calculate_crc((uint8_t *)config, read_len);
    if (current_crc != header.crc16)
    {
        printf("[ERROR] Backlight CRC Error! Data corrupted.\n");
        return 3;
    }
	
	printf("[info] Success at24cxx_get_backlight_manager_info \r\n");
	
    return 0;
}






/*-----------------------------------------------------
							小说
-------------------------------------------------------*/
uint8_t at24cxx_set_novel_manager_info(novel_control_t *novel)
{
    if (novel == NULL) return 1;
    
    LinkList_Item* pxItem;
    uint8_t save_index = 0;
    const uint16_t step_size = sizeof(novel_info_t);

    List_HeadFor(&(novel->List), pxItem) 
    {
        if (save_index >= 20) break;

        novel_info_t* pInfo = (novel_info_t*)List_ListItemGetData(pxItem);
        if (pInfo != NULL) 
        {
            // 1. 打上魔数标记
            pInfo->flag = NOVEL_INFO_MAGIC_FLAG; 

            // 2. 修正地址计算：基地址 + 索引 * 结构体大小
            // 假设从 EEPROM 地址 0 开始存，如果是从特定位置，请加偏置
            uint16_t addr = NOVEL_AT24CXX_ADDR + save_index * step_size; 

            // 3. 写入数据
            at24cxx_write_data(addr, (uint8_t*)pInfo, step_size);
            save_index++;
        }
    }
    return 0;
}


/**
 * @brief  从 AT24CXX 读取已存书籍信息并重建管理器链表
 * @param  novel: 指向小说管理器结构体的指针
 * @return uint8_t: 0-成功; 1-参数为空; 2-未检测到有效书籍
 */
uint8_t at24cxx_get_novel_manager_info(novel_control_t *novel)
{
    if (novel == NULL) return 1;

    novel_info_t temp_node;
    uint8_t found_count = 0;
    const uint16_t step_size = sizeof(novel_info_t);

    for (uint8_t i = 0; i < 20; i++)
    {
        // 确保地址计算与写入端完全一致
        uint16_t addr = NOVEL_AT24CXX_ADDR + (i * step_size);

        // 每次读取前必须清空，防止残留数据干扰判断
        memset(&temp_node, 0, sizeof(novel_info_t));

        // 从 EEPROM 读取一个结构体大小的数据
        at24cxx_read_data(addr, (uint8_t *)&temp_node, step_size);

        if (temp_node.flag == NOVEL_INFO_MAGIC_FLAG) 
        {
            // 匹配成功，插入链表
            LinkList_Item* pNewItem = List_Insert(&(novel->List));
            if (pNewItem != NULL)
            {
                List_ListItemWriteData(pNewItem, &temp_node);
                found_count++;
            }
        }
    }

	printf("[info] [success] at24cxx_get_novel_manager_info \r\n");

    return (found_count == 0) ? 2 : 0;
}




/*-----------------------------------------------------
							音乐
-------------------------------------------------------*/
/* 音乐存储头部 */
uint8_t at24cxx_save_music_list(music_list_t *mlist)
{
    if (mlist == NULL) 
	{
		printf("mlist == NULL");
		return 1;
	}

    music_storage_header_t header;
    header.magic = MUSIC_LIST_MAGIC_FLAG;
    header.music_count = mlist->info_count;
    header.dir_count = mlist->dir_count;
    
    /* 修正 1：增加 (void*) 强制转换，解决 __packed 指针报错 */
    memcpy((void *)header.dir_file_count, (void *)mlist->dir_file_count, sizeof(header.dir_file_count));
    
    header.crc16 = 0; 

    uint32_t addr = MUSIC_LIST_AT24CXX_ADDR;

    // A. 写入 Header
    at24cxx_write_data(addr, (uint8_t *)&header, sizeof(header));
    addr += sizeof(header);

    // B. 分行/分块写入全量目录池
    // 虽然 write_data 支持跨页，但对于 4KB 数据，建议分行写并监控
    for (int d = 0; d < MAX_DIR_COUNT; d++) {
		// 计算当前槽位的物理地址
        uint32_t current_addr = addr + (d * MAX_DIR_LEN);
        
        // 打印逻辑：如果该目录下有歌，打印详细路径；否则打印槽位复用提示
        if (mlist->dir_file_count[d] > 0) {
            printf("  Slot[%02d]: (Addr:0x%04X) [Active - %-3d songs] Path: %s\n", d, current_addr, mlist->dir_file_count[d], mlist->dir_pool[d]);
        } else if (mlist->dir_pool[d][0] != '\0') {
            printf("  Slot[%02d]: (Addr:0x%04X) [Reusable Empty] Last Path: %s\n", d, current_addr, mlist->dir_pool[d]);
        }
		// 执行写入操作
        at24cxx_write_data(current_addr, (uint8_t *)mlist->dir_pool[d], MAX_DIR_LEN);
    }
    addr += (MAX_DIR_COUNT * MAX_DIR_LEN);

    // C. 遍历链表，写入歌曲信息
    list_node_t *pos;
    LIST_FOR_EACH_NODE(pos, &mlist->list) 
    {
        music_info_t *info = LIST_NODE_GET_OBJ(pos, &mlist->list);
        music_storage_item_t s_item;

        memset(&s_item, 0, sizeof(s_item));
        strncpy(s_item.name, info->name, MAX_NAME_LEN - 1);
        s_item.type = (uint32_t)info->type;
        s_item.flag = (uint32_t)info->flag;
        s_item.dir_idx = (uint16_t)((info->dir - &mlist->dir_pool[0][0]) / MAX_DIR_LEN);

        at24cxx_write_data(addr, (uint8_t *)&s_item, sizeof(s_item));
        addr += sizeof(s_item);
    }
    
    printf("[EEPROM] Music list saved.\n");
    return 0;
}

uint8_t at24cxx_load_music_list(music_list_t *mlist)
{
    music_storage_header_t header;
    at24cxx_read_data(MUSIC_LIST_AT24CXX_ADDR, (uint8_t *)&header, sizeof(header));

    if (header.magic != MUSIC_LIST_MAGIC_FLAG) 
	{
		printf("header.magic != MUSIC_LIST_MAGIC_FLA\r\n");
		return 2;
	}

    music_list_clear(mlist); 
    
    uint32_t addr = MUSIC_LIST_AT24CXX_ADDR + sizeof(header);

    // 3. 恢复目录名字符串 (这里的 4096 现在可以被正确读取了)
    at24cxx_read_data(addr, (uint8_t *)mlist->dir_pool, MAX_DIR_COUNT * MAX_DIR_LEN);
    addr += (MAX_DIR_COUNT * MAX_DIR_LEN);

    // 打印调试
    printf("\r\n--- EEPROM Dir Pool Load Check ---\n");
    for (int d = 0; d < MAX_DIR_COUNT; d++) {
        if (mlist->dir_pool[d][0] != '\0') {
            printf("  Slot[%02d]: (Ref:%d) %s\n", d, header.dir_file_count[d], mlist->dir_pool[d]);
        }
    }

    // 4. 重建歌曲链表
    for (int i = 0; i < header.music_count; i++) 
    {
        music_storage_item_t s_item;
        at24cxx_read_data(addr, (uint8_t *)&s_item, sizeof(s_item));

        music_info_t temp;
        memset(&temp, 0, sizeof(temp));
        strncpy(temp.name, s_item.name, MAX_NAME_LEN - 1);
        
        /* 修正 2：枚举类型强制转换，消除警告 */
        temp.type = (music_type_t)s_item.type;
        temp.flag = (music_flag_t)s_item.flag;
        
        if (s_item.dir_idx < MAX_DIR_COUNT) {
            temp.dir = mlist->dir_pool[s_item.dir_idx];
        }

        // add 内部会自动恢复 mlist->dir_file_count 和 dir_count
        music_list_add(mlist, &temp); 
        addr += sizeof(s_item);
    }

    return 0;
}
