#ifndef __MUSIC_SERVICE_H
#define __MUSIC_SERVICE_H

#include "../../middle/linklist/linklist.h"
#include "../../middle/list/musiclist.h"
#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"


typedef enum {
    MUSIC_STATUS_STOPPED = 0,
    MUSIC_STATUS_PLAYING,
   // MUSIC_STATUS_PAUSED
} music_status_t;

typedef enum {
    MUSIC_MODE_LIST_LOOP = 0,
    MUSIC_MODE_RANDOM,
    MUSIC_MODE_SINGLE_LOOP
} music_mode_t;



// --- 1. 指令码定义 --- 
typedef enum 
{
    MSG_DMA_SYNC1      = (1 << 2) | 1, // DMA 同步信号 
	MSG_DMA_SYNC2      = (2 << 2) | 1, // DMA 同步信号 
	
    AUDIO_CMD_PLAY    = (1 << 2) | 2, // 播放
    AUDIO_CMD_STOP    = (2 << 2) | 2, // 暂停，挂起
	
    AUDIO_CMD_PAUSE   = (3 << 2) | 2, // 停止，退出
    AUDIO_CMD_RESUME  = (4 << 2) | 2, // 恢复	
	
    AUDIO_CMD_NEXT    = (5 << 2) | 2, // 下一首
    AUDIO_CMD_PREV    = (6 << 2) | 2, // 上一首
    AUDIO_CMD_SEEK    = (7 << 2) | 2, // 跳转 (Data 域存百分比)
    AUDIO_CMD_SELECT  = (8 << 2) | 2  // 选中播放 (Data 域存节点指针)
} AudioCmd_t;

// 字节消息结构体
typedef struct 
{
    uint32_t cmd;  // 存放 AudioCmd_t 或 DMA 状态
    uint32_t data; // 存放指针、百分比或偏移量
} AudioMsg_t;


typedef struct 
{
	//--- 音乐列表 ---
	music_list_t g_music_list;
	
	// --- 播放状态 ---
    music_mode_t mode;
    volatile music_status_t status; 
    uint8_t current_percent;

	// --- 内部消息队列句柄 ---
    QueueHandle_t msg_queue;
	
    // --- 公开 API ---
    void (*init)(void);
    void (*play)(void);
    void (*stop)(void);
    void (*prev)(void);
    void (*next)(void);
    void (*select)(void *info);
    void (*set_seek)(uint8_t percent);
    void (*cycle_mode)(void); // 切换模式：循环->随机->单曲
    uint8_t (*get_percent)(void);
} music_control_t;

extern music_control_t MusicControl;

#endif

