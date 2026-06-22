#ifndef __CONFIG_RCC_H
#define __CONFIG_RCC_H

#include "stm32h7xx.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// RCC-AHB3 //
#define SYS_RCC_SDMMC1_EN()                 do{ RCC->AHB3ENR |= 1 << 16;} while(0)
#define SYS_RCC_QSPI_EN()                   do{ RCC->AHB3ENR |= 1 << 14;} while(0)
#define SYS_RCC_FMC_EN()                    do{ RCC->AHB3ENR |= 1 << 12;} while(0)
#define SYS_RCC_JPGDEC_EN()                 do{ RCC->AHB3ENR |= 1 << 5;} while(0)
#define SYS_RCC_DMA2D_EN()                  do{ RCC->AHB3ENR |= 1 << 4;} while(0)
#define SYS_RCC_MDMA_EN()                   do{ RCC->AHB3ENR |= 1 << 0;} while(0)
// RCC-AHB1 //
#define SYS_RCC_USB_PHY2_EN()               do{ RCC->AHB1ENR |= 1 << 28;} while(0)
#define SYS_RCC_USB2OTG_EN()                do{ RCC->AHB1ENR |= 1 << 27;} while(0)
#define SYS_RCC_USB_PHY1_EN()               do{ RCC->AHB1ENR |= 1 << 26;} while(0)
#define SYS_RCC_USB1OTG_EN()                do{ RCC->AHB1ENR |= 1 << 25;} while(0)
#define SYS_RCC_ETH1RX_EN()                 do{ RCC->AHB1ENR |= 1 << 17;} while(0)
#define SYS_RCC_ETH1TX_EN()                 do{ RCC->AHB1ENR |= 1 << 16;} while(0)
#define SYS_RCC_ETHMAC_EN()                 do{ RCC->AHB1ENR |= 1 << 15;} while(0)
#define SYS_RCC_ADC12_EN()                  do{ RCC->AHB1ENR |= 1 << 5;} while(0)
#define SYS_RCC_DMA2_EN()                   do{ RCC->AHB1ENR |= 1 << 1;} while(0)
#define SYS_RCC_DMA1_EN()                   do{ RCC->AHB1ENR |= 1 << 0;} while(0)
// RCC-AHB2 //
#define SYS_RCC_SRAM3_EN()                  do{ RCC->AHB2ENR |= 1 << 31;} while(0)
#define SYS_RCC_SRAM2_EN()                  do{ RCC->AHB2ENR |= 1 << 30;} while(0)
#define SYS_RCC_SRMA1_EN()                  do{ RCC->AHB2ENR |= 1 << 29;} while(0)
#define SYS_RCC_SDMMC2_EN()                 do{ RCC->AHB2ENR |= 1 << 9;} while(0)
#define SYS_RCC_RNG_EN()                    do{ RCC->AHB2ENR |= 1 << 6;} while(0)
#define SYS_RCC_HASH_EN()                   do{ RCC->AHB2ENR |= 1 << 5;} while(0)
#define SYS_RCC_CRYPT_EN()                  do{ RCC->AHB2ENR |= 1 << 4;} while(0)
#define SYS_RCC_DCMI_EN()                   do{ RCC->AHB2ENR |= 1 << 0;} while(0)
// RCC-AHB4 //
#define SYS_RCC_RAM_EN()                    do{ RCC->AHB4ENR |= 1 << 28;} while (0)
#define SYS_RCC_HSEM_EN()                   do{ RCC->AHB4ENR |= 1 << 25;} while (0)
#define SYS_RCC_ADC3_EN()                   do{ RCC->AHB4ENR |= 1 << 24;} while (0)
#define SYS_RCC_BDMAandDNAMUX2_EN()         do{ RCC->AHB4ENR |= 1 << 21;} while (0)
#define SYS_RCC_CRC_EN()                    do{ RCC->AHB4ENR |= 1 << 19;} while (0)
#define ConfigRCC_GPIOK_EN()                  do{ RCC->AHB4ENR |= 1 << 10;} while (0)
#define ConfigRCC_GPIOJ_EN()                  do{ RCC->AHB4ENR |= 1 << 9;} while (0)
#define ConfigRCC_GPIOI_EN()                  do{ RCC->AHB4ENR |= 1 << 8;} while (0)
#define ConfigRCC_GPIOH_EN()                  do{ RCC->AHB4ENR |= 1 << 7;} while (0)
#define ConfigRCC_GPIOG_EN()                  do{ RCC->AHB4ENR |= 1 << 6;} while (0)
#define ConfigRCC_GPIOF_EN()                  do{ RCC->AHB4ENR |= 1 << 5;} while (0)
#define ConfigRCC_GPIOE_EN()                  do{ RCC->AHB4ENR |= 1 << 4;} while (0)
#define ConfigRCC_GPIOD_EN()                  do{ RCC->AHB4ENR |= 1 << 3;} while (0)
#define ConfigRCC_GPIOC_EN()                  do{ RCC->AHB4ENR |= 1 << 2;} while (0)
#define ConfigRCC_GPIOB_EN()                  do{ RCC->AHB4ENR |= 1 << 1;} while (0)
#define ConfigRCC_GPIOA_EN()                  do{ RCC->AHB4ENR |= 1 << 0;} while (0)


// RCC-APB3 //
#define SYS_RCC_WWDG1_EN()                  do{ RCC->APB3ENR |= 1 << 6;} while(0)
#define SYS_RCC_LTDC_EN()                   do{ RCC->APB3ENR |= 1 << 3;} while(0)
// RCC-PAB1L //
#define SYS_RCC_UART8_EN()                  do{ RCC->APB1LENR |= 1 << 31;} while(0)
#define SYS_RCC_UART7_EN()                  do{ RCC->APB1LENR |= 1 << 30;} while(0)
#define SYS_RCC_DAC12_EN()                  do{ RCC->APB1LENR |= 1 << 29;} while(0)
#define SYS_RCC_HDMI_CEC_EN()               do{ RCC->APB1LENR |= 1 << 27;} while(0)
#define SYS_RCC_I2C3_EN()                   do{ RCC->APB1LENR |= 1 << 23;} while(0)
#define SYS_RCC_I2C2_EN()                   do{ RCC->APB1LENR |= 1 << 22;} while(0)
#define SYS_RCC_I2C1_EN()                   do{ RCC->APB1LENR |= 1 << 21;} while(0)
#define SYS_RCC_UART5_EN()                  do{ RCC->APB1LENR |= 1 << 20;} while(0)
#define SYS_RCC_UART4_EN()                  do{ RCC->APB1LENR |= 1 << 19;} while(0)
#define SYS_RCC_USART3_EN()                 do{ RCC->APB1LENR |= 1 << 18;} while(0)
#define SYS_RCC_USART2_EN()                 do{ RCC->APB1LENR |= 1 << 17;} while(0)
#define SYS_RCC_SPDIFRX_EN()                do{ RCC->APB1LENR |= 1 << 16;} while(0)
#define SYS_RCC_SPI3_EN()                   do{ RCC->APB1LENR |= 1 << 15;} while(0)
#define SYS_RCC_SPI2_EN()                   do{ RCC->APB1LENR |= 1 << 14;} while(0)
#define SYS_RCC_LPTIM1_EN()                 do{ RCC->APB1LENR |= 1 << 9;} while(0)
#define SYS_RCC_TIM14_EN()                  do{ RCC->APB1LENR |= 1 << 8;} while(0)
#define SYS_RCC_TIM13_EN()                  do{ RCC->APB1LENR |= 1 << 7;} while(0)
#define SYS_RCC_TIM12_EN()                  do{ RCC->APB1LENR |= 1 << 6;} while(0)
#define SYS_RCC_TIM7_EN()                   do{ RCC->APB1LENR |= 1 << 5;} while(0)
#define SYS_RCC_TIM6_EN()                   do{ RCC->APB1LENR |= 1 << 4;} while(0)
#define SYS_RCC_TIM5_EN()                   do{ RCC->APB1LENR |= 1 << 3;} while(0)
#define SYS_RCC_TIM4_EN()                   do{ RCC->APB1LENR |= 1 << 2;} while(0)
#define SYS_RCC_TIM3_EN()                   do{ RCC->APB1LENR |= 1 << 1;} while(0)
#define SYS_RCC_TIM2_EN()                   do{ RCC->APB1LENR |= 1 << 0;} while(0)
// RCC-PAB1H //
#define SYS_RCC_FDCAN_EN()                  do{ RCC->APB1HENR |= 1 << 8;} while(0)
#define SYS_RCC_MDIOS_EN()                  do{ RCC->APB1HENR |= 1 << 5;} while(0)
#define SYS_RCC_OPAMP_EN()                  do{ RCC->APB1HENR |= 1 << 4;} while(0)
#define SYS_RCC_SWPMI_EN()                  do{ RCC->APB1HENR |= 1 << 2;} while(0)
#define SYS_RCC_CRS_EN()                    do{ RCC->APB1HENR |= 1 << 1;} while(0)
// RCC-PAB2 //
#define SYS_RCC_HRTIM_EN()                  do{ RCC->APB2ENR |= 1 << 29;} while(0)
#define SYS_RCC_DFSDM1_EN()                 do{ RCC->APB2ENR |= 1 << 28;} while(0)
#define SYS_RCC_SAI3_EN()                   do{ RCC->APB2ENR |= 1 << 24;} while(0)
#define SYS_RCC_SAI2_EN()                   do{ RCC->APB2ENR |= 1 << 23;} while(0)
#define SYS_RCC_SAI1_EN()                   do{ RCC->APB2ENR |= 1 << 22;} while(0)
#define SYS_RCC_SPI5_EN()                   do{ RCC->APB2ENR |= 1 << 20;} while(0)
#define SYS_RCC_TIM17_EN()                  do{ RCC->APB2ENR |= 1 << 18;} while(0)
#define SYS_RCC_TIM16_EN()                  do{ RCC->APB2ENR |= 1 << 17;} while(0)
#define SYS_RCC_TIM15_EN()                  do{ RCC->APB2ENR |= 1 << 16;} while(0)
#define SYS_RCC_SPI4_EN()                   do{ RCC->APB2ENR |= 1 << 13;} while(0)
#define SYS_RCC_SPI1_EN()                   do{ RCC->APB2ENR |= 1 << 12;} while(0)
#define SYS_RCC_USART6_EN()                 do{ RCC->APB2ENR |= 1 << 5;} while(0)
#define SYS_RCC_USART1_EN()                 do{ RCC->APB2ENR |= 1 << 4;} while(0)
#define SYS_RCC_TIM8_EN()                   do{ RCC->APB2ENR |= 1 << 1;} while(0)
#define SYS_RCC_TIM1_EN()                   do{ RCC->APB2ENR |= 1 << 0;} while(0)
// RCC-PAB4 //
#define SYS_RCC_SAI4_EN()                   do{ RCC->APB4ENR |= 1 << 21;} while(0)
#define SYS_RCC_RTC_APB_EN()                do{ RCC->APB4ENR |= 1 << 16;} while(0)
#define SYS_RCC_VREF_EN()                   do{ RCC->APB4ENR |= 1 << 15;} while(0)
#define SYS_RCC_COMP1_2_EN()                do{ RCC->APB4ENR |= 1 << 14;} while(0)
#define SYS_RCC_LPTIM5_EN()                 do{ RCC->APB4ENR |= 1 << 12;} while(0)
#define SYS_RCC_LPTIM4_EN()                 do{ RCC->APB4ENR |= 1 << 11;} while(0)
#define SYS_RCC_LPTIM3_EN()                 do{ RCC->APB4ENR |= 1 << 10;} while(0)
#define SYS_RCC_LPTIM2_EN()                 do{ RCC->APB4ENR |= 1 << 9;} while(0)
#define SYS_RCC_I2C4_EN()                   do{ RCC->APB4ENR |= 1 << 7;} while(0)
#define SYS_RCC_SPI6_EN()                   do{ RCC->APB4ENR |= 1 << 5;} while(0)
#define SYS_RCC_LPUART1_EN()                do{ RCC->APB4ENR |= 1 << 3;} while(0)
#define SYS_RCC_SYSCFG_EN()                 do{ RCC->APB4ENR |= 1 << 1;} while(0)

/*!< RCC时钟函数 >!*/
uint8_t config_clock_set(uint32_t plln, uint32_t pllm, uint32_t pllp, uint32_t pllq);      // 时钟设置函数
void config_stm32_clock_init(uint32_t plln, uint32_t pllm, uint32_t pllp, uint32_t pllq);  // 系统时钟初始化函数



#endif

