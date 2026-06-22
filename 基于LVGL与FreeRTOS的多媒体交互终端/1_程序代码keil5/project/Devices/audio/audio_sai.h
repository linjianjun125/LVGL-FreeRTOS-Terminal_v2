#ifndef __AUDIO_SAI_H
#define __AUDIO_SAI_H

#include "sys.h"
#include "sys.h"


#define AUDIO_SAI_ENABLE()      do{SYS_RCC_SAI1_EN(); ConfigRCC_GPIOE_EN();}while(0)
#define AUDIO_SAI_FS_GPIO       GPIOE
#define AUDIO_SAI_SCK_GPIO      GPIOE
#define AUDIO_SAI_MCLK_GPIO     GPIOE
#define AUDIO_SAI_SDA_GPIO      GPIOE
#define AUDIO_SAI_SDB_GPIO      GPIOE
#define AUDIO_SAI_FS_PIN        ConfigIO_Pin4
#define AUDIO_SAI_SCK_PIN       ConfigIO_Pin5
#define AUDIO_SAI_MCLK_PIN      ConfigIO_Pin2
#define AUDIO_SAI_SDA_PIN       ConfigIO_Pin6
#define AUDIO_SAI_SDB_PIN       ConfigIO_Pin3
#define AUDIO_SAI_FS_AF        	6
#define AUDIO_SAI_SCK_AF        6
#define AUDIO_SAI_MCLK_AF      	6
#define AUDIO_SAI_SDA_AF        6
#define AUDIO_SAI_SDB_AF        6

#define AUDIO_SAI_A               SAI1_Block_A
#define AUDIO_SAI_B               SAI1_Block_B

void audio_saia_init(uint8_t mode, uint8_t cpol, uint8_t datalen);
void audio_saib_init(uint8_t mode, uint8_t cpol, uint8_t datalen);
uint8_t audio_sai_samplerate(uint32_t samplerate);
void audio_saia_close(void);




















#endif
