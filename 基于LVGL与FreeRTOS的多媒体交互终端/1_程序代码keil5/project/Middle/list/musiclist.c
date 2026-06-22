#include "musiclist.h"
#include "../../middle/fatfs/ff.h"
#include "../../middle/malloc/malloc.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* 前置声明静态辅助函数 */
static const char* _get_or_create_dir_node(music_list_t *mlist, const char *dir_path);
static music_type_t _get_music_type_from_name(const char *name);
static int _music_scan_rec(music_list_t *mlist, char *path_buf, int level, uint32_t filter_mask);

/**
 * music_list_init - 初始化音乐列表管理器
 */
void music_list_init(music_list_t *mlist, const char *name) 
{
    if (!mlist) return;

    LIST_INIT(&mlist->list, name, music_info_t, node);
    mlist->it_obj = NULL;
    mlist->dir_count = 0;
    mlist->info_count = 0;
    mlist->flag = MUSIC_FLAG_NONE;

    /* 清空静态分配的目录池和计数器 */
    memset(mlist->dir_pool, 0, sizeof(mlist->dir_pool));
    memset(mlist->dir_file_count, 0, sizeof(mlist->dir_file_count));
}

/**
 * music_list_destroy - 销毁列表并释放所有动态内存
 * 注意：由于 dir_pool 现在是静态数组，不需要手动释放目录内存
 */
void music_list_destroy(music_list_t *mlist) 
{
    list_node_t *pos, *n;

    if (!mlist) return;

    /* 释放所有 music_info_t 业务对象内存 */
    LIST_FOR_EACH_NODE_SAFE(pos, n, &mlist->list) 
    {
        music_info_t *info = LIST_NODE_GET_OBJ(pos, &mlist->list);
        list_delete_node(&mlist->list, pos);
        myfree(SRAMIN, info);
    }

    mlist->dir_count = 0;
    mlist->info_count = 0;
    mlist->it_obj = NULL;
    /* 静态数组随 mlist 销毁自动释放 */
}

/**
 * music_list_clear - 清除列表中所有歌曲并重置目录池
 * @mlist: 列表管理器指针
 * * 说明：该函数保留 mlist 结构体本身，但释放所有歌曲节点，
 * 并重置目录池计数，使列表回到初始空状态。
 */
void music_list_clear(music_list_t *mlist)
{
    if (mlist == NULL) return;

    /* 1. 释放所有歌曲节点的内存 */
    list_node_t *pos, *n;
    // 使用 SAFE 宏确保在删除节点时遍历不会出错
    LIST_FOR_EACH_NODE_SAFE(pos, n, &mlist->list) 
    {
        music_info_t *info = LIST_NODE_GET_OBJ(pos, &mlist->list);
        
        // 从链表中删除该节点
        list_delete_node(&mlist->list, pos);
        
        // 释放 mymalloc 申请的 SRAM 内存
        myfree(SRAMIN, info);
    }

    /* 2. 重置列表基础计数器 */
    mlist->info_count = 0;
    mlist->it_obj = NULL;

    /* 3. 重置目录池状态 */
    mlist->dir_count = 0;
    // 将所有目录的文件计数归零，逻辑上释放所有目录槽位
    memset(mlist->dir_file_count, 0, sizeof(mlist->dir_file_count));
    
    // 可选：清空目录路径字符串数组（非必须，因为 count=0 后会被覆盖）
    memset(mlist->dir_pool, 0, sizeof(mlist->dir_pool));

    printf("[MusicList] List cleared, SRAM memory freed.\n");
}


int music_list_add(music_list_t *mlist, music_info_t *info) 
{
    list_node_t *pos;
    const char *pooled_dir;

    if (!mlist || !info || mlist->info_count >= MAX_FILE_COUNT) return -1;

    /* 1. 获取目录池地址（此时 count 可能还是 0） */
    pooled_dir = _get_or_create_dir_node(mlist, info->dir);
    if (!pooled_dir) return -1;

    /* 2. 查重：同目录 + 同名 */
    LIST_FOR_EACH_NODE(pos, &mlist->list) 
    {
        music_info_t *existing = (music_info_t *)LIST_NODE_GET_OBJ(pos, &mlist->list);
        if (existing && existing->dir == pooled_dir && strcmp(existing->name, info->name) == 0) 
        {
            return 1; 
        }
    }

    /* 3. 申请堆内存并拷贝 */
    music_info_t *new_node = (music_info_t *)mymalloc(SRAMIN, sizeof(music_info_t));
    if (!new_node) return -1;

    memcpy(new_node, info, sizeof(music_info_t));
    memset(&new_node->node, 0, sizeof(list_node_t)); 
    new_node->dir = pooled_dir;

    /* 4. 关键：增加目录文件计数 */
    int dir_idx = (pooled_dir - &mlist->dir_pool[0][0]) / MAX_DIR_LEN;
    // 理论上 dir_idx 此时一定合法
    mlist->dir_file_count[dir_idx]++; 

    /* 5. 挂载并维护总数 */
    list_add_tail(&mlist->list, &new_node->node);
    mlist->info_count++;

    if (mlist->it_obj == NULL) mlist->it_obj = new_node;

    return 0;
}


int music_list_remove(music_list_t *mlist, music_info_t *target)
{
    if (mlist == NULL || target == NULL || mlist->info_count == 0) return -1;

    /* 1. 更新计数器：通过地址偏移定位索引 */
    if (target->dir != NULL) {
        int dir_idx = (target->dir - &mlist->dir_pool[0][0]) / MAX_DIR_LEN;

        if (dir_idx >= 0 && dir_idx < MAX_DIR_COUNT) {
            if (mlist->dir_file_count[dir_idx] > 0) {
                mlist->dir_file_count[dir_idx]--;
            }
            // 提示：若 count 减到 0，该槽位在 _get_or_create_dir_node 中已视为“空闲”
        }
    }

    /* 2. 维护游标 */
    if (mlist->it_obj == target) {
        music_info_t *next_p = music_list_get_next(mlist);
        mlist->it_obj = (next_p == target) ? NULL : next_p;
    }

    /* 3. 释放内存 */
    list_delete_node(&mlist->list, &target->node);
    myfree(SRAMIN, target);
    mlist->info_count--;

    return 0;
}



music_info_t* music_list_get_next(music_list_t *mlist) 
{
    list_node_t *next_n;
    music_info_t *info;
    int search_count = 0;

    if (!mlist || mlist->info_count == 0) return NULL;

    next_n = mlist->it_obj ? &mlist->it_obj->node : &mlist->list.root;

    while (search_count < mlist->info_count) 
    {
        next_n = next_n->next;
        if (next_n == &mlist->list.root) next_n = next_n->next;

        info = (music_info_t *)LIST_NODE_GET_OBJ(next_n, &mlist->list);
        
        if (mlist->flag == MUSIC_FLAG_NONE || (info->flag & (uint32_t)mlist->flag)) {
            mlist->it_obj = info;
            return mlist->it_obj;
        }
        search_count++;
    }
    return NULL;
}

music_info_t* music_list_get_prev(music_list_t *mlist) 
{
    list_node_t *prev_n;
    music_info_t *info;
    int search_count = 0;

    if (!mlist || mlist->info_count == 0) return NULL;

    prev_n = mlist->it_obj ? &mlist->it_obj->node : &mlist->list.root;

    while (search_count < mlist->info_count) 
    {
        prev_n = prev_n->prev;
        if (prev_n == &mlist->list.root) prev_n = prev_n->prev;

        info = (music_info_t *)LIST_NODE_GET_OBJ(prev_n, &mlist->list);

        if (mlist->flag == MUSIC_FLAG_NONE || (info->flag & (uint32_t)mlist->flag)) {
            mlist->it_obj = info;
            return mlist->it_obj;
        }
        search_count++;
    }
    return NULL;
}

music_info_t* music_list_get_curr(music_list_t *mlist) 
{
    if (!mlist || mlist->info_count == 0) return NULL;

    if (mlist->it_obj == NULL || 
       (mlist->flag != MUSIC_FLAG_NONE && !(mlist->it_obj->flag & (uint32_t)mlist->flag))) 
    {
        return music_list_get_next(mlist);
    }
    return mlist->it_obj;
}

music_info_t* music_list_set_curr(music_list_t *mlist, music_info_t *target)
{
    if (mlist == NULL || target == NULL || mlist->info_count == 0) return NULL;
    mlist->it_obj = target;
    return mlist->it_obj;
}

music_info_t* music_list_set_flag(music_list_t *mlist, music_flag_t flag)
{
    if (mlist == NULL) return NULL;

    music_flag_t old_flag = mlist->flag;
    music_info_t *old_obj = mlist->it_obj;

    mlist->flag = flag;

    if (flag == MUSIC_FLAG_NONE) {
        if (mlist->it_obj == NULL) mlist->it_obj = music_list_get_next(mlist);
        return mlist->it_obj;
    }

    if (mlist->it_obj != NULL && (mlist->it_obj->flag & (uint32_t)flag)) return mlist->it_obj;

    mlist->it_obj = NULL; 
    music_info_t *found = music_list_get_next(mlist);

    if (found != NULL) return found;
    else {
        mlist->flag = old_flag;
        mlist->it_obj = old_obj;
        return NULL; 
    }
}

int music_list_scan_fatfs(music_list_t *mlist, const char *root_path, uint32_t filter_mask) 
{
    char path_worker[256];
    int len;

    if (!mlist || !root_path) return 0;

    strncpy(path_worker, root_path, 255);
    path_worker[255] = '\0';
    
    len = strlen(path_worker);
    if (len > 0 && (path_worker[len-1] == '/' || path_worker[len-1] == '\\')) path_worker[len-1] = '\0';

    return _music_scan_rec(mlist, path_worker, 0, filter_mask);
}

uint16_t music_list_get_count(music_list_t *mlist)
{
    return (mlist == NULL) ? 0 : (uint16_t)mlist->info_count;
}

/* --- Static 辅助函数实现 --- */

static int _music_scan_rec(music_list_t *mlist, char *path_buf, int level, uint32_t filter_mask) 
{
    DIR dir;
    FILINFO fno;
    int add_count = 0;
    int base_len = strlen(path_buf);

    // 限制递归深度（防止栈溢出），打开目录
    if (level > 10 || f_opendir(&dir, path_buf) != FR_OK) return 0;

    while (f_readdir(&dir, &fno) == FR_OK && fno.fname[0] != 0) 
    {
        // 忽略隐藏文件或系统目录 (. ..)
        if (fno.fname[0] == '.') continue;

        if (fno.fattrib & AM_DIR) 
        {
            // 递归进入子目录
            int name_len = strlen(fno.fname);
            if (base_len + name_len + 2 < 256) 
            {
                sprintf(path_buf + base_len, "/%s", fno.fname);
                add_count += _music_scan_rec(mlist, path_buf, level + 1, filter_mask);
                path_buf[base_len] = '\0'; // 递归回来后还原路径
            }
        } 
        else 
        {
            // 处理文件
            music_type_t mtype = _get_music_type_from_name(fno.fname);
            if (mtype != MUSIC_TYPE_UNKNOWN && (mtype & filter_mask)) 
            {
                /* --- 修改点：适配新的 music_list_add 接口 --- */
                music_info_t temp_info;
                memset(&temp_info, 0, sizeof(temp_info));
                
                // 填充临时结构体
                strncpy(temp_info.name, fno.fname, MAX_NAME_LEN - 1);
                temp_info.dir = path_buf; // 传入当前路径字符串，add 内部会进行池化处理
                temp_info.type = mtype;
                temp_info.flag = MUSIC_FLAG_NONE; // 扫描到的新歌默认无标记

                // 调用新版接口
                if (music_list_add(mlist, &temp_info) == 0) 
                {
                    add_count++;
                }
            }
        }
    }
    f_closedir(&dir);
    return add_count;
}

static music_type_t _get_music_type_from_name(const char *name) 
{
    const char *ext = strrchr(name, '.');
    if (!ext) return MUSIC_TYPE_UNKNOWN;

    if (strcasecmp(ext, ".mp3") == 0)  return MUSIC_TYPE_MP3;
    if (strcasecmp(ext, ".wav") == 0)  return MUSIC_TYPE_WAV;
    if (strcasecmp(ext, ".flac") == 0) return MUSIC_TYPE_FLAC;
    if (strcasecmp(ext, ".aac") == 0)  return MUSIC_TYPE_AAC;
    if (strcasecmp(ext, ".m4a") == 0)  return MUSIC_TYPE_M4A;

    return MUSIC_TYPE_UNKNOWN;
}

static const char* _get_or_create_dir_node(music_list_t *mlist, const char *dir_path) 
{
    int i;
    int empty_idx = -1; // 用来记录第一个发现的空槽位

    if (!mlist || !dir_path) return NULL;

    /* 直接遍历所有槽位 */
    for (i = 0; i < MAX_DIR_COUNT; i++) 
    {
        // 1. 如果这个槽位有人占了，看看路径是不是一样
        if (mlist->dir_file_count[i] > 0) 
        {
            if (strcmp(mlist->dir_pool[i], dir_path) == 0) 
            {
                return mlist->dir_pool[i]; // 找到了，直接返回
            }
        }
        // 2. 如果没被占，且我们还没记录过空位，就记下这个索引
        else if (empty_idx == -1) 
        {
            empty_idx = i;
        }
    }

    /* 如果走到这里，说明没找到相同的路径 */
    
    // 如果连空位都没发现，说明真的满了
    if (empty_idx == -1) return NULL;

    /* 在发现的空位上创建新目录 */
    if (strlen(dir_path) >= MAX_DIR_LEN) return NULL;

    strncpy(mlist->dir_pool[empty_idx], dir_path, MAX_DIR_LEN - 1);
    mlist->dir_pool[empty_idx][MAX_DIR_LEN - 1] = '\0';
    
    // 初始化计数（具体的 ++ 逻辑在 music_list_add 里）
    mlist->dir_file_count[empty_idx] = 0; 
    
    // 顺便维护一下 dir_count，确保它总是指向“被动过”的最大范围（可选）
    if (empty_idx >= mlist->dir_count) {
        mlist->dir_count = empty_idx + 1;
    }

    return mlist->dir_pool[empty_idx];
}
