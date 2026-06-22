#include "mp3.h"
#include "../../middle/malloc/malloc.h"
#include "../../middle/fatfs/ff.h"
#include "../../system/debug/debug.h"

/**
 * @brief       解析ID3V1
 * @param       buf: 输入数据缓存区(大小固定是128字节)
 * @param       pctrl: MP3控制器
 * @retval      0,正常; 其他, 失败
 */
uint8_t mp3_id3v1_decode(uint8_t *buf, __mp3ctrl *pctrl)
{
    ID3V1_Tag *id3v1tag;
    id3v1tag = (ID3V1_Tag *)buf;

    if (strncmp("TAG", (char *)id3v1tag->id, 3) == 0) /* 是MP3 ID3V1 TAG */
    {
        if (id3v1tag->title[0])strncpy((char *)pctrl->title, (char *)id3v1tag->title, 30);

        if (id3v1tag->artist[0])strncpy((char *)pctrl->artist, (char *)id3v1tag->artist, 30);
    }
    else return 1;

    return 0;
}

/**
 * @brief       解析ID3V2
 * @param       buf: 输入数据缓存区(大小固定是128字节)
 * @param       size: 数据大小
 * @param       pctrl: MP3控制器
 * @retval      0, 正常; 其他, 失败
 */
uint8_t mp3_id3v2_decode(uint8_t *buf, uint32_t size, __mp3ctrl *pctrl)
{
    ID3V2_TagHead *taghead;
    ID3V23_FrameHead *framehead;
    uint32_t t;
    uint32_t tagsize;       /* tag大小 */
    uint32_t frame_size;    /* 帧大小 */
    taghead = (ID3V2_TagHead *)buf;

    if (strncmp("ID3", (const char *)taghead->id, 3) == 0) /* 存在ID3? */
    {
        tagsize = ((uint32_t)taghead->size[0] << 21) | ((uint32_t)taghead->size[1] << 14) | ((uint16_t)taghead->size[2] << 7) | taghead->size[3]; /* 得到tag 大小 */
        pctrl->datastart = tagsize;         /* 得到mp3数据开始的偏移量 */

        if (tagsize > size)tagsize = size;  /* tagsize大于输入bufsize的时候,只处理输入size大小的数据 */

        if (taghead->mversion < 3)
        {
            printf("not supported mversion!\r\n");
            return 1;
        }

        t = 10;

        while (t < tagsize)
        {
            framehead = (ID3V23_FrameHead *)(buf + t);
            frame_size = ((uint32_t)framehead->size[0] << 24) | ((uint32_t)framehead->size[1] << 16) | ((uint32_t)framehead->size[2] << 8) | framehead->size[3]; /* 得到帧大小 */

            if (strncmp("TT2", (char *)framehead->id, 3) == 0 || strncmp("TIT2", (char *)framehead->id, 4) == 0) /* 找到歌曲标题帧,不支持unicode格式!! */
            {
                strncpy((char *)pctrl->title, (char *)(buf + t + sizeof(ID3V23_FrameHead) + 1), AUDIO_MIN(frame_size - 1, MP3_TITSIZE_MAX - 1));
            }

            if (strncmp("TP1", (char *)framehead->id, 3) == 0 || strncmp("TPE1", (char *)framehead->id, 4) == 0) /* 找到歌曲艺术家帧 */
            {
                strncpy((char *)pctrl->artist, (char *)(buf + t + sizeof(ID3V23_FrameHead) + 1), AUDIO_MIN(frame_size - 1, MP3_ARTSIZE_MAX - 1));
            }

            t += frame_size + sizeof(ID3V23_FrameHead);
        }
    }
    else pctrl->datastart = 0; /* 不存在ID3,mp3数据是从0开始 */

    return 0;
}

/**
 * @brief       获取MP3基本信息
 * @param       pname: MP3文件路径
 * @param       pctrl: MP3控制器
 * @retval      0, 成功
 *              其他, 失败
 */
uint8_t mp3_init_decode_info(char *pname, __mp3ctrl *pctrl)
{
    HMP3Decoder decoder;
    MP3FrameInfo frame_info;
    MP3_FrameXing *fxing;
    MP3_FrameVBRI *fvbri;
    FIL *fmp3;
    uint8_t *buf;
    uint32_t br;
    uint8_t res;
    int offset = 0;
    uint32_t p;
    short samples_per_frame;    /* 一帧的采样个数 */
    uint32_t totframes;         /* 总帧数 */

    fmp3 = mymalloc(SRAMIN, sizeof(FIL));
    buf = mymalloc(SRAMIN, 5 * 1024);   /* 申请5K内存 */

    if (fmp3 && buf) /* 内存申请成功 */
    {
        f_open(fmp3, (const TCHAR *)pname, FA_READ); /* 打开文件 */
        res = f_read(fmp3, (char *)buf, 5 * 1024, &br);

        if (res == 0) /* 读取文件成功,开始解析ID3V2/ID3V1以及获取MP3信息 */
        {
            mp3_id3v2_decode(buf, br, pctrl);           /* 解析ID3V2数据 */
            f_lseek(fmp3, fmp3->obj.objsize - 128);     /* 偏移到倒数128的位置 */
            f_read(fmp3, (char *)buf, 128, &br);        /* 读取128字节 */
            mp3_id3v1_decode(buf, pctrl);               /* 解析ID3V1数据 */
            decoder = MP3InitDecoder();                 /* MP3解码申请内存 */
            f_lseek(fmp3, pctrl->datastart);            /* 偏移到数据开始的地方 */
            f_read(fmp3, (char *)buf, 5 * 1024, &br);   /* 读取5K字节mp3数据 */
            offset = MP3FindSyncWord(buf, br);          /* 查找帧同步信息 */

            if (offset >= 0 && MP3GetNextFrameInfo(decoder, &frame_info, &buf[offset]) == 0) /* 找到帧同步信息了,且下一阵信息获取正常 */
            {
                p = offset + 4 + 32;
                fvbri = (MP3_FrameVBRI *)(buf + p);

                if (strncmp("VBRI", (char *)fvbri->id, 4) == 0) /* 存在VBRI帧(VBR格式) */
                {
                    if (frame_info.version == MPEG1)samples_per_frame = 1152; /* MPEG1,layer3每帧采样数等于1152 */
                    else samples_per_frame = 576; /* MPEG2/MPEG2.5,layer3每帧采样数等于576 */

                    totframes = ((uint32_t)fvbri->frames[0] << 24) | ((uint32_t)fvbri->frames[1] << 16) | ((uint16_t)fvbri->frames[2] << 8) | fvbri->frames[3]; /* 得到总帧数 */
                    pctrl->totsec = totframes * samples_per_frame / frame_info.samprate; /* 得到文件总长度 */
                }
                else    /* 不是VBRI帧,尝试是不是Xing帧(VBR格式) */
                {
                    if (frame_info.version == MPEG1)    /* MPEG1 */
                    {
                        p = frame_info.nChans == 2 ? 32 : 17;
                        samples_per_frame = 1152;       /* MPEG1,layer3每帧采样数等于1152 */
                    }
                    else
                    {
                        p = frame_info.nChans == 2 ? 17 : 9;
                        samples_per_frame = 576;        /* MPEG2/MPEG2.5,layer3每帧采样数等于576 */
                    }

                    p += offset + 4;
                    fxing = (MP3_FrameXing *)(buf + p);

                    if (strncmp("Xing", (char *)fxing->id, 4) == 0 || strncmp("Info", (char *)fxing->id, 4) == 0) /* 是Xng帧 */
                    {
                        if (fxing->flags[3] & 0X01)     /* 存在总frame字段 */
                        {
                            totframes = ((uint32_t)fxing->frames[0] << 24) | ((uint32_t)fxing->frames[1] << 16) | ((uint16_t)fxing->frames[2] << 8) | fxing->frames[3]; /* 得到总帧数 */
                            pctrl->totsec = totframes * samples_per_frame / frame_info.samprate; /* 得到文件总长度 */
                        }
                        else    /* 不存在总frames字段 */
                        {
                            pctrl->totsec = fmp3->obj.objsize / (frame_info.bitrate / 8);
                        }
                    }
                    else        /* CBR格式,直接计算总播放时间 */
                    {
                        pctrl->totsec = fmp3->obj.objsize / (frame_info.bitrate / 8);
                    }
                }

                pctrl->bitrate = frame_info.bitrate;        /* 得到当前帧的码率 */
                pctrl->samplerate = frame_info.samprate;  /* 得到采样率. */

                if (frame_info.nChans == 2)pctrl->outsamples = frame_info.outputSamps; /* 输出PCM数据量大小 */
                else pctrl->outsamples = frame_info.outputSamps * 2; /* 输出PCM数据量大小,对于单声道MP3,直接*2,补齐为双声道输出 */
            }
            else res = 0XFE; /* 未找到同步帧 */

            MP3FreeDecoder(decoder);/* 释放内存 */
        }

        f_close(fmp3);
    }
    else res = 0XFF;
	
//	printf("title:%s\r\n", pctrl->title);
//	printf("artist:%s\r\n", pctrl->artist);
//	printf("bitrate:%dbps\r\n", pctrl->bitrate);
//	printf("samplerate:%d\r\n", pctrl->samplerate);
//	printf("totalsec:%d\r\n", pctrl->totsec);

    myfree(SRAMIN, fmp3);
    myfree(SRAMIN, buf);
    return res;
}






