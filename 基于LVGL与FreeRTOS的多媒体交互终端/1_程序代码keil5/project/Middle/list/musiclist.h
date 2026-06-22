#ifndef __MUSIC_INFO_H
#define __MUSIC_INFO_H

#include "list.h"
#include <stdint.h>

/* 最大目录池缓存数量 */
#define MAX_DIR_COUNT   64

/* 最大文件记录数量限制 */
#define MAX_FILE_COUNT  500 

/* 文件目录长度宏定义 */
#define MAX_DIR_LEN      64

/* 文件名长度宏定义 */
#define MAX_NAME_LEN    64





/**
 * enum music_type_t - 音乐格式掩码
 * @MUSIC_TYPE_UNKNOWN: 未知格式
 * @MUSIC_TYPE_MP3:     MP3 格式位
 * @MUSIC_TYPE_WAV:     WAV 格式位
 * @MUSIC_TYPE_FLAC:    FLAC 格式位
 * @MUSIC_TYPE_AAC:     AAC 格式位
 * @MUSIC_TYPE_M4A:     M4A 格式位
 */
typedef enum 
{
    MUSIC_TYPE_UNKNOWN = 0,
    MUSIC_TYPE_MP3     = 0x01,
    MUSIC_TYPE_WAV     = 0x02,
    MUSIC_TYPE_FLAC    = 0x04,
    MUSIC_TYPE_AAC     = 0x08,
    MUSIC_TYPE_M4A     = 0x10,
    MUSIC_TYPE_ALL     = 0xFFFFFF
} music_type_t;

/**
 * music_flag_t - 音乐业务标记位枚举
 * 采用位移方式，确保每个标记占用独立的位
 */
typedef enum 
{
    MUSIC_FLAG_NONE = 0,            /* 无标记 */
    MUSIC_FLAG_1 	= (1 << 0),     /* 标记 1 */
    MUSIC_FLAG_2   	= (1 << 1),     /* 标记 2 */
    MUSIC_FLAG_3   	= (1 << 2),     /* 标记 3 */
    MUSIC_FLAG_4    = (1 << 3),     /* 标记 4 */
    MUSIC_FLAG_5 	= (1 << 4),     /* 标记 5 */
    
    /* 组合标记示例 */
    MUSIC_FLAG_ALL      = 0xFFFFFFF
} music_flag_t;

/* 设置标记 (置 1) */
#define MUSIC_INFO_SET_FLAG(info, f)    ((info)->flag |= (f))

/* 清除标记 (置 0) */
#define MUSIC_INFO_CLR_FLAG(info, f)    ((info)->flag &= ~(f))

/* 判断标记是否被设置 */
#define MUSIC_INFO_GET_FLAG(info, f)    (((info)->flag & (f)) == (f))




/**
 * struct music_info_t - 音乐信息业务对象
 * @name: 文件名 (不含路径)
 * @type: 音乐格式类型掩码
 * @dir:  指向目录池中去重后的路径字符串
 * @flag: 业务逻辑标记位
 * @node: 链表零件
 */
typedef struct 
{
    char name[MAX_NAME_LEN];
    music_type_t type;
    const char *dir;
    music_flag_t flag;      
    list_node_t node;
} music_info_t;


/**
 * PRINT_MUSIC_INFO - 打印单个音乐节点详细信息
 * @info: 指向 music_info_t 的指针
 */
#define PRINT_MUSIC_INFO(info) do { \
    if ((info) == NULL) { \
        printf("\r\n[MUSIC] Pointer is NULL\n"); \
    } else { \
        printf("\r\n-------------- <Music Info @%p> -------------\n", (void*)(info)); \
        printf("Name:   %s\n", (info)->name); \
        printf("Type:   0x%02X\n", (info)->type); \
        printf("Dir:    %s\n", (info)->dir ? (info)->dir : "NULL"); \
        printf("Flag:   0x%08X\n", (info)->flag); \
        printf("Node:   Next=%p, Prev=%p\n", (void*)(info)->node.next, (void*)(info)->node.prev); \
        printf("---------------------------------------------------\r\n"); \
    } \
} while(0)

/**
 * struct music_list_t - 音乐列表管理器
 * @list:       底层双向链表容器
 * @it_obj:     迭代游标指针
 * @dir_pool:   目录字符串去重池
 * @dir_count:  当前池中存储的目录数量
 * @info_count: 链表中总记录数
 */
typedef struct 
{
    list_t list;
    music_info_t *it_obj;
	
    /* 目录池：固定长度的二维数组 */
    char dir_pool[MAX_DIR_COUNT][MAX_DIR_LEN];
    
    /* 引用计数：记录每个目录下 info 添加的次数 */
    uint16_t dir_file_count[MAX_DIR_COUNT];
	
    int dir_count;
    int info_count;
	music_flag_t flag;
} music_list_t;


/**
 * PRINT_MUSIC_LIST - 打印整个音乐列表状态及目录池
 * @mlist_ptr: 指向 music_list_t 的指针
 */
#define PRINT_MUSIC_LIST(mlist_ptr) do { \
    if ((mlist_ptr) == NULL) break; \
    printf("\r\n================ [ LIST: %s ] ================\n", (mlist_ptr)->list.name); \
    printf("Total Files: %d / %d\n", (mlist_ptr)->info_count, MAX_FILE_COUNT); \
    printf("Directories: %d / %d\n", (mlist_ptr)->dir_count, MAX_DIR_COUNT); \
    printf("Global Flag Filter: 0x%08X\n", (mlist_ptr)->flag); \
    \
    /* 1. 打印目录池内容及每个目录下的文件数 */ \
    printf("--- Directory Pool (Static 2D Array) ---\n"); \
    for (int _d = 0; _d < (mlist_ptr)->dir_count; _d++) { \
        printf("  Pool[%02d]: %p | Files: %-3d | Path: %s\n", \
               _d, (void*)(mlist_ptr)->dir_pool[_d], \
               (mlist_ptr)->dir_file_count[_d], \
               (mlist_ptr)->dir_pool[_d]); \
    } \
    \
    /* 2. 打印当前游标状态 */ \
    printf("--- Iterator Status ---\n"); \
    printf("Current Pos: %p (%s)\n", \
            (void*)(mlist_ptr)->it_obj, \
            (mlist_ptr)->it_obj ? (mlist_ptr)->it_obj->name : "None"); \
    \
    /* 3. 遍历并打印所有文件 */ \
    printf("--- File Entries ---\n"); \
    list_node_t *_p; \
    int _i = 0; \
    LIST_FOR_EACH_NODE(_p, &((mlist_ptr)->list)) { \
        music_info_t *_info = (music_info_t *)((char *)_p - (mlist_ptr)->list.member_offset); \
        /* 计算该文件属于目录池的哪个索引 */ \
        int _d_idx = (_info->dir - &(mlist_ptr)->dir_pool[0][0]) / MAX_DIR_LEN; \
        printf("[%03d] %-32s (Type: 0x%02X, DirIdx: %02d, Flag: 0x%X)\n", \
               ++_i, _info->name, _info->type, _d_idx, _info->flag); \
    } \
    printf("====================================================\r\n"); \
} while(0)

/* --- API 接口 --- */
void music_list_init(music_list_t *mlist, const char *name);
void music_list_destroy(music_list_t *mlist);
void music_list_clear(music_list_t *mlist);
int music_list_add(music_list_t *mlist, music_info_t *info) ;
int music_list_remove(music_list_t *mlist, music_info_t *target);
music_info_t* music_list_get_next(music_list_t *mlist);
music_info_t* music_list_get_prev(music_list_t *mlist);
music_info_t* music_list_get_curr(music_list_t *mlist);
music_info_t* music_list_set_curr(music_list_t *mlist, music_info_t *target);
uint16_t music_list_get_count(music_list_t *mlist);
music_info_t* music_list_set_flag(music_list_t *mlist, music_flag_t flag);
int  music_list_scan_fatfs(music_list_t *mlist, const char *root_path, uint32_t filter_mask);


/**
 * MUSIC_LIST_FOR_EACH_FILTERED - 遍历符合当前列表 flag 要求的音乐项
 * @pos:   迭代用的 music_info_t 指针
 * @mlist: music_list_t 管理器指针
 */

#define MUSIC_LIST_FOR_EACH(pos, mlist) \
    for (list_node_t *_node = (mlist)->list.root.next; \
         _node != &(mlist)->list.root; \
         _node = _node->next) \
        if (((pos) = (music_info_t *)((char *)_node - (mlist)->list.member_offset)) != NULL) \
            if ((mlist)->flag == MUSIC_FLAG_NONE || ((pos)->flag & (uint32_t)(mlist)->flag))


#endif /* __MUSIC_INFO_H */
		
		
		
