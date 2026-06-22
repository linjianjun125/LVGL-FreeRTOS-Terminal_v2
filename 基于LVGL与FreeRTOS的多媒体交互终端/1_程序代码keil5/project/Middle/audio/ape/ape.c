#include "ape.h"
#include "../../middle/malloc/malloc.h"
#include "../../middle/fatfs/ff.h"
#include "../../system/debug/debug.h"


uint8_t ape_init_decode_info(char *pname, struct ape_ctx_t *apex, __apectrl* apectrl)
{
	uint32_t res, Ret = 0;
	uint32_t totalsamples;

	FIL* fx = (FIL*)mymalloc(SRAMIN, sizeof(FIL));
	if(fx == NULL){
		Ret = 1;
		goto error;
	}
	
	res = f_open(fx, (char *)pname, FA_READ); 
	if (res != FR_OK){
		Ret = 1;
		goto error;
	}
	
	res = ape_parseheader(fx, apex); 
	if(res != 0){
		Ret = 1;
		goto error;
	}
	
	/* 压缩率支持/版本支持/是16位音频格式 */
	if ((apex->compressiontype > 3000) || (apex->fileversion < APE_MIN_VERSION) || (apex->fileversion > APE_MAX_VERSION || apex->bps != 16))
	{
		Ret = 1;
	}
	else
	{
		apectrl->bps = apex->bps;   				/* 得到采样深度(ape,我们仅支持16位) */
		apectrl->samplerate = apex->samplerate;     /* 得到采样率,ape支持48Khz以下的采样率,在高就可能卡了... */

		if (apex->totalframes > 1)
			totalsamples = apex->finalframeblocks + apex->blocksperframe * (apex->totalframes - 1);
		else 
			totalsamples = apex->finalframeblocks;

		apectrl->totsec = totalsamples / apectrl->samplerate;   						/* 得到文件总时长 */
		apectrl->bitrate = (fx->obj.objsize - apex->firstframe) * 8 / apectrl->totsec; 	/* 得到位速 */
		apectrl->outsamples = APE_BLOCKS_PER_LOOP * 2;  								/* PCM输出数据量(16位为单位) */
		apectrl->datastart = apex->firstframe;      									/* 得到第一帧的地址 */
		
		printf("  Samplerate: %d\r\n", apectrl->samplerate);
        printf("  Bits per sample: %d\r\n", apectrl->bps);
        printf("  First frame pos: %d\r\n", apectrl->datastart);
        printf("  Duration: %d s\r\n", apectrl->totsec);
        printf("  Bitrate: %d kbps\r\n", apectrl->bitrate);
	}

	f_close(fx);
	myfree(SRAMIN, fx);
	return Ret;
	
error:
	f_close(fx);
	myfree(SRAMIN, fx);
	return Ret;
}


