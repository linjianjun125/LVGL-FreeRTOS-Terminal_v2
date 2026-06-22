#include "decode.h"

#include "devices_include.h"
#include "middle_include.h"
#include "app_include.h"
#include "ui_include.h"

__audiodev g_audiodev; 


AudioDec_Status audio_decode(__audiodev *audiodev)
{
	AudioDec_Status res = AudioDec_OK;
	printf("[info] audio_decode 正在播放 %s\n", audiodev->fname);

	switch(audiodev->ftype)
	{
	#if (USE_AUDIO_DECODE_WAV == 1)
	case MUSIC_TYPE_WAV:
		res = wav_play_song(audiodev);
		break;
	#endif
	#if (USE_AUDIO_DECODE_FLAC == 1)
	case MUSIC_TYPE_FLAC:
		res = flac_play_song(audiodev);
		break;
	#endif
	#if (USE_AUDIO_DECODE_MP3 == 1)
	case MUSIC_TYPE_MP3:
		res = mp3_play_song(audiodev);
		break;
	#endif

	default:
		break;
	}

	return res;
}




void audio_quit(void){}
void audio_play(void){}
void audio_stop(void){}
