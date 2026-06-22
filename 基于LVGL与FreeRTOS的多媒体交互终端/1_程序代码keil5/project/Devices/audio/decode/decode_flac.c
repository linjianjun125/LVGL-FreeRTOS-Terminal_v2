#include "decode.h"
#include "../audio_dma.h"
#include "../audio_sai.h"
#include "../audio_es8388.h"
#include "../../middle/audio/flac/flac.h"
#include "../../middle/malloc/malloc.h"
#include "../../middle/fatfs/ff.h"
#include "../../system/debug/debug.h"
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

#if USE_AUDIO_DECODE_FLAC



static uint32_t flac_file_seek(__audiodev *audiodev, uint32_t pos_t);
void flac_updata_curtime(__audiodev *audiodev);


static AudioDec_Status flac_decode_malloc1(__audiodev *audiodev);
static AudioDec_Status flac_decode_malloc2(__audiodev *audiodev);
static AudioDec_Status flac_decode_free(__audiodev *audiodev);
static void flac_decode_set(__audiodev *audiodev);

/**
 * @brief   播放一曲FLAC音乐
 * @param   pname: FLAC文件路径
 * @retval  0,正常播放完成
 */
uint8_t flac_play_song(__audiodev *audiodev)
{
    int bytesleft;
    int consumed;
    uint8_t res = 0,Ret = AudioDec_OK;
    uint32_t br = 0;
    uint8_t *p8 = 0;
    uint32_t flac_fptr = 0;

	audiodev->file_seek = flac_file_seek;

	// 分配内存
	if(flac_decode_malloc1(audiodev) == AudioDec_MemErr)
	{
		Ret = AudioDec_MemErr;
		goto ___exit;
	}
    

	// 得到文件的信息
	if(flac_init_decode_info(audiodev->fname, audiodev->infoctrl, audiodev->flacContext) != 0)
	{
		Ret = AudioDec_InfoErr;
		goto ___exit;
	}
	
	// 再次分配内存
	if(flac_decode_malloc2(audiodev) == AudioDec_MemErr)
	{
		Ret = AudioDec_MemErr;
		goto ___exit;
	}

	// 解码初始化
	flac_decode_set(audiodev);

	// 解码开始
	res = f_open(audiodev->file, (char *)audiodev->fname, FA_READ);  
	f_lseek(audiodev->file, ((__flacctrl*)(audiodev->infoctrl))->datastart);
	if(res != FR_OK)
	{
		Ret = AudioDec_FileErr;
		goto ___exit;
	}
	
    f_read(audiodev->file, audiodev->flacbuf, audiodev->flacContext->max_framesize, &br); 
	bytesleft = br;
	audiodev->flacContext->decoded0 = (int *)audiodev->flacDecbuf0;     // 解码数组0
	audiodev->flacContext->decoded1 = (int *)audiodev->flacDecbuf1;     // 解码数组1
	flac_fptr = audiodev->file->fptr;  									// 记录当前的文件位置
	

 	audio_set_txdma_callback(audiodev->txdma_callback);
	audio_tx_dma_start(); 
	
	while (bytesleft)
	{
		/*---------------------------------状态切换处理-------------------------------------------*/
		AudioDec_Status status = audiodev->control_callback(audiodev);
		
		// DMA 切换
		if (status == exAudioDec_DMA_SYNC_1 || status == exAudioDec_DMA_SYNC_2)
		{
			p8 = (status == exAudioDec_DMA_SYNC_1) ?  audiodev->saibuf1 : audiodev->saibuf2;
		}
		
		// 退出切歌
		else if(status == exAudioDec_EXIT)
		{
			Ret = status;
			goto ___exit;
		}
		/*---------------------------------状态切换处理end-------------------------------------------*/
		
		
		
		if (flac_fptr != audiodev->file->fptr) // 说明外部有进行文件快进/快退操作
		{
			if (audiodev->file->fptr < ((__flacctrl*)(audiodev->infoctrl))->datastart) // 在数据开始之前
			{
				f_lseek(audiodev->file, ((__flacctrl*)(audiodev->infoctrl))->datastart); // 偏移到数据开始的地方
			}

			f_read(audiodev->file, audiodev->flacbuf, audiodev->flacContext->max_framesize, &br); 	// 读取一个最大帧的数据量
			bytesleft = flac_seek_frame(audiodev->flacbuf, br, audiodev->flacContext);    			// 查找帧 

			if (bytesleft >= 0) /* 找到正确的帧头 */
			{
				f_lseek(audiodev->file, audiodev->file->fptr - audiodev->flacContext->max_framesize + bytesleft);
				f_read(audiodev->file, audiodev->flacbuf, audiodev->flacContext->max_framesize, &br);
			}
			else printf("flac seek error:%d\r\n", bytesleft);

			bytesleft = br;
		}


		res = (audiodev->flacContext->bps == 24) ? 
			  flac_decode_frame24(audiodev->flacContext, audiodev->flacbuf, bytesleft, (s32 *)p8) : 
			  flac_decode_frame16(audiodev->flacContext, audiodev->flacbuf, bytesleft, (s16 *)p8);
		if (res != 0) /* 解码出错了 */
		{
			Ret = AudioDec_DecErr;
			goto ___exit;
		}

		consumed = audiodev->flacContext->gb.index / 8;
		memmove(audiodev->flacbuf, &audiodev->flacbuf[consumed], bytesleft - consumed);
		bytesleft -= consumed;
		res = f_read(audiodev->file, &audiodev->flacbuf[bytesleft], audiodev->flacContext->max_framesize - bytesleft, &br);
		if (res) /* 读数据出错了 */
		{
			Ret = AudioDec_FileErr;
			goto ___exit;
		}
		if (br > 0)
		{
			bytesleft += br;
		}

		flac_fptr = audiodev->file->fptr; // 记录当前的文件位置
		flac_updata_curtime(audiodev);
	}
	
___exit:
	audio_tx_dma_close();
    f_close(audiodev->file);
	flac_decode_free(audiodev);
    return Ret;
}




static AudioDec_Status flac_decode_malloc1(__audiodev *audiodev)
{
	audiodev->flacContext = mymalloc(SRAMIN, sizeof(FLACContext));
    audiodev->infoctrl = mymalloc(SRAMIN, sizeof(__flacctrl));
    audiodev->file = (FIL *)mymalloc(SRAMIN, sizeof(FIL));
	
    if (audiodev->flacContext == NULL || audiodev->file == NULL || audiodev->infoctrl == NULL)
	{
		return AudioDec_MemErr;
	}
	
	memset(audiodev->flacContext, 0, sizeof(FLACContext));
	return AudioDec_NONE;
}

static AudioDec_Status flac_decode_malloc2(__audiodev *audiodev)
{
	uint32_t saisize = 0;
	
	if (audiodev->flacContext->min_blocksize == audiodev->flacContext->max_blocksize 
		&& audiodev->flacContext->max_blocksize != 0) // 必须min_blocksize等于max_blocksize
	{
		saisize = (audiodev->flacContext->bps == 24) ? 8 : 4; // 24位、16位音频数据
		saisize = saisize * audiodev->flacContext->max_blocksize;

		// 对帧大小不确定的flac, 设置默认帧大小(内存充足可以适当设大一点)
		if(audiodev->flacContext->max_framesize == 0)  {audiodev->flacContext->max_framesize = 0X4000;}
		
		audiodev->saibuf1 = mymalloc(SRAMIN, saisize);
		audiodev->saibuf2 = mymalloc(SRAMIN, saisize);
		audiodev->flacbuf = mymalloc(SRAMDTCM, audiodev->flacContext->max_framesize); 
		audiodev->flacDecbuf0 = mymalloc(SRAMDTCM, audiodev->flacContext->max_blocksize * 4);
		audiodev->flacDecbuf1 = mymalloc(SRAMDTCM, audiodev->flacContext->max_blocksize * 4);
		if(audiodev->saibuf1 == NULL || audiodev->saibuf2 == NULL || audiodev->flacbuf == NULL ||
		   audiodev->flacDecbuf0 == NULL || audiodev->flacDecbuf1== NULL)
		{
			return AudioDec_MemErr;
		}
	}
	else
	{
		return AudioDec_MemErr;
	}
	
	memset(audiodev->saibuf1, 0, saisize);
	memset(audiodev->saibuf2, 0, saisize);
	
	return AudioDec_NONE;
	
	
}
static AudioDec_Status flac_decode_free(__audiodev *audiodev)
{
    myfree(SRAMIN, audiodev->flacContext);
    myfree(SRAMIN, audiodev->infoctrl);
    myfree(SRAMIN, audiodev->file);
    myfree(SRAMIN, audiodev->saibuf1);
    myfree(SRAMIN, audiodev->saibuf2);
    myfree(SRAMDTCM, audiodev->flacbuf);
    myfree(SRAMDTCM, audiodev->flacDecbuf0);
    myfree(SRAMDTCM, audiodev->flacDecbuf1);
	return AudioDec_NONE;
}


static void flac_decode_set(__audiodev *audiodev)
{
	if (((__flacctrl*)(audiodev->infoctrl))->bps == 24)    
	{
		// 24位音频数据
		es8388_sai_cfg(0, 0);   		// 飞利浦标准,24位数据长度
		audio_saia_init(0,1,6);      	// 飞利浦标准,主机发送,时钟低电平有效,24位扩展帧长度
		audio_tx_dma_init((uint32_t)audiodev->saibuf1, (uint32_t)audiodev->saibuf2, ((__flacctrl*)(audiodev->infoctrl))->outsamples , 2);  
	}
	else    
	{
		// 16位音频数据
		es8388_sai_cfg(0, 3);   // 飞利浦标准,16位数据长度
		audio_saia_init(0,1,4); // 飞利浦标准,主机发送,时钟低电平有效,16位扩展帧长度
		audio_tx_dma_init((uint32_t)audiodev->saibuf1, (uint32_t)audiodev->saibuf2, ((__flacctrl*)(audiodev->infoctrl))->outsamples,1); 

	}
	audio_sai_samplerate(audiodev->flacContext->samplerate); // 设置采样率
}









/**
 * @brief   得到当前播放时间
 * @param   fx: 文件指针
 * @param   audiodev->infoctrl: flac播放控制器
 * @retval  无
 */
void flac_updata_curtime(__audiodev *audiodev)
{
    long long fpos = 0;
	FIL *fx = audiodev->file;
	__flacctrl *flacctrl = ((__flacctrl*)(audiodev->infoctrl));

    if (fx->fptr > flacctrl->datastart)fpos = fx->fptr - flacctrl->datastart;  // 得到当前文件播放到的地方

    flacctrl->cursec = fpos * flacctrl->totsec / (fx->obj.objsize - flacctrl->datastart);  // 当前播放到第多少秒了
	
	audiodev->totsec = flacctrl->totsec;     
	audiodev->cursec = flacctrl->cursec;
}



/**
 * @brief   flac文件快进快退函数
 * @param   pos: 需要定位到的文件位置
 * @retval	当前文件位置(即定位后的结果)
 */
static uint32_t flac_file_seek(__audiodev *audiodev, uint32_t pos_t)
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

