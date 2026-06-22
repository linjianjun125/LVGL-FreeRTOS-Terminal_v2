#include "wav.h"

#include "../../middle/malloc/malloc.h"
#include "../../middle/fatfs/ff.h"
#include "../../system/debug/debug.h"




/**
 * @brief   WAV解析初始化
 * @param   fname : 文件路径+文件名
 * @param   wavx  : 信息存放结构体指针
 * @retval  0,打开文件成功1,打开文件失败2,非WAV文件3,DATA区域未找到
 */
uint8_t wav_init_decode_info(char *fname, __wavctrl *wavx)
{
    FIL *ftemp;
    uint8_t *buf; 
    uint32_t br = 0;
    uint8_t res = 0;

    ChunkRIFF *riff;
    ChunkFMT *fmt;
    ChunkFACT *fact;
    ChunkDATA *data;
    
    ftemp = (FIL*)mymalloc(SRAMIN, sizeof(FIL));
    buf = mymalloc(SRAMIN, 512);

    if (ftemp && buf)    /* 内存申请成功 */
    {
        res = f_open(ftemp, (TCHAR*)fname, FA_READ);    /* 打开文件 */
        if (res==FR_OK)
        {
            f_read(ftemp, buf, 512, &br);               /* 读取512字节在数据 */
            riff = (ChunkRIFF *)buf;                    /* 获取RIFF块 */
            if (riff->Format == 0X45564157)             /* 是WAV文件 */
            {
                fmt = (ChunkFMT *)(buf + 12);           /* 获取FMT块 */
                fact = (ChunkFACT *)(buf + 12 + 8 + fmt->ChunkSize);                    /* 读取FACT块 */

                if (fact->ChunkID == 0X74636166 || fact->ChunkID == 0X5453494C)
                {
                    wavx->datastart = 12 + 8 + fmt->ChunkSize + 8 + fact->ChunkSize;    /* 具有fact/LIST块的时候(未测试) */
                }
                else
                {
                    wavx->datastart = 12 + 8 + fmt->ChunkSize;
                }

                data = (ChunkDATA *)(buf + wavx->datastart);    /* 读取DATA块 */

                if (data->ChunkID == 0X61746164)                /* 解析成功! */
                {
                    wavx->audioformat = fmt->AudioFormat;       /* 音频格式 */
                    wavx->nchannels = fmt->NumOfChannels;       /* 通道数 */
                    wavx->samplerate = fmt->SampleRate;         /* 采样率 */
                    wavx->bitrate = fmt->ByteRate * 8;          /* 得到位速 */
                    wavx->blockalign = fmt->BlockAlign;         /* 块对齐 */
                    wavx->bps = fmt->BitsPerSample;             /* 位数,16/24/32位 */
                    
                    wavx->datasize = data->ChunkSize;           /* 数据块大小 */
                    wavx->datastart = wavx->datastart + 8;      /* 数据流开始的地方. */
                     
//                    printf("wavx->audioformat:%d\r\n", wavx->audioformat);
//                    printf("wavx->nchannels:%d\r\n", wavx->nchannels);
//                    printf("wavx->samplerate:%d\r\n", wavx->samplerate);
//                    printf("wavx->bitrate:%d\r\n", wavx->bitrate);
//                    printf("wavx->blockalign:%d\r\n", wavx->blockalign);
//                    printf("wavx->bps:%d\r\n", wavx->bps);
//                    printf("wavx->datasize:%d\r\n", wavx->datasize);
//                    printf("wavx->datastart:%d\r\n", wavx->datastart);  
                }
                else
                {
                    res = 3;    /* data区域未找到. */
                }
            }
            else
            {
                res = 2;        /* 非wav文件 */
            }
        }
        else
        {
            res = 1;            /* 打开文件错误 */
        }
    }

    f_close(ftemp);             /* 关闭文件 */
    myfree(SRAMIN,ftemp);       /* 释放内存 */
    myfree(SRAMIN,buf); 

    return 0;
}



















