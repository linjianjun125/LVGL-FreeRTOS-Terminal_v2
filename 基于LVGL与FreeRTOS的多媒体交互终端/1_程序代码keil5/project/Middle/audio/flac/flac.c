#include "flac.h"
#include "../../middle/malloc/malloc.h"
#include "../../middle/fatfs/ff.h"
#include "../../system/debug/debug.h"




/**
 * @brief       分析FLAC文件
 * @param       fx : 文件指针
 * @param       fc: flac解码容器
 * @retval      0, 成功
 *              其他, 错误代码
 */
uint8_t flac_init_decode_info(char *f_name, __flacctrl *fctrl, FLACContext *fc)
{
    FLAC_Tag *flactag;
    MD_Block_Head *flacblkh;
    uint8_t *buf;
    uint8_t endofmetadata = 0;      /* 最后一个metadata标记 */
    int blocklength;
    uint32_t br;
    uint8_t res;
	
	FIL* fx = (FIL*)mymalloc(SRAMIN, sizeof(FIL));
    buf = mymalloc(SRAMIN, 512);    /* 申请512字节内存 */
    if (buf == NULL || fx == NULL)
		return 1;              /* 内存申请失败 */

	res = f_open(fx, (char *)f_name, FA_READ);  /* 读取文件错误 */
    f_lseek(fx, 0);                 /* 偏移到文件头 */
    f_read(fx, buf, 4, &br);        /* 读取4字节 */
    flactag = (FLAC_Tag *)buf;      /* 强制转换为flac tag标签 */

    if (strncmp("fLaC", (char *)flactag->id, 4) != 0)
    {
        myfree(SRAMIN, buf);        /* 释放内存 */
		f_close(fx);
        return 2;                   /* 非flac文件 */
    }

    while (!endofmetadata)
    {
        f_read(fx, buf, 4, &br);

        if (br < 4)break;

        flacblkh = (MD_Block_Head *)buf;
        endofmetadata = flacblkh->head & 0X80;  /* 判断是不是最后一个block? */
        blocklength = ((uint32_t)flacblkh->size[0] << 16) | ((uint16_t)flacblkh->size[1] << 8) | (flacblkh->size[2]); /* 得到块大小 */

        if ((flacblkh->head & 0x7f) == 0)       /* head最低7位为0,则表示是STREAMINFO块 */
        {
            res = f_read(fx, buf, blocklength, &br);
            if (res != FR_OK)
				break;

            fc->min_blocksize = ((uint16_t)buf[0] << 8) | buf[1];   /* 最小块大小 */
            fc->max_blocksize = ((uint16_t)buf[2] << 8) | buf[3];   /* 最大块大小 */
            fc->min_framesize = ((uint32_t)buf[4] << 16) | ((uint16_t)buf[5] << 8) | buf[6];                    /* 最小帧大小 */
            fc->max_framesize = ((uint32_t)buf[7] << 16) | ((uint16_t)buf[8] << 8) | buf[9];                    /* 最大帧大小 */
            fc->samplerate = ((uint32_t)buf[10] << 12) | ((uint16_t)buf[11] << 4) | ((buf[12] & 0xf0) >> 4);    /* 采样率 */
            fc->channels = ((buf[12] & 0x0e) >> 1) + 1;             /* 音频通道数 */
            fc->bps = ((((uint16_t)buf[12] & 0x01) << 4) | ((buf[13] & 0xf0) >> 4)) + 1;                        /* 采样位数16?24?32? */
            fc->totalsamples = ((uint32_t)buf[14] << 24) | ((uint32_t)buf[15] << 16) | ((uint16_t)buf[16] << 8) | buf[17];  /* 一个声道的总采样数 */
            fctrl->samplerate = fc->samplerate;
            fctrl->totsec = (fc->totalsamples / fc->samplerate);    /* 得到总时间 */
        }
        else    /* 忽略其他帧的处理 */
        {
            if (f_lseek(fx, fx->fptr + blocklength) != FR_OK)
            {
                myfree(SRAMIN, buf);
				f_close(fx);
                return 3;
            }
        }
    }

    myfree(SRAMIN, buf); /* 释放内存 */
	f_close(fx);
	
    if (fctrl->totsec)
    {
        fctrl->outsamples = fc->max_blocksize * 2;  /* PCM输出数据量(*2,表示2个声道的数据量) */
        fctrl->bps = fc->bps;                       /* 采样位数(16/24/32) */
        fctrl->datastart = fx->fptr;                /* FLAC数据帧开始的地址 */
        fctrl->bitrate = ((fx->obj.objsize - fctrl->datastart) * 8) / fctrl->totsec; /* 得到FLAC的位速 */
    }
    else 
		return 4;  /* 总时间为0?有问题的flac文件 */

//	printf("\r\n  Blocksize: %d .. %d\r\n", fc->min_blocksize, fc->max_blocksize);
//	printf("  Framesize: %d .. %d\r\n", fc->min_framesize, fc->max_framesize);
//	printf("  Samplerate: %d\r\n", fc->samplerate);
//	printf("  Channels: %d\r\n", fc->channels);
//	printf("  Bits per sample: %d\r\n", fc->bps);
//	printf("  Metadata length: %d\r\n", fctrl->datastart);
//	printf("  Total Samples: %lu\r\n", fc->totalsamples);
//	printf("  Duration: %d s\r\n", fctrl->totsec);
//	printf("  Bitrate: %d kbps\r\n", fctrl->bitrate);
	
    return 0;
}




















