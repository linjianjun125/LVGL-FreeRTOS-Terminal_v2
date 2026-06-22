#ifndef __NOVEL_MENAGER_H
#define __NOVEL_MENAGER_H

#include "sys.h"
#include "../../middle/linklist/linklist.h"

typedef enum {
    NOVEL_TYPE_TXT,
    NOVEL_TYPE_LOG,
    NOVEL_TYPE_UNKNOWN
} NovelType;


typedef struct __attribute__((packed))
{
	uint32_t flag;
	
    char fileName[64];      // 文件名
    uint32_t fileSize;      // 总字节大小
    uint32_t chapter_idx; 	// 当前章节
	uint8_t  cur_page_idx; 	// 当前章节页面
	
    int page_lines;         // 每页行数
    int line_max_chars;     // 每行字符宽度限制
	uint8_t theme;          // 阅读背景主题 (如 0:羊皮纸, 1:夜间)
	
    uint8_t type;           // NOVEL_TYPE_TXT 等
} novel_info_t;


/**
 * @brief  单条小说信息打印宏
 * @note   支持全路径显示，针对 packed 结构体进行了对齐优化
 */
#define DUMP_NOVEL_INFO(ptr) do { \
    if ((ptr) != NULL) { \
        printf("\r\n---------\n"); \
        printf("%s->fileName         = %s\n", #ptr, (ptr)->fileName); \
        printf("%s->fileSize         = %u bytes\n", #ptr, (ptr)->fileSize); \
        printf("%s->chapter_idx      = %lu\n", #ptr, (ptr)->chapter_idx); \
        printf("%s->cur_page_idx     = %u\n", #ptr, (ptr)->cur_page_idx); \
        printf("%s->page_lines       = %d\n", #ptr, (ptr)->page_lines); \
        printf("%s->line_max_chars   = %d\n", #ptr, (ptr)->line_max_chars); \
        printf("%s->theme            = %u\n", #ptr, (ptr)->theme); \
        printf("%s->type             = %u\n", #ptr, (ptr)->type); \
        printf("----------\r\n"); \
    } \
} while(0)


typedef struct 
{
    char* Dir;              // 小说根目录路径 (如 "0:/Book")
    LinkList List;          // 存储 NovelInfo 的双向链表
    novel_info_t *curr_info;// 当前正在阅读的小说信息指针
	
	void (*init)(void);
} novel_control_t;

extern novel_control_t NovelControl;

#endif

