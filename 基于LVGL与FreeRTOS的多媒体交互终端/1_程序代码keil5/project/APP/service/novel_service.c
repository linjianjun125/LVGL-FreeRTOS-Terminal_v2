#include "novel_service.h"
#include "at24cxx_data.h"
#include "../../middle/fatfs/ff.h"
#include "../../middle/linklist/linklist.h"
#include <stdio.h>
#include <string.h>

void Scan_Novel_Files_Smart(LinkList* pList, char* path, uint8_t max_to_add);
/**
 * @brief  初始化小说管理器数据
 */
void Novel_Manager_Init_Data(void)
{
    FRESULT res;
    FILINFO fno;
    char full_path[128];
    LinkList_Item *pxItem;
    
    NovelControl.Dir = "0:/NOVEL";
    List_Init(&NovelControl.List, sizeof(novel_info_t));

    // --- 1. 从 EEPROM 获取书籍信息 ---
    // 确保 at24cxx_get_novel_manager_info 内部是从 NOVEL_AT24CXX_ADDR (0x00) 开始读的
    at24cxx_get_novel_manager_info(&NovelControl);

    printf("--- Books loaded from EEPROM ---\n");
    List_HeadFor(&(NovelControl.List), pxItem) 
    {
        novel_info_t* pInfo = (novel_info_t*)List_ListItemGetData(pxItem);
        DUMP_NOVEL_INFO(pInfo);
    }

    // --- 2. 校验文件逻辑 (使用更安全的遍历删除) ---
    // 注意：在循环中删除节点时，pxItem 必须谨慎处理
	printf("Books Invalid, Removing From List\n");
    pxItem = List_GetListFirstItem(&(NovelControl.List));
    while (pxItem != (NovelControl.List.pxHead)) 
    {
        LinkList_Item* pxNext = pxItem->pxNext; // 预存下一个节点
        novel_info_t* pInfo = (novel_info_t*)List_ListItemGetData(pxItem);

        sprintf(full_path, "%s/%s", NovelControl.Dir, pInfo->fileName);
        res = f_stat(full_path, &fno);

        // 校验：文件不存在 OR 大小变化 OR 不是文件
        if (res != FR_OK || (fno.fattrib & AM_DIR) || fno.fsize != pInfo->fileSize)
        {
            printf("[Warning] File Invalid, Removing Record: %s\n", pInfo->fileName);
            List_Remove(pxItem); 
        }
        pxItem = pxNext; // 使用预存的指针跳转
    }

    // --- 3. 补全逻辑 ---
    uint8_t current_count = List_CurrentNumberOfItems(&(NovelControl.List));
    if (current_count < 20)
    {
        Scan_Novel_Files_Smart(&(NovelControl.List), NovelControl.Dir, 20 - current_count);
    }

    printf("Total Books in Manager: %d\n", List_CurrentNumberOfItems(&(NovelControl.List)));
}



/**
 * @brief  智能扫描 SD 卡文件
 */
void Scan_Novel_Files_Smart(LinkList* pList, char* path, uint8_t max_to_add)
{
    DIR dir;
    FILINFO fno;
    if (f_opendir(&dir, path) != FR_OK) return;

    uint8_t added = 0;
    while (f_readdir(&dir, &fno) == FR_OK && fno.fname[0] != 0 && added < max_to_add)
    {
        if (fno.fattrib & AM_DIR) continue;

        // 后缀判断
        char* ext = strrchr(fno.fname, '.');
        if (ext == NULL || strcasecmp(ext, ".txt") != 0) continue;

        // 查重逻辑
        bool already_exists = false;
        LinkList_Item* pxItem;
        List_HeadFor(pList, pxItem) 
        {
            novel_info_t* pInfo = (novel_info_t*)List_ListItemGetData(pxItem);
            if (strcmp(pInfo->fileName, fno.fname) == 0) {
                already_exists = true;
                break;
            }
        }

        if (!already_exists) 
        {
            LinkList_Item* pNewNode = List_Insert(pList);
            if (pNewNode) 
            {
                novel_info_t newInfo = {0};
                // --- 关键：新发现的文件也必须立即打上 Flag，否则 DeInit 时写进去没标记 ---
                newInfo.flag = NOVEL_INFO_MAGIC_FLAG; 
                
                strncpy(newInfo.fileName, fno.fname, 63);
                newInfo.fileSize = fno.fsize;
                newInfo.type = 0; 
                newInfo.page_lines = 10; 
                newInfo.line_max_chars = 14;
                
                List_ListItemWriteData(pNewNode, &newInfo);
                added++;
                printf("New File Found & Added: %s\n", fno.fname);
            }
        }
    }
    f_closedir(&dir);
}





novel_control_t NovelControl =
{
	.init = Novel_Manager_Init_Data,
};







