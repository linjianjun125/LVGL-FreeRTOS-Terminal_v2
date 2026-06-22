#ifndef __AUDIO_DMA_H
#define __AUDIO_DMA_H

#include "sys.h"
#include "sys.h"


#define AUDIO_TX_DMA_ENABLE()          	do{SYS_RCC_DMA2_EN();}while(0)
#define AUDIO_TX_DMA                   	SYS_DMA2                    // DMA2
#define AUDIO_TX_DMA_Stream            	(3)                         // DMA2通道3
#define AUDIO_TX_IRQn                  	DMA2_Stream3_IRQn           // 中断号
#define AUDIO_TX_IRQHandler            	DMA2_Stream3_IRQHandler     // 中断服务函数

#define AUDIO_TX_DMAMUX                SYS_DMAMUX1                 // DMAMUX
#define AUDIO_TX_DMAMUX_Stream         (3 + 8)                     // DMA对应的DMAMUX通道
#define AUDIO_TX_DMAMUX_Channel        (87)                        // DMAMUX外部链接线

void audio_tx_dma_init(uint32_t Sour0, uint32_t Sour1, uint16_t size, uint8_t dataLength);
void audio_tx_dma_start(void);
void audio_tx_dma_close(void);

/**
 * @brief 音频 DMA 传输完成回调函数类型
 * @param index: 当前完成的缓冲区索引 (例如 1 代表 saibuf1, 2 代表 saibuf2)
 */
typedef void (*audio_tx_callback_t)(int index);

// 声明设置接口
void audio_set_txdma_callback(audio_tx_callback_t callback);



#endif

