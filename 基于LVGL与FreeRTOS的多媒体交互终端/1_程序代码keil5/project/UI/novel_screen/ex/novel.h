#ifndef __NOVEL_H
#define __NOVEL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "../../middle/fatfs/ff.h"


#define NOVEL_MAX_CHAPTERS     	2000            
#define NOVEL_MAX_TITLE     	64       



// 特征码定义 (支持 GBK & UTF-8)
#define GBK_DI    "\xb5\xda"
#define GBK_ZHANG "\xd5\xc2"
#define GBK_JIE   "\xbd\xda"
#define GBK_HUI   "\xbb\xd8"
#define GBK_JUAN  "\xbe\xed"
#define GBK_JI    "\xbc\xaf"

#define UTF8_DI    "\xe7\xac\xac"
#define UTF8_ZHANG "\xe7\xab\xa0"
#define UTF8_JIE   "\xe8\x8a\x82"
#define UTF8_HUI   "\xe5\x9b\x9e"
#define UTF8_JUAN  "\xe5\x8d\xb7"
#define UTF8_JI    "\xe9\x9b\x86"

typedef FIL VFS_FILE;

typedef struct
{
    long start_offset;           // 章节在文件中的字节起点
    char title[NOVEL_MAX_TITLE]; // 章节标题
} NovelChapterIdx;

typedef struct
{
    int chapter_idx;             // 当前章节索引
    char title[NOVEL_MAX_TITLE]; // 指向当前章节标题
    long* page_offsets;          // 动态数组：每一页的字节起点
    int* page_lengths;           // 动态数组：每一页的字节长度
    int total_pages;             // 当前章总页数
    int cur_page_idx;            // 当前页码
    char* cur_page_content;      // 存储当前页文本的缓冲区

    int page_lines;              // 每页行数
    int line_max_chars;          // 每行字符宽度限制
} NovelContent;


typedef struct
{
    FIL* fp;                    // FatFs 文件指针
    char* dir_path;             // 小说所在目录路径
    char* file_name;            // 小说文件名
    int total_chapters;         // 总章节数
    NovelChapterIdx* all_chapters; // 章节索引表
    NovelContent active_chap;   // 当前活动章节及分页信息
} NovelReader;


// 索引文件头，用于版本校验
typedef struct
{
    uint32_t magic;          // 固定为 0x50495830 (ASCII 'IDX0')
    uint32_t novel_size;     // 原文件大小，如果文件变了，索引失效
    uint16_t total_chapters; // 章节总数
    uint16_t reserved;       // 保留对齐
} NovelIndexHeader;



uint8_t novel_init(NovelReader* reader, char* path, char* fname);
void novel_deinit(NovelReader* r);
void novel_set_display_params(NovelReader* r, int lines, int width);
void novel_build_index(NovelReader* r);
void novel_paginate(NovelReader* r, int idx);
void novel_load_page_data(NovelReader* r, int p_idx);

#endif

