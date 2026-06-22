#include "music_service.h"

#include "devices_include.h"
#include "middle_include.h"
#include "app_include.h"
#include "ui_include.h"



// =================================================================
// API 层实现：即时修改变量并唤醒任务
// =================================================================
static void _init(void) 
{
    FRESULT res;
    FILINFO fno;
    char full_path[256];
    uint8_t is_modified = 0;

    /* 1. 初始化列表结构 */
    music_list_init(&MusicControl.g_music_list, "SD_Music");

    // --- 步骤 A: 从 EEPROM 获取上次保存的歌曲列表 ---
    // 这个函数内部会恢复 dir_pool 和所有的 music_info_t 节点
    if (at24cxx_load_music_list(&MusicControl.g_music_list) == 0) {
        printf("--- Music list loaded from EEPROM ---\n");
		PRINT_MUSIC_LIST(&MusicControl.g_music_list);
    } else {
        printf("--- No EEPROM cache found, full scan required ---\n");
        is_modified = 1;
    }

    // --- 步骤 B: 校验文件物理完整性 (剔除 SD 卡中已删除的文件) ---
    if (MusicControl.g_music_list.info_count > 0) 
    {
        printf("Validating music files on SD Card...\n");
        list_node_t *pos, *n;
        // 使用 SAFE 宏，因为循环中可能涉及删除操作
        LIST_FOR_EACH_NODE_SAFE(pos, n, &MusicControl.g_music_list.list) 
        {
            music_info_t *info = LIST_NODE_GET_OBJ(pos, &MusicControl.g_music_list.list);
            
            // 拼接完整路径进行校验
            sprintf(full_path, "%s/%s", info->dir, info->name);
            res = f_stat(full_path, &fno);

            // 如果文件不存在、或者是目录、或者文件大小变成了0
            if (res != FR_OK || (fno.fattrib & AM_DIR)) 
            {
                printf("[Warning] File Invalid, Removing: %s\n", info->name);
                music_list_remove(&MusicControl.g_music_list, info); // 使用之前写的移除函数
                is_modified = 1;
            }
        }
    }

    // --- 步骤 C: 增量扫描 (补全或寻找新歌) ---
    // 如果列表为空，或者你想每次开机都看看有没有新歌
    uint32_t mask = MUSIC_TYPE_ALL; 
    
    printf("Scanning for new music...\n");
    // music_list_scan_fatfs 内部需要有查重逻辑（即 music_list_add 里的 strcmp）
    int new_count = music_list_scan_fatfs(&MusicControl.g_music_list, "0:", mask);
    
    if (new_count > 0) {
        printf("Found %d new songs!\n", new_count);
        is_modified = 1;
    }

    // --- 步骤 D: 如果列表有变动，同步到 EEPROM ---
    if (is_modified) {
        at24cxx_save_music_list(&MusicControl.g_music_list);
    }

    /* 打印最终结果 */
    printf("Music Manager Ready. Total: %d songs\n", MusicControl.g_music_list.info_count);
    PRINT_MUSIC_LIST(&MusicControl.g_music_list);

    /* 后续常规初始化 */
    MusicControl.status = MUSIC_STATUS_STOPPED;
    MusicControl.mode = MUSIC_MODE_LIST_LOOP;
    MusicControl.msg_queue = xQueueCreate(16, sizeof(AudioMsg_t));

    if (MusicControl.msg_queue == NULL) {
        printf("Error: Music Message Queue create failed!\r\n");
        return;
    }
}


/**
 * @brief 内部指令发送宏
 */
#define SEND_DEC_MSG(c, d) do { \
    AudioMsg_t _m = { .cmd = (c), .data = (d) }; \
    if(MusicControl.msg_queue) xQueueSend(MusicControl.msg_queue, &_m, 0); \
} while(0)

/**
 * @brief 播放按钮逻辑
 */
static void _play(void) 
{
    // 无论当前是停止还是暂停，UI 统一表现为“播放中”
    MusicControl.status = MUSIC_STATUS_PLAYING;
	vTaskResume(xMusicTaskHandle);
    SEND_DEC_MSG(AUDIO_CMD_PLAY, 0);
}

/**
 * @brief 停止按钮逻辑
 */
static void _stop(void) 
{
    MusicControl.status = MUSIC_STATUS_STOPPED;
    SEND_DEC_MSG(AUDIO_CMD_STOP, 0);
}

/**
 * @brief 切下一首
 * @note 切歌瞬间 UI 通常仍保持“播放中”状态
 */
static void _next(void) 
{
    MusicControl.status = MUSIC_STATUS_PLAYING; 
    SEND_DEC_MSG(AUDIO_CMD_NEXT, 0);
}

/**
 * @brief 切上一首
 */
static void _prev(void) 
{
    MusicControl.status = MUSIC_STATUS_PLAYING;
    SEND_DEC_MSG(AUDIO_CMD_PREV, 0);
}

/**
 * @brief 选中特定歌曲播放
 */
static void _select(void *info) 
{
    if (info == NULL) return;
	_play();
	
    SEND_DEC_MSG(AUDIO_CMD_SELECT, (uint32_t)info);
}

/**
 * @brief 设置进度
 */
static void _set_seek(uint8_t percent) 
{
    // 跳转进度不改变播放/暂停状态，仅发送数据
    SEND_DEC_MSG(AUDIO_CMD_SEEK, (uint32_t)percent);
}

/**
 * @brief 循环模式切换
 * @details 列表循环 -> 随机播放 -> 单曲循环。
 * 仅修改内存状态，不发消息。解码任务在当前歌曲播放完后会读取此 mode 决定跳转目标。
 */
static void _cycle_mode(void) {
    MusicControl.mode = (music_mode_t)((MusicControl.mode + 1) % 3);
}


static uint8_t _get_percent(void) 
{
    return MusicControl.current_percent;
}



// 导出单例
music_control_t MusicControl = 
{
    .init = _init,
    .play = _play,
    .stop = _stop,
    .prev = _prev,
    .next = _next,
    .select = _select,
    .set_seek = _set_seek,
    .cycle_mode = _cycle_mode,
    .get_percent = _get_percent,
};

