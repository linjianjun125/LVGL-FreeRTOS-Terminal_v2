#include "freertos_task.h"

#include "devices_include.h"
#include "middle_include.h"
#include "app_include.h"
#include "ui_include.h"


static music_info_t * _music_auto_next(uint8_t type) ;


/**
 * @brief 音乐播放控制回调 (不带参数版本)
 * @return AudioDec_Status 转换后的分类状态
 */
static AudioDec_Status _audio_control_handler(__audiodev *audiodev) 
{
    AudioMsg_t msg; 
	sensor_music_data_t sensor_music = {.status = 2, .music_info = NULL, .current_percent = 255};
	static uint8_t last_pushed_percent = 255; // 初始设为 255 确保第一次运行能通过校验
	
	
    // 阻塞接收队列消息
    if (xQueueReceive(MusicControl.msg_queue, &msg, portMAX_DELAY) != pdTRUE) 
	{
        return AudioDec_NONE; 
    }
	
	
	
    /* 消息处理 */
    AudioDec_Status converted = AudioDec_NONE;
    switch ((AudioCmd_t)msg.cmd) 
	{
        case MSG_DMA_SYNC1:     converted = exAudioDec_DMA_SYNC_1;     	break;
		case MSG_DMA_SYNC2:     converted = exAudioDec_DMA_SYNC_2;     	break;
		// 下一首
        case AUDIO_CMD_NEXT:    
		{
			converted = exAudioDec_EXIT;
			_music_auto_next(0);
			break;
		}
		// 上一首
        case AUDIO_CMD_PREV:   
		{			
			converted = exAudioDec_EXIT;
			_music_auto_next(1);
			break;
		}
		// 选择播放
		case AUDIO_CMD_SELECT:  
		{
			converted = exAudioDec_EXIT;
			music_list_set_curr(&MusicControl.g_music_list, (music_info_t*)msg.data);
			break;
		}
		// 暂停恢复
		case AUDIO_CMD_PLAY:    
		{
			audio_tx_dma_start();		
			
			sensor_music.status = 1;
			sensor_music.current_percent = (audiodev->cursec * 100 / audiodev->totsec);
			SensorHub.push(SENSOR_MUSIC_STATUS, &sensor_music);
			
			break;
		}
		// 暂停挂起
        case AUDIO_CMD_STOP:    
		{
			audio_tx_dma_close();
			
			sensor_music.status = 2;
			sensor_music.current_percent = (audiodev->cursec * 100 / audiodev->totsec);
			SensorHub.push(SENSOR_MUSIC_STATUS, &sensor_music);
			
			vTaskSuspend(NULL);		
			break;
		}
		// 切换播放时间
        case AUDIO_CMD_SEEK:    
		{
			if (audiodev->file_seek) 
			{
				uint8_t percent = (uint8_t)msg.data;
				uint32_t target = percent * audiodev->totsec / 100;
				audiodev->file_seek(audiodev, target);
				audiodev->cursec = target;
			}
			break;
        }
    }
	


	uint8_t current_percent = (audiodev->cursec * 100 / audiodev->totsec);

	// 只有当百分比发生变化，且当前状态为播放（status = 1）时才执行
	if (current_percent != last_pushed_percent) 
	{
		sensor_music.status = 1;
		sensor_music.current_percent = current_percent;
		
		// 调用推送到 SensorHub
		if (SensorHub.push(SENSOR_MUSIC_STATUS, &sensor_music)) 
		{
			// 只有推送成功后，才更新记录的百分比
			last_pushed_percent = current_percent;
		}
	}
	
	
    return converted; // 返回转换后的分类状态
}


/**
 * @brief  DMA传输回调发送回调函数
 */
void flac_i2s_dma_tx_callback(int index)
{
    if (index == 1)
    {
		AudioMsg_t msg = { .cmd = MSG_DMA_SYNC1};
		xQueueSendFromISR(MusicControl.msg_queue, &msg, NULL);
    }
    else 
    {
		AudioMsg_t msg = { .cmd = MSG_DMA_SYNC2};
		xQueueSendFromISR(MusicControl.msg_queue, &msg, NULL);
    }
}





void vMusicTask(void *pvParameters) 
{
    AudioMsg_t msg;
    AudioDec_Status status = AudioDec_NONE;
    bool auto_start = false; // 标记是否直接进入下一首播放
	__audiodev audiodev;
	music_info_t *next_music; 
	sensor_music_data_t sensor_music = {.status = 2, .music_info = NULL, .current_percent = 255};
	SensorHub.push(SENSOR_MUSIC_STATUS, &sensor_music);
	
    while(1) 
	{
        // --- 逻辑 A：等待启动信号 ---
        // 如果不是自动连播（例如刚开机或刚点过停止），则阻塞等待
        if (!auto_start) 
		{
            if (xQueueReceive(MusicControl.msg_queue, &msg, portMAX_DELAY) == pdTRUE) 
			{
                // 只有收到 PLAY 指令才打破阻塞开始工作
                if (msg.cmd == AUDIO_CMD_PLAY) {auto_start = true; } 
				else {continue;} // 忽略其他等无效信号
            }
        }

        // --- 逻辑 B：执行播放流程 ---
        if (auto_start) 
		{
			// 进入解码引擎（此处会阻塞直到播完或收到新指令）
			next_music = music_list_get_curr(&MusicControl.g_music_list);
			
			// 给 UI 发送一个消息，告诉它现在在播哪一首
			sensor_music.music_info = next_music;
			sensor_music.status = 1;
			SensorHub.push(SENSOR_MUSIC_INFO, &sensor_music);
			SensorHub.push(SENSOR_MUSIC_STATUS, &sensor_music);
			
			if(next_music != NULL)
			{
				snprintf(audiodev.fname, 128, "%s/%s", next_music->dir, next_music->name);
				audiodev.ftype = next_music->type;
				audiodev.control_callback =  _audio_control_handler;
				audiodev.txdma_callback = flac_i2s_dma_tx_callback;
		
				status = audio_decode(&audiodev);

				// 切歌退出
				if (status == exAudioDec_EXIT) 
				{
					// 保持 auto_start 为 true，这样循环回到顶部时不会被 portMAX_DELAY 阻塞
					auto_start = true; 
				} 
				
				// 播放结束或者解码失败
				else if (status & 0x01) 
				{
					// 出错处理：跳过这首歌，试下一首
					_music_auto_next(0);
					auto_start = true; 
					vTaskDelay(pdMS_TO_TICKS(500)); // 避免文件全错时死循环刷屏
				}
				
				// 清理队列，防止上首歌残留的 DMA 信号影响下首歌
				xQueueReset(MusicControl.msg_queue);
				
			}
			else
			{
				vTaskDelay(pdMS_TO_TICKS(500));
			}
        }
    }
}




/**
 * @param type  0: 下一首方向随机, 1: 上一首方向随机
 */
static music_info_t * _music_auto_next(uint8_t type) 
{
    music_info_t *music = NULL;
    uint8_t total = music_list_get_count(&MusicControl.g_music_list);
    
    if (total == 0) return NULL;

    if (MusicControl.mode == MUSIC_MODE_RANDOM) 
    {
        // 随机步进：跳过 1 ~ (total-1) 个节点
        uint8_t step = (uint8_t)sys_random_number_range(1, total - 1);
        
        while(step--) 
        {
            if (type == 1) {
                // 传入1，随机获取“上一首”方向的节点
                music = music_list_get_prev(&MusicControl.g_music_list);
            } else {
                // 传入0（或其它），随机获取“下一首”方向的节点
                music = music_list_get_next(&MusicControl.g_music_list);
            }
        }
    } 
    else if (MusicControl.mode == MUSIC_MODE_LIST_LOOP)
    {
        // 顺序切换到下一个
        music = music_list_get_next(&MusicControl.g_music_list);
    }
    // MUSIC_MODE_ONE (单曲循环) 不移动指针
    
    return music;
}
