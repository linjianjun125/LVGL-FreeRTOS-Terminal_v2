#include "audio_dma.h"
#include "audio_sai.h"
#include "../../system/debug/debug.h"

// 定义私有静态变量，记录回调函数地址
static audio_tx_callback_t audio_tx_dma_callback = NULL;




// dataLength == 0  8位
// dataLength == 1  16位
// dataLength == 2  32位
void audio_tx_dma_init(uint32_t Sour1, uint32_t Sour2, uint16_t size, uint8_t dataLength)
{
    uint8_t pos;

    /*!< RCC 使能 >!*/
    AUDIO_TX_DMA_ENABLE();

    /*!< DMAMUX 初始化 >!*/
    AUDIO_TX_DMAMUX->CCR[AUDIO_TX_DMAMUX_Stream] = AUDIO_TX_DMAMUX_Channel & 0XFF; // 使能DMAMUX请求通道--AUDIO_TX_DMAMUX_Channel

    /*!< DMA 初始化 >!*/
    AUDIO_TX_DMA->Stream[AUDIO_TX_DMA_Stream].CR = 0; // 先全部复位CR寄存器值
    while (AUDIO_TX_DMA->Stream[AUDIO_TX_DMA_Stream].CR & 0X01); // 等待DMA_Stream可配置

    pos = (AUDIO_TX_DMA_Stream % 4) * 6 + ((AUDIO_TX_DMA_Stream % 4) / 2) * 4;
    AUDIO_TX_DMA->IFCR[AUDIO_TX_DMA_Stream / 4] |= (uint32_t)0X3D << pos;     	// 清空(111101)中断标志--AUDIO_TX_DMA_Stream

    AUDIO_TX_DMA->Stream[AUDIO_TX_DMA_Stream].M0AR = (uint32_t)Sour1;         	// 内存1地址
    AUDIO_TX_DMA->Stream[AUDIO_TX_DMA_Stream].M1AR = (uint32_t)Sour2;         	// 内存2地址
    AUDIO_TX_DMA->Stream[AUDIO_TX_DMA_Stream].PAR = (uint32_t)&AUDIO_SAI_A->DR; // 外设地址
    AUDIO_TX_DMA->Stream[AUDIO_TX_DMA_Stream].NDTR = size;                    	// 设置长度
    AUDIO_TX_DMA->Stream[AUDIO_TX_DMA_Stream].CR |= 1 << 6;                   	// 存储器到外设模式
    AUDIO_TX_DMA->Stream[AUDIO_TX_DMA_Stream].CR |= 1 << 8;                   	// 循环模式
    AUDIO_TX_DMA->Stream[AUDIO_TX_DMA_Stream].CR |= 0 << 9;                   	// 外设非增量模式
    AUDIO_TX_DMA->Stream[AUDIO_TX_DMA_Stream].CR |= 1 << 10;                  	// 存储器增量模式
    AUDIO_TX_DMA->Stream[AUDIO_TX_DMA_Stream].CR |= dataLength << 11;         	// 外设数据长度
    AUDIO_TX_DMA->Stream[AUDIO_TX_DMA_Stream].CR |= dataLength << 13;         	// 存储器数据长度
    AUDIO_TX_DMA->Stream[AUDIO_TX_DMA_Stream].CR |= 2 << 16;                  	// 高优先级
    AUDIO_TX_DMA->Stream[AUDIO_TX_DMA_Stream].CR |= 1 << 18;                  	// 双缓冲模式
    AUDIO_TX_DMA->Stream[AUDIO_TX_DMA_Stream].CR |= 0 << 21;                  	// 外设突发单次传输
    AUDIO_TX_DMA->Stream[AUDIO_TX_DMA_Stream].CR |= 0 << 23;                  	// 存储器突发单次传输
    AUDIO_TX_DMA->Stream[AUDIO_TX_DMA_Stream].CR |= 0 << 25;                  	// 选择通道0 SAI1_A通道
    AUDIO_TX_DMA->Stream[AUDIO_TX_DMA_Stream].FCR &= ~(1 << 2);               	// 不使用FIFO模式 
    AUDIO_TX_DMA->Stream[AUDIO_TX_DMA_Stream].FCR &= ~(3 << 0);               	// 无FIFO设置
    AUDIO_TX_DMA->Stream[AUDIO_TX_DMA_Stream].CR |= 1 << 4;                   	// 开启传输完成中断

    config_nvic_init(6, 0, AUDIO_TX_IRQn);  
}

void audio_tx_dma_start(void)
{
    AUDIO_TX_DMA->Stream[AUDIO_TX_DMA_Stream].CR |= 1 << 0;// 开启DMA传输

}

void audio_tx_dma_close(void)
{
    AUDIO_TX_DMA->Stream[AUDIO_TX_DMA_Stream].CR &= ~(1 << 0);// 关闭DMA传输
}


/**
 * @brief 注册回调函数
 */
void audio_set_txdma_callback(audio_tx_callback_t callback)
{
    audio_tx_dma_callback = callback;
}

void AUDIO_TX_IRQHandler(void)
{
    uint8_t pos;
	int index ; 
	
    pos = (AUDIO_TX_DMA_Stream % 4) * 6 + ((AUDIO_TX_DMA_Stream % 4) / 2) * 4 + 5; // 计算CTCIF位

    if (AUDIO_TX_DMA->ISR[AUDIO_TX_DMA_Stream / 4] & (1 << pos))  // DMA2_Steam7传输完成标志    
    {
        AUDIO_TX_DMA->IFCR[AUDIO_TX_DMA_Stream / 4] |= 1 << pos;    // 清除DMA2_Steam7传输完成标志
		//printf("[info] dma_callback");
        if (audio_tx_dma_callback == NULL)
            printf("DMA回调函数为空, 请设置回调函数\n");
        else
		{
			index = (AUDIO_TX_DMA->Stream[AUDIO_TX_DMA_Stream].CR & (1 << 19))? 1 : 2;
			 audio_tx_dma_callback(index);
		}
           

        SCB_CleanInvalidateDCache();                        // 清除无效的D-Cache
    }
}









