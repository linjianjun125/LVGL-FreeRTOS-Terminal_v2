#include "decode.h"
#include "../audio_dma.h"
#include "../audio_sai.h"
#include "../audio_es8388.h"
#include "../../middle/audio/mp3/mp3.h"
#include "../../middle/malloc/malloc.h"
#include "../../system/debug/debug.h"
#include "../../system/delay/delay.h"
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

#if USE_AUDIO_DECODE_MP3



static void mp3_fill_buffer(__audiodev *audiodev, int index, uint16_t size, uint8_t nch);
static void mp3_updata_curtime(__audiodev *audiodev);
static uint32_t mp3_file_seek(__audiodev *audiodev, uint32_t pos_t);


static AudioDec_Status mp3_decode_malloc(__audiodev *audiodev);
static AudioDec_Status mp3_decode_free(__audiodev *audiodev);
static void mp3_decode_set(__audiodev *audiodev);

/**
 * @brief       播放一曲MP3音乐
 * @param       pname: MP3文件路径
 * @retval      0,正常播放完成
 */
uint8_t mp3_play_song(__audiodev *audiodev)
{
    HMP3Decoder mp3decoder;
    MP3FrameInfo mp3frameinfo;
    uint8_t res, Ret = AudioDec_OK;
    uint8_t *readptr;   // MP3解码读指针
    int offset = 0;     // 偏移量
    int bytesleft = 0;  // buffer还剩余的有效数据
    uint32_t br = 0;
    int err = 0;
	int index;
	
	audiodev->file_seek = mp3_file_seek;
	
	// 分配内存
	if(mp3_decode_malloc(audiodev) == AudioDec_MemErr)
	{
		Ret = AudioDec_MemErr;
		goto ___exit;
	}
	
	// 获取歌曲文件信息
    if (mp3_init_decode_info(audiodev->fname, (__mp3ctrl *)audiodev->infoctrl) != 0)
    {
		Ret = AudioDec_InfoErr;
        goto ___exit;
	}

	// 设置DMA、采样率等
	mp3_decode_set(audiodev);

	
	// 解码开始
	mp3decoder = MP3InitDecoder();              			
	if(f_open(audiodev->file, (char *)audiodev->fname, FA_READ) != FR_OK)
	{
		Ret = AudioDec_FileErr;
        goto ___exit;
    }
	f_lseek(audiodev->file, ((__mp3ctrl *)(audiodev->infoctrl))->datastart);   
	
	// 初始化指针和变量
    readptr = audiodev->mp3buf;
    bytesleft = 0;
    Ret = AudioDec_OK;
	
	audio_set_txdma_callback(audiodev->txdma_callback);
	audio_tx_dma_start(); 

    while (1) 
    {
        // 数据补充：如果剩余数据不足以支撑一帧解码 (MAINBUF_SIZE * 2)
        if (bytesleft < MAINBUF_SIZE * 2) 
        {
            if (bytesleft > 0 && readptr != audiodev->mp3buf) {
                memmove(audiodev->mp3buf, readptr, bytesleft);
            }
            readptr = audiodev->mp3buf;
            
            res = f_read(audiodev->file, audiodev->mp3buf + bytesleft, MP3_FILE_BUF_SZ - bytesleft, &br);
            if (res != FR_OK) { Ret = AudioDec_FileErr; goto ___exit; }//读取错误
            
            if (br == 0 && bytesleft == 0) { Ret = AudioDec_OK; goto ___exit; } // 文件读完且缓冲区空

            bytesleft += br;
            // 如果读到了文件末尾，补零以防解码器溢出
            if (br < (MP3_FILE_BUF_SZ - (bytesleft - br))) {
                memset(audiodev->mp3buf + bytesleft, 0, MP3_FILE_BUF_SZ - bytesleft);
            }
        }

        // 寻找同步词 
        offset = MP3FindSyncWord(readptr, bytesleft);
        if (offset < 0) {
            // 没找到同步词，说明这块数据全是垃圾，清空它准备下一轮读文件
            bytesleft = 0;
            continue; 
        }

        // 解码逻辑
        readptr += offset;
        bytesleft -= offset;

        err = MP3Decode(mp3decoder, &readptr, &bytesleft, (short *)audiodev->tbuf, 0);
        if (err != 0) {
			readptr++;
			bytesleft--;
			if (bytesleft <= 0) bytesleft = 0;
            printf("decode error:%d\r\n", err);
            // 某些非致命错误可以 continue 跳过，严重错误则 goto error
            continue; 
        }

        // 后处理：更新码率、填充音频、更新时间
        MP3GetLastFrameInfo(mp3decoder, &mp3frameinfo);
        if (((__mp3ctrl *)(audiodev->infoctrl))->bitrate != mp3frameinfo.bitrate) {
            ((__mp3ctrl *)(audiodev->infoctrl))->bitrate = mp3frameinfo.bitrate;
        }

		/*---------------------------------状态切换处理-------------------------------------------*/
		AudioDec_Status status = audiodev->control_callback(audiodev);
		// DMA 切换
		if (status == exAudioDec_DMA_SYNC_1 || status == exAudioDec_DMA_SYNC_2)
		{
			index = (status == exAudioDec_DMA_SYNC_1) ? 1 : 2;
			mp3_fill_buffer(audiodev, index, mp3frameinfo.outputSamps, mp3frameinfo.nChans);
		}
		
		// 退出切歌
		else if(status == exAudioDec_EXIT)
		{
			Ret = status;
			goto ___exit;
		}
		/*---------------------------------状态切换处理end-------------------------------------------*/
		
        // 更新播放进度
        mp3_updata_curtime(audiodev);
    }


___exit:
	MP3FreeDecoder(mp3decoder);
    audio_tx_dma_close();  
    f_close(audiodev->file);
    mp3_decode_free(audiodev);
	return Ret;
}


static AudioDec_Status mp3_decode_malloc(__audiodev *audiodev)
{
    audiodev->infoctrl = mymalloc(SRAMIN, sizeof(__mp3ctrl));
    audiodev->mp3buf = mymalloc(SRAMIN, MP3_FILE_BUF_SZ); 
    audiodev->file = (FIL *)mymalloc(SRAMIN, sizeof(FIL));
    audiodev->saibuf1 = mymalloc(SRAMIN, 2304 * 2);
    audiodev->saibuf2 = mymalloc(SRAMIN, 2304 * 2);
    audiodev->tbuf = mymalloc(SRAMIN, 2304 * 2);
    
    if (audiodev->infoctrl == NULL || audiodev->mp3buf == NULL || audiodev->file == NULL|| 
		audiodev->saibuf1 == NULL  || audiodev->saibuf2 == NULL|| audiodev->tbuf == NULL) 
    {
		return AudioDec_MemErr;
    }
    memset(audiodev->saibuf1, 0, 2304 * 2);  
    memset(audiodev->saibuf2, 0, 2304 * 2);   
    memset(audiodev->infoctrl, 0, sizeof(__mp3ctrl));  

	return AudioDec_NONE;
}

static AudioDec_Status mp3_decode_free(__audiodev *audiodev)
{
	myfree(SRAMIN, audiodev->infoctrl);
    myfree(SRAMIN, audiodev->mp3buf);
    myfree(SRAMIN, audiodev->file);
    myfree(SRAMIN, audiodev->saibuf1);
    myfree(SRAMIN, audiodev->saibuf2);
    myfree(SRAMIN, audiodev->tbuf);
	return AudioDec_NONE;
}


static void mp3_decode_set(__audiodev *audiodev)
{
	__mp3ctrl *mp3ctrl  = (__mp3ctrl *)audiodev->infoctrl;  
	
	
	es8388_sai_cfg(0, 3);   						// 飞利浦标准,16位数据长度
	audio_saia_init(0,1,4);      					// 飞利浦标准,主机发送,时钟低电平有效,16位扩展帧长度
	audio_sai_samplerate(mp3ctrl->samplerate);    	// 设置采样率
	audio_tx_dma_init((uint32_t)audiodev->saibuf1, (uint32_t)audiodev->saibuf2, mp3ctrl->outsamples,1);
}





/**
 * @brief       填充PCM数据到DAC
 * @param       buf: PCM数据首地址
 * @param       size: pcm数据量(16位为单位)
 * @param       nch: 声道数(1,单声道,2立体声)
 * @retval      无
 */
static void mp3_fill_buffer(__audiodev *audiodev, int index, uint16_t size, uint8_t nch)
{
    uint16_t i;
    
	uint16_t *buf = (uint16_t*)audiodev->tbuf;
	uint16_t *p = (index == 1) ? (uint16_t *)audiodev->saibuf1 : (uint16_t *)audiodev->saibuf2;

	
    if (nch == 2) // 双声道
    {
        for (i = 0; i < size; i++)
        {
            p[i] = buf[i];
        }
    }
    else    // 单声道
    {
        for (i = 0; i < size; i++)
        {
            p[2 * i] = buf[i];
            p[2 * i + 1] = buf[i];
        }
    }
}




/**
 * @brief   得到当前播放时间
 * @param    fx: 文件指针
 * @param    mp3x: mp3播放控制器
 * @retval   无
 */
static void mp3_updata_curtime(__audiodev *audiodev)
{
	FIL *fx = audiodev->file;
	__mp3ctrl *mp3x = audiodev->infoctrl;
    uint32_t fpos = 0;

    if (fx->fptr > mp3x->datastart)fpos = fx->fptr - mp3x->datastart;  // 得到当前文件播放到的地方

    mp3x->cursec = fpos * mp3x->totsec / (fx->obj.objsize - mp3x->datastart);  // 当前播放到第多少秒了
	
	audiodev->totsec = mp3x->totsec;     
	audiodev->cursec = mp3x->cursec;
}

/**
 * @brief       mp3文件快进快退函数
 * @param       pos: 需要定位到的文件位置
 * @retval      当前文件位置(即定位后的结果)
 */
static uint32_t mp3_file_seek(__audiodev *audiodev, uint32_t pos_t)
{
	uint32_t pos = audiodev->file->obj.objsize / audiodev->totsec * pos_t;	// 获取文件位置
		
    if (pos > audiodev->file->obj.objsize)
    {
        pos = audiodev->file->obj.objsize;
    }

    f_lseek(audiodev->file, pos);
    return audiodev->file->fptr;
}

#endif
