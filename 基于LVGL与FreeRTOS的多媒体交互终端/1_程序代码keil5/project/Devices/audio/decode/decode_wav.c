#include "decode.h"
#include "../audio_dma.h"
#include "../audio_sai.h"
#include "../audio_es8388.h"
#include "../../middle/audio/wav/wav.h"
#include "../../middle/malloc/malloc.h"
#include "../../system/debug/debug.h"
#include "../../middle/fatfs/ff.h"
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

#if USE_AUDIO_DECODE_WAV

// 定义你想截掉的时间（秒）
#define WAV_TRUNCATE_END_SEC  1

static uint32_t wav_buffill(__audiodev *audiodev, uint8_t index, uint16_t size);
static uint32_t wav_file_seek(__audiodev *audiodev, uint32_t pos_t);
static void wav_updata_curtime(__audiodev *audiodev);

static AudioDec_Status wav_decode_malloc(__audiodev *audiodev);
static AudioDec_Status wav_decode_free(__audiodev *audiodev);
static void wav_decode_set(__audiodev *audiodev);


/**
 * @brief WAV 播放核心引擎
 * @param audiodev 音频设备结构体指针
 * @param fname    文件名
 * @param control_cb 控制回调函数指针 (void 参数)
 */
AudioDec_Status wav_play_song(__audiodev *audiodev) 
{
    uint8_t res;  
	AudioDec_Status Ret = AudioDec_OK;
    uint32_t fillnum; 
    __wavctrl* wavctrl;
	int index;
	uint32_t stop_pos;
	
	audiodev->file_seek = wav_file_seek;
	
	// 内存分配
	if(wav_decode_malloc(audiodev)==AudioDec_MemErr)
    {
		Ret = AudioDec_MemErr;
		goto ___exit;
	}
	
	// 得到文件的信息
	wavctrl = (__wavctrl*)audiodev->infoctrl;
	res = wav_init_decode_info(audiodev->fname, wavctrl);
	if(res != 0)
	{
		Ret = AudioDec_InfoErr;
		goto ___exit;
	}
	
	wav_decode_set(audiodev);
	
	/*!< 解码开始 >!*/
	res = f_open(audiodev->file, (TCHAR*)audiodev->fname, FA_READ); 
	if (res != FR_OK)
	{
		Ret = AudioDec_FileErr;
		goto ___exit;
	}
	
	f_lseek(audiodev->file, wavctrl->datastart);     
	fillnum = wav_buffill(audiodev, 1, WAV_SAI_TX_DMA_BUFSIZE);
	fillnum = wav_buffill(audiodev, 2, WAV_SAI_TX_DMA_BUFSIZE);
	wav_updata_curtime(audiodev);
	
	audio_set_txdma_callback(audiodev->txdma_callback);
	audio_tx_dma_start();
	
	/*!< 计算提前结束的边界地址 >!*/
    // 计算公式：数据起始位置 + 总数据大小 - (每秒需要的字节数 * 截掉的秒数)
    stop_pos = wavctrl->datastart + wavctrl->datasize - (wavctrl->bitrate / 8 * WAV_TRUNCATE_END_SEC);
	
	while (1)
	{ 
		/* 1. 检查当前文件指针是否已经达到或超过了截断点 */
        if (audiodev->file->fptr >= stop_pos)
        {
            Ret = AudioDec_OK;
            break; 
        }
		
		/*---------------------------------状态切换处理-------------------------------------------*/
		AudioDec_Status status = audiodev->control_callback(audiodev);
		
		// DMA 切换
		if (status == exAudioDec_DMA_SYNC_1 || status == exAudioDec_DMA_SYNC_2)
		{
			index = (status == exAudioDec_DMA_SYNC_1) ? 1 : 2;
			fillnum = wav_buffill(audiodev, index, WAV_SAI_TX_DMA_BUFSIZE);
		}
		
		// 退出切歌
		else if(status == exAudioDec_EXIT)
		{
			Ret = status;
			goto ___exit;
		}
	
		/*---------------------------------状态切换处理-------------------------------------------*/
		
		wav_updata_curtime(audiodev);
		
		
		if ((fillnum != WAV_SAI_TX_DMA_BUFSIZE)) 
		{  
			Ret = AudioDec_OK;
			break;
		}
	}
	
___exit:
	printf("wav_play_song status %x\n", Ret);
    audio_tx_dma_close();
	f_close(audiodev->file);
	wav_decode_free(audiodev);
    return Ret;
}

static AudioDec_Status wav_decode_malloc(__audiodev *audiodev)
{
    audiodev->file = (FIL*)mymalloc(SRAMIN, sizeof(FIL));
    audiodev->saibuf1 = mymalloc(SRAMIN, WAV_SAI_TX_DMA_BUFSIZE);
    audiodev->saibuf2 = mymalloc(SRAMIN, WAV_SAI_TX_DMA_BUFSIZE);
    audiodev->tbuf = mymalloc(SRAMIN, WAV_SAI_TX_DMA_BUFSIZE);
	audiodev->infoctrl = (__wavctrl*)mymalloc(SRAMIN, sizeof(__wavctrl));
	audiodev->use_data = mymalloc(SRAMIN, 64);
    if (audiodev->file == NULL || audiodev->saibuf1 == NULL  || audiodev->saibuf2 == NULL|| 
		audiodev->tbuf == NULL || audiodev->infoctrl == NULL || audiodev->use_data == NULL)
	{
		return AudioDec_FileErr;
	}
	return AudioDec_NONE;
	
}
static AudioDec_Status wav_decode_free(__audiodev *audiodev)
{
	myfree(SRAMIN,audiodev->tbuf);    
    myfree(SRAMIN,audiodev->saibuf1); 
    myfree(SRAMIN,audiodev->saibuf2);  
    myfree(SRAMIN,audiodev->file);    
	myfree(SRAMIN,audiodev->infoctrl);    
	
	return AudioDec_NONE;
}


static void wav_decode_set(__audiodev *audiodev)
{
	__wavctrl* wavctrl = (__wavctrl*)audiodev->infoctrl;

	if (wavctrl->bps == 16)
	{
		es8388_sai_cfg(0, 3);      							
		audio_saia_init(0, 1, 4);  							
		audio_tx_dma_init((uint32_t)audiodev->saibuf1, (uint32_t)audiodev->saibuf2, WAV_SAI_TX_DMA_BUFSIZE / 2, 1);  
	}
	else if (wavctrl->bps == 24)
	{
		es8388_sai_cfg(0, 0);    								
		audio_saia_init(0, 1, 6); 								
		audio_tx_dma_init((uint32_t)audiodev->saibuf1, (uint32_t)audiodev->saibuf2, WAV_SAI_TX_DMA_BUFSIZE / 4, 2); 
	}
	audio_sai_samplerate(wavctrl->samplerate);  		
	
}







/**
 * @brief       填充buf
 * @param       buf  : 填充区
 * @param       size : 填充数据量
 * @param       bits : 位数(16/24)
 * @retval      读取到的数据长度
 */
static uint32_t wav_buffill(__audiodev *audiodev, uint8_t index, uint16_t size)
{
    uint16_t readlen = 0;
    uint32_t bread;
    uint16_t i;
    uint32_t *p, *pbuf;
	uint32_t bits = ((__wavctrl *)(audiodev->infoctrl))->bps;
	
	uint8_t *tbuf = audiodev->tbuf;
	uint8_t *saibuf = (index == 1 ? audiodev->saibuf1 : audiodev->saibuf2);
	
	// 24bit音频,需要处理一下
    if (bits == 24)
    {
        readlen = (size / 4) * 3;   							// 此次要读取的字节数
        f_read(audiodev->file, tbuf, readlen, (UINT *)&bread); 	// 读取数据

        pbuf = (uint32_t *)saibuf;
        for (i = 0; i < size / 4; i++)
        {
            p = (uint32_t *)(tbuf + i * 3);
            pbuf[i] = p[0];
        }

        bread = (bread * 4) / 3;   // 填充后的大小
    }
    else // 16bit音频,直接读取数据
    {
        f_read(audiodev->file, saibuf, size, (UINT*)&bread);     
		
        if (bread < size)// 不够数据了,补充0
        {
            for (i = bread; i < size; i++){saibuf[i] = 0;}
        }
    }
    
    return bread;
}

/**
 * @brief       获取当前播放时间
 * @param       fname : 文件指针
 * @param       wavx  : wavx播放控制器
 * @retval      无
 */
static void wav_updata_curtime(__audiodev *audiodev)
{
    long long fpos;
	FIL *fx = audiodev->file;
	__wavctrl *wavx= audiodev->infoctrl;

    wavx->totsec = wavx->datasize / (wavx->bitrate / 8);    // 歌曲总长度(单位:秒)
    fpos = fx->fptr-wavx->datastart;                        // 得到当前文件播放到的地方
    wavx->cursec = fpos*wavx->totsec / wavx->datasize;      // 当前播放到第多少秒了
	
	
	audiodev->totsec = wavx->totsec;     
	audiodev->cursec = wavx->cursec;
}


/**
 * @brief       wav文件快进快退函数
 * @param       pos:需要定位的时间
 * @retval      返回值:当前文件位置(即定位后的结果)
 */
static uint32_t wav_file_seek(__audiodev *audiodev, uint32_t pos_t)
{
    uint8_t temp;
    
	uint32_t pos = audiodev->file->obj.objsize / audiodev->totsec * pos_t;
	__wavctrl * wavctrl = (__wavctrl *)(audiodev->infoctrl);
	
    if (pos > audiodev->file->obj.objsize)
    {
        pos = audiodev->file->obj.objsize;
    }
    
    if (pos < wavctrl->datastart) 
		pos = wavctrl->datastart;
    if (wavctrl->bps == 16) temp = 8;    /* 必须是8的倍数 */
    if (wavctrl->bps == 24) temp = 12;   /* 必须是12的倍数 */
    if ((pos - wavctrl->datastart) % temp)
    {
        pos += temp - (pos - wavctrl->datastart) % temp;
    }
    
    f_lseek(audiodev->file,pos);
    
    return audiodev->file->fptr;
}



#endif

