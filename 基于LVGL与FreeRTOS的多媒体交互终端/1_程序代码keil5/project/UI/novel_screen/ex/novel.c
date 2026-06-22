#include "novel.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../middle/malloc/malloc.h"
#include "../../system/delay/delay.h"


// --- 内存管理宏定义 ---
#define vfs_malloc(length)	mymalloc(SRAMIN, length)
#define vfs_free(ptr)	    myfree(SRAMIN, ptr)

// --- 内部私有函数声明 ---
static int novel_internal_is_chapter(const char* line);
static int novel_internal_count_lines(const char* str, int max_w);
static void novel_free_active_chapter(NovelReader* r);
static uint8_t novel_try_load_sd_index(NovelReader* r);
static void novel_save_index_to_sd(NovelReader* r);
static long novel_internal_find_split_pos(const char* str, int max_w, int target_lines);

// --- 初始化与反初始化 ---

// 传入参数：reader结构体，目录路径 (如 "0:/BOOK")，文件名 (如 "test.txt")
uint8_t novel_init(NovelReader* reader, char* path, char* fname)
{
    char full_path[256];

    // 保存目录和文件名指针（或申请内存拷贝，此处暂存指针）
    reader->dir_path = path;
    reader->file_name = fname;

    // 申请章节索引表空间
    reader->all_chapters = (NovelChapterIdx*)vfs_malloc(sizeof(NovelChapterIdx) * NOVEL_MAX_CHAPTERS);
    if (!reader->all_chapters) 
	{
		printf("[error] 申请内存失败\r\n");
		return 1;
	}

    // 申请 FatFs 文件结构体空间
    reader->fp = (FIL*)vfs_malloc(sizeof(FIL));
    if (!reader->fp)
    {
        vfs_free(reader->all_chapters);
		printf("[error] 申请内存失败\r\n");
        return 1;
    }

    // 拼接完整路径进行文件打开
    sprintf(full_path, "%s/%s", path, fname);
    if (f_open(reader->fp, full_path, FA_READ) != FR_OK)
    {
        vfs_free(reader->fp);
        vfs_free(reader->all_chapters);
		printf("[error] 打开文件失败\r\n");
        return 1;
    }

    // 尝试加载索引：先找同目录下的 .idx 缓存，没有则扫描并保存
    if (novel_try_load_sd_index(reader) != 0)
    {
        novel_build_index(reader);
        novel_save_index_to_sd(reader);
    }

    // 初始化排版参数
    novel_set_display_params(reader, 20, 22);
    return 0;
}

void novel_deinit(NovelReader* r)
{
    // 释放分页缓存
    novel_free_active_chapter(r);
    // 释放章节索引表
    if (r->all_chapters) { vfs_free(r->all_chapters); r->all_chapters = NULL; }
    // 关闭并释放文件句柄
    if (r->fp) { f_close(r->fp); vfs_free(r->fp); r->fp = NULL; }
}

// --- 章节索引建立逻辑 ---

void novel_build_index(NovelReader* r)
{
    uint32_t br;
    char* buf = (char*)vfs_malloc(1024);
    if (!buf) return;

    f_lseek(r->fp, 0);
    r->total_chapters = 0;

    // BOM 检查
    unsigned char bom[3];
    if (f_read(r->fp, bom, 3, &br) == FR_OK && br == 3)
    {
        if (!(bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)) f_lseek(r->fp, 0);
    }
    else f_lseek(r->fp, 0);

    while (r->total_chapters < NOVEL_MAX_CHAPTERS)
    {
        long pos = f_tell(r->fp);
        if (!f_gets(buf, 1024, r->fp)) break;

        if (novel_internal_is_chapter(buf))
        {
            buf[strcspn(buf, "\r\n")] = 0;
            strncpy(r->all_chapters[r->total_chapters].title, buf, NOVEL_MAX_TITLE - 1);
            r->all_chapters[r->total_chapters].title[NOVEL_MAX_TITLE - 1] = '\0';
            r->all_chapters[r->total_chapters].start_offset = pos;
            r->total_chapters++;
        }
    }

    if (r->total_chapters == 0)
    {
        f_lseek(r->fp, 0);
        strncpy(r->all_chapters[0].title, "正文", NOVEL_MAX_TITLE - 1);
        r->all_chapters[0].start_offset = 0;
        r->total_chapters = 1;
    }

    vfs_free(buf);
}

// --- SD 卡索引持久化逻辑 ---
static uint8_t novel_try_load_sd_index(NovelReader* r) 
{
    char idx_full_path[256];
    uint32_t br;
    FIL f_idx;
    NovelIndexHeader head;

    // 1. 构建路径并打开文件
    sprintf(idx_full_path, "%s/%s.idx", r->dir_path, r->file_name);
    if (f_open(&f_idx, idx_full_path, FA_READ) != FR_OK) 
	{
		printf("[error] 打开文件失败\r\n");
		return 1;
	}
    // 2. 读取并校验 Header
    if (f_read(&f_idx, &head, sizeof(head), &br) != FR_OK || br != sizeof(head)) {
        f_close(&f_idx);
		printf("[error] 校验失败1\r\n");
        return 1;
    }

    if (head.magic != 0x50495830 || head.novel_size != f_size(r->fp)) {
        f_close(&f_idx);
		printf("[error] 校验失败2\r\n");
        return 1; 
    }

    r->total_chapters = head.total_chapters;

    /* --- 核心修改：分块读取 (每块 20 个章节) --- */
    uint32_t chapters_read = 0;               // 已读取的章节计数
    const uint32_t CHUNK_SIZE = 20;           // 步进长度

    while (chapters_read < r->total_chapters) 
    {
        // 计算本次实际需要读取的数量（防止最后一次越界）
        uint32_t to_read = r->total_chapters - chapters_read;
        if (to_read > CHUNK_SIZE) to_read = CHUNK_SIZE;

        // 计算存入内存的目标地址偏移
        NovelChapterIdx* dest_ptr = &r->all_chapters[chapters_read];

        // 执行读取
        if (f_read(&f_idx, dest_ptr, sizeof(NovelChapterIdx) * to_read, &br) != FR_OK) {
            f_close(&f_idx);
			printf("[error] 读取失败\r\n");
            return 2; // 读取异常退出
        }

        chapters_read += to_read;

        // 【可选】如果你在带操作系统的环境下，可以在这里交出 CPU
        sys_delay_ms(1); 
    }
    /* ------------------------------------------ */

    f_close(&f_idx);
    return 0;
}

static void novel_save_index_to_sd(NovelReader* r) 
{
    char idx_full_path[256];
    uint32_t bw;
    FIL f_idx;
    
    sprintf(idx_full_path, "%s/%s.idx", r->dir_path, r->file_name);

    if (f_open(&f_idx, idx_full_path, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) 
    {
        NovelIndexHeader head;
        head.magic = 0x50495830;
        head.novel_size = f_size(r->fp);
        head.total_chapters = r->total_chapters;

        f_write(&f_idx, &head, sizeof(head), &bw);
        f_write(&f_idx, r->all_chapters, sizeof(NovelChapterIdx) * r->total_chapters, &bw);
        
        f_close(&f_idx);
    }
}

// --- 自动分页与页面计算 ---
void novel_set_display_params(NovelReader* r, int lines, int width)
{
    r->active_chap.page_lines = (lines < 5) ? 5 : lines;
    r->active_chap.line_max_chars = (width < 10) ? 10 : width;
    
    if (r->fp && r->total_chapters > 0)
    {
        novel_paginate(r, r->active_chap.chapter_idx);
        novel_load_page_data(r, r->active_chap.cur_page_idx);
    }
}


void novel_paginate(NovelReader* r, int idx)
{
    if (idx < 0 || idx >= r->total_chapters) return;

    char* page_buf = (char*)vfs_malloc(2048);
    if (!page_buf) return;

    int cur_lines = r->active_chap.page_lines;
    int cur_width = r->active_chap.line_max_chars;

    novel_free_active_chapter(r);
    r->active_chap.chapter_idx = idx;
    strncpy(r->active_chap.title, r->all_chapters[idx].title, NOVEL_MAX_TITLE - 1);

    long start_pos = r->all_chapters[idx].start_offset;
    long end_limit = (idx + 1 < r->total_chapters) ? r->all_chapters[idx + 1].start_offset : -1;

    int capacity = 32; 
    int p_count = 0;
    long* offsets = (long*)vfs_malloc(sizeof(long) * capacity);
    int* lengths = (int*)vfs_malloc(sizeof(int) * capacity);
    
    f_lseek(r->fp, start_pos);
	
	int lines_processed = 0;


    while (1)
    {
        if (p_count >= capacity) 
        {
            int new_cap = capacity * 2;
            long* n_off = (long*)vfs_malloc(sizeof(long) * new_cap);
            int* n_len = (int*)vfs_malloc(sizeof(int) * new_cap);
            if (n_off && n_len) {
                memcpy(n_off, offsets, sizeof(long) * capacity);
                memcpy(n_len, lengths, sizeof(int) * capacity);
                vfs_free(offsets); vfs_free(lengths);
                offsets = n_off; lengths = n_len;
                capacity = new_cap;
            } else break;
        }

        offsets[p_count] = f_tell(r->fp);
        int lines_acc = 0;

        while (lines_acc < cur_lines)
        {
            long pre_line_pos = f_tell(r->fp);
            if (!f_gets(page_buf, 2048, r->fp)) break;

            // 1. 章节检查：如果是新章节开头，本页立即结束
            if (lines_acc > 0 && novel_internal_is_chapter(page_buf)) {
                f_lseek(r->fp, pre_line_pos);
                break;
            }

            // 2. 计算这一行（段）折行后的物理行数
            int phys = novel_internal_count_lines(page_buf, cur_width);

            // 3. 核心修改：如果这一段太长，尝试拆分它
            if (lines_acc + phys > cur_lines) {
                // 计算当前页还能放几行
                int can_put_lines = cur_lines - lines_acc;
                
                // 只有当剩余空间能放下至少一行，且该段确实很长时才拆分
                // 否则像以前一样整体挪到下一页
                if (can_put_lines > 0) {
                    // 找到在 can_put_lines 行处对应的文件偏移量
                    long split_pos = novel_internal_find_split_pos(page_buf, cur_width, can_put_lines);
                    f_lseek(r->fp, pre_line_pos + split_pos);
                    lines_acc += can_put_lines;
                } else {
                    f_lseek(r->fp, pre_line_pos);
                }
                break; // 当前页满了
            }

            lines_acc += phys;
            if (end_limit != -1 && f_tell(r->fp) >= end_limit) break;
        }

        lengths[p_count] = (int)(f_tell(r->fp) - offsets[p_count]);
        p_count++;

        if (f_eof(r->fp) || (end_limit != -1 && f_tell(r->fp) >= end_limit)) break;
		
		lines_processed++;
		if(lines_processed % 200 == 0) {
        sys_delay_ms(1); // 释放 CPU，让 LVGL 任务有机会刷新屏幕
		}
    }

    r->active_chap.total_pages = p_count;
    r->active_chap.page_offsets = offsets;
    r->active_chap.page_lengths = lengths;
    r->active_chap.cur_page_idx = 0;

    vfs_free(page_buf);
}

// --- 页面内容载入 ---
void novel_load_page_data(NovelReader* r, int p_idx)
{
    uint32_t br;
    if (p_idx < 0 || p_idx >= r->active_chap.total_pages) return;

    if (r->active_chap.cur_page_content) vfs_free(r->active_chap.cur_page_content);

    int len = r->active_chap.page_lengths[p_idx];
    r->active_chap.cur_page_content = (char*)vfs_malloc(len + 1);
    if (r->active_chap.cur_page_content)
    {
        f_lseek(r->fp, r->active_chap.page_offsets[p_idx]);
        f_read(r->fp, r->active_chap.cur_page_content, len, &br);
        r->active_chap.cur_page_content[len] = '\0';
    }
}

// --- 内部逻辑辅助函数 ---

static void novel_free_active_chapter(NovelReader* r)
{
    if (r->active_chap.cur_page_content) { vfs_free(r->active_chap.cur_page_content); r->active_chap.cur_page_content = NULL; }
    if (r->active_chap.page_offsets) { vfs_free(r->active_chap.page_offsets); r->active_chap.page_offsets = NULL; }
    if (r->active_chap.page_lengths) { vfs_free(r->active_chap.page_lengths); r->active_chap.page_lengths = NULL; }
}

static int novel_internal_is_chapter(const char* line)
{
    const unsigned char* ptr = (const unsigned char*)line;
    while (*ptr && *ptr <= 32) ptr++;
    if (*ptr == '\0' || strlen((const char*)ptr) > 60) return 0;

    const char* p_str = (const char*)ptr;
    if (strstr(p_str, GBK_DI) == p_str || strstr(p_str, UTF8_DI) == p_str)
    {
        if (strstr(p_str, GBK_ZHANG) || strstr(p_str, UTF8_ZHANG) || 
            strstr(p_str, GBK_JIE)   || strstr(p_str, UTF8_JIE))
            return 1;
    }
    return 0;
}

static int novel_internal_count_lines(const char* str, int max_w)
{
    int visual_w = 0, lines = 0;
    const unsigned char* p = (const unsigned char*)str;
    if (!*p) return 0;

    while (*p)
    {
        if (*p == '\r' || *p == '\n')
        {
            lines++; visual_w = 0;
            if (*p == '\r' && *(p + 1) == '\n') p++;
        }
        else
        {
            int step = (*p >= 0xE0) ? 3 : (*p >= 0x80 ? 2 : 1);
            p += (step - 1);
            visual_w++;
            if (visual_w >= max_w) { lines++; visual_w = 0; }
        }
        p++;
    }
    if (visual_w > 0) lines++;
    return lines;
}

/**
 * @brief 在一段文字中找到第 target_lines 行结束时的字节偏移量
 */
static long novel_internal_find_split_pos(const char* str, int max_w, int target_lines)
{
    int visual_w = 0, current_lines = 0;
    const unsigned char* p = (const unsigned char*)str;
    const unsigned char* start = p;

    while (*p)
    {
        if (*p == '\r' || *p == '\n')
        {
            current_lines++;
            if (current_lines >= target_lines) return (long)(p - start + 1);
            visual_w = 0;
            if (*p == '\r' && *(p + 1) == '\n') p++;
        }
        else
        {
            int step = (*p >= 0xE0) ? 3 : (*p >= 0x80 ? 2 : 1);
            visual_w++;
            if (visual_w >= max_w) {
                current_lines++;
                visual_w = 0;
                // 如果刚好达到目标行数，返回当前字符结束后的偏移
                if (current_lines >= target_lines) return (long)(p - start + step);
            }
            p += step;
        }
    }
    return (long)(p - start);
}

