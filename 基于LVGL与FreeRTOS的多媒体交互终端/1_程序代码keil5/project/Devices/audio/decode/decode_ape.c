#include "decode.h"
#include "../audio_dma.h"
#include "../audio_sai.h"
#include "../audio_es8388.h"
#include <stdlib.h>
#include <string.h>
#include "../../middle/audio/ape/ape.h"
#include "../../middle/malloc/malloc.h"
#include "../../middle/fatfs/ff.h"
#include "../../system/debug/debug.h"
#include "FreeRTOS.h"
#include "task.h"

#if USE_AUDIO_DECODE_APE

#define AUDIO_MIN(x,y)	((x)<(y)? (x):(y))


static void ape_delay_ms(void)
{
	 vTaskDelay(2);
}



__apectrl *apectrl;	/* APE播放控制结构体 */
/* apedecoder.c里面需要的数组 */
extern filter_int *filterbuf64;         /* 需要2816字节 */
volatile uint8_t apetransferend = 0;    /* i2s传输完成标志 */
volatile uint8_t apewitchbuf = 0;       /* i2sbufx指示标志 */
static void ape_i2s_dma_tx_callback(void);
static void ape_fill_buffer(uint32_t *buf, uint16_t size);
static uint32_t ape_file_seek(uint32_t pos);
static void ape_get_curtime(FIL *fx, __apectrl *apectrl);


/**
 * @brief       播放一曲APE音乐
 * @param       pname: APE 文件路径
 * @retval      0,正常播放完成
 */
uint8_t ape_play_song(char *fname)
{
    struct ape_ctx_t *apex;

    int currentframe;
    int nblocks;
    int bytesconsumed;
    int bytesinbuffer;
    int blockstodecode;
    int firstbyte;
    int n;

    uint8_t res, Ret = AudioDec_OK;
    uint8_t *readptr;
    uint8_t *buffer;
    int *decoded0;
    int *decoded1;


	/*!< 内存分配 >!*/
    filterbuf64 = mymalloc(SRAMIN, 2816);
    apectrl = mymalloc(SRAMIN, sizeof(__apectrl));
    apex = mymalloc(SRAMIN, sizeof(struct ape_ctx_t));
    decoded0 = mymalloc(SRAMDTCM, APE_BLOCKS_PER_LOOP * 4);
    decoded1 = mymalloc(SRAMDTCM, APE_BLOCKS_PER_LOOP * 4);
    g_audiodev.file = (FIL *)mymalloc(SRAMIN, sizeof(FIL));
    g_audiodev.saibuf1 = mymalloc(SRAMIN, APE_BLOCKS_PER_LOOP * 4);
    g_audiodev.saibuf2 = mymalloc(SRAMIN, APE_BLOCKS_PER_LOOP * 4);
    g_audiodev.file_seek = ape_file_seek;
    buffer = mymalloc(SRAMIN, APE_FILE_BUF_SZ);
    if (filterbuf64 == NULL || apectrl == NULL || apex == NULL || decoded0 == NULL || decoded1 == NULL ||
	g_audiodev.file == NULL || g_audiodev.saibuf1 == NULL || g_audiodev.saibuf2 == NULL || buffer == NULL)
    {
		Ret = AudioDec_MemErr;
		goto error;
	}
	memset(apex, 0, sizeof(struct ape_ctx_t));
	memset(apectrl, 0, sizeof(__apectrl));
	memset(g_audiodev.saibuf1, 0, APE_BLOCKS_PER_LOOP * 4);
	memset(g_audiodev.saibuf2, 0, APE_BLOCKS_PER_LOOP * 4);
		
		
	/*!< 得到文件的信息 >!*/
	res = ape_init_decode_info(fname, apex, apectrl);
	if(res != 0)
	{
		Ret = AudioDec_InfoErr;
		goto error;
	}

	
	/*!< 解码初始化 >!*/
	es8388_sai_cfg(0, 3);   /* 飞利浦标准,16位数据长度 */
	audio_saia_init(0,1,4);      /* 飞利浦标准,主机发送,时钟低电平有效,16位扩展帧长度 */
	audio_sai_samplerate(apex->samplerate);       /* 设置采样率 */
	audio_tx_dma_init((uint32_t)g_audiodev.saibuf1,(uint32_t)g_audiodev.saibuf2, APE_BLOCKS_PER_LOOP * 2,1);    /* 配置TX DMA */
	audio_tx_dma_callback = ape_i2s_dma_tx_callback;  /* 回调函数指ape_i2s_dma_tx_callback */


	/*!< 开始解码 >!*/
	currentframe = 0;
	res = f_open(g_audiodev.file, (char *)fname, FA_READ); 
	if (res != FR_OK)
	{
		Ret = AudioDec_FileErr;
		goto error;
	}
	f_lseek(g_audiodev.file, apex->firstframe);
	res = f_read(g_audiodev.file, buffer, APE_FILE_BUF_SZ, (uint32_t *)&bytesinbuffer);	
	firstbyte = 3;          /* Take account of the little-endian 32-bit byte ordering */
	readptr = buffer;
	
	
	audio_tx_dma_start();
	audio_play();    
	while (currentframe < apex->totalframes && res == 0) /* 还有未解码的帧? */
	{
		/* 计算一帧里面有多少个blocks? */
		if (currentframe == (apex->totalframes - 1))nblocks = apex->finalframeblocks;
		else nblocks = apex->blocksperframe;

		apex->currentframeblocks = nblocks;
		
		/* 初始化帧解码 */
		init_frame_decoder(apex, readptr, &firstbyte, &bytesconsumed);
		readptr += bytesconsumed;
		bytesinbuffer -= bytesconsumed;

		while (nblocks > 0) /* 开始帧解码 */
		{
			blockstodecode = AUDIO_MIN(APE_BLOCKS_PER_LOOP, nblocks); /* 获得一次要解码的blocks个数 */
			res = decode_chunk(apex, readptr, &firstbyte, &bytesconsumed, decoded0, decoded1, blockstodecode);
			if (res != 0)
			{
				printf("frame decode err\r\n");
				Ret = AudioDec_DecErr;
				goto error;
			}

			ape_fill_buffer((uint32_t *)decoded1, APE_BLOCKS_PER_LOOP);
			readptr += bytesconsumed;           /* 解码指针偏移到新数据位置 */
			bytesinbuffer -= bytesconsumed;     /* buffer里面的数据量减少 */

			if (bytesconsumed > 4 * APE_BLOCKS_PER_LOOP) /* 出现错误了 */
			{
				nblocks = 0;
				Ret = AudioDec_DecErr;
				printf("bytesconsumed:%d\r\n", bytesconsumed);
				goto error;
			}

			if (bytesinbuffer < 4 * APE_BLOCKS_PER_LOOP) /* 需要补充新数据了 */
			{
				memmove(buffer, readptr, bytesinbuffer);
				res = f_read(g_audiodev.file, buffer + bytesinbuffer, APE_FILE_BUF_SZ - bytesinbuffer, (uint32_t *)&n);
				if (res != FR_OK) /* 出错了 */
				{
					Ret = AudioDec_FileErr;
					goto error;
				}

				bytesinbuffer += n;
				readptr = buffer;
			}
			nblocks -= blockstodecode; /* block计数递减 */

			// 播放状态
			if(g_audiodev.status == Decode_Status_Play)
			{
				ape_get_curtime(g_audiodev.file, apectrl); 
				g_audiodev.totsec = apectrl->totsec;     
				g_audiodev.cursec = apectrl->cursec;
			}
			// 暂停状态
			else if(g_audiodev.status == Decode_Status_Stop)
			{
				ape_delay_ms();
			}
			// 播放退出
			else
			{
				nblocks = 0;
				res = 1;
				Ret = AudioDec_DecQue;
				break;
			}
		}
		currentframe++;
	}

	
error:
    audio_tx_dma_close();
    f_close(g_audiodev.file);
    myfree(SRAMIN, filterbuf64);
    myfree(SRAMIN, apectrl);
    myfree(SRAMIN, apex->seektable);
    myfree(SRAMIN, apex);
    myfree(SRAMDTCM, decoded0);
    myfree(SRAMDTCM, decoded1);
    myfree(SRAMIN, g_audiodev.file);
    myfree(SRAMIN, g_audiodev.saibuf1);
    myfree(SRAMIN, g_audiodev.saibuf2);
    myfree(SRAMIN, buffer);
    return Ret;
}

/**
 * @brief       APE DMA发送回调函数
 * @param       无
 * @retval      无
 */
static void ape_i2s_dma_tx_callback(void)
{
    uint16_t i;

    if (AUDIO_TX_DMA->Stream[AUDIO_TX_DMA_Stream].CR & (1 << 19))
    {
        apewitchbuf = 0;

        if (g_audiodev.status == Decode_Status_Stop) /* 暂停了,填充0 */
        {
            for (i = 0; i < APE_BLOCKS_PER_LOOP * 4; i++)
				g_audiodev.saibuf1[i] = 0;
        }
    }
    else
    {
        apewitchbuf = 1;

        if (g_audiodev.status == Decode_Status_Stop) /* 暂停了,填充0 */
        {
            for (i = 0; i < APE_BLOCKS_PER_LOOP * 4; i++)
				g_audiodev.saibuf2[i] = 0;
        }
    }

    apetransferend = 1;
}

/**
 * @brief       填充PCM数据到DAC
 * @param       buf             : PCM数据首地址
 * @param       size            : pcm数据量(32位为单位)
 * @retval      无
 */
static void ape_fill_buffer(uint32_t *buf, uint16_t size)
{
    uint16_t i;
    uint32_t *p;

    while (apetransferend == 0)/* 等待传输完成 */
	{
		ape_delay_ms(); 
	}

    apetransferend = 0;

    if (apewitchbuf == 0)
    {
        p = (uint32_t *)
		g_audiodev.saibuf1;
    }
    else
    {
        p = (uint32_t *)
		g_audiodev.saibuf2;
    }

    for (i = 0; i < size; i++)
		p[i] = buf[i];
}

/**
 * @brief       得到当前播放时间
 * @param       fx              : 文件指针
 * @param       apectrl         : apectrl播放控制器
 * @retval      无
 */
static void ape_get_curtime(FIL *fx, __apectrl *apectrl)
{
    long long fpos = 0;

    if (fx->fptr > apectrl->datastart)fpos = fx->fptr - apectrl->datastart;             /* 得到当前文件播放到的地方 */

    apectrl->cursec = fpos * apectrl->totsec / (fx->obj.objsize - apectrl->datastart);  /* 当前播放到第多少秒了? */
}

/**
 * @brief       ape 文件快进快退函数
 * @param       pos             : 需要定位到的文件位置
 * @retval      当前文件位置(即定位后的结果)
 */
static uint32_t ape_file_seek(uint32_t pos)
{
    return g_audiodev.file->fptr;/* ape文件不支持快进快退,直接返回当前播放位置 */
}

#endif


