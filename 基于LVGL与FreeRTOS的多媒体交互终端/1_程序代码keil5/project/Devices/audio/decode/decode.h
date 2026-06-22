#ifndef __AUDIO_DECODE_H
#define __AUDIO_DECODE_H

#include "sys.h"
#include "sys.h"
#include "../../middle/fatfs/ff.h"
#include "../../middle/linklist/linkList.h"
#include "../../app/service/music_service.h"
#include "../../middle/audio/flac/flac.h"
#include "../audio_dma.h"
#include "../audio_sai.h"
#include "../audio_es8388.h"


#define USE_AUDIO_DECODE_WAV	1
#define USE_AUDIO_DECODE_MP3	1
#define USE_AUDIO_DECODE_FLAC	1
#define USE_AUDIO_DECODE_APE	0

#if USE_AUDIO_DECODE_WAV
	// WAV解码时,文件buf大小
	#define WAV_SAI_TX_DMA_BUFSIZE    	10240
#endif
#if USE_AUDIO_DECODE_MP3
	// MP3解码时,文件buf大小
	#define MP3_FILE_BUF_SZ     		5*1024      
#endif

/**
 * @brief 音频解码器状态与指令枚举
 * * 编码规则：通过低三位 (Bit 0-2) 进行硬分类，高五位 (Bit 3-7) 标识具体项。
 * 1. 内部状态/错误 (xxxx x001): 由解码器底层返回。
 * 2. 硬件同步信号 (xxxx x010): 由 DMA/中断触发。
 * 3. 外部控制指令 (xxxx x100): 由 UI/用户操作触发。
 */
typedef enum 
{
	AudioDec_NONE      = 0x00,	// 无错误
	
    /* --- 第一类：内部状态/错误 (特征码: 0x01, 二进制末尾: 001) --- */
    AudioDec_OK        = 0x01,  // 0x01: 正常解码完成（播放结束）
    AudioDec_MemErr    = 0x11,  // 0x11: 内存分配失败（通常是 Buffer 或播放对象申请失败）
    AudioDec_DecErr    = 0x21,  // 0x21: 解码失败（码流损坏或格式不支持）
    AudioDec_FileErr   = 0x31,  // 0x31: 文件操作失败（SD卡掉线或文件不存在）
    AudioDec_InfoErr   = 0x41,  // 0x41: 文件信息提取错误（Header 解析失败）

    /* --- 第二类：硬件同步信号 (特征码: 0x02, 二进制末尾: 010) --- */
    exAudioDec_DMA_SYNC_1     = 0x02,  // 0x02: DMA 搬运完成信号（触发双缓冲 Ping-Pong 填充）
	exAudioDec_DMA_SYNC_2     = 0x12,  // 0x02: DMA 搬运完成信号（触发双缓冲 Ping-Pong 填充）

    /* --- 第三类：外部控制指令 (特征码: 0x04, 二进制末尾: 100) --- */
    exAudioDec_EXIT    = 0x44,  	// 0x44: 退出解码指令
} AudioDec_Status;





typedef struct __audiodev __audiodev;

/**
 * @brief 播放转态切换函数
 */
typedef AudioDec_Status (*AudioControlCallback)(__audiodev *);



typedef __packed struct __audiodev
{
    uint8_t *saibuf1;
    uint8_t *saibuf2;
    uint8_t *tbuf;                 
    
#if USE_AUDIO_DECODE_MP3
	uint8_t *mp3buf;
#endif	
	
#if USE_AUDIO_DECODE_FLAC
	
	FLACContext *flacContext;
    uint8_t *flacbuf;
    uint8_t *flacDecbuf0;
    uint8_t *flacDecbuf1;
#endif	
	
    FIL *file;                      				// 音频文件指针
	int ftype;										// 文件类型
	char fname[128];								// 文件名
	
	AudioControlCallback control_callback;			// 控制回调函数
	audio_tx_callback_t  txdma_callback;			// DMA回调函数
	
	
    uint32_t(*file_seek)(__audiodev *, uint32_t); 	// 文件快进快退函数
	
	
	
    uint8_t status;        							// AudioDec_Status
    uint32_t totsec ;       						// 整首歌时长,单位:秒
    uint32_t cursec ;       						// 当前播放时长
    uint32_t bitrate;       						// 比特率(位速)
    uint32_t samplerate;    						// 采样率
    uint16_t bps;           						// 位数,比如16bit,24bit,32bit
	
	void *infoctrl;
	
	void *use_data;
} __audiodev;
extern __audiodev g_audiodev; 


AudioDec_Status audio_decode(__audiodev *audiodev);
AudioDec_Status wav_play_song(__audiodev *audiodev);
AudioDec_Status mp3_play_song(__audiodev *audiodev);
AudioDec_Status flac_play_song(__audiodev *audiodev);






#endif


